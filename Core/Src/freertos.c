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
#include "rgb_led.h"
#include "tim.h"
#include <stdio.h>
#include <math.h>
/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */

/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */

/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/
/* USER CODE BEGIN Variables */

/* USER CODE END Variables */
osThreadId defaultTaskHandle;
osThreadId rgbLedTaskHandle;
osThreadId breathingLedTaskHandle;

/* Private function prototypes -----------------------------------------------*/
/* USER CODE BEGIN FunctionPrototypes */

/* USER CODE END FunctionPrototypes */

void StartDefaultTask(void const * argument);
void StartRgbLedTask(void const * argument);
void StartBreathingLedTask(void const * argument);

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
  /* USER CODE BEGIN Init */

  /* USER CODE END Init */

  /* USER CODE BEGIN RTOS_MUTEX */
  /* add mutexes, ... */
  /* USER CODE END RTOS_MUTEX */

  /* USER CODE BEGIN RTOS_SEMAPHORES */
  /* add semaphores, ... */
  /* USER CODE END RTOS_SEMAPHORES */

  /* USER CODE BEGIN RTOS_TIMERS */
  /* start timers, add new ones, ... */
  /* USER CODE END RTOS_TIMERS */

  /* USER CODE BEGIN RTOS_QUEUES */
  /* add queues, ... */
  /* USER CODE END RTOS_QUEUES */

  /* Create the thread(s) */
  /* definition and creation of defaultTask */
  osThreadDef(defaultTask, StartDefaultTask, osPriorityNormal, 0, 128);
  defaultTaskHandle = osThreadCreate(osThread(defaultTask), NULL);

  /* USER CODE BEGIN RTOS_THREADS */
  /* Create RGB LED control task */
  osThreadDef(rgbLedTask, StartRgbLedTask, osPriorityLow, 0, 256);
  rgbLedTaskHandle = osThreadCreate(osThread(rgbLedTask), NULL);
  
  /* Create Breathing LED task */
  osThreadDef(breathingLedTask, StartBreathingLedTask, osPriorityNormal, 0, 256);
  breathingLedTaskHandle = osThreadCreate(osThread(breathingLedTask), NULL);
  /* USER CODE END RTOS_THREADS */

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
  * @brief RGB LED control task
  * @param argument: Task argument
  * @retval None
  */
void StartRgbLedTask(void const * argument)
{
  printf("RGB LED task started\r\n");
  
  // Initialize RGB LED
  RGB_LED_Init();
  
  // Wait 2 seconds for system stability
  osDelay(2000);
  
  // RGB LED mode cycle
  RGB_Mode_t modes[] = {
    RGB_MODE_STATIC_RED,
    RGB_MODE_STATIC_GREEN,
    RGB_MODE_STATIC_BLUE,
    RGB_MODE_BLINK_RED,
    RGB_MODE_BLINK_GREEN,
    RGB_MODE_BLINK_BLUE,
    RGB_MODE_BREATHING,
    RGB_MODE_RAINBOW,
    RGB_MODE_ALTERNATE
  };
  
  const char* mode_names[] = {
    "Static Red",
    "Static Green",
    "Static Blue",
    "Blinking Red",
    "Blinking Green",
    "Blinking Blue",
    "Breathing",
    "Rainbow",
    "Alternating"
  };
  
  uint8_t mode_index = 0;
  uint32_t last_switch_time = osKernelSysTick();
  
  for(;;)
  {
    // Switch mode every 5 seconds
    if((osKernelSysTick() - last_switch_time) >= 5000)
    {
      printf("Switching RGB mode: %s\r\n", mode_names[mode_index]);
      RGB_LED_SetMode(modes[mode_index]);
      
      // Set corresponding color for different modes
      switch(modes[mode_index])
      {
        case RGB_MODE_STATIC_RED:
        case RGB_MODE_BLINK_RED:
          RGB_LED_SetColor(255, 0, 0);
          RGB_LED_SetBrightness(80);
          break;
          
        case RGB_MODE_STATIC_GREEN:
        case RGB_MODE_BLINK_GREEN:
          RGB_LED_SetColor(0, 255, 0);
          RGB_LED_SetBrightness(60);
          break;
          
        case RGB_MODE_STATIC_BLUE:
        case RGB_MODE_BLINK_BLUE:
          RGB_LED_SetColor(0, 0, 255);
          RGB_LED_SetBrightness(70);
          break;
          
        case RGB_MODE_BREATHING:
          RGB_LED_SetColor(255, 255, 255);
          break;
          
        case RGB_MODE_RAINBOW:
          RGB_LED_SetBrightness(75);
          break;
          
        case RGB_MODE_ALTERNATE:
          RGB_LED_SetColor(255, 100, 0);  // Orange
          RGB_LED_SetBrightness(90);
          break;
          
        default:
          break;
      }
      
      mode_index = (mode_index + 1) % (sizeof(modes)/sizeof(modes[0]));
      last_switch_time = osKernelSysTick();
    }
    
    osDelay(100);  // Delay 100ms
  }
}

#define M_PI 3.14159265358979323846f  // 单精度浮点版本
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
  const float breath_speed = 0.08f;  // 呼吸速度
  const uint32_t max_brightness = 800; // 最大亮度 (0-999)
  const uint32_t min_brightness = 10;  // 最小亮度
  
  for(;;)
  {
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
    
    // 延时控制呼吸频率
    osDelay(50);  // 50ms延时，可调节呼吸速度
  }
}

/* USER CODE END Application */
