#ifndef _delay_H
#define _delay_H

#include "main.h"

typedef uint32_t  u32;
typedef uint16_t u16;
typedef uint8_t  u8;

typedef const uint32_t uc32;  /*!< Read Only */
typedef const uint16_t uc16;  /*!< Read Only */
typedef const uint8_t uc8;   /*!< Read Only */

/*
*Parameter:SysClk
*					ϵͳʱ��Ƶ�ʣ���AHBԤ��Ƶ���ʱ��
*Role     :
*         ��ȷ��ʱ��ʼ��
*/
void SysTick_Init(uint8_t SysClk);
/*
*Parameter:Time
*					usʱ��
*Role     :
*         ��ʱ������us
*/
void Rough_DelayUs(uint16_t Time);
/*
*Parameter:Time
*					msʱ��
*Role     :
*         ��ʱ������ms
*/
void Rough_DelayMs(uint16_t Time);
/*
*Parameter:Time
*					usʱ��(1800,000)
*Role     :
*         ��ʱ������us
*/
void Correct_DelayUs(uint32_t Time);
/*
*Parameter:Time
*					msʱ��(1800)
*Role     :
*         ��ʱ������ms
*/
void Correct_DelayMs(uint16_t Time);


void delay_ms(uint32_t ms);

#endif
