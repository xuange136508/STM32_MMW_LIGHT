# USART2 16进制命令接收指南

## 硬件配置

**USART2引脚连接:**
- PA2: USART2_TX 
- PA3: USART2_RX
- 波特率: 115200
- 数据位: 8
- 停止位: 1
- 校验位: 无

## 支持的16进制命令

| 16进制命令 | 功能描述   | 中文说明     | 实际功能 |
|-----------|-----------|-------------|----------|
| 0xAA      | wakeup_uni | 你好小盛     | 系统唤醒 |
| 0xA3      | TurnOn     | 打开夜灯     | **开启呼吸灯** |
| 0xA4      | TurnOff    | 关闭夜灯     | **关闭呼吸灯** |
| 0xA1      | chatOn     | 开启聊天     |
| 0xA2      | chatOff    | 关闭聊天     |
| 0xA5      | play       | 播放胎教     | **发送播放状态到ESP32** |
| 0xA6      | play_background | 播放白噪音 | **发送播放状态到ESP32** |
| 0xA7      | pause      | 暂停播放     | **发送暂停状态到ESP32** |
| 0xA8      | stop       | 停止播放     | **发送停止状态到ESP32** |

### B系列命令（屏幕和音频控制）

| 16进制命令 | 功能描述      | 中文说明         | 实际功能 |
|-----------|-------------|-----------------|----------|
| 0xB1      | screen_off  | 关闭屏幕         | 屏幕控制 |
| 0xB2      | screen_on   | 亮屏            | 屏幕控制 |
| 0xB3      | light_up    | 夜灯调亮         | 夜灯亮度控制 |
| 0xB4      | light_down  | 夜灯调暗         | 夜灯亮度控制 |
| 0xB5      | mute        | 静音            | 音频控制 |
| 0xB6      | unmute      | 取消静音         | 音频控制 |
| 0xB7      | vol_up      | 声音调大         | 音量控制 |
| 0xB8      | vol_down    | 声音调小         | 音量控制 |
| 0xB9      | vol_max     | 声音调到最大      | 音量控制 |

### C系列命令（时间和记录控制）

| 16进制命令 | 功能描述      | 中文说明         | 实际功能 |
|-----------|-------------|-----------------|----------|
| 0xC1      | timer_stop  | 定时停止         | 定时器控制 |
| 0xC2      | prev_song   | 上一首          | 播放控制 |
| 0xC3      | next_song   | 下一首          | 播放控制 |
| 0xC4      | start_fetal | 开始记胎动       | 胎动记录 |
| 0xC5      | start_record| 开始记录         | 数据记录 |
| 0xC6      | start_sleep | 开始记录宝睡眠    | 睡眠记录 |
| 0xC7      | end_fetal   | 结束记胎动       | 胎动记录 |
| 0xC8      | end_record  | 结束记录         | 数据记录 |
| 0xC9      | end_sleep   | 结束记录宝睡眠    | 睡眠记录 |

## 工作原理

1. **中断驱动**: 每接收到一个字节立即触发中断处理
2. **实时处理**: 不需要等待换行符，收到命令立即解析执行
3. **自动识别**: 根据16进制值自动匹配对应的功能
4. **错误处理**: 未知命令会输出警告信息
5. **ESP32通信**: 播放相关命令会自动发送状态到ESP32

## ESP32播放状态JSON格式

当接收到播放相关命令时，会自动生成并发送JSON状态到ESP32：

### 播放胎教 (0xA5)
```json
{
  "device": "STM32_PlayControl",
  "timestamp": 12345678,
  "playback": {
    "status": "play",
    "content_type": "prenatal_education",
    "command_source": "USART2"
  },
  "controls": {
    "breathing_led": true,
    "rgb_led": false
  }
}
```

### 播放白噪音 (0xA6)
```json
{
  "device": "STM32_PlayControl",
  "timestamp": 12345678,
  "playback": {
    "status": "play",
    "content_type": "white_noise",
    "command_source": "USART2"
  },
  "controls": {
    "breathing_led": true,
    "rgb_led": false
  }
}
```

### 暂停/停止播放 (0xA7/0xA8)
```json
{
  "device": "STM32_PlayControl",
  "timestamp": 12345678,
  "playback": {
    "status": "pause", // 或 "stop"
    "content_type": "unknown",
    "command_source": "USART2"
  },
  "controls": {
    "breathing_led": true,
    "rgb_led": false
  }
}
```

## 调试输出示例

### 控制命令 (0xA3)
当接收到0xA3命令时，串口1会输出：

