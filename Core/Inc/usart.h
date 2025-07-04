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

// B系列命令（屏幕和音频控制）
#define CMD_SCREEN_OFF    0xB1    // 关闭屏幕
#define CMD_SCREEN_ON     0xB2    // 亮屏
#define CMD_LIGHT_UP      0xB3    // 夜灯调亮
#define CMD_LIGHT_DOWN    0xB4    // 夜灯调暗
#define CMD_MUTE          0xB5    // 静音
#define CMD_UNMUTE        0xB6    // 取消静音
#define CMD_VOL_UP        0xB7    // 声音调大
#define CMD_VOL_DOWN      0xB8    // 声音调小
#define CMD_VOL_MAX       0xB9    // 声音调到最大

// C系列命令（时间和记录控制）
#define CMD_TIMER_STOP    0xC1    // 定时停止
#define CMD_PREV_SONG     0xC2    // 上一首
#define CMD_NEXT_SONG     0xC3    // 下一首
#define CMD_START_FETAL   0xC4    // 开始记胎动
#define CMD_START_RECORD  0xC5    // 开始记录
#define CMD_START_SLEEP   0xC6    // 开始记录宝睡眠
#define CMD_END_FETAL     0xC7    // 结束记胎动
#define CMD_END_RECORD    0xC8    // 结束记录
#define CMD_END_SLEEP     0xC9    // 结束记录宝睡眠

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
void ESP32_BuildPlayStatusJSON(char* json_buffer, uint16_t buffer_size, const char* play_status, const char* content_type);

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

