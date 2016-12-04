#include <Homie.h>
#include <EEPROM.h>
#include <string>
#include <sstream>
#include <map>
#include <arduinojson.h>

struct ZoneControll{
    uint32_t duration;
    uint32_t startTime;
    uint32_t endTime;
};

#define ZONE1 D5
#define ZONE2 D6
#define ZONE3 D7
#define ZONE4 D8

std::map<uint8_t, ZoneControll> zonesToControll;
uint8_t EepromCounter = 0;
HomieNode zoneNode("zone", "switch");

void resetZone(){
    digitalWrite(ZONE1, LOW);
    digitalWrite(ZONE2, LOW);
    digitalWrite(ZONE3, LOW);
    digitalWrite(ZONE4, LOW);
}

void setupHandler(){
    pinMode(ZONE1, OUTPUT);
    pinMode(ZONE2, OUTPUT);
    pinMode(ZONE3, OUTPUT);
    pinMode(ZONE4, OUTPUT);
    resetZone();
}

void ZoneOn(std::map<uint8_t, ZoneControll>::iterator &zoneToControll){
    digitalWrite(zoneToControll->first, HIGH);

    Serial << "ZoneOn: " << zoneToControll->first << endl;

    StaticJsonBuffer<200> jsonBuffer;

    String statusUpdate = "{";
        statusUpdate += "event:'ZoneOn',";
        statusUpdate += "zone:" + zoneToControll->first + ',';
        statusUpdate += "duration: " + zoneToControll->second.duration + ',';
        statusUpdate += "startTime:" + zoneToControll->second.startTime + ',';
        statusUpdate += "expectedEndTime:" + zoneToControll->second.endTime + ',';
    statusUpdate += "}";

    zoneNode.setProperty("status").send(statusUpdate);
}

void ZoneOff(std::map<uint8_t, ZoneControll>::iterator &zoneToControll){
    digitalWrite(zoneToControll->first, LOW);

    Serial << "ZoneOff: " << zoneToControll->first << endl;

    String statusUpdate = "{";
        statusUpdate += "event:'ZoneOff',";
        statusUpdate += "zone:" + zoneToControll->first + ',';
        statusUpdate += "duration: " + zoneToControll->second.duration + ',';
        statusUpdate += "startTime:" + zoneToControll->second.startTime + ',';
        statusUpdate += "endTime:" + zoneToControll->second.endTime + ',';
    statusUpdate += "}";

    zoneNode.setProperty("status").send(statusUpdate);
}

uint8_t GetZone(uint8_t zone){
    switch(zone){
        case 1:
            return ZONE1;
        case 2:
            return ZONE2;
        case 3:
            return ZONE3;
        case 4:
            return ZONE4;
    }
}

template <class T> int EEPROM_writeAnything(int ee, const T& value){
    const byte* p = (const byte*)(const void*)&value;
    unsigned int i;
    for (i = 0; i < sizeof(value); i++)
          EEPROM.write(ee++, *p++);
    return i;
}

template <class T> int EEPROM_readAnything(int ee, T& value){
    byte* p = (byte*)(void*)&value;
    unsigned int i;
    for (i = 0; i < sizeof(value); i++)
          *p++ = EEPROM.read(ee++);
    return i;
}

void WriteToEeprom(){
    for(std::map<uint8_t, ZoneControll>::iterator zoneToControll = zonesToControll.begin(); zoneToControll != zonesToControll.end(); ++zoneToControll){
        uint8_t zone = zoneToControll->first;
        uint32_t remainingTime = zoneToControll->second.endTime - millis();
        EEPROM_writeAnything(zone, zone);
        EEPROM_writeAnything(zone+1, remainingTime);
    }
}

void ReadEeprom(){
    //uint8_t zone = EEPROM_readAnything();
    //int32_t remainingTime = zoneToControll->second.endTime - millis();
}

///Handlers
//Adds zone onto list to run for x amount of time or all the time
bool ZoneOnHandler(const HomieRange& range, const String& value) {
    ZoneControll relay;
    relay.duration = value.toInt();
    relay.startTime = 0;
    relay.endTime = 0;

    auto zone = GetZone(range.index);

    zonesToControll.insert(std::pair<uint8_t, ZoneControll>(zone, relay));

    Serial << "ZoneOnHandler: " << zone << " " << endl;

    WriteToEeprom();

    return true;
}

//Deals with off commands, sets the end time to the current amount of millis
bool ZoneOffHandler(const HomieRange& range, const String& value) {

    auto zone = GetZone(range.index);

    std::map<uint8_t, ZoneControll>::iterator zoneToControll = zonesToControll.find(zone);

    if (zoneToControll != zonesToControll.end()){
        zoneToControll->second.endTime = millis();
        Serial << "ZoneOffHandler: " << zone << endl;
    }

    WriteToEeprom();

    return true;
}

//handler for the loop item
void loopHandler(){
    for (std::map<uint8_t, ZoneControll>::iterator zoneToControll = zonesToControll.begin(); zoneToControll != zonesToControll.end(); ++zoneToControll){
        if(zoneToControll->second.endTime == 0) {
            if(zoneToControll->second.startTime == 0){
                zoneToControll->second.startTime = millis();
                ZoneOn(zoneToControll);
            }
            zoneToControll->second.endTime = zoneToControll->second.startTime + zoneToControll->second.duration;
        }else if(zoneToControll->second.startTime != zoneToControll->second.endTime && millis() > zoneToControll->second.endTime)
        {
            ZoneOff(zoneToControll);
            zonesToControll.erase(zoneToControll);
        }
    }

    EepromCounter++;

    if(EepromCounter == 300){
        EepromCounter = 0;
        WriteToEeprom();
    }
}

void setup() {
  	Serial.begin(115200);
  	Serial << endl << endl;

  	Homie_setFirmware("plantnanny-pumpcontroller", "0.0.9");
  	Homie_setBrand("plantnanny")
  	Homie.setSetupFunction(setupHandler).setLoopFunction(loopHandler);

  	// devices/c40f46e0/zone/on_3/set - for sending to 3rd relay
  	zoneNode.advertiseRange("on", 1, 4).settable(ZoneOnHandler);

    // devices/c40f46e0/zone/off_3/set - for sending to 3rd relay
  	zoneNode.advertiseRange("off", 1, 4).settable(ZoneOffHandler);

    ReadEeprom();

  	Homie.setup();

  	String statusUpdate = "{";
  		  statusUpdate += "event:'setup'";
  	statusUpdate += "}";
  	zoneNode.setProperty("status").send(statusUpdate);
}

void loop() {
    Homie.loop();
}
