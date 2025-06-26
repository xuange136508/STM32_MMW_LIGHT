/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * File Name          : freertos.c
  * Description        : Code for freertos applications
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
#include "FreeRTOS.h"
#include "task.h"
#include "main.h"
#include "cmsis_os.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include "tim.h"
#include <stdio.h>
#include <math.h>
#include "ws2812b.h"
#include "dht11.h"
#include "adc.h"
#include "lcd.h"
#include "lcd_init.h"
#include "CST816.h"
#include <string.h>
#include "lvgl.h"


// 全局传感器数据结构
typedef struct {
    float adc_voltage;
    uint8_t vibration_detected;
    uint8_t touch_detected;
    float humidity;
    float temperature;
    uint8_t dht11_valid;
} SensorData_t;

// 按钮控制状态
typedef struct {
    uint8_t breathing_led_enabled;
    uint8_t rgb_led_enabled;
} ControlState_t;

// 全局变量
volatile SensorData_t g_sensor_data = {0};
volatile ControlState_t g_control_state = {1, 1}; // 默认都开启

// 按钮区域定义
#define BTN_WIDTH 90
#define BTN_HEIGHT 35
#define BTN1_X 20    // BreathLED按钮
#define BTN1_Y 210
#define BTN2_X 125   // RGB LED按钮
#define BTN2_Y 210


/* USER CODE END Variables */
osThreadId defaultTaskHandle;
osThreadId rgbLedTaskHandle;
osThreadId breathingLedTaskHandle;
osThreadId sensorTaskHandle;
osThreadId dht11TaskHandle;
osThreadId lcdDisplayTaskHandle;
osThreadId lvglTaskHandle;

/* Private function prototypes -----------------------------------------------*/
/* USER CODE BEGIN FunctionPrototypes */

/* USER CODE END FunctionPrototypes */

void StartDefaultTask(void const * argument);
void StartRgbLedTask(void const * argument);
void StartBreathingLedTask(void const * argument);
void StartSensorTask(void const * argument);
void StartDHT11Task(void const * argument);
void StartLcdDisplayTask(void const * argument);
void StartLvglTask(void const * argument);

// LCD显示功能函数声明
void LCD_DrawUI(void);
void LCD_DrawButton(uint16_t x, uint16_t y, uint16_t width, uint16_t height, const char* text, uint8_t enabled);
void LCD_UpdateSensorData(void);
void LCD_HandleTouch(void);
void LCD_UpdateButtons(void);

// LVGL相关函数声明
void LVGL_CreateInterface(void);
static void btn_event_cb(lv_event_t * e);

void MX_FREERTOS_Init(void); /* (MISRA C 2004 rule 8.1) */

/* GetIdleTaskMemory prototype (linked to static allocation support) */
void vApplicationGetIdleTaskMemory( StaticTask_t **ppxIdleTaskTCBBuffer, StackType_t **ppxIdleTaskStackBuffer, uint32_t *pulIdleTaskStackSize );

/* USER CODE BEGIN GET_IDLE_TASK_MEMORY */
static StaticTask_t xIdleTaskTCBBuffer;
static StackType_t xIdleStack[configMINIMAL_STACK_SIZE];

void vApplicationGetIdleTaskMemory( StaticTask_t **ppxIdleTaskTCBBuffer, StackType_t **ppxIdleTaskStackBuffer, uint32_t *pulIdleTaskStackSize )
{
  *ppxIdleTaskTCBBuffer = &xIdleTaskTCBBuffer;
  *ppxIdleTaskStackBuffer = &xIdleStack[0];
  *pulIdleTaskStackSize = configMINIMAL_STACK_SIZE;
  /* place for user code */
}
/* USER CODE END GET_IDLE_TASK_MEMORY */

/**
  * @brief  FreeRTOS initialization
  * @param  None
  * @retval None
  */
