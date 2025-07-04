#include "lcd_init.h"
#include "delay.h"
#include "spi.h"

/* DMA transmission complete flag */
volatile uint8_t spi_dma_tx_complete = 1;

/******************************************************************************
      函数说明：LCD串行数据写入函数（DMA方式）
      入口数据：dat  要写入的串行数据
      返回值：  无
******************************************************************************/
void LCD_Writ_Bus(uint8_t dat) 
{    
// 【没有dma之前的代码】
// 	LCD_CS_Clr();
//    // 使用HAL库的SPI发送函数
//     HAL_SPI_Transmit(&hspi1, &dat, 1, 1000);
// 	//HAL_SPI_Transmit_DMA(&hspi1, &dat, 1); // 注意这里DMA的方式驱动，注释掉就点亮屏幕了
// 	//	delay_us(1);
// 	//    while(HAL_SPI_GetState(&hspi1) != HAL_SPI_STATE_READY);
//     // 等待传输完成（HAL_SPI_Transmit已经包含等待）
// 	LCD_CS_Set();

	LCD_CS_Clr();
    
    // 使用DMA发送数据
    spi_dma_tx_complete = 0;
    if(HAL_SPI_Transmit_DMA(&hspi1, &dat, 1) != HAL_OK)
    {
        // 如果DMA传输失败，回退到阻塞方式
        HAL_SPI_Transmit(&hspi1, &dat, 1, 1000);
        spi_dma_tx_complete = 1;
    }
    else
    {
        // 等待DMA传输完成
        while(spi_dma_tx_complete == 0)
        {
            // 可以在这里添加超时检查
        }
    }
    
	LCD_CS_Set();
}

/******************************************************************************
      函数说明：LCD DMA 批量数据写入函数
      入口数据：pData 数据指针，Size 数据长度
      返回值：  无
******************************************************************************/
void LCD_Writ_Bus_DMA(uint8_t *pData, uint16_t Size)
{
    LCD_CS_Clr();
    
    // 使用DMA发送数据
    spi_dma_tx_complete = 0;
    if(HAL_SPI_Transmit_DMA(&hspi1, pData, Size) != HAL_OK)
    {
        // 如果DMA传输失败，回退到阻塞方式
        HAL_SPI_Transmit(&hspi1, pData, Size, 1000);
        spi_dma_tx_complete = 1;
    }
    else
    {
        // 等待DMA传输完成
        while(spi_dma_tx_complete == 0)
        {
            // 可以在这里添加超时检查
        }
    }
    
    LCD_CS_Set();
}

/******************************************************************************
      函数说明：LCD写入数据
      入口数据：dat 写入的数据
      返回值：  无
******************************************************************************/
void LCD_WR_DATA8(uint8_t dat)
{
    LCD_Writ_Bus(dat);
}


/******************************************************************************
      函数说明：LCD写入数据
      入口数据：dat 写入的数据
      返回值：  无
******************************************************************************/
void LCD_WR_DATA(uint16_t dat)
{

	LCD_Writ_Bus(dat>>8);
	LCD_Writ_Bus(dat);
}


/******************************************************************************
      函数说明：LCD写入命令
      入口数据：dat 写入的命令
      返回值：  无
******************************************************************************/
void LCD_WR_REG(uint8_t dat)
{

    LCD_DC_Clr(); // 写命令
    LCD_Writ_Bus(dat);
	LCD_DC_Set();
}


/******************************************************************************
      函数说明：设置起始和结束地址
      入口数据：x1,x2 设置列的起始和结束地址
                y1,y2 设置行的起始和结束地址
      返回值：  无
******************************************************************************/
void LCD_Address_Set(uint16_t x1, uint16_t y1, uint16_t x2, uint16_t y2)
{
	if (USE_HORIZONTAL == 0)
	{
		LCD_WR_REG(0x2a); //列地址设置
		LCD_WR_DATA(x1 );
		LCD_WR_DATA(x2 );
		LCD_WR_REG(0x2b); //行地址设置
		LCD_WR_DATA(y1 + 20);
		LCD_WR_DATA(y2 + 20);
		LCD_WR_REG(0x2c); //储存器写
	}
	else if (USE_HORIZONTAL == 1)
	{
		LCD_WR_REG(0x2a); //列地址设置
		LCD_WR_DATA(x1);
		LCD_WR_DATA(x2);
		LCD_WR_REG(0x2b); //行地址设置
		LCD_WR_DATA(y1 + 80);
		LCD_WR_DATA(y2 + 80);
		LCD_WR_REG(0x2c); //储存器写
	}
	else if (USE_HORIZONTAL == 2)
	{
		LCD_WR_REG(0x2a); //列地址设置
		LCD_WR_DATA(x1);
		LCD_WR_DATA(x2);
		LCD_WR_REG(0x2b); //行地址设置
		LCD_WR_DATA(y1);
		LCD_WR_DATA(y2);
		LCD_WR_REG(0x2c); //储存器写
	}
	else
	{
		LCD_WR_REG(0x2a); //列地址设置
		LCD_WR_DATA(x1 + 80);
		LCD_WR_DATA(x2 + 80);
		LCD_WR_REG(0x2b); //行地址设置
		LCD_WR_DATA(y1);
		LCD_WR_DATA(y2);
		LCD_WR_REG(0x2c); //储存器写
	}
}

