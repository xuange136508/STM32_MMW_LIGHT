/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file    usart.c
  * @brief   This file provides code for the configuration
  *          of the USART instances.
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
/* Includes ------------------------------------------------------------------*/
#include "usart.h"

/* USER CODE BEGIN 0 */
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

// ESP32通信相关变量
static char esp32_tx_buffer[ESP32_TX_BUFFER_SIZE];
static char esp32_rx_buffer[ESP32_RX_BUFFER_SIZE];
static volatile uint16_t esp32_rx_index = 0;
static volatile ESP32_CommState_t esp32_comm_state = ESP32_COMM_IDLE;
static volatile bool esp32_data_ready = false;

// 外部传感器数据结构声明（从freertos.c引用）
extern volatile struct {
    float adc_voltage;
    uint8_t vibration_detected;
    uint8_t touch_detected;
    float humidity;
    float temperature;
    uint8_t dht11_valid;
} g_sensor_data;

extern volatile struct {
    uint8_t breathing_led_enabled;
    uint8_t rgb_led_enabled;
} g_control_state;

// USART2接收相关变量
static uint8_t usart2_rx_byte = 0;
static volatile bool usart2_cmd_ready = false;

/* USER CODE END 0 */

UART_HandleTypeDef huart1;

/* USER CODE BEGIN Private variables */
UART_HandleTypeDef huart3;  // 添加USART3句柄
UART_HandleTypeDef huart2;  // 添加USART2句柄
/* USER CODE END Private variables */

/* USART1 init function */

void MX_USART1_UART_Init(void)
{

  /* USER CODE BEGIN USART1_Init 0 */

  /* USER CODE END USART1_Init 0 */

  /* USER CODE BEGIN USART1_Init 1 */

  /* USER CODE END USART1_Init 1 */
  huart1.Instance = USART1;
  huart1.Init.BaudRate = 115200;
  huart1.Init.WordLength = UART_WORDLENGTH_8B;
  huart1.Init.StopBits = UART_STOPBITS_1;
  huart1.Init.Parity = UART_PARITY_NONE;
  huart1.Init.Mode = UART_MODE_TX_RX;
  huart1.Init.HwFlowCtl = UART_HWCONTROL_NONE;
  huart1.Init.OverSampling = UART_OVERSAMPLING_16;
  if (HAL_UART_Init(&huart1) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN USART1_Init 2 */

  /* USER CODE END USART1_Init 2 */

}

/* USER CODE BEGIN USART3_Init */
/* USART3 init function - 用于ESP32通信 */
void MX_USART3_UART_Init(void)
{
  huart3.Instance = USART3;
  huart3.Init.BaudRate = 115200;  // ESP32常用波特率
  huart3.Init.WordLength = UART_WORDLENGTH_8B;
  huart3.Init.StopBits = UART_STOPBITS_1;
  huart3.Init.Parity = UART_PARITY_NONE;
  huart3.Init.Mode = UART_MODE_TX_RX;
  huart3.Init.HwFlowCtl = UART_HWCONTROL_NONE;
  huart3.Init.OverSampling = UART_OVERSAMPLING_16;
  
  if (HAL_UART_Init(&huart3) != HAL_OK)
  {
    Error_Handler();
  }
  
  // 启动UART接收中断
  HAL_UART_Receive_IT(&huart3, (uint8_t*)&esp32_rx_buffer[esp32_rx_index], 1);
}
/* USER CODE END USART3_Init */

/* USER CODE BEGIN USART2_Init */
/* USART2 init function - 用于数据接收 */
void MX_USART2_UART_Init(void)
{
  huart2.Instance = USART2;
  huart2.Init.BaudRate = 115200;
  huart2.Init.WordLength = UART_WORDLENGTH_8B;
  huart2.Init.StopBits = UART_STOPBITS_1;
  huart2.Init.Parity = UART_PARITY_NONE;
  huart2.Init.Mode = UART_MODE_TX_RX;
  huart2.Init.HwFlowCtl = UART_HWCONTROL_NONE;
  huart2.Init.OverSampling = UART_OVERSAMPLING_16;
  
  if (HAL_UART_Init(&huart2) != HAL_OK)
  {
    Error_Handler();
  }
  
  // 启动UART接收中断
  HAL_UART_Receive_IT(&huart2, &usart2_rx_byte, 1);
}
/* USER CODE END USART2_Init */

void HAL_UART_MspInit(UART_HandleTypeDef* uartHandle)
{

  GPIO_InitTypeDef GPIO_InitStruct = {0};
  if(uartHandle->Instance==USART1)
  {
  /* USER CODE BEGIN USART1_MspInit 0 */

  /* USER CODE END USART1_MspInit 0 */
    /* USART1 clock enable */
    __HAL_RCC_USART1_CLK_ENABLE();

    __HAL_RCC_GPIOA_CLK_ENABLE();
    /**USART1 GPIO Configuration
    PA9     ------> USART1_TX
    PA10     ------> USART1_RX
    */
    GPIO_InitStruct.Pin = GPIO_PIN_9|GPIO_PIN_10;
    GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_VERY_HIGH;
    GPIO_InitStruct.Alternate = GPIO_AF7_USART1;
    HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

    /* USART1 interrupt Init */
    HAL_NVIC_SetPriority(USART1_IRQn, 4, 0);
    HAL_NVIC_EnableIRQ(USART1_IRQn);
  /* USER CODE BEGIN USART1_MspInit 1 */

  /* USER CODE END USART1_MspInit 1 */
  }
  /* USER CODE BEGIN USART3_MspInit */
  else if(uartHandle->Instance==USART3)
  {
    /* USART3 clock enable */
    __HAL_RCC_USART3_CLK_ENABLE();
    __HAL_RCC_GPIOB_CLK_ENABLE();
    
    /**USART3 GPIO Configuration
    PB10     ------> USART3_TX
    PB11     ------> USART3_RX
    */
    GPIO_InitStruct.Pin = GPIO_PIN_10|GPIO_PIN_11;
    GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_VERY_HIGH;
    GPIO_InitStruct.Alternate = GPIO_AF7_USART3;
    HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);

    /* USART3 interrupt Init */
    HAL_NVIC_SetPriority(USART3_IRQn, 5, 0);
    HAL_NVIC_EnableIRQ(USART3_IRQn);
  }
  /* USER CODE END USART3_MspInit */
  else if(uartHandle->Instance==USART2)
  {
    /* USART2 clock enable */
    __HAL_RCC_USART2_CLK_ENABLE();
    __HAL_RCC_GPIOA_CLK_ENABLE();
    
    /**USART2 GPIO Configuration
    PA2     ------> USART2_TX
    PA3     ------> USART2_RX
    */
    GPIO_InitStruct.Pin = GPIO_PIN_2|GPIO_PIN_3;
    GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_VERY_HIGH;
    GPIO_InitStruct.Alternate = GPIO_AF7_USART2;
    HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

    /* USART2 interrupt Init */
    HAL_NVIC_SetPriority(USART2_IRQn, 5, 0);
    HAL_NVIC_EnableIRQ(USART2_IRQn);
  }
}