void MX_FREERTOS_Init(void) {
  /* Create the thread(s) */
  /* definition and creation of defaultTask */
  osThreadDef(defaultTask, StartDefaultTask, osPriorityNormal, 0, 128);
  defaultTaskHandle = osThreadCreate(osThread(defaultTask), NULL);

  /* Create RGB LED control task */
  osThreadDef(rgbLedTask, StartRgbLedTask, osPriorityLow, 0, 256);
  rgbLedTaskHandle = osThreadCreate(osThread(rgbLedTask), NULL);
  
  /* Create Breathing LED task */
  osThreadDef(breathingLedTask, StartBreathingLedTask, osPriorityNormal, 0, 256);
  breathingLedTaskHandle = osThreadCreate(osThread(breathingLedTask), NULL);
  
  /* Create Sensor monitoring task */
  osThreadDef(sensorTask, StartSensorTask, osPriorityNormal, 0, 256);
  sensorTaskHandle = osThreadCreate(osThread(sensorTask), NULL);
  
  /* Create DHT11 temperature humidity task */
  osThreadDef(dht11Task, StartDHT11Task, osPriorityLow, 0, 512);
  dht11TaskHandle = osThreadCreate(osThread(dht11Task), NULL);
  
  /* Create LCD Display task */
  // osThreadDef(lcdDisplayTask, StartLcdDisplayTask, osPriorityNormal, 0, 512);
  // lcdDisplayTaskHandle = osThreadCreate(osThread(lcdDisplayTask), NULL);
  
  /* Create LVGL task - 处理GUI更新和按钮事件 */
  osThreadDef(lvglTask, StartLvglTask, osPriorityNormal, 0, 1024);
  lvglTaskHandle = osThreadCreate(osThread(lvglTask), NULL);

}

/* USER CODE BEGIN Header_StartDefaultTask */
/**
  * @brief  Function implementing the defaultTask thread.
  * @param  argument: Not used
  * @retval None
  */
/* USER CODE END Header_StartDefaultTask */
void StartDefaultTask(void const * argument)
{
  /* USER CODE BEGIN StartDefaultTask */
  /* Infinite loop */
  for(;;)
  {
    osDelay(1);
  }
  /* USER CODE END StartDefaultTask */
}

/* Private application code --------------------------------------------------*/
/* USER CODE BEGIN Application */

 /**
  * @brief RGB LED control task (WS2812B)
  * @param argument: Task argument
  * @retval None
  */
void StartRgbLedTask(void const * argument)
{
  printf("WS2812B RGB LED Task started\r\n");
  
  // 初始化WS2812B RGB彩灯
  WS2812B_Init();
  printf("WS2812B 初始化完成\r\n");
  
  // 基础颜色测试
  printf("设置LED颜色: LED0=红色, LED1=绿色\r\n");
  WS2812B_SetColorEnum(0, WS2812B_RED);
  WS2812B_SetColorEnum(1, WS2812B_GREEN);
  WS2812B_Update();
  osDelay(2000);
  
  // 测试多种颜色
  printf("开始颜色循环测试...\r\n");
  WS2812B_ColorEnum_t test_colors[] = {
    WS2812B_RED, WS2812B_GREEN, WS2812B_BLUE, WS2812B_YELLOW,
    WS2812B_MAGENTA, WS2812B_CYAN, WS2812B_WHITE, WS2812B_ORANGE
  };
  
  for(int cycle = 0; cycle < 2; cycle++) {
    for(int i = 0; i < 8; i++) {
      WS2812B_SetColorEnum(0, test_colors[i]);
      WS2812B_SetColorEnum(1, test_colors[(i+1)%8]);
      WS2812B_Update();
      printf("颜色 %d: LED0=%d, LED1=%d\r\n", i, test_colors[i], test_colors[(i+1)%8]);
      osDelay(500);
    }
  }
  
  // 彩虹效果测试
  printf("彩虹效果测试...\r\n");
  WS2812B_Test_Rainbow();
  
  // 呼吸灯效果测试
  printf("呼吸灯效果测试...\r\n");
  WS2812B_Test_Breathing();
  
  printf("WS2812B 初始测试完成!\r\n");
  
  // 无限循环 - 持续的RGB效果
  for(;;)
  {
    if(g_control_state.rgb_led_enabled) {
      // 循环彩虹效果
      WS2812B_Test_Rainbow();
      osDelay(1000);
      
      // 循环呼吸灯效果
      WS2812B_Test_Breathing();
      osDelay(1000);
      
      // 简单颜色切换
      for(int i = 0; i < 8; i++) {
        if(!g_control_state.rgb_led_enabled) break; // 检查是否被关闭
        WS2812B_SetColorEnum(0, test_colors[i]);
        WS2812B_SetColorEnum(1, test_colors[(i+1)%8]);
        WS2812B_Update();
        osDelay(800);
      }
    } else {
      // 关闭RGB LED
      WS2812B_Clear();
    }
    
    osDelay(1000);
  }
}

