#ifndef __LCD_INIT_H
#define __LCD_INIT_H

#include "main.h"

#define USE_HORIZONTAL 0  //���ú�������������ʾ 0��1Ϊ���� 2��3Ϊ����


#if USE_HORIZONTAL==0||USE_HORIZONTAL==1
#define LCD_W 240
#define LCD_H 280

#else
#define LCD_W 280
#define LCD_H 240
#endif

// ������ť���򣨷������½ǣ�
#define CLEAR_BTN_X1 (LCD_WIDTH - 60)
#define CLEAR_BTN_Y1 (LCD_HEIGHT - 40)
#define CLEAR_BTN_X2 (LCD_WIDTH - 10)
#define CLEAR_BTN_Y2 (LCD_HEIGHT - 10)



//-----------------LCD�˿ڶ���---------------- 

#define LCD_RES_Clr()  HAL_GPIO_WritePin(GPIOE, GPIO_PIN_13, GPIO_PIN_RESET)  // RES
#define LCD_RES_Set()  HAL_GPIO_WritePin(GPIOE, GPIO_PIN_13, GPIO_PIN_SET)

#define LCD_DC_Clr()   HAL_GPIO_WritePin(GPIOB, GPIO_PIN_15, GPIO_PIN_RESET)  // DC
#define LCD_DC_Set()   HAL_GPIO_WritePin(GPIOB, GPIO_PIN_15, GPIO_PIN_SET)
 		     
#define LCD_CS_Clr()   HAL_GPIO_WritePin(GPIOC, GPIO_PIN_5, GPIO_PIN_RESET)  // CS
#define LCD_CS_Set()   HAL_GPIO_WritePin(GPIOC, GPIO_PIN_5, GPIO_PIN_SET)

#define LCD_BLK_Clr()  HAL_GPIO_WritePin(GPIOB, GPIO_PIN_14, GPIO_PIN_RESET)  // BLK
#define LCD_BLK_Set()  HAL_GPIO_WritePin(GPIOB, GPIO_PIN_14, GPIO_PIN_SET)

void LCD_Writ_Bus(uint8_t dat);//ģ��SPIʱ��(DMA方式)
void LCD_Writ_Bus_DMA(uint8_t *pData, uint16_t Size);//DMA批量数据传输
void LCD_WR_DATA8(uint8_t dat);//д��һ���ֽ�
void LCD_WR_DATA(uint16_t dat);//д�������ֽ�
void LCD_WR_REG(uint8_t dat);//д��һ��ָ��
void LCD_Address_Set(uint16_t x1,uint16_t y1,uint16_t x2,uint16_t y2);//�������꺯��
void LCD_Init(void);//LCD��ʼ��

// LCD显示控制函数
void LCD_Display_Off(void);//LCD息屏
void LCD_Display_On(void);//LCD亮屏
void LCD_Display_On_Full_Init(void);//LCD亮屏（完全重新初始化版本）

// DMA传输相关
extern volatile uint8_t spi_dma_tx_complete;
#endif




