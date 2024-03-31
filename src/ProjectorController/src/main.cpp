#include <Arduino.h>

#include <SoftwareSerial.h>

SoftwareSerial serialProjector(10,11);

void setup() {
  // put your setup code here, to run once:
  Serial.begin(9600);
  serialProjector.begin(9600);
}

void loop() {
  // put your main code here, to run repeatedly:
  serialProjector.write("Hello World\r");
}