#define M_PI 3.14f  // 单精度浮点版本
/**
  * @brief PD12 PWM呼吸灯任务
  * @param argument: 任务参数
  * @retval None
  */
void StartBreathingLedTask(void const * argument)
{
  printf("PWM Breathing LED task started\r\n");
  
  // 启动PWM
  HAL_TIM_PWM_Start(&htim4, TIM_CHANNEL_1);
  
  // 呼吸灯参数
  float breath_phase = 0.0f;
  const float breath_speed = 0.12f;  // 呼吸速度
  const uint32_t max_brightness = 50; // 最大亮度 (0-999)
  const uint32_t min_brightness = 10;  // 最小亮度
  
  for(;;)
  {
    if(g_control_state.breathing_led_enabled) {
      // 使用正弦波生成呼吸效果
      float sine_value = (sin(breath_phase) + 1.0f) / 2.0f; // 0-1 范围
      uint32_t pwm_value = min_brightness + (uint32_t)(sine_value * (max_brightness - min_brightness));
      
      // 设置PWM占空比
      __HAL_TIM_SET_COMPARE(&htim4, TIM_CHANNEL_1, pwm_value);
      
      // 更新相位
      breath_phase += breath_speed;
      if (breath_phase >= 2.0f * M_PI) {
        breath_phase = 0.0f;
      }
    } else {
      // 关闭呼吸灯
      __HAL_TIM_SET_COMPARE(&htim4, TIM_CHANNEL_1, 0);
    }
    
    // 延时控制呼吸频率
    osDelay(50);  // 50ms延时，可调节呼吸速度
  }
}

/**
  * @brief 传感器监测任务 (ADC, 振动, 触摸) - 不包含DHT11
  * @param argument: 任务参数
  * @retval None
  */
void StartSensorTask(void const * argument)
{
  printf("传感器监测任务启动 (ADC/振动/触摸)\r\n");
  
  // 初始测试循环
  // printf("开始传感器初始化测试...\r\n");
  // for(int i = 0; i < 3; i++) {
  //     printf("=== 第 %d 次传感器测试 ===\r\n", i+1);
      
  //     // ADC测试
  //     uint16_t adc_value = Get_ADC_Value();
  //     float voltage = (adc_value * 3.3f) / 4095.0f;
  //     printf("  ADC: %hu (%.2fV)\r\n", adc_value, voltage);

  //     // 振动检测
  //     GPIO_PinState vibration_state = HAL_GPIO_ReadPin(GPIOA, GPIO_PIN_8);
  //     printf("  振动: %s\r\n", (vibration_state == GPIO_PIN_SET) ? "触发" : "未触发");

  //     // 触摸感应检测
  //     GPIO_PinState touch_state = HAL_GPIO_ReadPin(GPIOB, GPIO_PIN_1);
  //     printf("  触摸: %s\r\n", (touch_state == GPIO_PIN_SET) ? "触发" : "未触发");
      
  //     osDelay(1000); 
  // }
  
  // printf("传感器初始化测试完成，开始持续监测...\r\n");
  
  // 持续监测循环 - 每5秒读取一次
  for(;;)
  {
    // 读取ADC
    uint16_t adc_value = Get_ADC_Value();
    float voltage = (adc_value * 3.3f) / 4095.0f;
    
    // 检测振动和触摸
    GPIO_PinState vibration = HAL_GPIO_ReadPin(GPIOA, GPIO_PIN_8);
    GPIO_PinState touch = HAL_GPIO_ReadPin(GPIOB, GPIO_PIN_1);
    
    // 更新全局传感器数据
    g_sensor_data.adc_voltage = voltage;
    g_sensor_data.vibration_detected = (vibration == GPIO_PIN_SET) ? 1 : 0;
    g_sensor_data.touch_detected = (touch == GPIO_PIN_SET) ? 1 : 0;
    
    printf("传感器状态 - ADC: %.2fV  振动: %s  触摸: %s\r\n", 
           voltage,
           (vibration == GPIO_PIN_SET) ? "是" : "否",
           (touch == GPIO_PIN_SET) ? "是" : "否");
    
    // 延时5秒
    osDelay(5000);
  }
}

