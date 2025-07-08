# STM32_MMW_LIGHT

## 项目简介

STM32_MMW_LIGHT 是一个基于 STM32F407VETx 微控制器的多媒体控制项目，集成了 LCD 显示、LED 灯光控制、触摸交互和 ESP32 通信等功能。项目采用 FreeRTOS 实时操作系统，支持多任务并发处理，提供丰富的硬件控制接口和通信协议。

## 硬件规格

### 主控制器
- **MCU**: STM32F407VETx (ARM Cortex-M4F, 168MHz)
- **Flash**: 512KB
- **RAM**: 192KB
- **封装**: LQFP100

### 主要外设
- **LCD 显示屏**: 支持背光控制和睡眠模式
- **触摸控制器**: CST816 电容触摸芯片
- **LED 控制**: 支持呼吸灯效果和亮度调节 (0-100%)
- **WS2812B**: RGB LED 灯条控制
- **通信接口**: 
  - USART2: 命令控制接口
  - USART3: ESP32 通信接口
- **ADC**: 模拟信号采集
- **定时器**: PWM 输出和延时控制

## 软件架构

### 操作系统
- **FreeRTOS**: 实时多任务操作系统
- **LVGL v8.3**: 轻量级图形用户界面库
- **HAL 库**: STM32F4xx HAL 驱动库

### 核心功能模块

#### 1. LED 控制系统
- **亮度控制**: 0-100% 可调亮度，支持线性插值计算
- **呼吸灯效果**: 基于 PWM 的平滑呼吸灯动画
- **实时调节**: 支持通过命令实时调整亮度

```c
// LED 亮度控制结构
typedef struct {
    uint16_t rgb_led_r;      // RGB 红色通道
    uint16_t rgb_led_g;      // RGB 绿色通道  
    uint16_t rgb_led_b;      // RGB 蓝色通道
    uint16_t led_brightness; // LED 亮度 (0-100)
} ControlState_t;
```

#### 2. LCD 显示管理
- **显示控制**: 支持屏幕开启/关闭
- **背光管理**: 独立背光控制
- **睡眠模式**: 低功耗睡眠和唤醒
- **LVGL 集成**: 支持复杂 GUI 界面

#### 3. 通信协议

##### UART 命令系统 (USART2)
```
0x01 - CMD_MUTE      (静音)
0x02 - CMD_UNMUTE    (取消静音)
0x03 - CMD_VOL_UP    (音量增加)
0x04 - CMD_VOL_DOWN  (音量减少)
0x05 - CMD_VOL_MAX   (最大音量)
0x06 - CMD_LIGHT_UP  (亮度增加 +20)
0x07 - CMD_LIGHT_DOWN(亮度减少 -20)
0x08 - CMD_SCREEN_OFF(屏幕关闭)
0x09 - CMD_SCREEN_ON (屏幕开启)
```

##### ESP32 通信 (USART3)
- **协议**: JSON 格式数据交换
- **功能**: 双向通信，状态同步
- **数据字段**:
  - `rgb_led`: RGB 颜色控制
  - `led_brightness`: LED 亮度控制 (-1=忽略, 0-100=设置)
  - `screen_control`: 屏幕控制 ("screen_off"/"screen_on")
  - `play_status`: 播放状态信息

```json
{
  "rgb_led": {"r": 255, "g": 128, "b": 0},
  "led_brightness": 50,
  "screen_control": "screen_on"
}
```

### 任务架构
- **呼吸灯任务**: 独立 FreeRTOS 任务处理 LED 动画
- **触摸处理任务**: CST816 触摸事件处理
- **UART 接收任务**: 命令解析和响应
- **ESP32 通信任务**: JSON 数据处理和状态同步

## 项目特性

### ✅ 已实现功能
- [x] LED 亮度控制 (0-100%)
- [x] RGB LED 颜色控制
- [x] LCD 屏幕开关控制
- [x] 触摸交互支持
- [x] UART 命令处理
- [x] ESP32 双向通信
- [x] WS2812B LED 灯条控制
- [x] FreeRTOS 多任务管理
- [x] LVGL 图形界面支持

