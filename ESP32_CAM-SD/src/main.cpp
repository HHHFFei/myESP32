#include <Arduino.h>
#include "FS.h"                // SD Card ESP32
#include "SD_MMC.h"            // SD Card ESP32
#include "soc/soc.h"           // Disable brownour problems
#include "soc/rtc_cntl_reg.h"  // Disable brownour problems
#include "driver/rtc_io.h"
#include <EEPROM.h>            // read and write from flash memory
#include <driver/i2s.h>
#include "wave.h"

#define I2S_WS 15
#define I2S_SD 13
#define I2S_SCK 2
#define I2S_PORT I2S_NUM_0
// define the number of bytes you want to access
#define EEPROM_SIZE 1
const int record_time = 60; // 固定采样60s
const int wave_data_size = record_time * 88200; // 总数据大小
int fileNumber = 0;

void i2s_install()
{
  const i2s_config_t i2s_config =
      {
          .mode = i2s_mode_t(I2S_MODE_MASTER | I2S_MODE_RX),
          .sample_rate = 44100,
          .bits_per_sample = i2s_bits_per_sample_t(16),
          .channel_format = I2S_CHANNEL_FMT_ONLY_LEFT,
          .communication_format = i2s_comm_format_t(I2S_COMM_FORMAT_I2S | I2S_COMM_FORMAT_I2S_MSB),
          .intr_alloc_flags = 0,
          .dma_buf_count = 8,
          .dma_buf_len = 64,
          .use_apll = false};
  i2s_driver_install(I2S_PORT, &i2s_config, 0, NULL);
}

void i2s_setpin()
{
  const i2s_pin_config_t pin_config =
      {
          .bck_io_num = I2S_SCK,
          .ws_io_num = I2S_WS,
          .data_out_num = -1,
          .data_in_num = I2S_SD};
  i2s_set_pin(I2S_PORT, &pin_config);
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
  String path = "/test" + String(fileNumber) +".wav";

  fs::FS &fs = SD_MMC; 
  Serial.printf("file name: %s\n", path.c_str());
  
  File file = fs.open(path.c_str(), FILE_WRITE);
  if(!file){
    Serial.println("Failed to open file in writing mode");
  } 
  else {
    // 创建并写入wav文件头
    auto header = CreateWaveHeader(1, 44100, 16);
    header.riffSize = wave_data_size + 44 - 8;
    header.dataSize = wave_data_size;
    file.write((uint8_t*)&header, 44); 
    // 写入音频数据
    // uint8_t buf[100];
    // memcpy(buf,path.c_str(),100*sizeof(uint8_t));
    // file.write(buf, 100*sizeof(uint8_t)); // payload (image), payload length

    uint8_t buf[60];
    for(int i = 0;i<60;i++)
    {
      int32_t sample = 0;
      size_t bytes = 0;
      i2s_read(I2S_PORT, (char *)&sample, 8, &bytes, portMAX_DELAY);
      if (bytes > 0)
      {
        buf[i] = (uint8_t)sample;
        // Serial.println(sample);
      }
      else
      {
        // Serial.print("-1");
      }
    }
    file.write(buf, 60*sizeof(uint8_t)); // payload (image), payload length

    Serial.printf("Saved file to path: %s\n", path.c_str());
    EEPROM.write(0, fileNumber);
    EEPROM.commit();
  }
  file.close();
}

void setup() {
  WRITE_PERI_REG(RTC_CNTL_BROWN_OUT_REG, 0); //disable brownout detector
  Serial.begin(9600);
  Serial.println("Setup I2S");

  delay(1000);
  i2s_install();
  i2s_setpin();
  i2s_start(I2S_PORT);
  delay(1000);
}

void loop() {
  wirte_file();
  delay(1000);
}
