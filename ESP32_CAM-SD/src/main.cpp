#include <Arduino.h>
#include "FS.h"                // SD Card ESP32
#include "SD_MMC.h"            // SD Card ESP32
#include "soc/soc.h"           // Disable brownour problems
#include "soc/rtc_cntl_reg.h"  // Disable brownour problems
#include "driver/rtc_io.h"
#include <EEPROM.h>            // read and write from flash memory

// define the number of bytes you want to access
#define EEPROM_SIZE 1

int fileNumber = 0;

void setup() {
  WRITE_PERI_REG(RTC_CNTL_BROWN_OUT_REG, 0); //disable brownout detector
 
  Serial.begin(9600);
  //Serial.setDebugOutput(true);
  //Serial.println();
  
}

void wirte_file(){
  //Serial.println("Starting SD Card");
  if(!SD_MMC.begin()){
    Serial.println("SD Card Mount Failed");
    return;
  }
  
  uint8_t cardType = SD_MMC.cardType();
  if(cardType == CARD_NONE){
    Serial.println("No SD Card attached");
    return;
  }
  
  // initialize EEPROM with predefined size
  EEPROM.begin(EEPROM_SIZE);
  fileNumber = EEPROM.read(0) + 1;

  // Path where new picture will be saved in SD Card
  String path = "/test" + String(fileNumber) +".txt";

  fs::FS &fs = SD_MMC; 
  Serial.printf("file name: %s\n", path.c_str());
  
  File file = fs.open(path.c_str(), FILE_WRITE);
  if(!file){
    Serial.println("Failed to open file in writing mode");
  } 
  else {
    Serial.printf("Saved file to path: %s\n", path.c_str());
    EEPROM.write(0, fileNumber);
    EEPROM.commit();
  }
  file.close();
}

void loop() {
  wirte_file();
  delay(1000);
}