void HAL_UART_MspDeInit(UART_HandleTypeDef* uartHandle)
{

  if(uartHandle->Instance==USART1)
  {
  /* USER CODE BEGIN USART1_MspDeInit 0 */

  /* USER CODE END USART1_MspDeInit 0 */
    /* Peripheral clock disable */
    __HAL_RCC_USART1_CLK_DISABLE();

    /**USART1 GPIO Configuration
    PA9     ------> USART1_TX
    PA10     ------> USART1_RX
    */
    HAL_GPIO_DeInit(GPIOA, GPIO_PIN_9|GPIO_PIN_10);

    /* USART1 interrupt Deinit */
    HAL_NVIC_DisableIRQ(USART1_IRQn);
  /* USER CODE BEGIN USART1_MspDeInit 1 */

  /* USER CODE END USART1_MspDeInit 1 */
  }
  /* USER CODE BEGIN USART3_MspDeInit */
  else if(uartHandle->Instance==USART3)
  {
    /* Peripheral clock disable */
    __HAL_RCC_USART3_CLK_DISABLE();

    /**USART3 GPIO Configuration
    PB10     ------> USART3_TX
    PB11     ------> USART3_RX
    */
    HAL_GPIO_DeInit(GPIOB, GPIO_PIN_10|GPIO_PIN_11);

    /* USART3 interrupt Deinit */
    HAL_NVIC_DisableIRQ(USART3_IRQn);
  }
  /* USER CODE END USART3_MspDeInit */
  else if(uartHandle->Instance==USART2)
  {
    /* Peripheral clock disable */
    __HAL_RCC_USART2_CLK_DISABLE();

    /**USART2 GPIO Configuration
    PA2     ------> USART2_TX
    PA3     ------> USART2_RX
    */
    HAL_GPIO_DeInit(GPIOA, GPIO_PIN_2|GPIO_PIN_3);

    /* USART2 interrupt Deinit */
    HAL_NVIC_DisableIRQ(USART2_IRQn);
  }
}

