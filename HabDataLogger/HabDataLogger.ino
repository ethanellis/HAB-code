// libraries and variables for SD card
#include <SPI.h>
#include <SD.h>
// libraries and variables for BME sensors
#include <Wire.h>
#include "BlueDot_BME280.h"
BlueDot_BME280 bme1;
BlueDot_BME280 bme2;
// libraries for GPS
#include <TinyGPS++.h>
TinyGPSPlus gps;
String receivedMessage;

//set up variable for radiation detector
int count = 0;

int fileCountOnSD = 0; // for counting files
int datalogNum = 1; // for enumerating datalog
String datalogName; // root name for datalog file
File root;

void setup() {

  // Establish serial connections and wait for response:
  Serial.begin(9600);
  Serial1.begin(9600);
  while(!Serial);


  //set parameters for radiation sensor:
  attachInterrupt(digitalPinToInterrupt(2), countPulse, RISING);

  // set parameters for BME sensors:
  bme1.parameter.communication = 1;
  bme2.parameter.communication = 1;
  bme1.parameter.SPI_cs = 12;
  bme2.parameter.SPI_cs = 13;
  bme1.parameter.SPI_mosi = 9;
  bme2.parameter.SPI_mosi = 9;
  bme1.parameter.SPI_miso = 10;
  bme2.parameter.SPI_miso = 10;
  bme1.parameter.SPI_sck = 11;
  bme2.parameter.SPI_sck = 11;
  bme1.parameter.sensorMode = 0b11;
  bme2.parameter.sensorMode = 0b11;
  bme1.parameter.IIRfilter = 0b100;
  bme2.parameter.IIRfilter = 0b100;
  bme1.parameter.tempOversampling = 0b101;
  bme2.parameter.tempOversampling = 0b101;
  bme1.parameter.pressOversampling = 0b101;
  bme2.parameter.pressOversampling = 0b101;
  bme1.parameter.humidOversampling = 0b101;
  bme2.parameter.humidOversampling = 0b101;
  // standard pressure, temperature for BME sensor reference:
  bme1.parameter.pressureSeaLevel = 1013.25;
  bme2.parameter.pressureSeaLevel = 1013.25;
  bme1.parameter.tempOutsideCelsius = 15;
  bme1.parameter.tempOutsideFahrenheit = 59;
  bme2.parameter.tempOutsideCelsius = 15;
  bme2.parameter.tempOutsideFahrenheit = 59;

  // Initialize bme sensor 1:
  if(bme1.init() != 0x60){
    Serial.println(F("First BME280 Sensor not detected"));
    while(1);
  }
  Serial.println(F("First BME280 Sensor detected."));
  
  // Initialize bme sensor 2:
  if(bme2.init() != 0x60){
    Serial.println(F("Second BME280 Sensor not detected"));
    while(1);
  }
  Serial.println(F("Second BME280 Sensor detected."));

  // Initialize SD card:
  Serial.println(F("Initializing SD card..."));
  if(!SD.begin(53)) {
    Serial.println(F("Card failed."));
    while(1);
  }
  Serial.println(F("Card initialized."));


  // check SD card to see how many files already exist on it. Set datalogNum accordingly.
  root = SD.open("/");
  printDirectory(root, 0);
  datalogNum += fileCountOnSD - 3; // not sure why this offset is needed... maybe hidden files on card.

  datalogName = "log";
  datalogName += datalogNum;

}

// Trigger for logging headers at first line:
int firstLine = 0;

void loop() {
  
  // if card is removed, then halt loop. Resume when card is replaced.
  while(!SD.begin(53));
  // begin datalog
  File dataFile = SD.open(datalogName, FILE_WRITE);
  
  // If first line, print headers:
  if(!firstLine){
    dataFile.println(F("TIME(MS),LAT(degN),LONG(degE),ALT(M),SPD(MPS),TEMP1(C),HUM1(%),PRES1(hPa),ALT1(M),TEMP2(C),HUM2(%),PRES2(hPa),ALT2(M),RAD(COUNT)"));
    firstLine = 1;
  }

  String dateString = "";

  dateString += "H:";
  dateString += gps.time.hour();
  dateString += "M:";
  dateString += gps.time.minute();
  dateString += "S:";
  dateString += gps.time.second();

  dataFile.print(dateString);
  dataFile.print(F(","));
  dataFile.print(gps.location.lat(),6);
  dataFile.print(F(","));
  dataFile.print(gps.location.lng(),6);
  dataFile.print(F(","));
  dataFile.print(gps.altitude.meters(),6);
  dataFile.print(F(","));
  dataFile.print(gps.speed.mps(),6);
  dataFile.print(F(","));

  // record GPS module values:
   while(Serial1.available() > 0){
    char rchar = Serial1.read();
    if(rchar == '\n'){
      //Serial.println(receivedMessage);
      //Serial.println(gps.location.lat());
      //Serial.println(gps.location.lng());
      //Serial.println(gps.altitude.meters());
      //Serial.println();
      receivedMessage = "";
    }else{
      gps.encode(rchar);
      receivedMessage += rchar;
    }
  }

  // BME sensor 1 values:
  dataFile.print(String(bme1.readTempC()));
  dataFile.print(F(","));
  dataFile.print(String(bme1.readHumidity()));
  dataFile.print(F(","));
  dataFile.print(String(bme1.readPressure()));
  dataFile.print(F(","));
  dataFile.print(String(bme1.readAltitudeMeter()));
  dataFile.print(F(","));

  // BME sensor 2 values:
  dataFile.print(String(bme2.readTempC()));
  dataFile.print(F(","));
  dataFile.print(String(bme2.readHumidity()));
  dataFile.print(F(","));
  dataFile.print(String(bme2.readPressure()));
  dataFile.print(F(","));
  dataFile.print(String(bme2.readAltitudeMeter()));
  dataFile.print(F(","));

  dataFile.println(count);
  
  // Close file to save out, indicate successful measurement:
  dataFile.close();
  Serial.println(F("MEASUREMENT"));
  // perform measurements every second.
  delay(1000);
}



// assorted junk for datalog file naming
void printDirectory(File dir, int numTabs) {
  while (true) {

    File entry =  dir.openNextFile();
    if (! entry) {
      // no more files
      break;
    }
    for (uint8_t i = 0; i < numTabs; i++) {
      Serial.print('\t');
    }
    Serial.print(entry.name());

    // for each file count it
    fileCountOnSD++;

    if (entry.isDirectory()) {
      Serial.println("/");
      printDirectory(entry, numTabs + 1);
    } else {
      // files have sizes, directories do not
      Serial.print("\t\t");
      Serial.println(entry.size(), DEC);
    }
    entry.close();
  }
}

void countPulse(){
  count = count + 1;
}





