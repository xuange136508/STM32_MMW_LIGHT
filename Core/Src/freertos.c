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


// ȫ�ִ��������ݽṹ
typedef struct {
    float adc_voltage;
    uint8_t vibration_detected;
    uint8_t touch_detected;
    float humidity;
    float temperature;
    uint8_t dht11_valid;
} SensorData_t;

// ��ť����״̬
typedef struct {
    uint8_t breathing_led_enabled;
    uint8_t rgb_led_enabled;
} ControlState_t;

// ȫ�ֱ���
volatile SensorData_t g_sensor_data = {0};
volatile ControlState_t g_control_state = {1, 1}; // Ĭ�϶�����

// ��ť������
#define BTN_WIDTH 90
#define BTN_HEIGHT 35
#define BTN1_X 20    // BreathLED��ť
#define BTN1_Y 210
#define BTN2_X 125   // RGB LED��ť
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

// LCD��ʾ���ܺ�������
void LCD_DrawUI(void);
void LCD_DrawButton(uint16_t x, uint16_t y, uint16_t width, uint16_t height, const char* text, uint8_t enabled);
void LCD_UpdateSensorData(void);
void LCD_HandleTouch(void);
void LCD_UpdateButtons(void);

// LVGL��غ�������
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
  
  /* Create LVGL task - ����GUI���ºͰ�ť�¼� */
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
    if(g_control_state.rgb_led_enabled) {
      // ѭ���ʺ�Ч��
      WS2812B_Test_Rainbow();
      osDelay(1000);
      
      // ѭ��������Ч��
      WS2812B_Test_Breathing();
      osDelay(1000);
      
      // ����ɫ�л�
      for(int i = 0; i < 8; i++) {
        if(!g_control_state.rgb_led_enabled) break; // ����Ƿ񱻹ر�
        WS2812B_SetColorEnum(0, test_colors[i]);
        WS2812B_SetColorEnum(1, test_colors[(i+1)%8]);
        WS2812B_Update();
        osDelay(800);
      }
    } else {
      // �ر�RGB LED
      WS2812B_Clear();
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
    if(g_control_state.breathing_led_enabled) {
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
    } else {
      // �رպ�����
      __HAL_TIM_SET_COMPARE(&htim4, TIM_CHANNEL_1, 0);
    }
    
    // ��ʱ���ƺ���Ƶ��
    osDelay(50);  // 50ms��ʱ���ɵ��ں����ٶ�
  }
}

/**
  * @brief ������������� (ADC, ��, ����) - ������DHT11
  * @param argument: �������
  * @retval None
  */
void StartSensorTask(void const * argument)
{
  printf("����������������� (ADC/��/����)\r\n");
  
  // ��ʼ����ѭ��
  // printf("��ʼ��������ʼ������...\r\n");
  // for(int i = 0; i < 3; i++) {
  //     printf("=== �� %d �δ��������� ===\r\n", i+1);
      
  //     // ADC����
  //     uint16_t adc_value = Get_ADC_Value();
  //     float voltage = (adc_value * 3.3f) / 4095.0f;
  //     printf("  ADC: %hu (%.2fV)\r\n", adc_value, voltage);

  //     // �񶯼��
  //     GPIO_PinState vibration_state = HAL_GPIO_ReadPin(GPIOA, GPIO_PIN_8);
  //     printf("  ��: %s\r\n", (vibration_state == GPIO_PIN_SET) ? "����" : "δ����");

  //     // ������Ӧ���
  //     GPIO_PinState touch_state = HAL_GPIO_ReadPin(GPIOB, GPIO_PIN_1);
  //     printf("  ����: %s\r\n", (touch_state == GPIO_PIN_SET) ? "����" : "δ����");
      
  //     osDelay(1000); 
  // }
  
  // printf("��������ʼ��������ɣ���ʼ�������...\r\n");
  
  // �������ѭ�� - ÿ5���ȡһ��
  for(;;)
  {
    // ��ȡADC
    uint16_t adc_value = Get_ADC_Value();
    float voltage = (adc_value * 3.3f) / 4095.0f;
    
    // ����񶯺ʹ���
    GPIO_PinState vibration = HAL_GPIO_ReadPin(GPIOA, GPIO_PIN_8);
    GPIO_PinState touch = HAL_GPIO_ReadPin(GPIOB, GPIO_PIN_1);
    
    // ����ȫ�ִ���������
    g_sensor_data.adc_voltage = voltage;
    g_sensor_data.vibration_detected = (vibration == GPIO_PIN_SET) ? 1 : 0;
    g_sensor_data.touch_detected = (touch == GPIO_PIN_SET) ? 1 : 0;
    
    printf("������״̬ - ADC: %.2fV  ��: %s  ����: %s\r\n", 
           voltage,
           (vibration == GPIO_PIN_SET) ? "��" : "��",
           (touch == GPIO_PIN_SET) ? "��" : "��");
    
    // ��ʱ5��
    osDelay(5000);
  }
}

