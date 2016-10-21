#include <Homie.h>

const int READING_INTERVAL = 300; // 30 = 30 seconds
unsigned long lastReadingSent = 0;
const char* PROPERTY = "moisture";
const char* UNIT = "percent";

const int SENSOR_PIN = 0;
const int PWR_PIN = 2;

HomieNode moistureNode(PROPERTY, UNIT);

void setupHandler(){
  Homie.setNodeProperty(moistureNode, "unit").setRetained(true).send(UNIT);
}

void loopHandler(){
  if(millis() - lastReadingSent >= READING_INTERVAL * 1000UL || lastReadingSent == 0){

    lastReadingSent = millis();

    digitalWrite(PWR_PIN, HIGH);
    float moisture = ( (float)analogRead(SENSOR_PIN) / 900.00) * 100; // keeping it at 900 instead of 1024
    digitalWrite(PWR_PIN, LOW);

    Serial << "Reading: " << analogRead(SENSOR_PIN) << endl;
    Serial << "Moisture: " << moisture << UNIT << endl;
    Homie.setNodeProperty(moistureNode, "value").send(String(moisture));
  }
}

void setup() {
  Serial.begin(115200);
  Serial << endl << endl;

  // Drive analog through digital pin (to stop current going through the sensor when not required)
  pinMode(PWR_PIN, OUTPUT);

  Homie_setFirmware("plantnanny-moisture", "0.0.1");
  Homie_setBrand("plantnanny")
  Homie.setSetupFunction(setupHandler).setLoopFunction(loopHandler);

  moistureNode.advertise("unit");
  moistureNode.advertise(PROPERTY);

  Homie.setup();
}

void loop() {
  Homie.loop();
}
