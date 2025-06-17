#include "dht11.h"

// DHT11数据引脚定义 (PD10)
#define DHT11_GPIO_Port GPIOD
#define DHT11_Pin GPIO_PIN_10

/**
 * @brief 设置DHT11引脚为输出模式
 */
void DHT11_SetPinOutput(void)
{
    GPIO_InitTypeDef GPIO_InitStruct = {0};
    GPIO_InitStruct.Pin = DHT11_Pin;
    GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
    GPIO_InitStruct.Pull = GPIO_PULLUP;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_HIGH;
    HAL_GPIO_Init(DHT11_GPIO_Port, &GPIO_InitStruct);
}

/**
 * @brief 设置DHT11引脚为输入模式
 */
void DHT11_SetPinInput(void)
{
    GPIO_InitTypeDef GPIO_InitStruct = {0};
    GPIO_InitStruct.Pin = DHT11_Pin;
    GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
    GPIO_InitStruct.Pull = GPIO_PULLUP;
    HAL_GPIO_Init(DHT11_GPIO_Port, &GPIO_InitStruct);
}

/**
 * @brief 设置DHT11引脚电平
 */
void DHT11_SetPin(GPIO_PinState state)
{
    HAL_GPIO_WritePin(DHT11_GPIO_Port, DHT11_Pin, state);
}

/**
 * @brief 读取DHT11引脚电平
 */
uint8_t DHT11_ReadPin(void)
{
    return HAL_GPIO_ReadPin(DHT11_GPIO_Port, DHT11_Pin);
}

/**
 * @brief 微秒延时函数
 * @param us 延时微秒数
 */
void DHT11_DelayUs(uint32_t us)
{
    uint32_t start = DWT->CYCCNT;
    uint32_t cycles = us * (SystemCoreClock / 1000000);
    while ((DWT->CYCCNT - start) < cycles);
}

/**
 * @brief DHT11启动信号
 * @return 1-成功, 0-失败
 */
uint8_t DHT11_Start(void)
{
    uint8_t retry = 0;
    
    // 设置为输出模式
    DHT11_SetPinOutput();
    
    // 发送启动信号
    DHT11_SetPin(GPIO_PIN_RESET);  // 拉低18ms
    HAL_Delay(18);
    DHT11_SetPin(GPIO_PIN_SET);    // 拉高
    DHT11_DelayUs(30);             // 延时30us
    
    // 设置为输入模式
    DHT11_SetPinInput();
    
    // 等待DHT11响应
    while(DHT11_ReadPin() && retry < 100) {
        retry++;
        DHT11_DelayUs(1);
    }
    
    if(retry >= 100) return 0;  // 超时，启动失败
    
    retry = 0;
    
    // 等待DHT11拉高
    while(!DHT11_ReadPin() && retry < 100) {
        retry++;
        DHT11_DelayUs(1);
    }
    
    if(retry >= 100) return 0;  // 超时，启动失败
    
    return 1;  // 启动成功
}

/**
 * @brief 读取一个字节数据
 * @return 读取到的字节数据
 */
uint8_t DHT11_ReadByte(void)
{
    uint8_t data = 0;
    uint8_t retry;
    
    for(int i = 0; i < 8; i++) {
        retry = 0;
        
        // 等待变为高电平
        while(!DHT11_ReadPin() && retry < 100) {
            retry++;
            DHT11_DelayUs(1);
        }
        
        DHT11_DelayUs(40);  // 延时40us
        
        if(DHT11_ReadPin()) {
            data |= (1 << (7-i));  // 如果还是高电平，则数据位为1
        }
        
        retry = 0;
        // 等待变为低电平
        while(DHT11_ReadPin() && retry < 100) {
            retry++;
            DHT11_DelayUs(1);
        }
    }
    
    return data;
}

/**
 * @brief 读取DHT11数据
 * @param data DHT11数据结构指针
 * @return 1-成功, 0-失败
 */
uint8_t DHT11_ReadData(DHT11_Data_t *data)
{
    uint8_t checksum_calc;
    
    // 启动DHT11
    if(!DHT11_Start()) {
        return 0;  // 启动失败
    }
    
    // 读取5个字节数据
    data->humidity_int = DHT11_ReadByte();
    data->humidity_dec = DHT11_ReadByte();
    data->temperature_int = DHT11_ReadByte();
    data->temperature_dec = DHT11_ReadByte();
    data->checksum = DHT11_ReadByte();
    
    // 校验数据
    checksum_calc = data->humidity_int + data->humidity_dec + 
                   data->temperature_int + data->temperature_dec;
    
    if(checksum_calc == data->checksum) {
        return 1;  // 数据有效
    } else {
        return 0;  // 校验失败
    }
} 