void LCD_Init(void)
{
	LCD_RES_Clr();//复位
	delay_ms(100);
	LCD_RES_Set();
	delay_ms(100);
	
	//打开背光(电平拉低)
	LCD_BLK_Clr();
	// LCD_BLK_Set();
  	delay_ms(100);
	
	//************* Start Initial Sequence **********//
	LCD_WR_REG(0x11); //Sleep out 
	delay_ms(120);              //Delay 120ms 
	//************* Start Initial Sequence **********// 
	LCD_WR_REG(0x36);
	if(USE_HORIZONTAL==0)LCD_WR_DATA8(0x00);
	else if(USE_HORIZONTAL==1)LCD_WR_DATA8(0xC0);
	else if(USE_HORIZONTAL==2)LCD_WR_DATA8(0x70);
	else LCD_WR_DATA8(0xA0);

	LCD_WR_REG(0x3A);			
	LCD_WR_DATA8(0x05);

	LCD_WR_REG(0xB2);			
	LCD_WR_DATA8(0x0C);
	LCD_WR_DATA8(0x0C); 
	LCD_WR_DATA8(0x00); 
	LCD_WR_DATA8(0x33); 
	LCD_WR_DATA8(0x33); 			

	LCD_WR_REG(0xB7);			
	LCD_WR_DATA8(0x35);

	LCD_WR_REG(0xBB);			
	LCD_WR_DATA8(0x32); //Vcom=1.35V
					
	LCD_WR_REG(0xC2);
	LCD_WR_DATA8(0x01);

	LCD_WR_REG(0xC3);			
	LCD_WR_DATA8(0x15); //GVDD=4.8V  颜色深度
				
	LCD_WR_REG(0xC4);			
	LCD_WR_DATA8(0x20); //VDV, 0x20:0v

	LCD_WR_REG(0xC6);			
	LCD_WR_DATA8(0x0F); //0x0F:60Hz        	

	LCD_WR_REG(0xD0);			
	LCD_WR_DATA8(0xA4);
	LCD_WR_DATA8(0xA1); 

	LCD_WR_REG(0xE0);
	LCD_WR_DATA8(0xD0);   
	LCD_WR_DATA8(0x08);   
	LCD_WR_DATA8(0x0E);   
	LCD_WR_DATA8(0x09);   
	LCD_WR_DATA8(0x09);   
	LCD_WR_DATA8(0x05);   
	LCD_WR_DATA8(0x31);   
	LCD_WR_DATA8(0x33);   
	LCD_WR_DATA8(0x48);   
	LCD_WR_DATA8(0x17);   
	LCD_WR_DATA8(0x14);   
	LCD_WR_DATA8(0x15);   
	LCD_WR_DATA8(0x31);   
	LCD_WR_DATA8(0x34);   

	LCD_WR_REG(0xE1);     
	LCD_WR_DATA8(0xD0);   
	LCD_WR_DATA8(0x08);   
	LCD_WR_DATA8(0x0E);   
	LCD_WR_DATA8(0x09);   
	LCD_WR_DATA8(0x09);   
	LCD_WR_DATA8(0x15);   
	LCD_WR_DATA8(0x31);   
	LCD_WR_DATA8(0x33);   
	LCD_WR_DATA8(0x48);   
	LCD_WR_DATA8(0x17);   
	LCD_WR_DATA8(0x14);   
	LCD_WR_DATA8(0x15);   
	LCD_WR_DATA8(0x31);   
	LCD_WR_DATA8(0x34);
	LCD_WR_REG(0x21); 

	LCD_WR_REG(0x29);
} 

/******************************************************************************
      函数说明：SPI DMA传输完成回调函数
      入口数据：hspi SPI句柄指针
      返回值：  无
******************************************************************************/
void HAL_SPI_TxCpltCallback(SPI_HandleTypeDef *hspi)
{
    if(hspi->Instance == SPI1)
    {
        spi_dma_tx_complete = 1;  // 设置传输完成标志
    }
}

/******************************************************************************
      函数说明：SPI DMA传输错误回调函数
      入口数据：hspi SPI句柄指针
      返回值：  无
******************************************************************************/
void HAL_SPI_ErrorCallback(SPI_HandleTypeDef *hspi)
{
    if(hspi->Instance == SPI1)
    {
        spi_dma_tx_complete = 1;  // 错误时也设置完成标志，避免死锁
    }
}