/**
  * @brief DHT11��ʪ�ȴ�������������
  * @param argument: �������
  * @retval None
  */
void StartDHT11Task(void const * argument)
{
  // ����DWTѭ����������DHT11ר�ã�����΢����ʱ��
  CoreDebug->DEMCR |= CoreDebug_DEMCR_TRCENA_Msk;
  DWT->CYCCNT = 0;
  DWT->CTRL |= DWT_CTRL_CYCCNTENA_Msk;
  
  DHT11_Data_t dht11_data;
  uint8_t dht11_success_count = 0;
  
  // printf("DHT11: ��ʼ��ʼ������...\r\n");
  // for(int i = 0; i < 5; i++) {
  //     printf("DHT11: === �� %d �ζ�ȡ ===\r\n", i+1);
      
  //     uint8_t read_result = DHT11_ReadData(&dht11_data);
  //             if(read_result) {
  //           dht11_success_count++;
  //           printf("  DHT11��ȡ�ɹ�: ʪ��=%d.%d%% �¶�=%d.%d��C\r\n", 
  //                  dht11_data.humidity_int, dht11_data.humidity_dec,
  //                  dht11_data.temperature_int, dht11_data.temperature_dec);
            
  //           // ����ȫ������
  //           g_sensor_data.humidity = dht11_data.humidity_int + dht11_data.humidity_dec / 10.0f;
  //           g_sensor_data.temperature = dht11_data.temperature_int + dht11_data.temperature_dec / 10.0f;
  //           g_sensor_data.dht11_valid = 1;
            
  //           // ���ݺ����Լ��
  //           if(dht11_data.humidity_int <= 99 && dht11_data.temperature_int < 60) {
  //               printf("  ���ݺ����Լ��: ͨ��\r\n");
  //           } else {
  //               printf("  ���ݺ����Լ��: �쳣��\r\n");
  //           }
  //       } else {
  //           printf("  DHT11��ȡʧ��\r\n");
  //           g_sensor_data.dht11_valid = 0;
  //       }
      
  //     // DHT11��Ҫ����2����
  //     osDelay(3000); 
  // }
  
  // printf("DHT11��ʼ����ɣ��ɹ���: %d/5 (%.1f%%)\r\n", 
  //        dht11_success_count, (float)dht11_success_count/5*100);
         
  // �������ѭ�� - ÿ3���ȡһ��
  for(;;)
  {
    uint8_t read_result = DHT11_ReadData(&dht11_data);
    
    if(read_result) {
      g_sensor_data.humidity = dht11_data.humidity_int + dht11_data.humidity_dec / 10.0f;
      g_sensor_data.temperature = dht11_data.temperature_int + dht11_data.temperature_dec / 10.0f;
      g_sensor_data.dht11_valid = 1;
      
      printf("DHT11����: ʪ��=%.1f%% �¶�=%.1f��C\r\n", 
             g_sensor_data.humidity, g_sensor_data.temperature);
    } else {
      g_sensor_data.dht11_valid = 0;
      printf("DHT11��ȡʧ��\r\n");
    }
    // ��ʱ3��
    osDelay(3000);
  }
}

/**
  * @brief LCD��ʾ���� - ��ʾ������״̬�Ͱ�ť����
  * @param argument: �������
  * @retval None
  */
