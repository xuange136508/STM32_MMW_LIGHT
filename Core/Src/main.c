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



// �ض��� printf �� UART
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

// �ض��� scanf �� UART
int __io_getchar(void)
{
    uint8_t ch = 0;
    HAL_UART_Receive(&huart1, &ch, 1, HAL_MAX_DELAY);
    return ch;
}

uint16_t Get_ADC_Value(void) {
    HAL_ADC_Start(&hadc1);              // ���� ADC
    if (HAL_ADC_PollForConversion(&hadc1, 10) == HAL_OK) {  // �ȴ�ת�����
        return HAL_ADC_GetValue(&hadc1);  // ���� 12 λ���ݣ�0-4095��
    }
    return 0;  // ʧ�ܷ��� 0
}


void SystemClock_Config(void);
void MX_FREERTOS_Init(void);

// LCD���Ժ�������
void LCD_Test_Basic(void);
void LCD_Test_Graphics(void);
void LCD_Test_Text(void);
void LCD_Test_Colors(void);
void LCD_Test_All(void);



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

// ��Ӽ�ʱ���ĳ�ʼ��
  MX_TIM4_Init();  

  char msg[] = "UART��ʼ�����\r\n";
  HAL_UART_Transmit(&huart1, (uint8_t *)msg, 25, HAL_MAX_DELAY);



  // LCD��Ļ��ʼ��
  LCD_Init();
  LCD_Fill(0, 0, 239, 279, GREEN);
  // ������ʾ����
  // LCD_Fill(0, 0, SCREEN_WIDTH-1, SCREEN_HEIGHT-1, BLACK);
  // LCD_ShowString(20, 120, (uint8_t*)"STM32F407 Ready", WHITE, BLACK, 16, 0);
  // printf("LCD������ʾ�������\r\n");
  // ����LCD����
  // LCD_Test_All();
  // ���ߵ�������ĳ�����ܣ�
  // LCD_Test_Basic();      // ������ɫ������
  // LCD_Test_Graphics();   // ͼ�λ��Ʋ���
  // LCD_Test_Text();       // ������ʾ����
  LCD_Test_Colors();     // ��ɫЧ������

  // ��ʼ��������
  CST816_Init();
  // ���Դ�����
  // static int lastX = -1, lastY = -1;
  // while (1){
  // CST816_Get_XY_AXIS(); // ���´�������
  //   if (CST816_Get_FingerNum() > 0) {
  //           if(lastX != -1 && lastY != -1) {
  //             // ʹ��Bresenham�㷨����
  //             LCD_DrawLine(lastX, lastY, CST816_Instance.X_Pos, CST816_Instance.Y_Pos, WHITE);
  //           }
  //           lastX = CST816_Instance.X_Pos;
  //           lastY = CST816_Instance.Y_Pos;
  //         } else {
  //           lastX = lastY = -1; // ��ָ̧��ʱ����
  //         }
  // }
  
  /* Call init function for freertos objects (in cmsis_os2.c) */
  MX_FREERTOS_Init();
  /* Start scheduler */
  osKernelStart();

  while (1)
  {
    // ����Ĵ��벻��ִ�У���ΪFreeRTOS�������Ѿ��ӹܿ���Ȩ
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

/* ========== LCD���Ժ���ʵ�� ========== */

/**
  * @brief LCD�������ܲ���
  */
void LCD_Test_Basic(void)
{
    printf("LCD�������Կ�ʼ...\r\n");
    
    // ����1����������
    printf("  ����1: ���� - ��ɫ\r\n");
    LCD_Fill(0, 0, SCREEN_WIDTH-1, SCREEN_HEIGHT-1, BLACK);
    HAL_Delay(1000);
    
    printf("  ����2: ���� - ��ɫ\r\n");
    LCD_Fill(0, 0, SCREEN_WIDTH-1, SCREEN_HEIGHT-1, WHITE);
    HAL_Delay(1000);
    
    printf("  ����3: ���� - ��ɫ\r\n");
    LCD_Fill(0, 0, SCREEN_WIDTH-1, SCREEN_HEIGHT-1, RED);
    HAL_Delay(1000);
    
    printf("  ����4: ���� - ��ɫ\r\n");
    LCD_Fill(0, 0, SCREEN_WIDTH-1, SCREEN_HEIGHT-1, GREEN);
    HAL_Delay(1000);
    
    printf("  ����5: ���� - ��ɫ\r\n");
    LCD_Fill(0, 0, SCREEN_WIDTH-1, SCREEN_HEIGHT-1, BLUE);
    HAL_Delay(1000);
    
    printf("LCD�����������\r\n");
}

/**
  * @brief LCDͼ�λ��Ʋ���
  */
void LCD_Test_Graphics(void)
{
    printf("LCDͼ�β��Կ�ʼ...\r\n");
    
    // ����Ϊ��ɫ
    LCD_Fill(0, 0, SCREEN_WIDTH-1, SCREEN_HEIGHT-1, BLACK);
    
    printf("  ����1: ����\r\n");
    // ����Ļ�ϻ�һЩ�����
    for(int i = 0; i < 50; i++) {
        uint16_t x = i * 4;
        uint16_t y = i * 5;
        if(x < SCREEN_WIDTH && y < SCREEN_HEIGHT) {
            LCD_DrawPoint(x, y, WHITE);
        }
    }
    HAL_Delay(2000);
    
    // ����
    LCD_Fill(0, 0, SCREEN_WIDTH-1, SCREEN_HEIGHT-1, BLACK);
    
    printf("  ����2: ����\r\n");
    // ���Խ���
    LCD_DrawLine(0, 0, SCREEN_WIDTH-1, SCREEN_HEIGHT-1, RED);
    LCD_DrawLine(0, SCREEN_HEIGHT-1, SCREEN_WIDTH-1, 0, GREEN);
    // ��ʮ����
    LCD_DrawLine(SCREEN_WIDTH/2, 0, SCREEN_WIDTH/2, SCREEN_HEIGHT-1, BLUE);
    LCD_DrawLine(0, SCREEN_HEIGHT/2, SCREEN_WIDTH-1, SCREEN_HEIGHT/2, YELLOW);
    HAL_Delay(2000);
    
    // ����
    LCD_Fill(0, 0, SCREEN_WIDTH-1, SCREEN_HEIGHT-1, BLACK);
    
    printf("  ����3: ������\r\n");
    // ��������ͬ��С�ľ���
    LCD_DrawRectangle(10, 10, 60, 40, RED);
    LCD_DrawRectangle(80, 50, 150, 100, GREEN);
    LCD_DrawRectangle(50, 120, 200, 180, BLUE);
    HAL_Delay(2000);
    
    // ����
    LCD_Fill(0, 0, SCREEN_WIDTH-1, SCREEN_HEIGHT-1, BLACK);
    
    printf("  ����4: ��Բ\r\n");
    // ��������ͬ��С��Բ
    Draw_Circle(60, 60, 30, RED);
    Draw_Circle(180, 80, 25, GREEN);
    Draw_Circle(120, 160, 40, BLUE);
    Draw_Circle(120, 220, 20, YELLOW);
    HAL_Delay(2000);
    
    printf("LCDͼ�β������\r\n");
}

/**
  * @brief LCD������ʾ����
  */
void LCD_Test_Text(void)
{
    printf("LCD���ֲ��Կ�ʼ...\r\n");
    
    // ����Ϊ��ɫ
    LCD_Fill(0, 0, SCREEN_WIDTH-1, SCREEN_HEIGHT-1, BLACK);
    
    printf("  ����1: ��ʾӢ���ַ���\r\n");
    LCD_ShowString(10, 10, (uint8_t*)"STM32F407", WHITE, BLACK, 16, 0);
    LCD_ShowString(10, 30, (uint8_t*)"LCD Test", RED, BLACK, 16, 0);
    LCD_ShowString(10, 50, (uint8_t*)"Hello World!", GREEN, BLACK, 16, 0);
    HAL_Delay(2000);
    
    printf("  ����2: ��ʾ����\r\n");
    LCD_ShowIntNum(10, 80, 12345, 5, BLUE, BLACK, 16);
    LCD_ShowIntNum(10, 100, 67890, 5, YELLOW, BLACK, 16);
    HAL_Delay(2000);
    
    printf("  ����3: ��ʾ������\r\n");
    LCD_ShowFloatNum1(10, 120, 3.14159, 4, CYAN, BLACK, 16);
    LCD_ShowFloatNum1(10, 140, 25.68, 4, MAGENTA, BLACK, 16);
    HAL_Delay(2000);
    
    printf("  ����4: ��ͬ�����С\r\n");
    LCD_Fill(0, 0, SCREEN_WIDTH-1, SCREEN_HEIGHT-1, BLACK);
    LCD_ShowString(10, 10, (uint8_t*)"Size 12", WHITE, BLACK, 12, 0);
    LCD_ShowString(10, 30, (uint8_t*)"Size 16", WHITE, BLACK, 16, 0);
    LCD_ShowString(10, 50, (uint8_t*)"Size 24", WHITE, BLACK, 24, 0);
    HAL_Delay(2000);
    
    printf("LCD���ֲ������\r\n");
}

/**
  * @brief LCD��ɫ����
  */
void LCD_Test_Colors(void)
{
    printf("LCD��ɫ���Կ�ʼ...\r\n");
    
    // ������ɫ��
    printf("  ����1: ��ɫ����\r\n");
    uint16_t colors[] = {RED, GREEN, BLUE, YELLOW, CYAN, MAGENTA, WHITE, BLACK};
    uint8_t color_count = sizeof(colors) / sizeof(colors[0]);
    uint16_t strip_height = SCREEN_HEIGHT / color_count;
    
    for(int i = 0; i < color_count; i++) {
        uint16_t y_start = i * strip_height;
        uint16_t y_end = (i + 1) * strip_height - 1;
        if(i == color_count - 1) y_end = SCREEN_HEIGHT - 1; // ���һ������
        
        LCD_Fill(0, y_start, SCREEN_WIDTH-1, y_end, colors[i]);
    }
    HAL_Delay(3000);
    
    // ���Խ���Ч�����򵥰棩
    printf("  ����2: ��ɫ����\r\n");
    for(int x = 0; x < SCREEN_WIDTH; x++) {
        uint8_t red_value = (x * 31) / SCREEN_WIDTH; // 0-31
        uint16_t color = (red_value << 11); // ��ɫ�����ڸ�5λ
        
        for(int y = 0; y < SCREEN_HEIGHT; y++) {
            LCD_DrawPoint(x, y, color);
        }
    }
    HAL_Delay(2000);
    
    printf("  ����3: ���̸�\r\n");
    uint8_t square_size = 20;
    for(int x = 0; x < SCREEN_WIDTH; x += square_size) {
        for(int y = 0; y < SCREEN_HEIGHT; y += square_size) {
            uint16_t color = ((x/square_size) + (y/square_size)) % 2 ? WHITE : BLACK;
            uint16_t x_end = (x + square_size - 1 < SCREEN_WIDTH) ? x + square_size - 1 : SCREEN_WIDTH - 1;
            uint16_t y_end = (y + square_size - 1 < SCREEN_HEIGHT) ? y + square_size - 1 : SCREEN_HEIGHT - 1;
            LCD_Fill(x, y, x_end, y_end, color);
        }
    }
    HAL_Delay(2000);
    
    printf("LCD��ɫ�������\r\n");
}

/**
  * @brief LCD��������
  */
void LCD_Test_All(void)
{
    printf("======== LCD�������Կ�ʼ ========\r\n");
    
    // ��ʾ���Ա���
    LCD_Fill(0, 0, SCREEN_WIDTH-1, SCREEN_HEIGHT-1, BLACK);
    LCD_ShowString(50, 120, (uint8_t*)"LCD Testing...", WHITE, BLACK, 16, 0);
    HAL_Delay(2000);
    
    // ����ִ�����в���
    LCD_Test_Basic();
    HAL_Delay(1000);
    
    LCD_Test_Graphics();
    HAL_Delay(1000);
    
    LCD_Test_Text();
    HAL_Delay(1000);
    
    LCD_Test_Colors();
    HAL_Delay(1000);
    
    // ��ʾ�������
    LCD_Fill(0, 0, SCREEN_WIDTH-1, SCREEN_HEIGHT-1, GREEN);
    LCD_ShowString(30, 120, (uint8_t*)"LCD Test", BLACK, GREEN, 24, 0);
    LCD_ShowString(30, 150, (uint8_t*)"Complete!", BLACK, GREEN, 24, 0);
    
    printf("======== LCD����������� ========\r\n");
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
