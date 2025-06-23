#ifndef __WS2812B_H
#define __WS2812B_H

#ifdef __cplusplus
extern "C" {
#endif

#include "stm32f4xx_hal.h"

// WS2812B配置参数
#define WS2812B_LED_COUNT       2       // LED数量
#define WS2812B_RESET_PULSE     50      // 复位脉冲数量

// WS2812B时序参数 (基于84MHz TIM4时钟)
#define WS2812B_TIMER_PERIOD    104     // 1.25us周期 (84MHz/105 = 800kHz)
#define WS2812B_0_CODE          33      // 0码：高电平0.4us
#define WS2812B_1_CODE          67      // 1码：高电平0.8us

// 颜色结构体
typedef struct {
    uint8_t r;  // 红色 0-255
    uint8_t g;  // 绿色 0-255  
    uint8_t b;  // 蓝色 0-255
} WS2812B_Color_t;

// 预定义颜色
typedef enum {
    WS2812B_BLACK = 0,
    WS2812B_RED,
    WS2812B_GREEN,
    WS2812B_BLUE,
    WS2812B_YELLOW,
    WS2812B_MAGENTA,
    WS2812B_CYAN,
    WS2812B_WHITE,
    WS2812B_ORANGE,
    WS2812B_PURPLE
} WS2812B_ColorEnum_t;

// 函数声明
void WS2812B_Init(void);
void WS2812B_SetColor(uint16_t index, uint8_t r, uint8_t g, uint8_t b);
void WS2812B_SetColorEnum(uint16_t index, WS2812B_ColorEnum_t color);
void WS2812B_Update(void);
void WS2812B_Clear(void);
void WS2812B_SetBrightness(uint8_t brightness);

// 测试函数
void WS2812B_Test_Rainbow(void);
void WS2812B_Test_Breathing(void);

#ifdef __cplusplus
}
#endif

#endif /* __WS2812B_H */ 