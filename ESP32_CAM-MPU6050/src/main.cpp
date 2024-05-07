#include <Arduino.h>
#include <WiFi.h>

////////////////////////////////////STA/////////////////////////////////////////
const char *ssid = "f"; //wifi名
const char *password = "10011001";//wifi密码

const IPAddress serverIP(192,168,1,245); //欲访问的服务端IP地址
uint16_t serverPort = 56050;         //服务端口号

WiFiClient client; //声明一个ESP32客户端对象，用于与服务器进行连接
////////////////////////////////////STA/////////////////////////////////////////

////////////////////////////////////MPU6050/////////////////////////////////////////
#include "I2Cdev.h"
#include "MPU6050_6Axis_MotionApps612.h"
#if I2CDEV_IMPLEMENTATION == I2CDEV_ARDUINO_WIRE
#include "Wire.h"
#endif

MPU6050 mpu;
#define OUTPUT_READABLE_REALACCEL
#define INTERRUPT_PIN 2  // use pin 2 on Arduino Uno & most boards
#define LED_PIN 13 // (Arduino is 13, Teensy is 11, Teensy++ is 6)
bool blinkState = false;
// MPU control/status vars
bool dmpReady = false;  // set true if DMP init was successful
uint8_t mpuIntStatus;   // holds actual interrupt status byte from MPU
uint8_t devStatus;      // return status after each device operation (0 = success, !0 = error)
uint16_t packetSize;    // expected DMP packet size (default is 42 bytes)
uint16_t fifoCount;     // count of all bytes currently in FIFO
uint8_t fifoBuffer[64]; // FIFO storage buffer
// orientation/motion vars
Quaternion q;           // [w, x, y, z]         quaternion container
VectorInt16 aa;         // [x, y, z]            accel sensor measurements
VectorInt16 gy;         // [x, y, z]            gyro sensor measurements
VectorInt16 aaReal;     // [x, y, z]            gravity-free accel sensor measurements
VectorInt16 aaWorld;    // [x, y, z]            world-frame accel sensor measurements
VectorFloat gravity;    // [x, y, z]            gravity vector
float euler[3];         // [psi, theta, phi]    Euler angle container
float ypr[3];           // [yaw, pitch, roll]   yaw/pitch/roll container and gravity vector
// packet structure for InvenSense teapot demo
uint8_t teapotPacket[14] = { '$', 0x02, 0, 0, 0, 0, 0, 0, 0, 0, 0x00, 0x00, '\r', '\n' };
// ================================================================
// ===               INTERRUPT DETECTION ROUTINE                ===
// ================================================================

volatile bool mpuInterrupt = false;     // indicates whether MPU interrupt pin has gone high
void dmpDataReady() {
  mpuInterrupt = true;
}
////////////////////////////////////MPU6050/////////////////////////////////////////

