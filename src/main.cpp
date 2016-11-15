#include <Homie.h>
#include <string>
#include <sstream>
#include <list>

struct RelayRun{
    uint8_t zone;
    uint32_t durationMilliseconds;
    uint32_t endTime;
};

#define OUTPUT1 D5 // solenoid 1
#define OUTPUT2 D6 // solenoid 2
#define OUTPUT3 D7 // solenoid 3
#define OUTPUT4 D8 // solenoid 4

std::vector<RelayRun> toWater;

HomieNode zoneNode("zone", "switch");

void resetOutputs(){
  digitalWrite(OUTPUT1, LOW);
  digitalWrite(OUTPUT2, LOW);
  digitalWrite(OUTPUT3, LOW);
  digitalWrite(OUTPUT4, LOW);
}

void setupHandler(){
  pinMode(OUTPUT1, OUTPUT);
  pinMode(OUTPUT2, OUTPUT);
  pinMode(OUTPUT3, OUTPUT);
  pinMode(OUTPUT4, OUTPUT);

  resetOutputs();
}

void startRelay(uint8_t zone){
    Serial << "Enabling output " << zone << endl;
    digitalWrite(zone, HIGH);
}

void stopRelay(uint8_t zone){
    Serial << "Disabling output " << zone << endl;
    digitalWrite(zone, LOW);
}

bool addZoneHandler(const HomieRange& range, const String& value) {
      RelayRun relay;
      Serial << "Add output to array " << OUTPUT1 << " for " << value.toInt() << endl;
    switch(range.index){
        case 1:
          relay.zone = OUTPUT1;
          relay.durationMilliseconds = value.toInt();
          break;
        case 2:
          relay.zone = OUTPUT2;
          relay.durationMilliseconds = value.toInt();
          break;
        case 3:
          relay.zone = OUTPUT3;
          relay.durationMilliseconds = value.toInt();
          break;
        case 4:
          relay.zone = OUTPUT4;
          relay.durationMilliseconds = value.toInt();
          break;
      }

    toWater.push_back(
        relay
    );
    return true;
}

void loopHandler(){
    Serial << "ARRAY SIZE: " << toWater.size() << endl;
    for (RelayRun r : toWater){
        Serial << "Looping through array " << r.zone << endl;
        if(r.endTime == 0){
            r.endTime = millis() + r.durationMilliseconds;
            startRelay(r.zone);
            Serial << "Starting " << r.zone << " this will end " << r.endTime << endl;
        }else{
            if(millis() > r.endTime){
                stopRelay(r.zone);
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
