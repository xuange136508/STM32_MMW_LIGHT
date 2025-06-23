#include "ws2812b.h"
#include "tim.h"
#include <string.h>

// 全局变量
// static TIM_HandleTypeDef htim4;
// static DMA_HandleTypeDef hdma_tim4_ch2;
static WS2812B_Color_t led_colors[WS2812B_LED_COUNT];
static uint16_t pwm_buffer[WS2812B_LED_COUNT * 24 + WS2812B_RESET_PULSE];
static uint8_t global_brightness = 255;

// 静态函数声明
static void WS2812B_GPIO_Init(void);
static void WS2812B_DMA_Init(void);
static void WS2812B_TIM4_Init(void);
static void WS2812B_FillBuffer(void);

/**
 * @brief 初始化WS2812B (包含GPIO, TIM4, DMA初始化)
 */
void WS2812B_Init(void)
{
    // 使能时钟
    // __HAL_RCC_GPIOD_CLK_ENABLE();
    // __HAL_RCC_TIM4_CLK_ENABLE();
    // __HAL_RCC_DMA1_CLK_ENABLE();
    
    // // 初始化GPIO
    // WS2812B_GPIO_Init();
    
    // // 初始化DMA
    // WS2812B_DMA_Init();
    
    // // 初始化TIM4
    // WS2812B_TIM4_Init();
    
    // 清空LED
    WS2812B_Clear();
}

/**
 * @brief 初始化GPIO (PD13 -> TIM4_CH2)
 */
static void WS2812B_GPIO_Init(void)
{
    GPIO_InitTypeDef GPIO_InitStruct = {0};
    
    // 配置PD13为TIM4_CH2复用功能
    GPIO_InitStruct.Pin = GPIO_PIN_13;
    GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_HIGH;
    GPIO_InitStruct.Alternate = GPIO_AF2_TIM4;
    HAL_GPIO_Init(GPIOD, &GPIO_InitStruct);
}

/**
 * @brief 初始化DMA1_Stream3 for TIM4_CH2
 */
static void WS2812B_DMA_Init(void)
{
    // DMA配置
    hdma_tim4_ch2.Instance = DMA1_Stream3;
    hdma_tim4_ch2.Init.Channel = DMA_CHANNEL_2;
    hdma_tim4_ch2.Init.Direction = DMA_MEMORY_TO_PERIPH;
    hdma_tim4_ch2.Init.PeriphInc = DMA_PINC_DISABLE;
    hdma_tim4_ch2.Init.MemInc = DMA_MINC_ENABLE;
    hdma_tim4_ch2.Init.PeriphDataAlignment = DMA_PDATAALIGN_HALFWORD;
    hdma_tim4_ch2.Init.MemDataAlignment = DMA_MDATAALIGN_HALFWORD;
    hdma_tim4_ch2.Init.Mode = DMA_NORMAL;
    hdma_tim4_ch2.Init.Priority = DMA_PRIORITY_HIGH;
    hdma_tim4_ch2.Init.FIFOMode = DMA_FIFOMODE_DISABLE;
    
    if (HAL_DMA_Init(&hdma_tim4_ch2) != HAL_OK) {
        Error_Handler();
    }
    
    // 链接DMA到TIM4
    __HAL_LINKDMA(&htim4, hdma[TIM_DMA_ID_CC2], hdma_tim4_ch2);
    
    // DMA中断配置
    HAL_NVIC_SetPriority(DMA1_Stream3_IRQn, 0, 0);
    HAL_NVIC_EnableIRQ(DMA1_Stream3_IRQn);
}

/**
 * @brief 初始化TIM4
 */