void setup()
{
    Serial.begin(9600);
    Serial.println();

    WiFi.mode(WIFI_STA);
    WiFi.setSleep(false); //关闭STA模式下wifi休眠，提高响应速度
    WiFi.begin(ssid, password);
    WiFi.setAutoReconnect(true);
    while (WiFi.status() != WL_CONNECTED)
    {
        delay(500);
        Serial.print(".");
    }
    Serial.print("是否连接:");
    Serial.println(WiFi.isConnected());
    Serial.print("本地IP:");
    Serial.println(WiFi.localIP());
    Serial.print("本地IPv6:");
    Serial.println(WiFi.localIPv6());
    Serial.print("mac地址:");
    Serial.println(WiFi.macAddress());
    Serial.print("网络ID:");
    Serial.println(WiFi.networkID());
    Serial.print("休息:");
    Serial.println(WiFi.getSleep());
    Serial.print("获取状态吗:");
    Serial.println(WiFi.getStatusBits());
    Serial.print("getTxPower:");
    Serial.println(WiFi.getTxPower());
    Serial.print("是否自动连接:");
    Serial.println(WiFi.getAutoConnect());
    Serial.print("是否自动重连:");
    Serial.println(WiFi.getAutoReconnect());
    Serial.print("获取模式:");
    Serial.println(WiFi.getMode());
    Serial.print("获取主机名:");
    Serial.println(WiFi.getHostname());
    Serial.print("获取网关IP:");
    Serial.println(WiFi.gatewayIP());
    Serial.print("dnsIP:");
    Serial.println(WiFi.dnsIP());
    Serial.print("状态:");
    Serial.println(WiFi.status());

    #if I2CDEV_IMPLEMENTATION == I2CDEV_ARDUINO_WIRE
    Wire.begin(14, 15);//SDA14，SCL15
    Wire.setClock(400000); // 400kHz I2C clock. Comment this line if having compilation difficulties
#elif I2CDEV_IMPLEMENTATION == I2CDEV_BUILTIN_FASTWIRE
    Fastwire::setup(400, true);
#endif
    // initialize device
    Serial.println(F("Initializing I2C devices..."));
    mpu.initialize();
    pinMode(INTERRUPT_PIN, INPUT);

    // verify connection
    Serial.println(F("Testing device connections..."));
    Serial.println(mpu.testConnection() ? F("MPU6050 connection successful") : F("MPU6050 connection failed"));

    // wait for ready
    Serial.println(F("\nSend any character to begin DMP programming and demo: "));
    // while (Serial.available() && Serial.read()); // empty buffer
    // while (!Serial.available());                 // wait for data
    // while (Serial.available() && Serial.read()); // empty buffer again

    // load and configure the DMP
    Serial.println(F("Initializing DMP..."));
    devStatus = mpu.dmpInitialize();

    // supply your own gyro offsets here, scaled for min sensitivity
    mpu.setXGyroOffset(51);
    mpu.setYGyroOffset(8);
    mpu.setZGyroOffset(21);
    mpu.setXAccelOffset(1150);
    mpu.setYAccelOffset(-50);
    mpu.setZAccelOffset(1060);
    // make sure it worked (returns 0 if so)
    if (devStatus == 0) {
        // Calibration Time: generate offsets and calibrate our MPU6050
        mpu.CalibrateAccel(6);
        mpu.CalibrateGyro(6);
        Serial.println();
        mpu.PrintActiveOffsets();
        // turn on the DMP, now that it's ready
        Serial.println(F("Enabling DMP..."));
        mpu.setDMPEnabled(true);

        // enable Arduino interrupt detection
        Serial.print(F("Enabling interrupt detection (Arduino external interrupt "));
        Serial.print(digitalPinToInterrupt(INTERRUPT_PIN));
        Serial.println(F(")..."));
        attachInterrupt(digitalPinToInterrupt(INTERRUPT_PIN), dmpDataReady, RISING);
        mpuIntStatus = mpu.getIntStatus();

        // set our DMP Ready flag so the main loop() function knows it's okay to use it
        Serial.println(F("DMP ready! Waiting for first interrupt..."));
        dmpReady = true;

        // get expected DMP packet size for later comparison
        packetSize = mpu.dmpGetFIFOPacketSize();
    } else {
        // ERROR!
        // 1 = initial memory load failed
        // 2 = DMP configuration updates failed
        // (if it's going to break, usually the code will be 1)
        Serial.print(F("DMP Initialization failed (code "));
        Serial.print(devStatus);
        Serial.println(F(")"));
    }
    // configure LED for output
    pinMode(LED_PIN, OUTPUT);
}

void loop()
{
    Serial.println("尝试访问服务器");
    if (client.connect(serverIP, serverPort)) //尝试访问目标地址
    {
        Serial.println("访问成功");
        while (client.connected() || client.available()) //如果已连接或有收到的未读取的数据
        {
            // if (client.available()) //如果有数据可读取
            // {
            //     String line = client.readStringUntil('\n'); //读取数据到换行符
            //     Serial.print("读取到数据：");
            //     Serial.println(line);
            //     client.write(line.c_str()); //将收到的数据回发
            // }
            // 发送数据
            if (!dmpReady) continue;
            // read a packet from FIFO
            if (mpu.dmpGetCurrentFIFOPacket(fifoBuffer)) { // Get the Latest packet 
                // display initial world-frame acceleration, adjusted to remove 
                // gravity and rotated based on known orientation from quaternion
                //显示初始世界帧加速度，已调整以消除重力，并根据四元数的已知方向进行了旋转
                mpu.dmpGetQuaternion(&q, fifoBuffer);
                mpu.dmpGetAccel(&aa, fifoBuffer);
                mpu.dmpGetGravity(&gravity, &q);
                mpu.dmpGetLinearAccel(&aaReal, &aa, &gravity);
                mpu.dmpGetLinearAccelInWorld(&aaWorld, &aaReal, &q);
                Serial.print(String(aaWorld.x).c_str());
                Serial.print(",");
                Serial.print(String(aaWorld.y).c_str());
                Serial.print(",");
                Serial.print(String(aaWorld.z).c_str());
                Serial.println(";");

                client.write(String(aaWorld.x).c_str());
                client.write(",");
                client.write(String(aaWorld.y).c_str());
                client.write(",");
                client.write(String(aaWorld.z).c_str());
                client.write(";");
                // blink LED to indicate activity
                blinkState = !blinkState;
                digitalWrite(LED_PIN, blinkState);
            }
            delay(1);
        }
        Serial.println("关闭当前连接");
        client.stop(); //关闭客户端
    }
    else
    {
        Serial.println("访问失败");
        client.stop(); //关闭客户端
    }
    delay(5000);
}