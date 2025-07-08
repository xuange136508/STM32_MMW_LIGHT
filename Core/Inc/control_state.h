#ifndef __CONTROL_STATE_H__
#define __CONTROL_STATE_H__

#ifdef __cplusplus
extern "C" {
#endif

#include "stdint.h"

/**
  * @brief 按钮控制状态结构体
  */
typedef struct {
    uint8_t breathing_led_enabled;     // 呼吸灯使能状态
    uint8_t rgb_led_enabled;           // RGB彩灯使能状态
    uint16_t led_brightness;           // LED亮度 (0-100)
} ControlState_t;

/**
  * @brief 全局控制状态变量声明
  */
extern volatile ControlState_t g_control_state;

/**
  * @brief LED控制函数声明
  */
void Set_LED_Brightness(uint16_t brightness);

#ifdef __cplusplus
}
#endif

#endif /* __CONTROL_STATE_H__ */ 