static void WS2812B_TIM4_Init(void)
{
    TIM_ClockConfigTypeDef sClockSourceConfig = {0};
    TIM_MasterConfigTypeDef sMasterConfig = {0};
    TIM_OC_InitTypeDef sConfigOC = {0};

    // TIM4基本配置
    htim4.Instance = TIM4;
    htim4.Init.Prescaler = 0;                           // 不分频，84MHz
    htim4.Init.CounterMode = TIM_COUNTERMODE_UP;
    htim4.Init.Period = WS2812B_TIMER_PERIOD;           // ARR = 104, 周期1.25us
    htim4.Init.ClockDivision = TIM_CLOCKDIVISION_DIV1;
    htim4.Init.AutoReloadPreload = TIM_AUTORELOAD_PRELOAD_DISABLE;
    
    if (HAL_TIM_Base_Init(&htim4) != HAL_OK) {
        Error_Handler();
    }
    
    // 时钟源配置
    sClockSourceConfig.ClockSource = TIM_CLOCKSOURCE_INTERNAL;
    if (HAL_TIM_ConfigClockSource(&htim4, &sClockSourceConfig) != HAL_OK) {
        Error_Handler();
    }
    
    // PWM初始化
    if (HAL_TIM_PWM_Init(&htim4) != HAL_OK) {
        Error_Handler();
    }
    
    // Master配置
    sMasterConfig.MasterOutputTrigger = TIM_TRGO_RESET;
    sMasterConfig.MasterSlaveMode = TIM_MASTERSLAVEMODE_DISABLE;
    if (HAL_TIMEx_MasterConfigSynchronization(&htim4, &sMasterConfig) != HAL_OK) {
        Error_Handler();
    }
    
    // PWM通道2配置
    sConfigOC.OCMode = TIM_OCMODE_PWM1;
    sConfigOC.Pulse = 0;
    sConfigOC.OCPolarity = TIM_OCPOLARITY_HIGH;
    sConfigOC.OCFastMode = TIM_OCFAST_DISABLE;
    if (HAL_TIM_PWM_ConfigChannel(&htim4, &sConfigOC, TIM_CHANNEL_2) != HAL_OK) {
        Error_Handler();
    }
}

/**
 * @brief 设置单个LED颜色
 * @param index LED索引 (0到WS2812B_LED_COUNT-1)
 * @param r 红色值 (0-255)
 * @param g 绿色值 (0-255)
 * @param b 蓝色值 (0-255)
 */
void WS2812B_SetColor(uint16_t index, uint8_t r, uint8_t g, uint8_t b)
{
    if (index < WS2812B_LED_COUNT) {
        // 应用亮度调节
        led_colors[index].r = (r * global_brightness) / 255;
        led_colors[index].g = (g * global_brightness) / 255;
        led_colors[index].b = (b * global_brightness) / 255;
    }
}

/**
 * @brief 通过枚举设置LED颜色
 * @param index LED索引
 * @param color 预定义颜色枚举
 */
void WS2812B_SetColorEnum(uint16_t index, WS2812B_ColorEnum_t color)
{
    switch (color) {
        case WS2812B_BLACK:   WS2812B_SetColor(index, 0, 0, 0); break;
        case WS2812B_RED:     WS2812B_SetColor(index, 255, 0, 0); break;
        case WS2812B_GREEN:   WS2812B_SetColor(index, 0, 255, 0); break;
        case WS2812B_BLUE:    WS2812B_SetColor(index, 0, 0, 255); break;
        case WS2812B_YELLOW:  WS2812B_SetColor(index, 255, 255, 0); break;
        case WS2812B_MAGENTA: WS2812B_SetColor(index, 255, 0, 255); break;
        case WS2812B_CYAN:    WS2812B_SetColor(index, 0, 255, 255); break;
        case WS2812B_WHITE:   WS2812B_SetColor(index, 255, 255, 255); break;
        case WS2812B_ORANGE:  WS2812B_SetColor(index, 255, 165, 0); break;
        case WS2812B_PURPLE:  WS2812B_SetColor(index, 128, 0, 128); break;
        default:              WS2812B_SetColor(index, 0, 0, 0); break;
    }
}

/**
 * @brief 设置全局亮度
 * @param brightness 亮度值 (0-255)
 */
void WS2812B_SetBrightness(uint8_t brightness)
{
    global_brightness = brightness;
}

/**
 * @brief 填充PWM缓冲区
 */