```
USART2接收到16进制: 0xA3
处理命令: 0xA3 - 打开夜灯
执行: 打开夜灯
USART2接收16进制命令: 0xA3 (打开夜灯)
USART2任务记录命令: 0xA3
```

### 播放命令 (0xA5)
当接收到0xA5命令时，串口1会输出：

```
USART2接收到16进制: 0xA5
处理命令: 0xA5 - 播放胎教
执行: 播放胎教
ESP32发送JSON: {"device":"STM32_PlayControl","timestamp":12345678...}
已发送播放胎教状态到ESP32
USART2接收16进制命令: 0xA5 (播放胎教)
USART2任务记录命令: 0xA5
```

## 使用方法

### 1. 自动运行
- 系统启动后自动初始化USART2
- 接收到16进制命令立即处理
- 无需额外配置

### 2. 测试命令
使用串口调试助手发送以下16进制数据进行测试：

**A系列命令：**
- 发送 `AA` (16进制) → 你好小盛
- 发送 `A3` (16进制) → 打开夜灯
- 发送 `A4` (16进制) → 关闭夜灯
- 发送 `A5` (16进制) → 播放胎教
- 发送 `A6` (16进制) → 播放白噪音

**B系列命令：**
- 发送 `B1` (16进制) → 关闭屏幕
- 发送 `B2` (16进制) → 亮屏
- 发送 `B3` (16进制) → 夜灯调亮
- 发送 `B5` (16进制) → 静音
- 发送 `B7` (16进制) → 声音调大

**C系列命令：**
- 发送 `C1` (16进制) → 定时停止
- 发送 `C2` (16进制) → 上一首
- 发送 `C3` (16进制) → 下一首
- 发送 `C4` (16进制) → 开始记胎动
- 发送 `C7` (16进制) → 结束记胎动

### 3. 添加自定义处理
在 `USART2_ProcessHexCommand()` 函数中的对应case分支添加你的处理逻辑：

```c
case CMD_TURN_ON:
    printf("执行: 打开夜灯\r\n");
    // 在这里添加打开夜灯的处理逻辑
    // 例如：控制GPIO、发送其他命令等
    break;
```

## API函数

### 命令处理函数
```c
void USART2_ProcessHexCommand(uint8_t cmd);
```

### 获取命令名称
```c
const char* USART2_GetCommandName(uint8_t cmd);
```

### 初始化函数
```c
void USART2_Init(void);
```

### ESP32通信函数
```c
// 构建播放状态JSON
void ESP32_BuildPlayStatusJSON(char* json_buffer, uint16_t buffer_size, 
                               const char* play_status, const char* content_type);

// 发送JSON到ESP32
bool ESP32_SendJSON(const char* json_string);
```

## 特性

- **实时响应**: 收到命令立即处理，无延迟
- **中断驱动**: 不阻塞其他任务
- **自动识别**: 支持27个命令自动解析
- **错误容错**: 未知命令不会导致系统异常
- **调试友好**: 详细的串口输出日志
- **分类管理**: A/B/C系列命令分类管理

## 注意事项

1. **数据格式**: 发送纯16进制数据，不需要前缀（如0x）
2. **单字节命令**: 每个命令占用1个字节
3. **连续发送**: 可以连续发送多个命令，每个都会被独立处理
4. **大小写**: 16进制值不区分大小写（A3和a3效果相同）

## 扩展功能

### 添加新命令
1. 在 `usart.h` 中定义新的命令宏：
```c
#define CMD_NEW_COMMAND  0xD1  // 新命令（使用D系列）
```

2. 在 `USART2_ProcessHexCommand()` 中添加处理：
```c
case CMD_NEW_COMMAND:
    printf("执行: 新命令\r\n");
    // 添加处理逻辑
    break;
```

3. 在 `USART2_GetCommandName()` 中添加名称：
```c
case CMD_NEW_COMMAND: return "新命令";
```

### 命令分类建议
- **A系列 (0xA1-0xAF)**: 基础功能和播放控制
- **B系列 (0xB1-0xBF)**: 屏幕和音频控制  
- **C系列 (0xC1-0xCF)**: 时间和记录控制
- **D系列 (0xD1-0xDF)**: 扩展功能（预留）

### 当前支持的27个命令
```
A系列: AA, A1-A8 (9个命令)
B系列: B1-B9 (9个命令)  
C系列: C1-C9 (9个命令)
总计: 27个命令
```

这样就完成了新命令的添加！ 