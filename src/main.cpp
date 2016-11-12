#include <Homie.h>
#include <string>
#include <sstream>

#define OUTPUT1 D8 // solenoid 3
#define OUTPUT2 D7 // solenoid 2
#define OUTPUT3 D6 // solenoid 1
#define PUMP D5 // pump

HomieNode zoneNode("zone", "switch");
bool isPumpOn = false;

void resetOutputs(){
  digitalWrite(PUMP, LOW);
  digitalWrite(OUTPUT1, LOW);
  digitalWrite(OUTPUT2, LOW);
  digitalWrite(OUTPUT3, LOW);
}

void setupHandler(){
  // Setup all outputs
  pinMode(PUMP, OUTPUT);
  pinMode(OUTPUT1, OUTPUT);
  pinMode(OUTPUT2, OUTPUT);
  pinMode(OUTPUT3, OUTPUT);

  resetOutputs();
}

bool startPump(){
  if(!isPumpOn){
    Serial << "Starting pump " << endl;
    isPumpOn = true;
    digitalWrite(PUMP, HIGH);
    return true;
  }
}

bool stopPump(){
  if(isPumpOn){
    Serial << "Stopping pump " << endl;
    isPumpOn = false;
    digitalWrite(PUMP, LOW);
    return false;
  }
}

void waterZone(uint8_t zone, uint32_t milliseconds){
  Serial << "Enabling output " << zone << " for " << milliseconds << endl;
  startPump();
  digitalWrite(zone, HIGH);
  Serial << "Sleeping for " << milliseconds << "ms" << endl;
  delay(milliseconds);
  Serial << "Disabling output " << zone << " for " << milliseconds << endl;
  digitalWrite(zone, LOW);
  stopPump();
}

bool zoneOnHandler(const HomieRange& range, const String& value) {

  Serial << range.index << " set to " << value << endl;
  uint8_t output;

  long delayMs = value.toInt();

  switch(range.index){
    case 1:
      Serial << range.index << " pumping for " << delayMs << endl;
      output = OUTPUT1;
      break;
    case 2:
      Serial << range.index << " pumping for " << delayMs << endl;
      output = OUTPUT2;
      break;
    case 3:
      Serial << range.index << " pumping for " << delayMs << endl;
      output = OUTPUT3;
      break;
  }

  // TODO This section needs to be cleaned up.
  // listen on devices/c40f46e0/zone/status to get those updates
  // Report status
  String statusUpdate = "{";
  statusUpdate += "event:'zoneOnStart',";
  statusUpdate += "zone:";
    statusUpdate += range.index;
    statusUpdate += ",";
  statusUpdate += "millis:";
    statusUpdate += millis();
    statusUpdate += ",";
  statusUpdate += "duration:";
    statusUpdate += delayMs;
  statusUpdate += "}";
  zoneNode.setProperty("status").send(statusUpdate);

  waterZone(output, (uint32_t)delayMs);

  statusUpdate = "{";
  statusUpdate += "event:'zoneOnStop',";
  statusUpdate += "zone:";
    statusUpdate += range.index;
    statusUpdate += ",";
  statusUpdate += "millis:";
    statusUpdate += millis();
    statusUpdate += ",";
  statusUpdate += "duration:";
    statusUpdate += delayMs;
  statusUpdate += "}";
  zoneNode.setProperty("status").send(statusUpdate);

  return true;
}

void loopHandler(){
  //TODO This eventually to be used for controlling concurrent watering
  // Not used
}

void setup() {
  Serial.begin(115200);
  Serial << endl << endl;

  Homie_setFirmware("plantnanny-pumpcontroller", "0.0.1");
  Homie_setBrand("plantnanny")
  Homie.setSetupFunction(setupHandler).setLoopFunction(loopHandler);

  // devices/c40f46e0/zone/on_3/set - for sending to 3rd relay
  // devices/c40f46e0/zone/on_3
  zoneNode.advertiseRange("on", 1, 3).settable(zoneOnHandler);

  Homie.setup();
}

void loop() {
  Homie.loop();
}