void StartLcdDisplayTask(void const * argument)
{
  printf("LCD��ʾ��������\r\n");
  
  // �ȴ�ϵͳ��ʼ�����
  osDelay(2000);
  
  // ���Ƴ�ʼUI
  LCD_DrawUI();
  
  // ������ʾһ�δ���������
  LCD_UpdateSensorData();
  
  static uint32_t last_sensor_update = 0;
  last_sensor_update = HAL_GetTick(); // ���ó�ʼʱ��
  
  for(;;)
  {
    uint32_t current_time = HAL_GetTick();
    
    // ÿ3�����һ�δ�����������ʾ
    if(current_time - last_sensor_update >= 1000) {
      LCD_UpdateSensorData();
      last_sensor_update = current_time;
    }
    
    // ���������� (���ֿ�����Ӧ)
    LCD_HandleTouch();
    
    // ˢ�°�ť״̬
    LCD_UpdateButtons();
    
    // ÿ100ms���һ�δ��������ְ�ť��Ӧ�ٶ�
    osDelay(100);
  }
}

/**
  * @brief ����LCD�û�����
  */
void LCD_DrawUI(void)
{
  // ����
  LCD_Fill(0, 0, SCREEN_WIDTH-1, SCREEN_HEIGHT-1, BLACK);
  
  // ����
  LCD_ShowChinese(30, 5, (uint8_t*)"���������������ϵͳ", WHITE, BLACK, 16, 0);
  
  // ������״̬�������
  LCD_ShowChinese(10, 30, (uint8_t*)"��������״̬��", YELLOW, BLACK, 12, 0);
  
  // ��ť�������
  LCD_ShowChinese(10, 190, (uint8_t*)"��������塿", YELLOW, BLACK, 12, 0);
  
  // ���ư�ť
  LCD_DrawButton(BTN1_X, BTN1_Y, BTN_WIDTH, BTN_HEIGHT, "������", g_control_state.breathing_led_enabled);
  LCD_DrawButton(BTN2_X, BTN2_Y, BTN_WIDTH, BTN_HEIGHT, "��ɫ��", g_control_state.rgb_led_enabled);
}

/**
  * @brief ���ư�ť
  */
void LCD_DrawButton(uint16_t x, uint16_t y, uint16_t width, uint16_t height, const char* text, uint8_t enabled)
{
  uint16_t bg_color = enabled ? GREEN : GRAY;
  uint16_t text_color = enabled ? BLACK : WHITE;
  
  // ���ư�ť����
  LCD_Fill(x, y, x + width - 1, y + height - 1, bg_color);
  
  // ���ư�ť�߿�
  LCD_DrawRectangle(x, y, x + width - 1, y + height - 1, WHITE);
  
  // ���ư�ť���� (����)
  uint8_t text_len = strlen(text);
  uint16_t text_x = x + (width - text_len * 8) / 2;  // �����ַ����8����
  uint16_t text_y = y + (height - 16) / 2;           // �����ַ��߶�16����
  
  LCD_ShowChinese(text_x, text_y, (uint8_t*)text, text_color, bg_color, 16, 0);
}

/**
  * @brief ���´�����������ʾ
  */
