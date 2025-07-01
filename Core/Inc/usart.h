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

// ESP32通信缓冲区大小
#define ESP32_TX_BUFFER_SIZE 512
#define ESP32_RX_BUFFER_SIZE 512

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

// ESP32通信相关函数
void ESP32_Init(void);
bool ESP32_SendJSON(const char* json_string);
bool ESP32_ReceiveJSON(char* json_buffer, uint16_t buffer_size);
void ESP32_ProcessReceivedData(void);

// JSON构建函数
void ESP32_BuildSensorJSON(char* json_buffer, uint16_t buffer_size);
void ESP32_BuildControlJSON(char* json_buffer, uint16_t buffer_size);

/* USER CODE END Prototypes */

#ifdef __cplusplus
}
#endif

#endif /* __USART_H__ */

