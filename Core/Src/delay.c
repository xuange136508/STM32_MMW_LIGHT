#include "delay.h"

// #define SysTick_CLKSource_HCLK_Div8    ((uint32_t)0xFFFFFFFB)
// #define SysTick_CLKSource_HCLK         ((uint32_t)0x00000004)

static uint8_t us=0;
static uint16_t ms=0;

// 简单的毫秒延时函数
void delay_ms(uint32_t ms)
{
    HAL_Delay(ms);
}

// 微秒延时函数
void delay_us(uint32_t us)
{
    uint32_t start = DWT->CYCCNT;
    uint32_t cycles = us * (SystemCoreClock / 1000000);
    while ((DWT->CYCCNT - start) < cycles);
}

// 微秒延时函数
void Rough_DelayUs(uint16_t Time)
{
    delay_us(Time);
}

// 毫秒延时函数
void Rough_DelayMs(uint16_t Time)
{
    delay_ms(Time);
}

// 使用HAL库的SysTick，这里不需要额外初始化
void SysTick_Init(uint8_t SysClk)
{
	//SysTick_CLKSourceConfig(SysTick_CLKSource_HCLK_Div8);
	HAL_SYSTICK_CLKSourceConfig(SYSTICK_CLKSOURCE_HCLK_DIV8);
	us=SysClk/8;
	ms=us*1000;
}

// 微秒延时函数
void Correct_DelayUs(uint32_t Time)
{
    delay_us(Time);
}

// 毫秒延时函数
void Correct_DelayMs(uint16_t Time)
{
    delay_ms(Time);
}