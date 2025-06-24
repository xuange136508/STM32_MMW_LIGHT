#include "main.h"
#include "cmsis_os.h"
#include "adc.h"
#include "usart.h"
#include "gpio.h"

#include "adc.h"
#include "dht11.h"
#include "spi.h"
#include "i2c.h"
#include "tim.h"

#include <stdio.h>

#include "CST816.h"
#include "lcd.h"
#include "lcd_init.h"
#include "ws2812b.h"



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
  MX_SPI1_Init();
  MX_I2C1_Init();

// 添加计时器的初始化
  MX_TIM4_Init();  


  // // LCD屏幕初始化
  // LCD_Init();
  // printf("LCD初始化完成\r\n");
  // // 填充为蓝色
  // LCD_Fill(0, 0, 239, 279, BLUE);
  // HAL_Delay(1000);
  // // 初始化触摸屏
  // CST816_GPIO_Init();
  // printf("触摸屏初始化完成\r\n");


  char msg[] = "UART初始化完成\r\n";
  HAL_UART_Transmit(&huart1, (uint8_t *)msg, 25, HAL_MAX_DELAY);
  
 
  /* Call init function for freertos objects (in cmsis_os2.c) */
  MX_FREERTOS_Init();
  /* Start scheduler */
  osKernelStart();

  while (1)
  {
    // 这里的代码不会执行，因为FreeRTOS调度器已经接管控制权
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
