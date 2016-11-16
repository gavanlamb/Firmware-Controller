#include <Homie.h>
#include <string>
#include <sstream>
#include <vector>

struct RelayControll{
    uint8_t zone;
    uint32_t duration;
    uint32_t endTime;
};

#define RELAY1 D5 // solenoid 1
#define RELAY2 D6 // solenoid 2
#define RELAY3 D7 // solenoid 3
#define RELAY4 D8 // solenoid 4

std::vector<RelayControll> relaysToControll;

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

bool startRelay(uint8_t zone){
    //Serial << "Enabling RELAY " << zone << endl;
    digitalWrite(zone, HIGH);
    return true;
}

bool stopRelay(uint8_t zone){
    //Serial << "Disabling RELAY " << zone << endl;
    digitalWrite(zone, LOW);
    return true;
}

bool addZoneHandler(const HomieRange& range, const String& value) {
    RelayControll relay;
    switch(range.index){
        case 1:
          relay.zone = RELAY1;
          relay.duration = value.toInt();
          relay.endTime = 0;
          //Serial << "Add RELAY to array: " << relay.zone << " for " << relay.durationMilliseconds << endl;
          break;
        case 2:
          relay.zone = RELAY2;
          relay.duration = value.toInt();
          relay.endTime = 0;
          //Serial << "Add RELAY to array: " << relay.zone << " for " << relay.durationMilliseconds << endl;
          break;
        case 3:
          relay.zone = RELAY3;
          relay.duration = value.toInt();
          relay.endTime = 0;
          //Serial << "Add RELAY to array: " << relay.zone << " for " << relay.durationMilliseconds << endl;
          break;
        case 4:
          relay.zone = RELAY4;
          relay.duration = value.toInt();
          relay.endTime = 0;
          //Serial << "Add RELAY to array: " << relay.zone << " for " << relay.durationMilliseconds << endl;
          break;
      }

    relaysToControll.push_back(
        relay
    );
    //Serial << "Array size " << relaysToControll.size() << endl;
    return true;
}

void loopHandler(){
    //Serial << "ARRAY SIZE: " << relaysToControll.size() << endl;
    for (int i = 0; i < relaysToControll.size(); i++){
        /*Serial << "Looping through array---Looping through array---Looping through array " << endl;
        Serial << "Zone: " << relaysToControll[i].zone << endl;
        Serial << "Endtime: " << relaysToControll[i].endTime << endl;
        Serial << "Duration: " << relaysToControll[i].durationMilliseconds << endl;*/
        if(relaysToControll[i].endTime == 0){
            relaysToControll[i].endTime = millis() + relaysToControll[i].duration;
            startRelay(relaysToControll[i].zone);
            // Serial << "Starting " << relaysToControll[i].zone << " this will end " << relaysToControll[i].endTime << endl;
        }else{
            if(millis() > relaysToControll[i].endTime){
                stopRelay(relaysToControll[i].zone);
                relaysToControll.erase(relaysToControll.begin()+i);
                i = 0;//seems cheaper than a calculation to check if there are anymore a head, whether it is on zero or not etc - prevent out of issues.
            }
        }
    }
}

void setup() {
  Serial.begin(115200);
  Serial << endl << endl;

  Homie_setFirmware("plantnanny-pumpcontroller", "0.0.1");
  Homie_setBrand("plantnanny")
  Homie.setSetupFunction(setupHandler).setLoopFunction(loopHandler);

  // devices/c40f46e0/zone/on_3/set - for sending to 3rd relay
  // devices/c40f46e0/zone/on_3
  zoneNode.advertiseRange("on", 1, 4).settable(addZoneHandler);

  Homie.setup();
}

void loop() {
  Homie.loop();
}
