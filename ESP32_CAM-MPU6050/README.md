# ESP32-CAM & MPU6050
## 1. 开发板/模块  
1. ESP32-CAM  
2. MPU6050  

## 2. 接线顺序
|TTL|ESP32-CAM|MPU6050|
|---|---------|-------|
|5V |5V       |VCC    |
|GND|GND      |GND    |
|TXD|U0R      |       |
|RXD|U0T      |       |
|   |IO15     |SCL    |
|   |IO14     |SDA    |

烧录程序时，ESP32-CAM的IO0与GND相接  

## 3. 接口协议
1. I2C

## 4. 接口驱动

## 