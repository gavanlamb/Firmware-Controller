#include <Homie.h>
#include <DHT.h>

// UNITS
#define MOISTURE_UNIT "percent"
#define HUMIDITY_UNIT "percent"
#define TEMPERATURE_UNIT "celcius"

// Reading
#define READING_VALUE "value"
#define READING_UNIT "unit"

#define READING_INTERVAL 300 // 30 = 30 seconds, 300 = 5 min

// Pins

// -- Moisture
#define MOISTURE_SENSOR_PIN 0
#define MOISTURE_SENSOR_PWR_PIN D3

// -- Temperature
#define TEMPERATURE_SENSOR_PIN D7
#define TEMPERATURE_SENSOR_TYPE DHT22   // tempSensor 22  (AM2302), AM2321

// Vars
unsigned long lastReadingSent = 0; // used for non blocking reading

// Nodes & settings
HomieNode moistureNode("moisture", MOISTURE_UNIT);
HomieNode temperatureNode("temperature", TEMPERATURE_UNIT);
HomieNode humidityNode("humidity", HUMIDITY_UNIT);
HomieSetting<unsigned long> frequencySetting("readingFrequency", "Frequency of the reading in seconds");

DHT tempSensor(TEMPERATURE_SENSOR_PIN, TEMPERATURE_SENSOR_TYPE);

void setupHandler(){
  // Send units just for clarity
  moistureNode.setProperty(READING_UNIT).send(MOISTURE_UNIT);
  temperatureNode.setProperty(READING_UNIT).send(TEMPERATURE_UNIT);
  humidityNode.setProperty(READING_UNIT).send(HUMIDITY_UNIT);
}

void readMoisture(){
  digitalWrite(MOISTURE_SENSOR_PWR_PIN, HIGH);
  delay(5000); // power up the sensor for the duration of reading
  float moisture = ( (float)analogRead(MOISTURE_SENSOR_PIN) / 1024.00) * 100; // keeping it at 900 instead of 1024
  Serial << "Reading: " << analogRead(MOISTURE_SENSOR_PIN) << endl;
  digitalWrite(MOISTURE_SENSOR_PWR_PIN, LOW);

  Serial << "Moisture: " << moisture << MOISTURE_UNIT << endl;
  moistureNode.setProperty(READING_VALUE).send(String(moisture));
}

void readTemp(){
  // Reading temperature or humidity takes about 250 milliseconds!
  // Sensor readings may also be up to 2 seconds 'old' (its a very slow sensor)
  float h = tempSensor.readHumidity();
  // Read temperature as Celsius (the default)
  float t = tempSensor.readTemperature();

  // Check if any reads failed and exit early (to try again).
  if (isnan(h) || isnan(t)) {
    // Issue with sensor
    Serial.println("Failed to read from tempSensor sensor!");
    h = -1;
    t = -1;
  }

  temperatureNode.setProperty(READING_VALUE).send(String(t));
  humidityNode.setProperty(READING_VALUE).send(String(h));

  Serial << "Humidity: " << h << HUMIDITY_UNIT << endl;
  Serial << "Temperature: " << t << TEMPERATURE_UNIT << endl;
}

void loopHandler(){
  if(millis() - lastReadingSent >= (int) frequencySetting.get() * 1000UL || lastReadingSent == 0){
    lastReadingSent = millis();

    readMoisture();
    readTemp();
  }
}

void setup() {
  Serial.begin(115200);
  Serial << endl << endl;

  // Drive analog through digital pin (to stop current going through the sensor when not required)
  pinMode(MOISTURE_SENSOR_PWR_PIN, OUTPUT);
  tempSensor.begin();

  Homie_setFirmware("plantnanny-moisture", "0.0.5");
  Homie_setBrand("plantnanny")
  Homie.setSetupFunction(setupHandler).setLoopFunction(loopHandler);

  frequencySetting.setDefaultValue(READING_INTERVAL).setValidator([] (unsigned long candidate){
    return (candidate >= 1) && (candidate <= 604800); // a week
  });

  Serial << "FrequencySetting: " << frequencySetting.get() << endl;

  // Setup each node with what info will be send
  moistureNode.advertise(READING_UNIT);
  moistureNode.advertise(READING_VALUE);

  temperatureNode.advertise(READING_UNIT);
  temperatureNode.advertise(READING_VALUE);

  humidityNode.advertise(READING_UNIT);
  humidityNode.advertise(READING_VALUE);

  Homie.setup();
}

void loop() {
  Homie.loop();
}