/**
  * @brief DHT11温湿度传感器独立任务
  * @param argument: 任务参数
  * @retval None
  */
void StartDHT11Task(void const * argument)
{
  // 启用DWT循环计数器（DHT11专用，用于微秒延时）
  CoreDebug->DEMCR |= CoreDebug_DEMCR_TRCENA_Msk;
  DWT->CYCCNT = 0;
  DWT->CTRL |= DWT_CTRL_CYCCNTENA_Msk;
  
  DHT11_Data_t dht11_data;
  uint8_t dht11_success_count = 0;
  
  // printf("DHT11: 开始初始化测试...\r\n");
  // for(int i = 0; i < 5; i++) {
  //     printf("DHT11: === 第 %d 次读取 ===\r\n", i+1);
      
  //     uint8_t read_result = DHT11_ReadData(&dht11_data);
  //             if(read_result) {
  //           dht11_success_count++;
  //           printf("  DHT11读取成功: 湿度=%d.%d%% 温度=%d.%d°C\r\n", 
  //                  dht11_data.humidity_int, dht11_data.humidity_dec,
  //                  dht11_data.temperature_int, dht11_data.temperature_dec);
            
  //           // 更新全局数据
  //           g_sensor_data.humidity = dht11_data.humidity_int + dht11_data.humidity_dec / 10.0f;
  //           g_sensor_data.temperature = dht11_data.temperature_int + dht11_data.temperature_dec / 10.0f;
  //           g_sensor_data.dht11_valid = 1;
            
  //           // 数据合理性检查
  //           if(dht11_data.humidity_int <= 99 && dht11_data.temperature_int < 60) {
  //               printf("  数据合理性检查: 通过\r\n");
  //           } else {
  //               printf("  数据合理性检查: 异常！\r\n");
  //           }
  //       } else {
  //           printf("  DHT11读取失败\r\n");
  //           g_sensor_data.dht11_valid = 0;
  //       }
      
  //     // DHT11需要至少2秒间隔
  //     osDelay(3000); 
  // }
  
  // printf("DHT11初始化完成，成功率: %d/5 (%.1f%%)\r\n", 
  //        dht11_success_count, (float)dht11_success_count/5*100);
         
  // 持续监测循环 - 每3秒读取一次
  for(;;)
  {
    uint8_t read_result = DHT11_ReadData(&dht11_data);
    
    if(read_result) {
      g_sensor_data.humidity = dht11_data.humidity_int + dht11_data.humidity_dec / 10.0f;
      g_sensor_data.temperature = dht11_data.temperature_int + dht11_data.temperature_dec / 10.0f;
      g_sensor_data.dht11_valid = 1;
      
      printf("DHT11更新: 湿度=%.1f%% 温度=%.1f°C\r\n", 
             g_sensor_data.humidity, g_sensor_data.temperature);
    } else {
      g_sensor_data.dht11_valid = 0;
      printf("DHT11读取失败\r\n");
    }
    // 延时3秒
    osDelay(3000);
  }
}

/**
  * @brief LCD显示任务 - 显示传感器状态和按钮控制
  * @param argument: 任务参数
  * @retval None
  */