### 🔧 技术亮点
- **高精度 PWM**: 支持 0.5% 精度的亮度调节
- **平滑动画**: 基于数学插值的呼吸灯效果
- **实时通信**: 低延迟 ESP32 状态同步
- **模块化设计**: 清晰的代码架构和接口定义
- **错误处理**: 完善的边界检查和异常处理

## 文件结构

```
STM32_MMW_LIGHT/
├── Core/
│   ├── Inc/              # 头文件
│   │   ├── control_state.h    # 控制状态定义
│   │   ├── lcd_init.h         # LCD 初始化
│   │   ├── CST816.h          # 触摸控制器
│   │   └── ...
│   └── Src/              # 源文件
│       ├── freertos.c         # FreeRTOS 任务
│       ├── lcd_init.c         # LCD 驱动
│       ├── main.c             # 主程序
│       └── ...
├── Drivers/              # STM32 HAL 驱动
├── Middlewares/          # 中间件
│   ├── LVGL/             # 图形界面库
│   └── Third_Party/      # 第三方库
├── EIDE/                 # EIDE 工程文件
├── MDK-ARM/              # Keil 工程文件
└── README.md
```

## 开发环境

### 推荐工具链
- **IDE**: STM32CubeIDE / Keil MDK-ARM / EIDE
- **调试器**: ST-LINK V2/V3
- **编译器**: ARM GCC / ARMCC

### 依赖库
- STM32F4xx HAL Library
- FreeRTOS Kernel
- LVGL v8.3+
- CMSIS DSP Library

## 编译和烧录

### 1. 环境准备
```bash
# 安装 STM32CubeIDE 或配置 ARM GCC 工具链
# 连接 ST-LINK 调试器
```

### 2. 编译项目
```bash
# 使用 STM32CubeIDE
File -> Import -> Existing Projects into Workspace

# 或使用命令行 (需配置 Makefile)
make clean && make
```

### 3. 烧录程序
```bash
# 使用 ST-LINK Utility 或 STM32CubeProgrammer
# 或通过 IDE 直接 Debug/Run
```

## 配置说明

### 系统配置
- **系统时钟**: 168MHz (HSE + PLL)
- **堆栈大小**: 0x400 (1KB)
- **堆大小**: 0x200 (512B)

### 外设配置
- **USART2**: 115200 8N1 (命令接口)
- **USART3**: 115200 8N1 (ESP32 通信)
- **SPI1**: LCD 通信接口
- **I2C1**: 触摸控制器接口
- **TIM3**: PWM 输出 (LED 控制)

## 使用示例

### LED 亮度控制
```c
// 设置 LED 亮度为 50%
Set_LED_Brightness(50);

// 亮度增加
g_control_state.led_brightness = MIN(100, g_control_state.led_brightness + 20);
```

### 屏幕控制
```c
// 关闭屏幕
LCD_Display_Off();

// 开启屏幕
LCD_Display_On();
```

### ESP32 通信
```c
// 发送状态到 ESP32
ESP32_BuildPlayStatusJSON();
ESP32_SendJSON();
```

## 故障排除

### 常见问题
1. **编译错误**: 检查包含路径和库依赖
2. **烧录失败**: 确认 ST-LINK 连接和驱动
3. **屏幕不显示**: 检查 SPI 连接和电源
4. **触摸无响应**: 验证 I2C 通信和中断配置

### 调试技巧
- 使用 UART 输出调试信息
- 监控 FreeRTOS 任务状态
- 检查中断优先级配置

## 贡献指南

1. Fork 本项目
2. 创建功能分支 (`git checkout -b feature/AmazingFeature`)
3. 提交更改 (`git commit -m 'Add some AmazingFeature'`)
4. 推送分支 (`git push origin feature/AmazingFeature`)
5. 创建 Pull Request

## 许可证

本项目基于多个开源库构建，请参考各组件的许可证:
- STM32 HAL Library: BSD 3-Clause License
- FreeRTOS: MIT License  
- LVGL: MIT License

## 联系方式

- 项目维护者: [您的姓名]
- 邮箱: [您的邮箱]
- 项目地址: [GitHub 链接]

---

**注意**: 本项目为嵌入式开发项目，需要相应的硬件平台支持。使用前请确保硬件连接正确并了解相关安全事项。
