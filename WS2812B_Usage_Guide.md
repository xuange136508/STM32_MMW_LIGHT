# STM32F407 WS2812B RGB彩灯驱动完整指南

## 概述

本驱动为STM32F407提供完整的WS2812B RGB彩灯控制功能，包含GPIO、TIM4、DMA的完整配置。

## 硬件连接

### 引脚配置
- **PD13** → WS2812B的DIN引脚 (TIM4_CH2)
- **5V** → WS2812B的VCC电源
- **GND** → WS2812B的地线

### 硬件要求
- STM32F407系列微控制器
- WS2812B RGB LED灯珠
- 5V电源（支持足够电流，每个LED约60mA）

## 技术配置

### 定时器配置
- **定时器**: TIM4
- **通道**: CH2 (对应PD13)
- **时钟频率**: 84MHz (不分频)
- **PWM周期**: 105个时钟周期 = 1.25μs
- **逻辑0**: 33个时钟周期 ≈ 0.4μs高电平
- **逻辑1**: 67个时钟周期 ≈ 0.8μs高电平

### DMA配置
- **DMA控制器**: DMA1
- **数据流**: Stream3
- **通道**: Channel2
- **方向**: 内存到外设
- **模式**: 正常模式

## API函数说明

### 初始化函数

```c
void WS2812B_Init(void);
```
- 功能：初始化WS2812B驱动（包含GPIO、TIM4、DMA配置）
- 参数：无
- 返回：无

### 颜色控制函数

```c
void WS2812B_SetColor(uint16_t index, uint8_t r, uint8_t g, uint8_t b);
```
- 功能：设置指定LED的RGB颜色
- 参数：
  - `index`: LED索引（0到WS2812B_LED_COUNT-1）
  - `r`: 红色值（0-255）
  - `g`: 绿色值（0-255）
  - `b`: 蓝色值（0-255）

```c
void WS2812B_SetColorEnum(uint16_t index, WS2812B_ColorEnum_t color);
```
- 功能：使用预定义颜色设置LED
- 参数：
  - `index`: LED索引
  - `color`: 预定义颜色枚举

### 显示更新函数

```c
void WS2812B_Update(void);
```
- 功能：更新LED显示（将颜色数据发送到WS2812B）
- 参数：无
- 返回：无

```c
void WS2812B_Clear(void);
```
- 功能：清空所有LED（设置为黑色并更新）
- 参数：无
- 返回：无

### 亮度控制函数

```c
void WS2812B_SetBrightness(uint8_t brightness);
```
- 功能：设置全局亮度
- 参数：`brightness`: 亮度值（0-255）
- 返回：无

### 测试函数

```c
void WS2812B_Test_Rainbow(void);
void WS2812B_Test_Breathing(void);
```
- 功能：运行彩虹效果和呼吸灯效果测试

## 预定义颜色

```c
typedef enum {
    WS2812B_BLACK = 0,    // 黑色（关闭）
    WS2812B_RED,          // 红色
    WS2812B_GREEN,        // 绿色
    WS2812B_BLUE,         // 蓝色
    WS2812B_YELLOW,       // 黄色
    WS2812B_MAGENTA,      // 洋红
    WS2812B_CYAN,         // 青色
    WS2812B_WHITE,        // 白色
    WS2812B_ORANGE,       // 橙色
    WS2812B_PURPLE        // 紫色
} WS2812B_ColorEnum_t;
```

## 使用示例

### 基础使用

```c
#include "ws2812b.h"

int main(void)
{
    // 系统初始化...
    HAL_Init();
    SystemClock_Config();
    
    // 初始化WS2812B
    WS2812B_Init();
    
    // 设置LED颜色
    WS2812B_SetColor(0, 255, 0, 0);        // LED0设为红色
    WS2812B_SetColorEnum(1, WS2812B_BLUE); // LED1设为蓝色
    
    // 更新显示
    WS2812B_Update();
    
    while(1)
    {
        // 主循环
    }
}
```

### 动态效果示例

```c
void LED_Effects_Demo(void)
{
    // 彩虹效果
    WS2812B_Test_Rainbow();
    
    // 呼吸灯效果
    WS2812B_Test_Breathing();
    
    // 自定义流水灯
    for(int i = 0; i < 10; i++) {
        WS2812B_Clear();
        WS2812B_SetColorEnum(i % WS2812B_LED_COUNT, WS2812B_WHITE);
        WS2812B_Update();
        HAL_Delay(200);
    }
}
```

### 亮度控制示例

```c
void Brightness_Demo(void)
{
    // 设置红色
    WS2812B_SetColor(0, 255, 0, 0);
    WS2812B_SetColor(1, 255, 0, 0);
    
    // 逐渐变亮
    for(uint8_t brightness = 0; brightness <= 255; brightness += 10) {
        WS2812B_SetBrightness(brightness);
        WS2812B_Update();
        HAL_Delay(100);
    }
    
    // 逐渐变暗
    for(uint8_t brightness = 255; brightness > 0; brightness -= 10) {
        WS2812B_SetBrightness(brightness);
        WS2812B_Update();
        HAL_Delay(100);
    }
}
```

## 配置参数

### 可修改的宏定义

在`ws2812b.h`中可以修改以下参数：

```c
#define WS2812B_LED_COUNT       2       // LED数量（根据实际使用修改）
#define WS2812B_RESET_PULSE     50      // 复位脉冲数量
#define WS2812B_TIMER_PERIOD    104     // 定时器周期
#define WS2812B_0_CODE          33      // 0码脉宽
#define WS2812B_1_CODE          67      // 1码脉宽
```

## 注意事项

### 电源要求
1. **电压**: WS2812B需要5V电源
2. **电流**: 每个LED最大60mA，确保电源容量足够
3. **稳定性**: 建议使用电源滤波电容

### 信号完整性
1. **电平**: STM32输出3.3V，建议在信号线串联330Ω电阻
2. **线长**: 信号线应尽量短，避免干扰
3. **接地**: 确保良好的地线连接

### 时序要求
1. **精确性**: 避免在DMA传输期间进行高优先级中断
2. **复位**: 数据传输后需要至少50μs的低电平复位信号

## 故障排查

### LED不亮
1. 检查5V电源是否正常
2. 检查PD13信号线连接
3. 验证GPIO复用功能配置
4. 检查DMA传输是否成功

### 颜色错误
1. 确认RGB顺序（WS2812B使用GRB顺序）
2. 检查时序参数配置
3. 验证亮度设置

### 部分LED不工作
1. 检查电源容量
2. 验证LED级联连接
3. 检查是否有损坏的LED

## 扩展功能

基于此驱动可以实现：
- 更多LED数量支持（修改WS2812B_LED_COUNT）
- 音乐律动效果
- 温度显示效果
- 网络控制功能
- 保存/恢复颜色配置

## 文件结构

```
Core/
├── Inc/
│   └── ws2812b.h          # WS2812B驱动头文件
└── Src/
    ├── ws2812b.c          # WS2812B驱动源文件
    └── main.c             # 主程序（包含测试代码）
```

## 版本信息

- 版本：v1.0
- 适用：STM32F407系列
- 测试：STM32F407VET6
- 编译器：ARM GCC 