void StartLcdDisplayTask(void const * argument)
{
  printf("LCD显示任务启动\r\n");
  
  // 等待系统初始化完成
  osDelay(2000);
  
  // 绘制初始UI
  LCD_DrawUI();
  
  // 立即显示一次传感器数据
  LCD_UpdateSensorData();
  
  static uint32_t last_sensor_update = 0;
  last_sensor_update = HAL_GetTick(); // 设置初始时间
  
  for(;;)
  {
    uint32_t current_time = HAL_GetTick();
    
    // 每3秒更新一次传感器数据显示
    if(current_time - last_sensor_update >= 1000) {
      LCD_UpdateSensorData();
      last_sensor_update = current_time;
    }
    
    // 处理触摸输入 (保持快速响应)
    LCD_HandleTouch();
    
    // 刷新按钮状态
    LCD_UpdateButtons();
    
    // 每100ms检查一次触摸，保持按钮响应速度
    osDelay(100);
  }
}

/**
  * @brief 绘制LCD用户界面
  */
void LCD_DrawUI(void)
{
  // 清屏
  LCD_Fill(0, 0, SCREEN_WIDTH-1, SCREEN_HEIGHT-1, BLACK);
  
  // 标题
  LCD_ShowChinese(30, 5, (uint8_t*)"妈妈网传感器监控系统", WHITE, BLACK, 16, 0);
  
  // 传感器状态区域标题
  LCD_ShowChinese(10, 30, (uint8_t*)"【传感器状态】", YELLOW, BLACK, 12, 0);
  
  // 按钮区域标题
  LCD_ShowChinese(10, 190, (uint8_t*)"【控制面板】", YELLOW, BLACK, 12, 0);
  
  // 绘制按钮
  LCD_DrawButton(BTN1_X, BTN1_Y, BTN_WIDTH, BTN_HEIGHT, "呼吸灯", g_control_state.breathing_led_enabled);
  LCD_DrawButton(BTN2_X, BTN2_Y, BTN_WIDTH, BTN_HEIGHT, "彩色灯", g_control_state.rgb_led_enabled);
}

/**
  * @brief 绘制按钮
  */
void LCD_DrawButton(uint16_t x, uint16_t y, uint16_t width, uint16_t height, const char* text, uint8_t enabled)
{
  uint16_t bg_color = enabled ? GREEN : GRAY;
  uint16_t text_color = enabled ? BLACK : WHITE;
  
  // 绘制按钮背景
  LCD_Fill(x, y, x + width - 1, y + height - 1, bg_color);
  
  // 绘制按钮边框
  LCD_DrawRectangle(x, y, x + width - 1, y + height - 1, WHITE);
  
  // 绘制按钮文字 (居中)
  uint8_t text_len = strlen(text);
  uint16_t text_x = x + (width - text_len * 8) / 2;  // 假设字符宽度8像素
  uint16_t text_y = y + (height - 16) / 2;           // 假设字符高度16像素
  
  LCD_ShowChinese(text_x, text_y, (uint8_t*)text, text_color, bg_color, 16, 0);
}

/**
  * @brief 更新传感器数据显示
  */
