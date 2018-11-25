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
//array of pointers
ZoneControl * ptrZonesToControl;
//empty zone
ZoneControl emptyZoneControl;
//object for override
ZoneOverride zoneOverride;

//Set this home node to a zone node
HomieNode zoneNode("zone", "switch");

//Zone functions
//Reset all zones to LOW(OFF)
void resetZone() {
	String statusUpdate = "{";
		statusUpdate += "event:'resetZone'";
	statusUpdate += "}";
	zoneNode.setProperty("status").send(statusUpdate);

	digitalWrite(OVERRIDE, LOW);
	digitalWrite(ZONE1, LOW);
	digitalWrite(ZONE2, LOW);
	digitalWrite(ZONE3, LOW);
	digitalWrite(ZONE4, LOW);
}
//setup all zones as outputs and reset.
void setupHandler() {
	String statusUpdate = "{";
		statusUpdate += "event:'setupHandler',";
    statusUpdate += "}";
	
    zoneNode.setProperty("status").send(statusUpdate);
	
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
	zoneOverride.isLow = false;
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
//Send the zone status
void sendZoneStatus(ZoneControl *zoneToControl, String eventName){
	String statusUpdate = "{";
		statusUpdate += "event:'" + eventName + "',";
		statusUpdate += "zone:" + zoneToControl->zone + ',';
		statusUpdate += "duration: " + zoneToControl->duration + ',';
		statusUpdate += "startTime:" + zoneToControl->startTime + ',';
		statusUpdate += "expectedEndTime:" + zoneToControl->endTime;
	statusUpdate += "}";
	zoneNode.setProperty("status").send(statusUpdate);
}

//Zone control Functions
//Turn a particular zone on
void zoneOn(ZoneControl *zoneToControl) {
	digitalWrite(zoneToControl->zone, HIGH);

	sendZoneStatus(zoneToControl, "zoneOn");
}
//Turn a particular zone off
void zoneOff(ZoneControl *zoneToControl) {
	digitalWrite(zoneToControl->zone, LOW);

	sendZoneStatus(zoneToControl, "zoneOff");

	*zoneToControl = emptyZoneControl;
}
//Turn all zones off
void allZonesOff(){
	for (int i = 0; i < MAXZONESTOCONTROLL; i++) {
		if (ptrZonesToControl[i].zone != 0) {
			zoneOff(&ptrZonesToControl[i]);
		}
	}
}
//Override triggered
void overrideTriggered(){
	String statusUpdate = "{";
		statusUpdate += "event:'overrideTriggered'";
	statusUpdate += "}";
	zoneNode.setProperty("status").send(statusUpdate);

	allZonesOff();
}

//Handlers
//Adds zone onto list to run for x amount of time or all the time
bool zoneOnHandler(const HomieRange& range, const String& value) {
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
bool zoneOffHandler(const HomieRange& range, const String& value) {
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

	//Check to see if override has been triggered
	int readOverride = digitalRead(zoneOverride.overridePin);

	if(readOverride == LOW && !zoneOverride.isLow){
		zoneOverride.isLow = true;
		zoneOverride.lastLowReadTime = currentMillis;
	}else if(readOverride == HIGH && zoneOverride.isLow){
		zoneOverride.isLow = false;
	}

	if(zoneOverride.lastLowReadTime + zoneOverride.readInterval < currentMillis && zoneOverride.isLow){
		zoneOverride.isLow = false;
		overrideTriggered();
	}

	//loop to control the zones
	for (int i = 0; i < MAXZONESTOCONTROLL; i++) {
		if (ptrZonesToControl[i].zone != 0) {
			if (ptrZonesToControl[i].endTime == 0) {
				if (ptrZonesToControl[i].startTime == 0) {
					ptrZonesToControl[i].startTime = currentMillis;
					zoneOn(&ptrZonesToControl[i]);
				}
				ptrZonesToControl[i].endTime = ptrZonesToControl[i].startTime + ptrZonesToControl[i].duration;
			}
			else if(ptrZonesToControl[i].startTime != ptrZonesToControl[i].endTime && currentMillis > ptrZonesToControl[i].endTime){
				zoneOff(&ptrZonesToControl[i]);
			}
		}
	}
}

void setup() {
	Serial.begin(115200);
	Serial << endl << endl;

	Homie_setFirmware("plantnanny-controller", "1.0.0");
	Homie_setBrand("plantnanny")

	Homie.setSetupFunction(setupHandler).setLoopFunction(loopHandler);

	// devices/c40f46e0/zone/on_3/set - for sending to 3rd relay
	zoneNode.advertiseRange("on", 1, 4).settable(zoneOnHandler);
	zoneNode.advertiseRange("off", 1, 4).settable(zoneOffHandler);

	Homie.setup();
}

void loop() {
	Homie.loop();
}
