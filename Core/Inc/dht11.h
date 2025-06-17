#ifndef __DHT11_H
#define __DHT11_H

#include "main.h"
#include "gpio.h"

// DHT11数据结构
typedef struct {
    uint8_t humidity_int;    // 湿度整数部分
    uint8_t humidity_dec;    // 湿度小数部分  
    uint8_t temperature_int; // 温度整数部分
    uint8_t temperature_dec; // 温度小数部分
    uint8_t checksum;        // 校验和
} DHT11_Data_t;

// 函数声明
void DHT11_SetPinOutput(void);
void DHT11_SetPinInput(void);
void DHT11_SetPin(GPIO_PinState state);
uint8_t DHT11_ReadPin(void);
void DHT11_DelayUs(uint32_t us);
uint8_t DHT11_Start(void);
uint8_t DHT11_ReadByte(void);
uint8_t DHT11_ReadData(DHT11_Data_t *data);

#endif /* __DHT11_H */ 