void LCD_UpdateSensorData(void)
{
  char buffer[50];
  
  // 清除传感器数据显示区域 (保留其他UI元素)
  LCD_Fill(10, 50, SCREEN_WIDTH-10, 180, BLACK);
  
  // 显示光敏传感器
  LCD_ShowChinese(10, 50, (uint8_t*)"光敏：", WHITE, BLACK, 16, 0);
  sprintf(buffer, "%.2fV", g_sensor_data.adc_voltage);
  LCD_ShowString(70, 50, (uint8_t*)buffer, WHITE, BLACK, 16, 0);
  
  // 显示振动传感器
  LCD_ShowChinese(10, 70, (uint8_t*)"振动：", WHITE, BLACK, 16, 0);
  sprintf(buffer, "%s", g_sensor_data.vibration_detected ? "是" : "否");
  LCD_ShowChinese(70, 70, (uint8_t*)buffer, WHITE, BLACK, 16, 0);
  
  // 显示触摸传感器
  LCD_ShowChinese(10, 90, (uint8_t*)"触摸：", WHITE, BLACK, 16, 0);
  sprintf(buffer, "%s", g_sensor_data.touch_detected ? "是" : "否");
  LCD_ShowChinese(70, 90, (uint8_t*)buffer, WHITE, BLACK, 16, 0);
  
  // 显示温湿度数据
  // if(g_sensor_data.dht11_valid) {
    LCD_ShowChinese(10, 110, (uint8_t*)"温度：", CYAN, BLACK, 16, 0);
    sprintf(buffer, "%.1f", g_sensor_data.temperature);
    LCD_ShowString(50, 110, (uint8_t*)buffer, CYAN, BLACK, 16, 0);
    LCD_ShowChinese(90, 115, (uint8_t*)"℃", CYAN, BLACK, 16, 0);
    
    LCD_ShowChinese(130, 110, (uint8_t*)"湿度：", CYAN, BLACK, 16, 0);
    sprintf(buffer, "%.1f%%", g_sensor_data.humidity);
    LCD_ShowString(180, 110, (uint8_t*)buffer, CYAN, BLACK, 16, 0);
  // }
  
  // 显示运行时间
  LCD_ShowChinese(10, 130, (uint8_t*)"运行：", LIGHTGREEN, BLACK, 16, 0);
  sprintf(buffer, "%lu", HAL_GetTick()/1000);
  LCD_ShowString(50, 130, (uint8_t*)buffer, LIGHTGREEN, BLACK, 16, 0);
  LCD_ShowChinese(70, 130, (uint8_t*)(uint8_t*)"秒", LIGHTGREEN, BLACK, 16, 0);
}

/**
  * @brief 处理触摸输入
  */
void LCD_HandleTouch(void)
{
  // 获取触摸数据
  CST816_Get_XY_AXIS();
  
  if(CST816_Get_FingerNum() > 0) {
    uint16_t touch_x = CST816_Instance.X_Pos;
    uint16_t touch_y = CST816_Instance.Y_Pos;
    
    // 检查是否点击了呼吸灯按钮
    if(touch_x >= BTN1_X && touch_x <= BTN1_X + BTN_WIDTH &&
       touch_y >= BTN1_Y && touch_y <= BTN1_Y + BTN_HEIGHT) {
      // 切换呼吸灯状态
      g_control_state.breathing_led_enabled = !g_control_state.breathing_led_enabled;
      printf("Breathing LED state: %s\r\n", g_control_state.breathing_led_enabled ? "ON" : "OFF");
      
      // 重绘按钮
      LCD_DrawButton(BTN1_X, BTN1_Y, BTN_WIDTH, BTN_HEIGHT, "呼吸灯", g_control_state.breathing_led_enabled);
      
      // 延时防抖
      osDelay(300);
    }
    
    // 检查是否点击了RGB彩灯按钮
    if(touch_x >= BTN2_X && touch_x <= BTN2_X + BTN_WIDTH &&
       touch_y >= BTN2_Y && touch_y <= BTN2_Y + BTN_HEIGHT) {
      // 切换RGB LED状态
      g_control_state.rgb_led_enabled = !g_control_state.rgb_led_enabled;
      printf("RGB LED state: %s\r\n", g_control_state.rgb_led_enabled ? "ON" : "OFF");
      
      // 重绘按钮
      LCD_DrawButton(BTN2_X, BTN2_Y, BTN_WIDTH, BTN_HEIGHT, "彩色灯", g_control_state.rgb_led_enabled);
      
      // 延时防抖
      osDelay(300);
    }
  }
}

/**
  * @brief 更新按钮状态显示
  */
void LCD_UpdateButtons(void)
{
  static uint8_t last_breathing_state = 255; // 初始化为无效值
  static uint8_t last_rgb_state = 255;
  
  // 只有状态改变时才重绘按钮，避免频繁刷新
  if(last_breathing_state != g_control_state.breathing_led_enabled) {
    LCD_DrawButton(BTN1_X, BTN1_Y, BTN_WIDTH, BTN_HEIGHT, "呼吸灯", g_control_state.breathing_led_enabled);
    last_breathing_state = g_control_state.breathing_led_enabled;
  }
  
  if(last_rgb_state != g_control_state.rgb_led_enabled) {
    LCD_DrawButton(BTN2_X, BTN2_Y, BTN_WIDTH, BTN_HEIGHT, "彩色灯", g_control_state.rgb_led_enabled);
    last_rgb_state = g_control_state.rgb_led_enabled;
  }
}

