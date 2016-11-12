#include <Arduino.h>

#define OUTPUT1 D8
#define OUTPUT2 D7
#define OUTPUT3 D6
#define OUTPUT4 D5

void test_setup() {
  Serial.begin(115200);

  // Drive analog through digital pin (to stop current going through the sensor when not required)
  pinMode(OUTPUT1, OUTPUT);
  pinMode(OUTPUT2, OUTPUT);
  pinMode(OUTPUT3, OUTPUT);
  pinMode(OUTPUT4, OUTPUT);

  pinMode(D4, OUTPUT);

}

void test_loop() {
  Serial.println("Output1");
  digitalWrite(OUTPUT1, HIGH);
  digitalWrite(OUTPUT2, LOW);
  digitalWrite(OUTPUT3, LOW);
  digitalWrite(OUTPUT4, LOW);

  digitalWrite(D4, HIGH);
  delay(500);
  digitalWrite(D4, LOW);

  delay(10000);
  Serial.println("Output2");
  digitalWrite(OUTPUT1, LOW);
  digitalWrite(OUTPUT2, HIGH);
  digitalWrite(OUTPUT3, LOW);
  digitalWrite(OUTPUT4, LOW);

  digitalWrite(D4, HIGH);
  delay(500);
  digitalWrite(D4, LOW);

  delay(10000);
  Serial.println("Output3");
  digitalWrite(OUTPUT1, LOW);
  digitalWrite(OUTPUT2, LOW);
  digitalWrite(OUTPUT3, HIGH);
  digitalWrite(OUTPUT4, LOW);

  digitalWrite(D4, HIGH);
  delay(500);
  digitalWrite(D4, LOW);

  delay(10000);
  Serial.println("Output4");
  digitalWrite(OUTPUT1, LOW);
  digitalWrite(OUTPUT2, LOW);
  digitalWrite(OUTPUT3, LOW);
  digitalWrite(OUTPUT4, HIGH);
  delay(10000);
}
