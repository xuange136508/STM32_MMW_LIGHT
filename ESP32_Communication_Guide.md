# STM32与ESP32串口通信使用指南

## 硬件连接

STM32F407 与 ESP32 的串口连接：

```
STM32F407     ESP32
PB10 (TX) --> GPIO18 (RX)
PB11 (RX) <-- GPIO17 (TX)
GND       --- GND
```

## 通信协议

- **波特率**: 115200
- **数据位**: 8
- **停止位**: 1
- **校验位**: 无
- **流控**: 无
- **数据格式**: JSON字符串 + \r\n

## JSON消息格式

### 1. STM32发送传感器数据

```json
{
    "device": "STM32_Sensor",
    "timestamp": 123456,
    "sensors": {
        "light_voltage": 1.65,
        "vibration": false,
        "touch": true,
        "temperature": 25.3,
        "humidity": 60.2,
        "dht11_valid": true
    },
    "controls": {
        "breathing_led": true,
        "rgb_led": false
    }
}
```

### 2. STM32发送控制状态

```json
{
    "device": "STM32_Control",
    "timestamp": 123456,
    "controls": {
        "breathing_led": true,
        "rgb_led": false
    }
}
```

### 3. ESP32发送控制命令

```json
{
    "device": "ESP32",
    "command": "control",
    "breathing_led": "on",
    "rgb_led": "off"
}
```

## 功能特性

1. **自动发送**: STM32每10秒自动发送完整传感器数据，每5秒发送控制状态
2. **实时接收**: ESP32可以随时发送控制命令给STM32
3. **中断处理**: 使用UART中断方式接收数据，不会阻塞其他任务
4. **错误处理**: 包含通信错误检测和恢复机制

## API函数说明

### ESP32通信初始化
```c
void ESP32_Init(void);
```

### 发送JSON数据
```c
bool ESP32_SendJSON(const char* json_string);
```

### 接收JSON数据
```c
bool ESP32_ReceiveJSON(char* json_buffer, uint16_t buffer_size);
```

### 处理接收数据
```c
void ESP32_ProcessReceivedData(void);
```

### 构建传感器JSON
```c
void ESP32_BuildSensorJSON(char* json_buffer, uint16_t buffer_size);
```

### 构建控制JSON
```c
void ESP32_BuildControlJSON(char* json_buffer, uint16_t buffer_size);
```

## 使用示例

### 手动发送数据
```c
char json_buffer[512];
ESP32_BuildSensorJSON(json_buffer, sizeof(json_buffer));
if(ESP32_SendJSON(json_buffer)) {
    printf("数据发送成功\r\n");
}
```

### 手动处理接收数据
```c
// 在主循环或定时器中调用
ESP32_ProcessReceivedData();
```

## ESP32端示例代码

```cpp
// ESP32 Arduino代码示例
void setup() {
    Serial.begin(115200);
    Serial2.begin(115200, SERIAL_8N1, 18, 17); // RX=18, TX=17
}

void loop() {
    // 接收STM32数据
    if (Serial2.available()) {
        String jsonData = Serial2.readStringUntil('\n');
        Serial.println("接收到STM32数据: " + jsonData);
        // 解析JSON数据...
    }
    
    // 发送控制命令示例
    static unsigned long lastSend = 0;
    if (millis() - lastSend > 15000) { // 每15秒发送一次
        String cmd = "{\"device\":\"ESP32\",\"command\":\"control\",\"breathing_led\":\"on\",\"rgb_led\":\"off\"}\r\n";
        Serial2.print(cmd);
        Serial.println("发送控制命令: " + cmd);
        lastSend = millis();
    }
    
    delay(100);
}
```

## 调试信息

STM32会通过USART1（调试串口）输出详细的通信日志：

```
ESP32 通信初始化...
ESP32 通信初始化完成
ESP32通信任务启动
发送JSON到ESP32成功: {"device":"STM32_Sensor"...}
定时发送传感器数据到ESP32
从ESP32接收JSON: {"device":"ESP32"...}
ESP32命令: 开启呼吸灯
```

## 注意事项

1. 确保硬件连接正确，特别是TX/RX交叉连接
2. 两端波特率必须一致
3. JSON字符串长度不要超过缓冲区大小（512字节）
4. ESP32发送的命令必须以\r\n结尾
5. 可以通过串口调试助手测试通信功能 