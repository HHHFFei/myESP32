# ESP32-CAM & INM441
https://www.bilibili.com/video/BV1xA411Q76y/
https://www.youtube.com/watch?v=m8LwPNXqK9o
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
|   |IO15     |WS     |
|   |IO2      |SCK    |
|   |IO13     |SD     |

烧录程序时，ESP32-CAM的IO0与GND相接  

## 3. 接口协议
1. I2S

## 4. 接口驱动

## 5. 文件格式
1. wav文件头
```
typedef struct WAV_HEADER_S
{
    char	        riff_tag[4]{'R','I','F','F'};   // 4byte, 资源交换文件标志:RIFF
    unsigned int	riff_length;                    // 4byte, 从下个地址到文件结尾的总字符数 = 文件总大小-8
    char	        wave_tag[4]{'W','A','V','E'};   // 4byte, wav文件标志:WAVE
    char	        fmt_tag[4]{'f','m','t',' '};    // 4byte, 波形文件标志:fmt
    unsigned int	fmt_length{16};                 // 4byte, fmt chunk块的大小（下面六个数据）
    unsigned short	audio_format{1};                // 2byte, 格式种类
    unsigned short	num_channels;                   // 2byte, 通道数
    unsigned int	sample_rate;                    // 4byte, 采样率
    unsigned int	byte_rate;                      // 4byte, 每秒传输速率
    unsigned short	block_align;                    // 2byte, 数据块的对齐
    unsigned short	bits_per_sample;                // 2byte, 采样精度-PCM位宽
    char	        data_tag[4]{'d','a','t','a};    // 4byte, 数据标志：data
    unsigned int	data_length;                    // 4byte, 从下个地址到文件结尾的总字节数
}WAV_HEADER;
```  
2. 数据


## 