/* USER CODE BEGIN 1 */

/**
  * @brief ESP32通信初始化
  */
void ESP32_Init(void)
{
    // 清空缓冲区
    memset(esp32_tx_buffer, 0, ESP32_TX_BUFFER_SIZE);
    memset(esp32_rx_buffer, 0, ESP32_RX_BUFFER_SIZE);
    esp32_rx_index = 0;
    esp32_comm_state = ESP32_COMM_IDLE;
    esp32_data_ready = false;
}

/**
  * @brief 发送JSON数据到ESP32
  */
bool ESP32_SendJSON(const char* json_string)
{
    if (json_string == NULL || strlen(json_string) == 0) {
        return false;
    }
    
    esp32_comm_state = ESP32_COMM_SENDING;
    
    // 发送JSON字符串
    HAL_StatusTypeDef status = HAL_UART_Transmit(&huart3, (uint8_t*)json_string, strlen(json_string), 1000);
    
    // 发送换行符作为结束标志
    HAL_UART_Transmit(&huart3, (uint8_t*)"\r\n", 2, 100);
    
    esp32_comm_state = ESP32_COMM_IDLE;
    
    if (status == HAL_OK) {
        printf("发送JSON到ESP32成功: %s\r\n", json_string);
        return true;
    } else {
        printf("发送JSON到ESP32失败\r\n");
        esp32_comm_state = ESP32_COMM_ERROR;
        return false;
    }
}

/**
  * @brief 接收ESP32的JSON数据
  */
bool ESP32_ReceiveJSON(char* json_buffer, uint16_t buffer_size)
{
    if (!esp32_data_ready || json_buffer == NULL) {
        return false;
    }
    
    // 复制接收到的数据
    strncpy(json_buffer, esp32_rx_buffer, buffer_size - 1);
    json_buffer[buffer_size - 1] = '\0';
    
    // 清空接收缓冲区
    memset(esp32_rx_buffer, 0, ESP32_RX_BUFFER_SIZE);
    esp32_rx_index = 0;
    esp32_data_ready = false;
    
    printf("从ESP32接收JSON: %s\r\n", json_buffer);
    return true;
}

/**
  * @brief 处理从ESP32接收到的数据
  */
void ESP32_ProcessReceivedData(void)
{
    char json_buffer[ESP32_RX_BUFFER_SIZE];
    
    if (ESP32_ReceiveJSON(json_buffer, sizeof(json_buffer))) {
        // 简单的JSON解析（实际项目可使用cJSON等库）
        printf("处理ESP32数据: %s\r\n", json_buffer);
        
        // 查找控制命令
        if (strstr(json_buffer, "breathing_led")) {
            if (strstr(json_buffer, "\"on\"") || strstr(json_buffer, "true")) {
                *(uint8_t*)&g_control_state.breathing_led_enabled = 1;
                printf("ESP32命令: 开启呼吸灯\r\n");
            } else if (strstr(json_buffer, "\"off\"") || strstr(json_buffer, "false")) {
                *(uint8_t*)&g_control_state.breathing_led_enabled = 0;
                printf("ESP32命令: 关闭呼吸灯\r\n");
            }
        }
        
        if (strstr(json_buffer, "rgb_led")) {
            if (strstr(json_buffer, "\"on\"") || strstr(json_buffer, "true")) {
                *(uint8_t*)&g_control_state.rgb_led_enabled = 1;
                printf("ESP32命令: 开启RGB灯\r\n");
            } else if (strstr(json_buffer, "\"off\"") || strstr(json_buffer, "false")) {
                *(uint8_t*)&g_control_state.rgb_led_enabled = 0;
                printf("ESP32命令: 关闭RGB灯\r\n");
            }
        }
    }
}

/**
  * @brief 构建传感器数据JSON
  */
