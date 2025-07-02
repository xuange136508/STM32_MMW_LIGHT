/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file    usart.h
  * @brief   This file contains all the function prototypes for
  *          the usart.c file
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2025 STMicroelectronics.
  * All rights reserved.
  *
  * This software is licensed under terms that can be found in the LICENSE file
  * in the root directory of this software component.
  * If no LICENSE file comes with this software, it is provided AS-IS.
  *
  ******************************************************************************
  */
/* USER CODE END Header */
/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __USART_H__
#define __USART_H__

#ifdef __cplusplus
extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include "main.h"

/* USER CODE BEGIN Includes */
#include <stdint.h>
#include <stdbool.h>
/* USER CODE END Includes */

extern UART_HandleTypeDef huart1;

/* USER CODE BEGIN Private defines */
// 添加USART3用于ESP32通信
extern UART_HandleTypeDef huart3;

// 添加USART2用于数据接收
extern UART_HandleTypeDef huart2;

// ESP32通信缓冲区大小
#define ESP32_TX_BUFFER_SIZE 512
#define ESP32_RX_BUFFER_SIZE 512

// USART2接收缓冲区大小
#define USART2_RX_BUFFER_SIZE 256

// USART2命令定义（16进制）
#define CMD_WAKEUP_UNI    0xAA    // 你好小盛
#define CMD_TURN_ON       0xA3    // 打开夜灯  
#define CMD_TURN_OFF      0xA4    // 关闭夜灯
#define CMD_CHAT_ON       0xA1    // 开启聊天
#define CMD_CHAT_OFF      0xA2    // 关闭聊天
#define CMD_PLAY          0xA5    // 播放胎教
#define CMD_PLAY_BG       0xA6    // 播放白噪音
#define CMD_PAUSE         0xA7    // 暂停播放
#define CMD_STOP          0xA8    // 停止播放

// ESP32通信状态
typedef enum {
    ESP32_COMM_IDLE = 0,
    ESP32_COMM_SENDING,
    ESP32_COMM_RECEIVING,
    ESP32_COMM_ERROR
} ESP32_CommState_t;

/* USER CODE END Private defines */

void MX_USART1_UART_Init(void);

/* USER CODE BEGIN Prototypes */
// 添加USART3初始化函数
void MX_USART3_UART_Init(void);

// 添加USART2初始化函数
void MX_USART2_UART_Init(void);

// ESP32通信相关函数
void ESP32_Init(void);
bool ESP32_SendJSON(const char* json_string);
bool ESP32_ReceiveJSON(char* json_buffer, uint16_t buffer_size);
void ESP32_ProcessReceivedData(void);

// JSON构建函数
void ESP32_BuildSensorJSON(char* json_buffer, uint16_t buffer_size);
void ESP32_BuildControlJSON(char* json_buffer, uint16_t buffer_size);

// USART2接收相关函数
void USART2_Init(void);
bool USART2_GetReceivedData(char* buffer, uint16_t buffer_size);
void USART2_ProcessHexCommand(uint8_t cmd);
const char* USART2_GetCommandName(uint8_t cmd);

/* USER CODE END Prototypes */

#ifdef __cplusplus
}
#endif

#endif /* __USART_H__ */

