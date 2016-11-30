#include <Homie.h>
#include <string>
#include <sstream>
#include <map>


struct RelayControll{
    uint32_t duration;
    uint32_t endTime;
};

#define RELAY1 D5 // solenoid 1
#define RELAY2 D6 // solenoid 2
#define RELAY3 D7 // solenoid 3
#define RELAY4 D8 // solenoid 4

//std::vector<RelayControll> relaysToControll;
std::map<uint8_t, RelayControll> relaysToControll;

HomieNode zoneNode("zone", "switch");

void resetRelays(){
    digitalWrite(RELAY1, LOW);
    digitalWrite(RELAY2, LOW);
    digitalWrite(RELAY3, LOW);
    digitalWrite(RELAY4, LOW);
}

void setupHandler(){
    pinMode(RELAY1, OUTPUT);
    pinMode(RELAY2, OUTPUT);
    pinMode(RELAY3, OUTPUT);
    pinMode(RELAY4, OUTPUT);
    resetRelays();
}

void RelayOn(uint8_t zone){
    digitalWrite(zone, HIGH);
}

void RelayOff(uint8_t zone){
    digitalWrite(zone, LOW);
}

void RelaysOn(){}

uint8_t GetZone(uint8_t index){

  uint8_t pin = 0;

  switch(index){
      case 1:
        pin = RELAY1;
        break;
      case 2:
        pin = RELAY2;
        break;
      case 3:
        pin = RELAY3;
        break;
      case 4:
        pin = RELAY4;
        break;
    }
    return pin;
}

bool ZoneTimedOnHandler(const HomieRange& range, const String& value) {
    RelayControll relay;

    relay.duration = value.toInt();
    relay.endTime = 0;

    relaysToControll.insert(std::pair<uint8_t, RelayControll>(GetZone(range.index),relay));

    return true;
}

bool ZoneOnHandler(const HomieRange& range, const String& value) {
    RelayControll relay;
    //if range is not between expected then don't do a thing
    relay.duration = value.toInt();
    relay.endTime = 0;

    //relaysToControll.push_back(
    //    relay
    //);
    return true;
}

bool ZoneOffHandler(const HomieRange& range, const String& value) {
    RelayControll relay;
    //if range is not between expected then don't do a thing
    relay.duration = value.toInt();
    relay.endTime = 0;

    //relaysToControll.push_back(
    //    relay
    //);
    return true;
}

void loopHandler(){
    //for (int i = 0; i < relaysToControll.size(); i++){
    //    if(relaysToControll[i].endTime == 0){
    //        relaysToControll[i].endTime = millis() + relaysToControll[i].duration;
    //        RelayOn(relaysToControll[i].zone);
    //    }else{
    //        if(millis() > relaysToControll[i].endTime){
    //            RelayOff(relaysToControll[i].zone);
    //            relaysToControll.erase(relaysToControll.begin()+i);
    //            i = 0;//seems cheaper than a calculation to check if there are anymore a head, whether it is on zero or not etc - prevent out of issues.
    //        }
    //    }
    //}
}

void setup() {
  Serial.begin(115200);
  Serial << endl << endl;

  Homie_setFirmware("plantnanny-pumpcontroller", "0.0.1");
  Homie_setBrand("plantnanny")
  Homie.setSetupFunction(setupHandler).setLoopFunction(loopHandler);

  // devices/c40f46e0/zone/on_3/set - for sending to 3rd relay
  // devices/c40f46e0/zone/on_3
  zoneNode.advertiseRange("timed", 1, 4).settable(ZoneOnHandler);
  zoneNode.advertiseRange("on", 1, 4).settable(ZoneOnHandler);
  zoneNode.advertiseRange("off", 1, 4).settable(ZoneOffHandler);

  Homie.setup();
}

void loop() {
  Homie.loop();
}
