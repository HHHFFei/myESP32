# ESP32-CAM & INM441
https://www.bilibili.com/video/BV1xA411Q76y/
## 1. 开发板/模块  
1. ESP32-CAM  
2. INM441  

## 2. 接线顺序
|TTL|ESP32-CAM|INM441|
|---|---------|-------|
|5V |5V       |VDD    |
|GND|GND      |GND    |
|TXD|U0R      |       |
|RXD|U0T      |       |
|   |IO16     |WS    |
|   |IO13     |SCK    |
|   |IO12     |SD    |

烧录程序时，ESP32-CAM的IO0与GND相接  

## 3. 接口协议
1. I2S

## 4. 接口驱动

## 