void LCD_UpdateSensorData(void)
{
  char buffer[50];
  
  // ���������������ʾ���� (��������UIԪ��)
  LCD_Fill(10, 50, SCREEN_WIDTH-10, 180, BLACK);
  
  // ��ʾ����������
  LCD_ShowChinese(10, 50, (uint8_t*)"������", WHITE, BLACK, 16, 0);
  sprintf(buffer, "%.2fV", g_sensor_data.adc_voltage);
  LCD_ShowString(70, 50, (uint8_t*)buffer, WHITE, BLACK, 16, 0);
  
  // ��ʾ�񶯴�����
  LCD_ShowChinese(10, 70, (uint8_t*)"�񶯣�", WHITE, BLACK, 16, 0);
  sprintf(buffer, "%s", g_sensor_data.vibration_detected ? "��" : "��");
  LCD_ShowChinese(70, 70, (uint8_t*)buffer, WHITE, BLACK, 16, 0);
  
  // ��ʾ����������
  LCD_ShowChinese(10, 90, (uint8_t*)"������", WHITE, BLACK, 16, 0);
  sprintf(buffer, "%s", g_sensor_data.touch_detected ? "��" : "��");
  LCD_ShowChinese(70, 90, (uint8_t*)buffer, WHITE, BLACK, 16, 0);
  
  // ��ʾ��ʪ������
  // if(g_sensor_data.dht11_valid) {
    LCD_ShowChinese(10, 110, (uint8_t*)"�¶ȣ�", CYAN, BLACK, 16, 0);
    sprintf(buffer, "%.1f", g_sensor_data.temperature);
    LCD_ShowString(50, 110, (uint8_t*)buffer, CYAN, BLACK, 16, 0);
    LCD_ShowChinese(90, 115, (uint8_t*)"��", CYAN, BLACK, 16, 0);
    
    LCD_ShowChinese(130, 110, (uint8_t*)"ʪ�ȣ�", CYAN, BLACK, 16, 0);
    sprintf(buffer, "%.1f%%", g_sensor_data.humidity);
    LCD_ShowString(180, 110, (uint8_t*)buffer, CYAN, BLACK, 16, 0);
  // }
  
  // ��ʾ����ʱ��
  LCD_ShowChinese(10, 130, (uint8_t*)"���У�", LIGHTGREEN, BLACK, 16, 0);
  sprintf(buffer, "%lu", HAL_GetTick()/1000);
  LCD_ShowString(50, 130, (uint8_t*)buffer, LIGHTGREEN, BLACK, 16, 0);
  LCD_ShowChinese(70, 130, (uint8_t*)(uint8_t*)"��", LIGHTGREEN, BLACK, 16, 0);
}

/**
  * @brief ����������
  */
void LCD_HandleTouch(void)
{
  // ��ȡ��������
  CST816_Get_XY_AXIS();
  
  if(CST816_Get_FingerNum() > 0) {
    uint16_t touch_x = CST816_Instance.X_Pos;
    uint16_t touch_y = CST816_Instance.Y_Pos;
    
    // ����Ƿ����˺����ư�ť
    if(touch_x >= BTN1_X && touch_x <= BTN1_X + BTN_WIDTH &&
       touch_y >= BTN1_Y && touch_y <= BTN1_Y + BTN_HEIGHT) {
      // �л�������״̬
      g_control_state.breathing_led_enabled = !g_control_state.breathing_led_enabled;
      printf("Breathing LED state: %s\r\n", g_control_state.breathing_led_enabled ? "ON" : "OFF");
      
      // �ػ水ť
      LCD_DrawButton(BTN1_X, BTN1_Y, BTN_WIDTH, BTN_HEIGHT, "������", g_control_state.breathing_led_enabled);
      
      // ��ʱ����
      osDelay(300);
    }
    
    // ����Ƿ�����RGB�ʵư�ť
    if(touch_x >= BTN2_X && touch_x <= BTN2_X + BTN_WIDTH &&
       touch_y >= BTN2_Y && touch_y <= BTN2_Y + BTN_HEIGHT) {
      // �л�RGB LED״̬
      g_control_state.rgb_led_enabled = !g_control_state.rgb_led_enabled;
      printf("RGB LED state: %s\r\n", g_control_state.rgb_led_enabled ? "ON" : "OFF");
      
      // �ػ水ť
      LCD_DrawButton(BTN2_X, BTN2_Y, BTN_WIDTH, BTN_HEIGHT, "��ɫ��", g_control_state.rgb_led_enabled);
      
      // ��ʱ����
      osDelay(300);
    }
  }
}

/**
  * @brief ���°�ť״̬��ʾ
  */
