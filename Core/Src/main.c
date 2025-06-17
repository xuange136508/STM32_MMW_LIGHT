#include "main.h"
#include "cmsis_os.h"
#include "adc.h"
#include "usart.h"
#include "gpio.h"

#include "adc.h"
#include "dht11.h"
//#include "cpp_func.h"
#include <stdio.h>


// 重定向 printf 到 UART
#ifdef __GNUC__
#define PUTCHAR_PROTOTYPE int __io_putchar(int ch)
#else
#define PUTCHAR_PROTOTYPE int fputc(int ch, FILE *f)
#endif

PUTCHAR_PROTOTYPE
{
    HAL_UART_Transmit(&huart1, (uint8_t *)&ch, 1, HAL_MAX_DELAY);
    return ch;
}

// 重定向 scanf 到 UART
int __io_getchar(void)
{
    uint8_t ch = 0;
    HAL_UART_Receive(&huart1, &ch, 1, HAL_MAX_DELAY);
    return ch;
}

uint16_t Get_ADC_Value(void) {
    HAL_ADC_Start(&hadc1);              // 启动 ADC
    if (HAL_ADC_PollForConversion(&hadc1, 10) == HAL_OK) {  // 等待转换完成
        return HAL_ADC_GetValue(&hadc1);  // 返回 12 位数据（0-4095）
    }
    return 0;  // 失败返回 0
}


void SystemClock_Config(void);
void MX_FREERTOS_Init(void);



/**
  * @brief  The application entry point.
  * @retval int
  */
int main(void)
{

  /* Reset of all peripherals, Initializes the Flash interface and the Systick. */
  HAL_Init();

  /* Configure the system clock */
  SystemClock_Config();

  /* Initialize all configured peripherals */
  MX_GPIO_Init();
  MX_USART1_UART_Init();
  MX_ADC1_Init();
  /* USER CODE BEGIN 2 */

  // 启用DWT循环计数器（用于微秒延时）
  CoreDebug->DEMCR |= CoreDebug_DEMCR_TRCENA_Msk;
  DWT->CYCCNT = 0;
  DWT->CTRL |= DWT_CTRL_CYCCNTENA_Msk;

  // 初始化完成后立即测试串口
  char msg[] = "UART初始化完成\r\n";
  HAL_UART_Transmit(&huart1, (uint8_t *)msg, 25, HAL_MAX_DELAY);
  
  // DHT11读取测试
   DHT11_Data_t dht11_data;
   uint8_t dht11_success_count = 0;

   for(int i = 0; i < 20; i++) {
       printf("=== 第 %d 次读取 ===\r\n", i+1);
       
       // 读取DHT11数据（单独进行，避免其他操作干扰时序）
       if(DHT11_ReadData(&dht11_data)) {
           dht11_success_count++;
           printf("  DHT11传感器读取成功:\r\n");
           printf("  湿度: %d.%d%%\r\n", dht11_data.humidity_int, dht11_data.humidity_dec);
           printf("  温度: %d.%d°C\r\n", dht11_data.temperature_int, dht11_data.temperature_dec);
           
           // 数据合理性检查
           if(dht11_data.humidity_int <= 99 && dht11_data.temperature_int < 60) {
               printf("  数据合理 \r\n");
           } else {
               printf("  数据可能异常！\r\n");
           }
       } else {
           printf("  DHT11读取失败\r\n");
       }
       HAL_Delay(1000);  // 延时1秒后再测试其他传感器
       
       // ADC测试
       uint16_t adc_value = Get_ADC_Value();
       float voltage = (adc_value * 3.3f) / 4095.0f;
       printf("  ADC: %hu (%.2fV)\r\n", adc_value, voltage);

       // 振动检测
       GPIO_PinState state = HAL_GPIO_ReadPin(GPIOA, GPIO_PIN_8);
       printf("  振动: %s\r\n", (state == GPIO_PIN_SET) ? "触发" : "正常");
       
       printf("  成功率: %d/%d (%.1f%%)\r\n\r\n", 
              dht11_success_count, i+1, 
              (float)dht11_success_count/(i+1)*100);
       
       // DHT11需要至少2秒间隔，使用3秒更保险
       HAL_Delay(2000); 
   }
   
   printf("DHT11测试完成，成功率: %d/20 (%.1f%%)\r\n", 
          dht11_success_count, (float)dht11_success_count/20*100);

  /* Call init function for freertos objects (in cmsis_os2.c) */
  MX_FREERTOS_Init();

  /* Start scheduler */
  osKernelStart();

  while (1)
  {
    // 这里的代码永远不会执行，因为FreeRTOS调度器已经接管控制权
  }
  
}