void ESP32_BuildSensorJSON(char* json_buffer, uint16_t buffer_size)
{
    snprintf(json_buffer, buffer_size,
        "{"
        "\"device\":\"STM32_Sensor\","
        "\"timestamp\":%lu,"
        "\"sensors\":{"
            "\"light_voltage\":%.2f,"
            "\"vibration\":%s,"
            "\"touch\":%s,"
            "\"temperature\":%.1f,"
            "\"humidity\":%.1f,"
            "\"dht11_valid\":%s"
        "},"
        "\"controls\":{"
            "\"breathing_led\":%s,"
            "\"rgb_led\":%s"
        "}"
        "}",
        HAL_GetTick(),
        g_sensor_data.adc_voltage,
        g_sensor_data.vibration_detected ? "true" : "false",
        g_sensor_data.touch_detected ? "true" : "false",
        g_sensor_data.temperature,
        g_sensor_data.humidity,
        g_sensor_data.dht11_valid ? "true" : "false",
        g_control_state.breathing_led_enabled ? "true" : "false",
        g_control_state.rgb_led_enabled ? "true" : "false"
    );
}

/**
  * @brief 构建控制状态JSON
  */
void ESP32_BuildControlJSON(char* json_buffer, uint16_t buffer_size)
{
    snprintf(json_buffer, buffer_size,
        "{"
        "\"device\":\"STM32_Control\","
        "\"timestamp\":%lu,"
        "\"controls\":{"
            "\"breathing_led\":%s,"
            "\"rgb_led\":%s"
        "}"
        "}",
        HAL_GetTick(),
        g_control_state.breathing_led_enabled ? "true" : "false",
        g_control_state.rgb_led_enabled ? "true" : "false"
    );
}

/**
  * @brief 构建播放状态JSON
  */
void ESP32_BuildPlayStatusJSON(char* json_buffer, uint16_t buffer_size, const char* play_status, const char* content_type)
{
    snprintf(json_buffer, buffer_size,
        "{"
        "\"device\":\"STM32_PlayControl\","
        "\"timestamp\":%lu,"
        "\"playback\":{"
            "\"status\":\"%s\","
            "\"content_type\":\"%s\","
            "\"command_source\":\"USART2\""
        "},"
        "\"controls\":{"
            "\"breathing_led\":%s,"
            "\"rgb_led\":%s"
        "}"
        "}",
        HAL_GetTick(),
        play_status,
        content_type,
        g_control_state.breathing_led_enabled ? "true" : "false",
        g_control_state.rgb_led_enabled ? "true" : "false"
    );
}

/**
  * @brief USART2通信初始化
  */
void USART2_Init(void)
{
    // 清空接收变量
    usart2_rx_byte = 0;
    usart2_cmd_ready = false;
}

/**
  * @brief 获取USART2接收到的16进制命令
  */
bool USART2_GetReceivedData(char* buffer, uint16_t buffer_size)
{
    if (!usart2_cmd_ready || buffer == NULL) {
        return false;
    }
    
    // 将16进制数据转换为字符串格式
    snprintf(buffer, buffer_size, "0x%02X", usart2_rx_byte);
    
    // 清空接收标志
    usart2_cmd_ready = false;
    
    printf("USART2接收16进制命令: 0x%02X (%s)\r\n", usart2_rx_byte, USART2_GetCommandName(usart2_rx_byte));
    return true;
}

/**
  * @brief 处理USART2 16进制命令
  */