/**
  * @brief LVGL任务 - 处理GUI更新和按钮事件
  * @param argument: 任务参数
  * @retval None
  */
void StartLvglTask(void const * argument)
{
  printf("LVGL任务启动 - 支持按钮事件处理\r\n");
  
  // 等待LVGL驱动初始化完成
  osDelay(2000);
  
  printf("开始创建LVGL界面...\r\n");
  
  // 创建LVGL界面
  LVGL_CreateInterface();
  
  printf("LVGL界面创建完成，开始主循环\r\n");
  
  // LVGL主循环
  for(;;)
  {
    // 更新LVGL tick计数
    lv_tick_inc(5);
    
    // 处理LVGL任务 - 这是LVGL的核心处理函数，包括事件处理
    lv_timer_handler();
    
    // 每5ms处理一次LVGL任务，保证界面流畅和事件响应
    osDelay(5);
  }
}

/**
  * @brief 创建LVGL界面
  * @retval None
  */
void LVGL_CreateInterface(void)
{
  printf("正在创建LVGL界面组件...\r\n");
  
  // 立即尝试刷新屏幕看看显示驱动是否被调用
  lv_disp_load_scr(lv_scr_act());
  
  // 先测试LVGL基础显示 - 简单的屏幕填充
  lv_obj_t * scr = lv_scr_act();
  lv_obj_set_style_bg_color(scr, lv_color_hex(0x003a57), LV_PART_MAIN);
  
  // 创建简单测试标签
  lv_obj_t *label = lv_label_create(scr);
  lv_label_set_text(label, "LVGL Test");
  lv_obj_set_style_text_color(label, lv_color_white(), LV_PART_MAIN);
  lv_obj_align(label, LV_ALIGN_CENTER, 0, -50);
  
  // 创建按钮测试
  lv_obj_t *btn = lv_btn_create(scr);
  lv_obj_set_size(btn, 120, 50);
  lv_obj_align(btn, LV_ALIGN_CENTER, 0, 0);
  lv_obj_t *btn_label = lv_label_create(btn);
  lv_label_set_text(btn_label, "Test Button");
  lv_obj_center(btn_label);
  
  // 为按钮添加点击事件处理
  lv_obj_add_event_cb(btn, btn_event_cb, LV_EVENT_CLICKED, NULL);
  
  printf("LVGL界面组件创建完成\r\n");
  
  // 初始渲染几次确保界面显示
  for(int i = 0; i < 10; i++) {
    lv_timer_handler();  
    osDelay(5);
  }
}

/**
  * @brief LVGL按钮点击事件处理函数
  * @param e: 事件参数
  * @retval None
  */
static void btn_event_cb(lv_event_t * e)
{
    lv_event_code_t code = lv_event_get_code(e);
    lv_obj_t * btn = lv_event_get_target(e);
    
    if(code == LV_EVENT_CLICKED) {
        printf("===== LVGL按钮被点击了! =====\r\n");
        printf("事件类型: LV_EVENT_CLICKED\r\n");
        printf("按钮对象地址: 0x%08X\r\n", (uint32_t)btn);
        
        // 获取当前时间戳（可选）
        uint32_t tick = HAL_GetTick();
        printf("时间戳: %lu ms\r\n", tick);
        
        // 可以在这里添加更多的功能
        // 比如改变按钮颜色、文字等
        static uint8_t click_count = 0;
        click_count++;
        printf("按钮点击次数: %d\r\n", click_count);
        
        // 更新按钮文字显示点击次数
        char btn_text[32];
        snprintf(btn_text, sizeof(btn_text), "Clicked %d", click_count);
        lv_obj_t * btn_label = lv_obj_get_child(btn, 0);
        if(btn_label != NULL) {
            lv_label_set_text(btn_label, btn_text);
        }
        printf("=============================\r\n");
    }
}



/* USER CODE END Application */
