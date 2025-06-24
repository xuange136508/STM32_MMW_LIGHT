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
/* USER CODE END Includes */


/* USER CODE END Variables */
osThreadId defaultTaskHandle;
osThreadId rgbLedTaskHandle;
osThreadId breathingLedTaskHandle;
osThreadId sensorTaskHandle;

/* Private function prototypes -----------------------------------------------*/
/* USER CODE BEGIN FunctionPrototypes */

/* USER CODE END FunctionPrototypes */

void StartDefaultTask(void const * argument);
void StartRgbLedTask(void const * argument);
void StartBreathingLedTask(void const * argument);
void StartSensorTask(void const * argument);

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
  
  /* Create Sensor monitoring task */
  osThreadDef(sensorTask, StartSensorTask, osPriorityNormal, 0, 512);
  sensorTaskHandle = osThreadCreate(osThread(sensorTask), NULL);
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
  * @brief RGB LED control task (WS2812B)
  * @param argument: Task argument
  * @retval None
  */
void StartRgbLedTask(void const * argument)
{
  printf("WS2812B RGB LED Task started\r\n");
  
  // ��ʼ��WS2812B RGB�ʵ�
  WS2812B_Init();
  printf("WS2812B ��ʼ�����\r\n");
  
  // ������ɫ����
  printf("����LED��ɫ: LED0=��ɫ, LED1=��ɫ\r\n");
  WS2812B_SetColorEnum(0, WS2812B_RED);
  WS2812B_SetColorEnum(1, WS2812B_GREEN);
  WS2812B_Update();
  osDelay(2000);
  
  // ���Զ�����ɫ
  printf("��ʼ��ɫѭ������...\r\n");
  WS2812B_ColorEnum_t test_colors[] = {
    WS2812B_RED, WS2812B_GREEN, WS2812B_BLUE, WS2812B_YELLOW,
    WS2812B_MAGENTA, WS2812B_CYAN, WS2812B_WHITE, WS2812B_ORANGE
  };
  
  for(int cycle = 0; cycle < 2; cycle++) {
    for(int i = 0; i < 8; i++) {
      WS2812B_SetColorEnum(0, test_colors[i]);
      WS2812B_SetColorEnum(1, test_colors[(i+1)%8]);
      WS2812B_Update();
      printf("��ɫ %d: LED0=%d, LED1=%d\r\n", i, test_colors[i], test_colors[(i+1)%8]);
      osDelay(500);
    }
  }
  
  // �ʺ�Ч������
  printf("�ʺ�Ч������...\r\n");
  WS2812B_Test_Rainbow();
  
  // ������Ч������
  printf("������Ч������...\r\n");
  WS2812B_Test_Breathing();
  
  printf("WS2812B ��ʼ�������!\r\n");
  
  // ����ѭ�� - ������RGBЧ��
  for(;;)
  {
    // ѭ���ʺ�Ч��
    WS2812B_Test_Rainbow();
    osDelay(1000);
    
    // ѭ��������Ч��
    WS2812B_Test_Breathing();
    osDelay(1000);
    
    // ����ɫ�л�
    for(int i = 0; i < 8; i++) {
      WS2812B_SetColorEnum(0, test_colors[i]);
      WS2812B_SetColorEnum(1, test_colors[(i+1)%8]);
      WS2812B_Update();
      osDelay(800);
    }
    
    osDelay(1000);
  }
}

#define M_PI 3.14f  // �����ȸ���汾
/**
  * @brief PD12 PWM����������
  * @param argument: �������
  * @retval None
  */
void StartBreathingLedTask(void const * argument)
{
  printf("PWM Breathing LED task started\r\n");
  
  // ����PWM
  HAL_TIM_PWM_Start(&htim4, TIM_CHANNEL_1);
  
  // �����Ʋ���
  float breath_phase = 0.0f;
  const float breath_speed = 0.12f;  // �����ٶ�
  const uint32_t max_brightness = 50; // ������� (0-999)
  const uint32_t min_brightness = 10;  // ��С����
  
  for(;;)
  {
    // ʹ�����Ҳ����ɺ���Ч��
    float sine_value = (sin(breath_phase) + 1.0f) / 2.0f; // 0-1 ��Χ
    uint32_t pwm_value = min_brightness + (uint32_t)(sine_value * (max_brightness - min_brightness));
    
    // ����PWMռ�ձ�
    __HAL_TIM_SET_COMPARE(&htim4, TIM_CHANNEL_1, pwm_value);
    
    // ������λ
    breath_phase += breath_speed;
    if (breath_phase >= 2.0f * M_PI) {
      breath_phase = 0.0f;
    }
    
    // ��ʱ���ƺ���Ƶ��
    osDelay(50);  // 50ms��ʱ���ɵ��ں����ٶ�
  }
}

/**
  * @brief ������������� (DHT11, ADC, ��, ����)
  * @param argument: �������
  * @retval None
  */
void StartSensorTask(void const * argument)
{
  // ����DWTѭ��������������΢����ʱ��
  CoreDebug->DEMCR |= CoreDebug_DEMCR_TRCENA_Msk;
  DWT->CYCCNT = 0;
  DWT->CTRL |= DWT_CTRL_CYCCNTENA_Msk;
  
  DHT11_Data_t dht11_data;
  uint8_t dht11_success_count = 0;
  
  // ��ʼ����ѭ��
  for(int i = 0; i < 5; i++) {
      printf("=== �� %d �ζ�ȡ ===\r\n", i+1);
      
      // ��ȡDHT11���ݣ��������У�����������������ʱ��
      if(DHT11_ReadData(&dht11_data)) {
          dht11_success_count++;
          printf("  DHT11��������ȡ�ɹ�:\r\n");
          printf("  ʪ��: %d.%d%%\r\n", dht11_data.humidity_int, dht11_data.humidity_dec);
          printf("  �¶�: %d.%d��C\r\n", dht11_data.temperature_int, dht11_data.temperature_dec);
          
          // ���ݺ����Լ��
          if(dht11_data.humidity_int <= 99 && dht11_data.temperature_int < 60) {
              printf("  ��ʪ�����ݺ��� \r\n");
          } else {
              printf("  ��ʪ�����ݿ����쳣��\r\n");
          }
      } else {
          printf("  DHT11��ȡʧ��\r\n");
      }
      osDelay(1000);  // ��ʱ1����ٲ�������������
      
      // ADC����
      uint16_t adc_value = Get_ADC_Value();
      float voltage = (adc_value * 3.3f) / 4095.0f;
      printf("  ADC: %hu (%.2fV)\r\n", adc_value, voltage);

      // �񶯼��
      GPIO_PinState state = HAL_GPIO_ReadPin(GPIOA, GPIO_PIN_8);
      printf("  ��: %s\r\n", (state == GPIO_PIN_SET) ? "����" : "δ����");

      // ������Ӧ���
      GPIO_PinState state1 = HAL_GPIO_ReadPin(GPIOB, GPIO_PIN_1);
      printf("  ����: %s\r\n", (state1 == GPIO_PIN_SET) ? "����" : "δ����");
      
      // DHT11��Ҫ����2������ʹ��3�������
      osDelay(2000); 
  }
  printf("DHT11������ɣ��ɹ���: %d/5 (%.1f%%)\r\n", 
         dht11_success_count, (float)dht11_success_count/5*100);
  
}

/* USER CODE END Application */
