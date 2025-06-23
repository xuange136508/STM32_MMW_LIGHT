/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file    tim.c
  * @brief   This file provides code for the configuration
  *          of the TIM instances.
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2025 STMicroelectronics.
  * All rights reserved.
  *
  * This software is licensed under terms that can be found in the LICENSE file
  * in the root directory of this software component.
  * If no LICENSE file comes with this software, it is provided AS-IS.
  *
  ******************************************************************************
  */
#include "tim.h"


#define WS2812B_TIMER_PERIOD    104     // 1.25us周期 (84MHz/105 = 800kHz)

TIM_HandleTypeDef htim4;
DMA_HandleTypeDef hdma_tim4_ch2;


void HAL_TIM_Base_MspInit(TIM_HandleTypeDef* tim_baseHandle)
{
  if(tim_baseHandle->Instance==TIM4)
  {
    // 使能时钟
    __HAL_RCC_TIM4_CLK_ENABLE();
    __HAL_RCC_DMA1_CLK_ENABLE();
  }
}

void HAL_TIM_Base_MspDeInit(TIM_HandleTypeDef* tim_baseHandle)
{

   if(tim_baseHandle->Instance==TIM4)
  {
    __HAL_RCC_TIM4_CLK_DISABLE();
    __HAL_RCC_DMA1_CLK_DISABLE();
  }
}



void WS2812B_DMA_Init(void)
{
  // 使能时钟
    __HAL_RCC_TIM4_CLK_ENABLE();
    __HAL_RCC_DMA1_CLK_ENABLE();
    
    // DMA配置
    hdma_tim4_ch2.Instance = DMA1_Stream3;
    hdma_tim4_ch2.Init.Channel = DMA_CHANNEL_2;
    hdma_tim4_ch2.Init.Direction = DMA_MEMORY_TO_PERIPH;
    hdma_tim4_ch2.Init.PeriphInc = DMA_PINC_DISABLE;
    hdma_tim4_ch2.Init.MemInc = DMA_MINC_ENABLE;
    hdma_tim4_ch2.Init.PeriphDataAlignment = DMA_PDATAALIGN_HALFWORD;
    hdma_tim4_ch2.Init.MemDataAlignment = DMA_MDATAALIGN_HALFWORD;
    hdma_tim4_ch2.Init.Mode = DMA_NORMAL;
    hdma_tim4_ch2.Init.Priority = DMA_PRIORITY_HIGH;
    hdma_tim4_ch2.Init.FIFOMode = DMA_FIFOMODE_DISABLE;
    
    if (HAL_DMA_Init(&hdma_tim4_ch2) != HAL_OK) {
        Error_Handler();
    }
    
    // 链接DMA到TIM4
    __HAL_LINKDMA(&htim4, hdma[TIM_DMA_ID_CC2], hdma_tim4_ch2);
    
    // DMA中断配置
    HAL_NVIC_SetPriority(DMA1_Stream3_IRQn, 0, 0);
    HAL_NVIC_EnableIRQ(DMA1_Stream3_IRQn);
}


void MX_TIM4_Init(void)
{
    // 初始化DMA  
  WS2812B_DMA_Init();
  
  TIM_ClockConfigTypeDef sClockSourceConfig = {0};
    TIM_MasterConfigTypeDef sMasterConfig = {0};
    TIM_OC_InitTypeDef sConfigOC = {0};

    // TIM4基本配置
    htim4.Instance = TIM4;
    htim4.Init.Prescaler = 0;                           // 不分频，84MHz
    htim4.Init.CounterMode = TIM_COUNTERMODE_UP;
    htim4.Init.Period = WS2812B_TIMER_PERIOD;           // ARR = 104, 周期1.25us
    htim4.Init.ClockDivision = TIM_CLOCKDIVISION_DIV1;
    htim4.Init.AutoReloadPreload = TIM_AUTORELOAD_PRELOAD_DISABLE;
    
    if (HAL_TIM_Base_Init(&htim4) != HAL_OK) {
        Error_Handler();
    }
    
    // 时钟源配置
    sClockSourceConfig.ClockSource = TIM_CLOCKSOURCE_INTERNAL;
    if (HAL_TIM_ConfigClockSource(&htim4, &sClockSourceConfig) != HAL_OK) {
        Error_Handler();
    }
    
    // PWM初始化
    if (HAL_TIM_PWM_Init(&htim4) != HAL_OK) {
        Error_Handler();
    }
    
    // Master配置
    sMasterConfig.MasterOutputTrigger = TIM_TRGO_RESET;
    sMasterConfig.MasterSlaveMode = TIM_MASTERSLAVEMODE_DISABLE;
    if (HAL_TIMEx_MasterConfigSynchronization(&htim4, &sMasterConfig) != HAL_OK) {
        Error_Handler();
    }
    
    // PWM通道2配置
    sConfigOC.OCMode = TIM_OCMODE_PWM1;
    sConfigOC.Pulse = 0;
    sConfigOC.OCPolarity = TIM_OCPOLARITY_HIGH;
    sConfigOC.OCFastMode = TIM_OCFAST_DISABLE;

    // 配置TIM4通道2 (PD13)
    if (HAL_TIM_PWM_ConfigChannel(&htim4, &sConfigOC, TIM_CHANNEL_2) != HAL_OK) {
        Error_Handler();
    }

  // 配置TIM4通道1 (PD12)
  if (HAL_TIM_PWM_ConfigChannel(&htim4, &sConfigOC, TIM_CHANNEL_1) != HAL_OK)
  {
    Error_Handler();
  }
}
