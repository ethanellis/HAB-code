#include <Wire.h>
#include <Adafruit_Sensor.h>
#include <Adafruit_BME280.h>

Adafruit_BME280 bme1 (0x77); // I2C
Adafruit_BME280 bme2 (0x76); // I2C

unsigned long delayTime;

void setup() {
  // put your setup code here, to run once:
  Serial.begin(9600);
  while(!Serial);
  Serial.println(F("BME280 test"));

  unsigned status;
  status = bme1.begin();
  if (!status) {
    Serial.println("Could not find valid BME280 sensor");
    Serial.print("SensorID was: 0x"); Serial.println(bme1.sensorID(),16);
    Serial.print("        ID of 0xFF probably means a bad address, a BMP 180 or BMP 085\n");
    Serial.print("   ID of 0x56-0x58 represents a BMP 280,\n");
    Serial.print("        ID of 0x60 represents a BME 280.\n");
    Serial.print("        ID of 0x61 represents a BME 680.\n");
    while (1);
  }

  Serial.println("-- Default Test --");
  delayTime = 1000;

  Serial.println();

}

void loop() {
  // put your main code here, to run repeatedly:
  printValues();
  delay(delayTime);
}



void printValues() {
  Serial.print("BME 1 Temperature = ");
  Serial.print(bme1.readTemperature());
  Serial.println(" *C");

  Serial.print("BME 1 Pressure = ");
  Serial.print(bme1.readPressure() / 100.0F);
  Serial.println(" hPa");

  Serial.print("BME 1 Humidity = ");
  Serial.print(bme1.readHumidity());
  Serial.println(" %");

  Serial.print("BME 2 Temperature = ");
  Serial.print(bme2.readTemperature());
  Serial.println(" *C");

  Serial.print("BME 2 Pressure = ");
  Serial.print(bme2.readPressure() / 100.0F);
  Serial.println(" hPa");

  Serial.print("BME 2 Humidity = ");
  Serial.print(bme2.readHumidity());
  Serial.println(" %");

  Serial.println();
}