void LCD_UpdateButtons(void)
{
  static uint8_t last_breathing_state = 255; // ��ʼ��Ϊ��Чֵ
  static uint8_t last_rgb_state = 255;
  
  // ֻ��״̬�ı�ʱ���ػ水ť������Ƶ��ˢ��
  if(last_breathing_state != g_control_state.breathing_led_enabled) {
    LCD_DrawButton(BTN1_X, BTN1_Y, BTN_WIDTH, BTN_HEIGHT, "������", g_control_state.breathing_led_enabled);
    last_breathing_state = g_control_state.breathing_led_enabled;
  }
  
  if(last_rgb_state != g_control_state.rgb_led_enabled) {
    LCD_DrawButton(BTN2_X, BTN2_Y, BTN_WIDTH, BTN_HEIGHT, "��ɫ��", g_control_state.rgb_led_enabled);
    last_rgb_state = g_control_state.rgb_led_enabled;
  }
}

/**
  * @brief LVGL���� - ����GUI���ºͰ�ť�¼�
  * @param argument: �������
  * @retval None
  */
void StartLvglTask(void const * argument)
{
  printf("LVGL�������� - ֧�ְ�ť�¼�����\r\n");
  
  // �ȴ�LVGL������ʼ�����
  osDelay(2000);
  
  printf("��ʼ����LVGL����...\r\n");
  
  // ����LVGL����
  LVGL_CreateInterface();
  
  printf("LVGL���洴����ɣ���ʼ��ѭ��\r\n");
  
  // LVGL��ѭ��
  for(;;)
  {
    // ����LVGL tick����
    lv_tick_inc(5);
    
    // ����LVGL���� - ����LVGL�ĺ��Ĵ������������¼�����
    lv_timer_handler();
    
    // ÿ5ms����һ��LVGL���񣬱�֤�����������¼���Ӧ
    osDelay(5);
  }
}

/**
  * @brief ����LVGL����
  * @retval None
  */
void LVGL_CreateInterface(void)
{
  printf("���ڴ���LVGL�������...\r\n");
  
  // ��������ˢ����Ļ������ʾ�����Ƿ񱻵���
  lv_disp_load_scr(lv_scr_act());
  
  // �Ȳ���LVGL������ʾ - �򵥵���Ļ���
  lv_obj_t * scr = lv_scr_act();
  lv_obj_set_style_bg_color(scr, lv_color_hex(0x003a57), LV_PART_MAIN);
  
  // �����򵥲��Ա�ǩ
  lv_obj_t *label = lv_label_create(scr);
  lv_label_set_text(label, "LVGL Test");
  lv_obj_set_style_text_color(label, lv_color_white(), LV_PART_MAIN);
  lv_obj_align(label, LV_ALIGN_CENTER, 0, -50);
  
  // ������ť����
  lv_obj_t *btn = lv_btn_create(scr);
  lv_obj_set_size(btn, 120, 50);
  lv_obj_align(btn, LV_ALIGN_CENTER, 0, 0);
  lv_obj_t *btn_label = lv_label_create(btn);
  lv_label_set_text(btn_label, "Test Button");
  lv_obj_center(btn_label);
  
  // Ϊ��ť��ӵ���¼�����
  lv_obj_add_event_cb(btn, btn_event_cb, LV_EVENT_CLICKED, NULL);
  
  printf("LVGL��������������\r\n");
  
  // ��ʼ��Ⱦ����ȷ��������ʾ
  for(int i = 0; i < 10; i++) {
    lv_timer_handler();  
    osDelay(5);
  }
}

/**
  * @brief LVGL��ť����¼�������
  * @param e: �¼�����
  * @retval None
  */
static void btn_event_cb(lv_event_t * e)
{
    lv_event_code_t code = lv_event_get_code(e);
    lv_obj_t * btn = lv_event_get_target(e);
    
    if(code == LV_EVENT_CLICKED) {
        printf("===== LVGL��ť�������! =====\r\n");
        printf("�¼�����: LV_EVENT_CLICKED\r\n");
        printf("��ť�����ַ: 0x%08X\r\n", (uint32_t)btn);
        
        // ��ȡ��ǰʱ�������ѡ��
        uint32_t tick = HAL_GetTick();
        printf("ʱ���: %lu ms\r\n", tick);
        
        // ������������Ӹ���Ĺ���
        // ����ı䰴ť��ɫ�����ֵ�
        static uint8_t click_count = 0;
        click_count++;
        printf("��ť�������: %d\r\n", click_count);
        
        // ���°�ť������ʾ�������
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
