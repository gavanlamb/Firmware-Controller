#include <Homie.h>
#include <ZoneControl.h>
#include <ZoneOverride.h>

#define OVERRIDE D4

#define ZONE1 D5
#define ZONE2 D6
#define ZONE3 D7
#define ZONE4 D8


//Globals and definitions
//GLobal array of zones to control
static const uint8_t MAXZONESTOCONTROLL = 4;
//objects to control
ZoneControl * ptrZonesToControl; //array of pointers
ZoneControl emptyZoneControl; //empty zone
//object for override
ZoneOverride zoneOverride;

//Set this home node to a zone node
HomieNode zoneNode("zone", "switch");

//Zone functions
//Reset all zones to LOW(OFF)
void resetZone() {
	digitalWrite(OVERRIDE, LOW);
	digitalWrite(ZONE1, LOW);
	digitalWrite(ZONE2, LOW);
	digitalWrite(ZONE3, LOW);
	digitalWrite(ZONE4, LOW);
}
//setup all zones as outputs and reset.
void setupHandler() {
  pinMode(OVERRIDE, INPUT_PULLUP);
	pinMode(ZONE1, OUTPUT);
	pinMode(ZONE2, OUTPUT);
	pinMode(ZONE3, OUTPUT);
	pinMode(ZONE4, OUTPUT);

  ptrZonesToControl = new ZoneControl[MAXZONESTOCONTROLL];

	emptyZoneControl.zone = 0;
	emptyZoneControl.duration = 0;
	emptyZoneControl.startTime = 0;
	emptyZoneControl.endTime = 0;

	for (int i = 0; i < MAXZONESTOCONTROLL; i++) {
		ptrZonesToControl[i] = emptyZoneControl;
	}

  zoneOverride.overridePin = OVERRIDE;
  zoneOverride.lastRead = 0;
  zoneOverride.readInterval = 1000;

	resetZone();
}
//Get the corresponding zone pin
uint8_t getZonePin(uint16_t zone){
  switch (zone) {
		case 1:
			return ZONE1;
		case 2:
			return ZONE2;
		case 3:
			return ZONE3;
		case 4:
			return ZONE4;
		default:
			return 0;
	}
}
//check to see if zone is being controlled
bool isZoneBeingControlled(uint8_t zone){
  bool isZoneBeingControlled = false;

  for (uint8_t i = 0; i < MAXZONESTOCONTROLL; i++) {
    if (ptrZonesToControl[i].zone == zone) {
      isZoneBeingControlled = true;
      break;
    }
  }

  return isZoneBeingControlled;
}
//write to SPIFFS
void saveToSpiffs(){

}

//Zone control Functions
//Turn a particular zone on
void ZoneOn(ZoneControl *zoneToControl) {
	digitalWrite(zoneToControl->zone, HIGH);
}
//Turn a particular zone off
void ZoneOff(ZoneControl *zoneToControl) {
	digitalWrite(zoneToControl->zone, LOW);

  *zoneToControl = emptyZoneControl;
}
//Turn all zones off
void AllZonesOff(){
  for (int i = 0; i < MAXZONESTOCONTROLL; i++) {
		if (ptrZonesToControl[i].zone != 0) {
      ZoneOff(&ptrZonesToControl[i]);
    }
	}
}

//Handlers
//Adds zone onto list to run for x amount of time or all the time
bool ZoneOnHandler(const HomieRange& range, const String& value) {
	uint8_t zone = getZonePin(range.index);

	ZoneControl zoneToControl;
		zoneToControl.zone = zone;
		zoneToControl.duration = value.toInt();
		zoneToControl.startTime = 0;
		zoneToControl.endTime = 0;

  bool zoneUnderControlled = isZoneBeingControlled(zone);

  if(!zoneUnderControlled){
  	for (uint8_t i = 0; i < MAXZONESTOCONTROLL; i++) {
  		if (ptrZonesToControl[i].zone == 0) {
  			ptrZonesToControl[i] = zoneToControl;
  			break;
  		}
  	}
  }
	return !zoneUnderControlled;
}
//Deals with off commands, sets the end time to the current amount of millis
bool ZoneOffHandler(const HomieRange& range, const String& value) {
    uint8_t zone = getZonePin(range.index);
    bool zoneUnderControlled = isZoneBeingControlled(zone);
    if (zoneUnderControlled){
        for(int i=0; i < MAXZONESTOCONTROLL; i++){
            if(ptrZonesToControl[i].zone == zone){
                ptrZonesToControl[i].endTime = millis();
                break;
            }
        }
    }
    return zoneUnderControlled;
}
//handler for the loop item
void loopHandler() {
	uint32_t currentMillis = millis();

  if(zoneOverride.lastRead + zoneOverride.readInterval < currentMillis){
		zoneOverride.lastRead = currentMillis;
		int readOverride = digitalRead(zoneOverride.overridePin);
		Serial << "Override value: " << readOverride << endl;
    if(readOverride == HIGH){
      AllZonesOff();
    }
  }
	for (int i = 0; i < MAXZONESTOCONTROLL; i++) {
		if (ptrZonesToControl[i].zone != 0) {
			if (ptrZonesToControl[i].endTime == 0) {
				if (ptrZonesToControl[i].startTime == 0) {
					ptrZonesToControl[i].startTime = currentMillis;
					ZoneOn(&ptrZonesToControl[i]);
				}
				ptrZonesToControl[i].endTime = ptrZonesToControl[i].startTime + ptrZonesToControl[i].duration;
			}
      else if(ptrZonesToControl[i].startTime != ptrZonesToControl[i].endTime && currentMillis > ptrZonesToControl[i].endTime){
          ZoneOff(&ptrZonesToControl[i]);
      }
		}
	}
}

void setup() {
	Serial.begin(115200);
	Serial << endl << endl;

	Homie_setFirmware("plantnanny-controller", "0.0.9");
	Homie_setBrand("plantnanny")

	Homie.setSetupFunction(setupHandler).setLoopFunction(loopHandler);

	// devices/c40f46e0/zone/on_3/set - for sending to 3rd relay
	zoneNode.advertiseRange("on", 1, 4).settable(ZoneOnHandler);
	zoneNode.advertiseRange("off", 1, 4).settable(ZoneOffHandler);

	Homie.setup();
}

void loop() {
	Homie.loop();
}