/**
  * @brief System Clock Configuration
  * @retval None
  */
void SystemClock_Config(void)
{
  RCC_OscInitTypeDef RCC_OscInitStruct = {0};
  RCC_ClkInitTypeDef RCC_ClkInitStruct = {0};

  /** Configure the main internal regulator output voltage
  */
  __HAL_RCC_PWR_CLK_ENABLE();
  __HAL_PWR_VOLTAGESCALING_CONFIG(PWR_REGULATOR_VOLTAGE_SCALE1);

  /** Initializes the RCC Oscillators according to the specified parameters
  * in the RCC_OscInitTypeDef structure.
  */
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSE;
  RCC_OscInitStruct.HSEState = RCC_HSE_ON;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
  RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSE;
  RCC_OscInitStruct.PLL.PLLM = 4;
  RCC_OscInitStruct.PLL.PLLN = 168;
  RCC_OscInitStruct.PLL.PLLP = RCC_PLLP_DIV2;
  RCC_OscInitStruct.PLL.PLLQ = 4;
  if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
  {
    Error_Handler();
  }

  /** Initializes the CPU, AHB and APB buses clocks
  */
  RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK|RCC_CLOCKTYPE_SYSCLK
                              |RCC_CLOCKTYPE_PCLK1|RCC_CLOCKTYPE_PCLK2;
  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
  RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV4;
  RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV2;

  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_5) != HAL_OK)
  {
    Error_Handler();
  }
}

/* USER CODE BEGIN 4 */

/* USER CODE END 4 */

/**
  * @brief  Period elapsed callback in non blocking mode
  * @note   This function is called  when TIM1 interrupt took place, inside
  * HAL_TIM_IRQHandler(). It makes a direct call to HAL_IncTick() to increment
  * a global variable "uwTick" used as application time base.
  * @param  htim : TIM handle
  * @retval None
  */
void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef *htim)
{
  /* USER CODE BEGIN Callback 0 */

  /* USER CODE END Callback 0 */
  if (htim->Instance == TIM1)
  {
    HAL_IncTick();
  }
  /* USER CODE BEGIN Callback 1 */

  /* USER CODE END Callback 1 */
}

/**
  * @brief  This function is executed in case of error occurrence.
  * @retval None
  */
void Error_Handler(void)
{
  /* USER CODE BEGIN Error_Handler_Debug */
  /* User can add his own implementation to report the HAL error return state */
  __disable_irq();
  while (1)
  {
  }
  /* USER CODE END Error_Handler_Debug */
}

#ifdef  USE_FULL_ASSERT
/**
  * @brief  Reports the name of the source file and the source line number
  *         where the assert_param error has occurred.
  * @param  file: pointer to the source file name
  * @param  line: assert_param error line source number
  * @retval None
  */
void assert_failed(uint8_t *file, uint32_t line)
{
  /* USER CODE BEGIN 6 */
  /* User can add his own implementation to report the file name and line number,
     ex: printf("Wrong parameters value: file %s on line %d\r\n", file, line) */
  /* USER CODE END 6 */
}
#endif /* USE_FULL_ASSERT */