static void WS2812B_FillBuffer(void)
{
    uint32_t buffer_index = 0;
    
    for (uint16_t led = 0; led < WS2812B_LED_COUNT; led++) {
        // WS2812B数据格式: GRB (绿-红-蓝)
        uint32_t color = (led_colors[led].g << 16) | (led_colors[led].r << 8) | led_colors[led].b;
        
        // 发送24位数据 (从高位开始)
        for (int8_t bit = 23; bit >= 0; bit--) {
            if (color & (1 << bit)) {
                pwm_buffer[buffer_index] = WS2812B_1_CODE;  // 1码
            } else {
                pwm_buffer[buffer_index] = WS2812B_0_CODE;  // 0码
            }
            buffer_index++;
        }
    }
    
    // 添加复位脉冲 (低电平)
    for (uint16_t i = 0; i < WS2812B_RESET_PULSE; i++) {
        pwm_buffer[buffer_index++] = 0;
    }
}

/**
 * @brief 更新LED显示
 */
void WS2812B_Update(void)
{
    // 填充缓冲区
    WS2812B_FillBuffer();
    
    // 停止之前的传输
    HAL_TIM_PWM_Stop_DMA(&htim4, TIM_CHANNEL_2);
    
    // 启动DMA传输
    HAL_TIM_PWM_Start_DMA(&htim4, TIM_CHANNEL_2, 
                          (uint32_t*)pwm_buffer, 
                          WS2812B_LED_COUNT * 24 + WS2812B_RESET_PULSE);
}

/**
 * @brief 清空所有LED
 */
void WS2812B_Clear(void)
{
    for (uint16_t i = 0; i < WS2812B_LED_COUNT; i++) {
        led_colors[i].r = 0;
        led_colors[i].g = 0;
        led_colors[i].b = 0;
    }
    WS2812B_Update();
}

/**
 * @brief 彩虹测试效果
 */
void WS2812B_Test_Rainbow(void)
{
    WS2812B_ColorEnum_t colors[] = {
        WS2812B_RED, WS2812B_ORANGE, WS2812B_YELLOW, WS2812B_GREEN,
        WS2812B_CYAN, WS2812B_BLUE, WS2812B_PURPLE, WS2812B_MAGENTA
    };
    
    uint8_t color_count = sizeof(colors) / sizeof(colors[0]);
    
    for (uint8_t i = 0; i < color_count; i++) {
        for (uint16_t led = 0; led < WS2812B_LED_COUNT; led++) {
            WS2812B_SetColorEnum(led, colors[i]);
        }
        WS2812B_Update();
        HAL_Delay(300);
    }
}

/**
 * @brief 呼吸灯测试效果
 */
void WS2812B_Test_Breathing(void)
{
    // 红色呼吸灯
    for (uint8_t brightness = 0; brightness < 255; brightness += 5) {
        WS2812B_SetBrightness(brightness);
        for (uint16_t led = 0; led < WS2812B_LED_COUNT; led++) {
            WS2812B_SetColor(led, 255, 0, 0);
        }
        WS2812B_Update();
        HAL_Delay(20);
    }
    
    for (uint8_t brightness = 255; brightness > 0; brightness -= 5) {
        WS2812B_SetBrightness(brightness);
        for (uint16_t led = 0; led < WS2812B_LED_COUNT; led++) {
            WS2812B_SetColor(led, 255, 0, 0);
        }
        WS2812B_Update();
        HAL_Delay(20);
    }
    
    WS2812B_SetBrightness(255); // 恢复默认亮度
}

/**
 * @brief DMA传输完成回调函数
 */
void HAL_TIM_PWM_PulseFinishedCallback(TIM_HandleTypeDef *htim)
{
    if (htim->Instance == TIM4) {
        HAL_TIM_PWM_Stop_DMA(&htim4, TIM_CHANNEL_2);
    }
}


void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef *htim)
{
    // Keep original TIM1 callback
    if (htim->Instance == TIM1) {
        HAL_IncTick();
    }
} 

/**
 * @brief DMA中断处理函数
 */
// void DMA1_Stream3_IRQHandler(void)
// {
//     HAL_DMA_IRQHandler(&hdma_tim4_ch2);
// } 