void USART2_ProcessHexCommand(uint8_t cmd)
{
    printf("处理命令: 0x%02X - %s\r\n", cmd, USART2_GetCommandName(cmd));
    
    switch(cmd) {
        case CMD_WAKEUP_UNI:
            printf("执行: 你好小盛\r\n");
            break;
            
        case CMD_TURN_ON:
            printf("执行: 打开夜灯\r\n");
            *(uint8_t*)&g_control_state.breathing_led_enabled = 1;
            *(uint8_t*)&g_control_state.rgb_led_enabled = 1;
            break;
            
        case CMD_TURN_OFF:
            printf("执行: 关闭夜灯\r\n");
            *(uint8_t*)&g_control_state.breathing_led_enabled = 0;
            *(uint8_t*)&g_control_state.rgb_led_enabled = 0;
            break;
        case CMD_CHAT_ON:
            printf("执行: 开启聊天\r\n");
            break;
            
        case CMD_CHAT_OFF:
            printf("执行: 关闭聊天\r\n");
            break;
            
        case CMD_PLAY:
            printf("执行: 播放胎教\r\n");
            char json_buffer1[256];
            ESP32_BuildPlayStatusJSON(json_buffer1, sizeof(json_buffer1), "play", "prenatal_education");
            ESP32_SendJSON(json_buffer1);
            break;
            
        case CMD_PLAY_BG:
            printf("执行: 播放白噪音\r\n");
            char json_buffer2[256];
            ESP32_BuildPlayStatusJSON(json_buffer2, sizeof(json_buffer2), "play", "white_noise");
            ESP32_SendJSON(json_buffer2);
            break;
            
        case CMD_PAUSE:
            printf("执行: 暂停播放\r\n");
            char json_buffer3[256];
            ESP32_BuildPlayStatusJSON(json_buffer3, sizeof(json_buffer3), "pause", "unknown");
            ESP32_SendJSON(json_buffer3);
            break;
            
        case CMD_STOP:
            printf("执行: 停止播放\r\n");
            char json_buffer4[256];
            ESP32_BuildPlayStatusJSON(json_buffer4, sizeof(json_buffer4), "stop", "unknown");
            ESP32_SendJSON(json_buffer4);
            break;
            
        default:
            printf("未知命令: 0x%02X\r\n", cmd);
            break;
    }
}

/**
  * @brief 获取命令名称
  */
const char* USART2_GetCommandName(uint8_t cmd)
{
    switch(cmd) {
        case CMD_WAKEUP_UNI: return "你好小盛";
        case CMD_TURN_ON:    return "打开夜灯";
        case CMD_TURN_OFF:   return "关闭夜灯";
        case CMD_CHAT_ON:    return "开启聊天";
        case CMD_CHAT_OFF:   return "关闭聊天";
        case CMD_PLAY:       return "播放胎教";
        case CMD_PLAY_BG:    return "播放白噪音";
        case CMD_PAUSE:      return "暂停播放";
        case CMD_STOP:       return "停止播放";
        default:             return "未知命令";
    }
}

/**
  * @brief UART接收完成中断回调函数
  */
void HAL_UART_RxCpltCallback(UART_HandleTypeDef *huart)
{
    if (huart->Instance == USART2) {
        // USART2 16进制命令接收处理
        printf("USART2接收到16进制: 0x%02X\r\n", usart2_rx_byte);
        
        // 标记命令就绪
        usart2_cmd_ready = true;
        
        // 立即处理命令
        USART2_ProcessHexCommand(usart2_rx_byte);
        
        // 重新启动接收下一个字节
        HAL_UART_Receive_IT(&huart2, &usart2_rx_byte, 1);
    }
    else if (huart->Instance == USART3) {
        // ESP32数据接收处理
        char received_char = esp32_rx_buffer[esp32_rx_index];
        
        if (received_char == '\n' || received_char == '\r') {
            // 接收完成
            esp32_rx_buffer[esp32_rx_index] = '\0';
            esp32_data_ready = true;
            esp32_comm_state = ESP32_COMM_IDLE;
        } else {
            // 继续接收
            esp32_rx_index++;
            if (esp32_rx_index >= ESP32_RX_BUFFER_SIZE - 1) {
                // 缓冲区溢出，重置
                esp32_rx_index = 0;
                memset(esp32_rx_buffer, 0, ESP32_RX_BUFFER_SIZE);
            }
        }
        
        // 重新启动接收
        HAL_UART_Receive_IT(&huart3, (uint8_t*)&esp32_rx_buffer[esp32_rx_index], 1);
    }
}

/**
  * @brief UART错误回调函数
  */
void HAL_UART_ErrorCallback(UART_HandleTypeDef *huart)
{
    if (huart->Instance == USART2) {
        //printf("USART2通信错误\r\n");
        
        // 重置接收
        usart2_rx_byte = 0;
        usart2_cmd_ready = false;
        HAL_UART_Receive_IT(&huart2, &usart2_rx_byte, 1);
    }
    else if (huart->Instance == USART3) {
        esp32_comm_state = ESP32_COMM_ERROR;
        // printf("ESP32通信错误\r\n");
        
        // 重置接收
        esp32_rx_index = 0;
        memset(esp32_rx_buffer, 0, ESP32_RX_BUFFER_SIZE);
        HAL_UART_Receive_IT(&huart3, (uint8_t*)&esp32_rx_buffer[0], 1);
    }
}

/* USER CODE END 1 */
