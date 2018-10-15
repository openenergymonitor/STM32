/**
  ******************************************************************************
  * File Name          : ADC.c
  * Description        : This file provides code for the configuration
  *                      of the ADC instances.
  ******************************************************************************
  ** This notice applies to any and all portions of this file
  * that are not between comment pairs USER CODE BEGIN and
  * USER CODE END. Other portions of this file, whether 
  * inserted by the user or by software development tools
  * are owned by their respective copyright owners.
  *
  * COPYRIGHT(c) 2018 STMicroelectronics
  *
  * Redistribution and use in source and binary forms, with or without modification,
  * are permitted provided that the following conditions are met:
  *   1. Redistributions of source code must retain the above copyright notice,
  *      this list of conditions and the following disclaimer.
  *   2. Redistributions in binary form must reproduce the above copyright notice,
  *      this list of conditions and the following disclaimer in the documentation
  *      and/or other materials provided with the distribution.
  *   3. Neither the name of STMicroelectronics nor the names of its contributors
  *      may be used to endorse or promote products derived from this software
  *      without specific prior written permission.
  *
  * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
  * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
  * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
  * DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE
  * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
  * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
  * SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
  * CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
  * OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
  * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
  *
  ******************************************************************************
  */

/* Includes ------------------------------------------------------------------*/
#include "adc.h"

#include "gpio.h"
#include "dma.h"

/* USER CODE BEGIN 0 */

#include "stm32f3xx_ll_adc.h"
#include "power.h"
#include "tim.h"
#include "usart.h"

//
// Tells us which ISR we're interested in, i.e. which is coming in late.
// 3 for ADC4 (Voltage), 0-2 for any of the others (which one doesn't matter)
//
static int isr_index;

//
// The DMA controller is constantly writing to this buffer so only read half of it when
// you know the DMA controller is busy working in the other half (see callback routines
// below).
//
static volatile uint16_t adc_dma_buff[MAX_ADC][ADC_DMA_BUFFSIZE];

/* USER CODE END 0 */

ADC_HandleTypeDef hadc1;
ADC_HandleTypeDef hadc2;
ADC_HandleTypeDef hadc3;
ADC_HandleTypeDef hadc4;
DMA_HandleTypeDef hdma_adc1;
DMA_HandleTypeDef hdma_adc2;
DMA_HandleTypeDef hdma_adc3;
DMA_HandleTypeDef hdma_adc4;

/* ADC1 init function */
void MX_ADC1_Init(void)
{
  ADC_MultiModeTypeDef multimode;
  ADC_ChannelConfTypeDef sConfig;

    /**Common config 
    */
  hadc1.Instance = ADC1;
  hadc1.Init.ClockPrescaler = ADC_CLOCK_SYNC_PCLK_DIV2;
  hadc1.Init.Resolution = ADC_RESOLUTION_12B;
  hadc1.Init.ScanConvMode = ADC_SCAN_ENABLE;
  hadc1.Init.ContinuousConvMode = ENABLE;
  hadc1.Init.DiscontinuousConvMode = DISABLE;
  hadc1.Init.ExternalTrigConvEdge = ADC_EXTERNALTRIGCONVEDGE_FALLING;
  hadc1.Init.ExternalTrigConv = ADC_EXTERNALTRIGCONV_T8_TRGO2;
  hadc1.Init.DataAlign = ADC_DATAALIGN_RIGHT;
  hadc1.Init.NbrOfConversion = 7;
  hadc1.Init.DMAContinuousRequests = ENABLE;
  hadc1.Init.EOCSelection = ADC_EOC_SEQ_CONV;
  hadc1.Init.LowPowerAutoWait = DISABLE;
  hadc1.Init.Overrun = ADC_OVR_DATA_OVERWRITTEN;
  if (HAL_ADC_Init(&hadc1) != HAL_OK)
  {
    _Error_Handler(__FILE__, __LINE__);
  }

    /**Configure the ADC multi-mode 
    */
  multimode.Mode = ADC_MODE_INDEPENDENT;
  if (HAL_ADCEx_MultiModeConfigChannel(&hadc1, &multimode) != HAL_OK)
  {
    _Error_Handler(__FILE__, __LINE__);
  }

    /**Configure Regular Channel 
    */
  sConfig.Channel = ADC_CHANNEL_1;
  sConfig.Rank = ADC_REGULAR_RANK_1;
  sConfig.SingleDiff = ADC_SINGLE_ENDED;
  sConfig.SamplingTime = ADC_SAMPLETIME_601CYCLES_5;
  sConfig.OffsetNumber = ADC_OFFSET_NONE;
  sConfig.Offset = 0;
  if (HAL_ADC_ConfigChannel(&hadc1, &sConfig) != HAL_OK)
  {
    _Error_Handler(__FILE__, __LINE__);
  }

    /**Configure Regular Channel 
    */
  sConfig.Channel = ADC_CHANNEL_2;
  sConfig.Rank = ADC_REGULAR_RANK_2;
  if (HAL_ADC_ConfigChannel(&hadc1, &sConfig) != HAL_OK)
  {
    _Error_Handler(__FILE__, __LINE__);
  }

    /**Configure Regular Channel 
    */
  sConfig.Channel = ADC_CHANNEL_6;
  sConfig.Rank = ADC_REGULAR_RANK_3;
  if (HAL_ADC_ConfigChannel(&hadc1, &sConfig) != HAL_OK)
  {
    _Error_Handler(__FILE__, __LINE__);
  }

    /**Configure Regular Channel 
    */
  sConfig.Channel = ADC_CHANNEL_8;
  sConfig.Rank = ADC_REGULAR_RANK_4;
  if (HAL_ADC_ConfigChannel(&hadc1, &sConfig) != HAL_OK)
  {
    _Error_Handler(__FILE__, __LINE__);
  }

    /**Configure Regular Channel 
    */
  sConfig.Channel = ADC_CHANNEL_9;
  sConfig.Rank = ADC_REGULAR_RANK_5;
  if (HAL_ADC_ConfigChannel(&hadc1, &sConfig) != HAL_OK)
  {
    _Error_Handler(__FILE__, __LINE__);
  }

    /**Configure Regular Channel 
    */
  sConfig.Channel = ADC_CHANNEL_14;
  sConfig.Rank = ADC_REGULAR_RANK_6;
  if (HAL_ADC_ConfigChannel(&hadc1, &sConfig) != HAL_OK)
  {
    _Error_Handler(__FILE__, __LINE__);
  }

    /**Configure Regular Channel 
    */
  sConfig.Channel = ADC_CHANNEL_TEMPSENSOR;
  sConfig.Rank = ADC_REGULAR_RANK_7;
  if (HAL_ADC_ConfigChannel(&hadc1, &sConfig) != HAL_OK)
  {
    _Error_Handler(__FILE__, __LINE__);
  }

}
/* ADC2 init function */
void MX_ADC2_Init(void)
{
  ADC_ChannelConfTypeDef sConfig;

    /**Common config 
    */
  hadc2.Instance = ADC2;
  hadc2.Init.ClockPrescaler = ADC_CLOCK_SYNC_PCLK_DIV2;
  hadc2.Init.Resolution = ADC_RESOLUTION_12B;
  hadc2.Init.ScanConvMode = ADC_SCAN_ENABLE;
  hadc2.Init.ContinuousConvMode = ENABLE;
  hadc2.Init.DiscontinuousConvMode = DISABLE;
  hadc2.Init.ExternalTrigConvEdge = ADC_EXTERNALTRIGCONVEDGE_FALLING;
  hadc2.Init.ExternalTrigConv = ADC_EXTERNALTRIGCONV_T8_TRGO2;
  hadc2.Init.DataAlign = ADC_DATAALIGN_RIGHT;
  hadc2.Init.NbrOfConversion = 7;
  hadc2.Init.DMAContinuousRequests = ENABLE;
  hadc2.Init.EOCSelection = ADC_EOC_SEQ_CONV;
  hadc2.Init.LowPowerAutoWait = DISABLE;
  hadc2.Init.Overrun = ADC_OVR_DATA_OVERWRITTEN;
  if (HAL_ADC_Init(&hadc2) != HAL_OK)
  {
    _Error_Handler(__FILE__, __LINE__);
  }

    /**Configure Regular Channel 
    */
  sConfig.Channel = ADC_CHANNEL_3;
  sConfig.Rank = ADC_REGULAR_RANK_1;
  sConfig.SingleDiff = ADC_SINGLE_ENDED;
  sConfig.SamplingTime = ADC_SAMPLETIME_601CYCLES_5;
  sConfig.OffsetNumber = ADC_OFFSET_NONE;
  sConfig.Offset = 0;
  if (HAL_ADC_ConfigChannel(&hadc2, &sConfig) != HAL_OK)
  {
    _Error_Handler(__FILE__, __LINE__);
  }

    /**Configure Regular Channel 
    */
  sConfig.Channel = ADC_CHANNEL_1;
  sConfig.Rank = ADC_REGULAR_RANK_2;
  if (HAL_ADC_ConfigChannel(&hadc2, &sConfig) != HAL_OK)
  {
    _Error_Handler(__FILE__, __LINE__);
  }

    /**Configure Regular Channel 
    */
  sConfig.Channel = ADC_CHANNEL_3;
  sConfig.Rank = ADC_REGULAR_RANK_3;
  if (HAL_ADC_ConfigChannel(&hadc2, &sConfig) != HAL_OK)
  {
    _Error_Handler(__FILE__, __LINE__);
  }

    /**Configure Regular Channel 
    */
  sConfig.Channel = ADC_CHANNEL_5;
  sConfig.Rank = ADC_REGULAR_RANK_4;
  if (HAL_ADC_ConfigChannel(&hadc2, &sConfig) != HAL_OK)
  {
    _Error_Handler(__FILE__, __LINE__);
  }

    /**Configure Regular Channel 
    */
  sConfig.Channel = ADC_CHANNEL_7;
  sConfig.Rank = ADC_REGULAR_RANK_5;
  if (HAL_ADC_ConfigChannel(&hadc2, &sConfig) != HAL_OK)
  {
    _Error_Handler(__FILE__, __LINE__);
  }

    /**Configure Regular Channel 
    */
  sConfig.Channel = ADC_CHANNEL_12;
  sConfig.Rank = ADC_REGULAR_RANK_6;
  if (HAL_ADC_ConfigChannel(&hadc2, &sConfig) != HAL_OK)
  {
    _Error_Handler(__FILE__, __LINE__);
  }

    /**Configure Regular Channel 
    */
  sConfig.Rank = ADC_REGULAR_RANK_7;
  if (HAL_ADC_ConfigChannel(&hadc2, &sConfig) != HAL_OK)
  {
    _Error_Handler(__FILE__, __LINE__);
  }

}
/* ADC3 init function */
void MX_ADC3_Init(void)
{
  ADC_MultiModeTypeDef multimode;
  ADC_ChannelConfTypeDef sConfig;

    /**Common config 
    */
  hadc3.Instance = ADC3;
  hadc3.Init.ClockPrescaler = ADC_CLOCK_SYNC_PCLK_DIV2;
  hadc3.Init.Resolution = ADC_RESOLUTION_12B;
  hadc3.Init.ScanConvMode = ADC_SCAN_ENABLE;
  hadc3.Init.ContinuousConvMode = ENABLE;
  hadc3.Init.DiscontinuousConvMode = DISABLE;
  hadc3.Init.ExternalTrigConvEdge = ADC_EXTERNALTRIGCONVEDGE_FALLING;
  hadc3.Init.ExternalTrigConv = ADC_EXTERNALTRIGCONV_T8_TRGO2;
  hadc3.Init.DataAlign = ADC_DATAALIGN_RIGHT;
  hadc3.Init.NbrOfConversion = 7;
  hadc3.Init.DMAContinuousRequests = ENABLE;
  hadc3.Init.EOCSelection = ADC_EOC_SEQ_CONV;
  hadc3.Init.LowPowerAutoWait = DISABLE;
  hadc3.Init.Overrun = ADC_OVR_DATA_OVERWRITTEN;
  if (HAL_ADC_Init(&hadc3) != HAL_OK)
  {
    _Error_Handler(__FILE__, __LINE__);
  }

    /**Configure the ADC multi-mode 
    */
  multimode.Mode = ADC_MODE_INDEPENDENT;
  if (HAL_ADCEx_MultiModeConfigChannel(&hadc3, &multimode) != HAL_OK)
  {
    _Error_Handler(__FILE__, __LINE__);
  }

    /**Configure Regular Channel 
    */
  sConfig.Channel = ADC_CHANNEL_1;
  sConfig.Rank = ADC_REGULAR_RANK_1;
  sConfig.SingleDiff = ADC_SINGLE_ENDED;
  sConfig.SamplingTime = ADC_SAMPLETIME_601CYCLES_5;
  sConfig.OffsetNumber = ADC_OFFSET_NONE;
  sConfig.Offset = 0;
  if (HAL_ADC_ConfigChannel(&hadc3, &sConfig) != HAL_OK)
  {
    _Error_Handler(__FILE__, __LINE__);
  }

    /**Configure Regular Channel 
    */
  sConfig.Channel = ADC_CHANNEL_12;
  sConfig.Rank = ADC_REGULAR_RANK_2;
  if (HAL_ADC_ConfigChannel(&hadc3, &sConfig) != HAL_OK)
  {
    _Error_Handler(__FILE__, __LINE__);
  }

    /**Configure Regular Channel 
    */
  sConfig.Channel = ADC_CHANNEL_5;
  sConfig.Rank = ADC_REGULAR_RANK_3;
  if (HAL_ADC_ConfigChannel(&hadc3, &sConfig) != HAL_OK)
  {
    _Error_Handler(__FILE__, __LINE__);
  }

    /**Configure Regular Channel 
    */
  sConfig.Channel = ADC_CHANNEL_VREFINT;
  sConfig.Rank = ADC_REGULAR_RANK_4;
  if (HAL_ADC_ConfigChannel(&hadc3, &sConfig) != HAL_OK)
  {
    _Error_Handler(__FILE__, __LINE__);
  }

    /**Configure Regular Channel 
    */
  sConfig.Rank = ADC_REGULAR_RANK_5;
  if (HAL_ADC_ConfigChannel(&hadc3, &sConfig) != HAL_OK)
  {
    _Error_Handler(__FILE__, __LINE__);
  }

    /**Configure Regular Channel 
    */
  sConfig.Rank = ADC_REGULAR_RANK_6;
  if (HAL_ADC_ConfigChannel(&hadc3, &sConfig) != HAL_OK)
  {
    _Error_Handler(__FILE__, __LINE__);
  }

    /**Configure Regular Channel 
    */
  sConfig.Rank = ADC_REGULAR_RANK_7;
  if (HAL_ADC_ConfigChannel(&hadc3, &sConfig) != HAL_OK)
  {
    _Error_Handler(__FILE__, __LINE__);
  }

}
/* ADC4 init function */
void MX_ADC4_Init(void)
{
  ADC_ChannelConfTypeDef sConfig;

    /**Common config 
    */
  hadc4.Instance = ADC4;
  hadc4.Init.ClockPrescaler = ADC_CLOCK_SYNC_PCLK_DIV2;
  hadc4.Init.Resolution = ADC_RESOLUTION_12B;
  hadc4.Init.ScanConvMode = ADC_SCAN_ENABLE;
  hadc4.Init.ContinuousConvMode = ENABLE;
  hadc4.Init.DiscontinuousConvMode = DISABLE;
  hadc4.Init.ExternalTrigConvEdge = ADC_EXTERNALTRIGCONVEDGE_RISING;
  hadc4.Init.ExternalTrigConv = ADC_EXTERNALTRIGCONV_T8_TRGO2;
  hadc4.Init.DataAlign = ADC_DATAALIGN_RIGHT;
  hadc4.Init.NbrOfConversion = 7;
  hadc4.Init.DMAContinuousRequests = ENABLE;
  hadc4.Init.EOCSelection = ADC_EOC_SEQ_CONV;
  hadc4.Init.LowPowerAutoWait = DISABLE;
  hadc4.Init.Overrun = ADC_OVR_DATA_OVERWRITTEN;
  if (HAL_ADC_Init(&hadc4) != HAL_OK)
  {
    _Error_Handler(__FILE__, __LINE__);
  }

    /**Configure Regular Channel 
    */
  sConfig.Channel = ADC_CHANNEL_3;
  sConfig.Rank = ADC_REGULAR_RANK_1;
  sConfig.SingleDiff = ADC_SINGLE_ENDED;
  sConfig.SamplingTime = ADC_SAMPLETIME_601CYCLES_5;
  sConfig.OffsetNumber = ADC_OFFSET_NONE;
  sConfig.Offset = 0;
  if (HAL_ADC_ConfigChannel(&hadc4, &sConfig) != HAL_OK)
  {
    _Error_Handler(__FILE__, __LINE__);
  }

    /**Configure Regular Channel 
    */
  sConfig.Channel = ADC_CHANNEL_4;
  sConfig.Rank = ADC_REGULAR_RANK_2;
  if (HAL_ADC_ConfigChannel(&hadc4, &sConfig) != HAL_OK)
  {
    _Error_Handler(__FILE__, __LINE__);
  }

    /**Configure Regular Channel 
    */
  sConfig.Channel = ADC_CHANNEL_5;
  sConfig.Rank = ADC_REGULAR_RANK_3;
  if (HAL_ADC_ConfigChannel(&hadc4, &sConfig) != HAL_OK)
  {
    _Error_Handler(__FILE__, __LINE__);
  }

    /**Configure Regular Channel 
    */
  sConfig.Channel = ADC_CHANNEL_3;
  sConfig.Rank = ADC_REGULAR_RANK_4;
  if (HAL_ADC_ConfigChannel(&hadc4, &sConfig) != HAL_OK)
  {
    _Error_Handler(__FILE__, __LINE__);
  }

    /**Configure Regular Channel 
    */
  sConfig.Channel = ADC_CHANNEL_4;
  sConfig.Rank = ADC_REGULAR_RANK_5;
  if (HAL_ADC_ConfigChannel(&hadc4, &sConfig) != HAL_OK)
  {
    _Error_Handler(__FILE__, __LINE__);
  }

    /**Configure Regular Channel 
    */
  sConfig.Channel = ADC_CHANNEL_5;
  sConfig.Rank = ADC_REGULAR_RANK_6;
  if (HAL_ADC_ConfigChannel(&hadc4, &sConfig) != HAL_OK)
  {
    _Error_Handler(__FILE__, __LINE__);
  }

    /**Configure Regular Channel 
    */
  sConfig.Channel = ADC_CHANNEL_3;
  sConfig.Rank = ADC_REGULAR_RANK_7;
  if (HAL_ADC_ConfigChannel(&hadc4, &sConfig) != HAL_OK)
  {
    _Error_Handler(__FILE__, __LINE__);
  }

}

static uint32_t HAL_RCC_ADC12_CLK_ENABLED=0;
static uint32_t HAL_RCC_ADC34_CLK_ENABLED=0;

void HAL_ADC_MspInit(ADC_HandleTypeDef* adcHandle)
{

  GPIO_InitTypeDef GPIO_InitStruct;
  if(adcHandle->Instance==ADC1)
  {
  /* USER CODE BEGIN ADC1_MspInit 0 */

  /* USER CODE END ADC1_MspInit 0 */
    /* ADC1 clock enable */
    HAL_RCC_ADC12_CLK_ENABLED++;
    if(HAL_RCC_ADC12_CLK_ENABLED==1){
      __HAL_RCC_ADC12_CLK_ENABLE();
    }
  
    /**ADC1 GPIO Configuration    
    PC0     ------> ADC1_IN6
    PC2     ------> ADC1_IN8
    PC3     ------> ADC1_IN9
    PA0     ------> ADC1_IN1
    PA1     ------> ADC1_IN2
    PB11     ------> ADC1_IN14 
    */
    GPIO_InitStruct.Pin = GPIO_PIN_0|GPIO_PIN_2|GPIO_PIN_3;
    GPIO_InitStruct.Mode = GPIO_MODE_ANALOG;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    HAL_GPIO_Init(GPIOC, &GPIO_InitStruct);

    GPIO_InitStruct.Pin = GPIO_PIN_0|ADC1_IN2_Pin;
    GPIO_InitStruct.Mode = GPIO_MODE_ANALOG;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

    GPIO_InitStruct.Pin = GPIO_PIN_11;
    GPIO_InitStruct.Mode = GPIO_MODE_ANALOG;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);

    /* ADC1 DMA Init */
    /* ADC1 Init */
    hdma_adc1.Instance = DMA1_Channel1;
    hdma_adc1.Init.Direction = DMA_PERIPH_TO_MEMORY;
    hdma_adc1.Init.PeriphInc = DMA_PINC_DISABLE;
    hdma_adc1.Init.MemInc = DMA_MINC_ENABLE;
    hdma_adc1.Init.PeriphDataAlignment = DMA_PDATAALIGN_HALFWORD;
    hdma_adc1.Init.MemDataAlignment = DMA_MDATAALIGN_HALFWORD;
    hdma_adc1.Init.Mode = DMA_CIRCULAR;
    hdma_adc1.Init.Priority = DMA_PRIORITY_HIGH;
    if (HAL_DMA_Init(&hdma_adc1) != HAL_OK)
    {
      _Error_Handler(__FILE__, __LINE__);
    }

    __HAL_LINKDMA(adcHandle,DMA_Handle,hdma_adc1);

  /* USER CODE BEGIN ADC1_MspInit 1 */

  /* USER CODE END ADC1_MspInit 1 */
  }
  else if(adcHandle->Instance==ADC2)
  {
  /* USER CODE BEGIN ADC2_MspInit 0 */

  /* USER CODE END ADC2_MspInit 0 */
    /* ADC2 clock enable */
    HAL_RCC_ADC12_CLK_ENABLED++;
    if(HAL_RCC_ADC12_CLK_ENABLED==1){
      __HAL_RCC_ADC12_CLK_ENABLE();
    }
  
    /**ADC2 GPIO Configuration    
    PC1     ------> ADC2_IN7
    PA4     ------> ADC2_IN1
    PC4     ------> ADC2_IN5
    PB2     ------> ADC2_IN12 
    */
    GPIO_InitStruct.Pin = ADC2_IN7_Pin|GPIO_PIN_4;
    GPIO_InitStruct.Mode = GPIO_MODE_ANALOG;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    HAL_GPIO_Init(GPIOC, &GPIO_InitStruct);

    GPIO_InitStruct.Pin = ADC2_IN1_Pin;
    GPIO_InitStruct.Mode = GPIO_MODE_ANALOG;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    HAL_GPIO_Init(ADC2_IN1_GPIO_Port, &GPIO_InitStruct);

    GPIO_InitStruct.Pin = GPIO_PIN_2;
    GPIO_InitStruct.Mode = GPIO_MODE_ANALOG;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);

    /* ADC2 DMA Init */
    /* ADC2 Init */
    hdma_adc2.Instance = DMA2_Channel1;
    hdma_adc2.Init.Direction = DMA_PERIPH_TO_MEMORY;
    hdma_adc2.Init.PeriphInc = DMA_PINC_DISABLE;
    hdma_adc2.Init.MemInc = DMA_MINC_ENABLE;
    hdma_adc2.Init.PeriphDataAlignment = DMA_PDATAALIGN_HALFWORD;
    hdma_adc2.Init.MemDataAlignment = DMA_MDATAALIGN_HALFWORD;
    hdma_adc2.Init.Mode = DMA_CIRCULAR;
    hdma_adc2.Init.Priority = DMA_PRIORITY_HIGH;
    if (HAL_DMA_Init(&hdma_adc2) != HAL_OK)
    {
      _Error_Handler(__FILE__, __LINE__);
    }

    __HAL_LINKDMA(adcHandle,DMA_Handle,hdma_adc2);

  /* USER CODE BEGIN ADC2_MspInit 1 */

  /* USER CODE END ADC2_MspInit 1 */
  }
  else if(adcHandle->Instance==ADC3)
  {
  /* USER CODE BEGIN ADC3_MspInit 0 */

  /* USER CODE END ADC3_MspInit 0 */
    /* ADC3 clock enable */
    HAL_RCC_ADC34_CLK_ENABLED++;
    if(HAL_RCC_ADC34_CLK_ENABLED==1){
      __HAL_RCC_ADC34_CLK_ENABLE();
    }
  
    /**ADC3 GPIO Configuration    
    PB0     ------> ADC3_IN12
    PB1     ------> ADC3_IN1
    PB13     ------> ADC3_IN5 
    */
    GPIO_InitStruct.Pin = ADC3_IN12_Pin|GPIO_PIN_1|GPIO_PIN_13;
    GPIO_InitStruct.Mode = GPIO_MODE_ANALOG;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);

    /* ADC3 DMA Init */
    /* ADC3 Init */
    hdma_adc3.Instance = DMA2_Channel5;
    hdma_adc3.Init.Direction = DMA_PERIPH_TO_MEMORY;
    hdma_adc3.Init.PeriphInc = DMA_PINC_DISABLE;
    hdma_adc3.Init.MemInc = DMA_MINC_ENABLE;
    hdma_adc3.Init.PeriphDataAlignment = DMA_PDATAALIGN_HALFWORD;
    hdma_adc3.Init.MemDataAlignment = DMA_MDATAALIGN_HALFWORD;
    hdma_adc3.Init.Mode = DMA_CIRCULAR;
    hdma_adc3.Init.Priority = DMA_PRIORITY_HIGH;
    if (HAL_DMA_Init(&hdma_adc3) != HAL_OK)
    {
      _Error_Handler(__FILE__, __LINE__);
    }

    __HAL_LINKDMA(adcHandle,DMA_Handle,hdma_adc3);

  /* USER CODE BEGIN ADC3_MspInit 1 */

  /* USER CODE END ADC3_MspInit 1 */
  }
  else if(adcHandle->Instance==ADC4)
  {
  /* USER CODE BEGIN ADC4_MspInit 0 */

  /* USER CODE END ADC4_MspInit 0 */
    /* ADC4 clock enable */
    HAL_RCC_ADC34_CLK_ENABLED++;
    if(HAL_RCC_ADC34_CLK_ENABLED==1){
      __HAL_RCC_ADC34_CLK_ENABLE();
    }
  
    /**ADC4 GPIO Configuration    
    PB12     ------> ADC4_IN3
    PB14     ------> ADC4_IN4
    PB15     ------> ADC4_IN5 
    */
    GPIO_InitStruct.Pin = VT1_Pin|VT2_Pin|VT3_Pin;
    GPIO_InitStruct.Mode = GPIO_MODE_ANALOG;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);

    /* ADC4 DMA Init */
    /* ADC4 Init */
    hdma_adc4.Instance = DMA2_Channel2;
    hdma_adc4.Init.Direction = DMA_PERIPH_TO_MEMORY;
    hdma_adc4.Init.PeriphInc = DMA_PINC_DISABLE;
    hdma_adc4.Init.MemInc = DMA_MINC_ENABLE;
    hdma_adc4.Init.PeriphDataAlignment = DMA_PDATAALIGN_HALFWORD;
    hdma_adc4.Init.MemDataAlignment = DMA_MDATAALIGN_HALFWORD;
    hdma_adc4.Init.Mode = DMA_CIRCULAR;
    hdma_adc4.Init.Priority = DMA_PRIORITY_HIGH;
    if (HAL_DMA_Init(&hdma_adc4) != HAL_OK)
    {
      _Error_Handler(__FILE__, __LINE__);
    }

    __HAL_LINKDMA(adcHandle,DMA_Handle,hdma_adc4);

  /* USER CODE BEGIN ADC4_MspInit 1 */

  /* USER CODE END ADC4_MspInit 1 */
  }
}

void HAL_ADC_MspDeInit(ADC_HandleTypeDef* adcHandle)
{

  if(adcHandle->Instance==ADC1)
  {
  /* USER CODE BEGIN ADC1_MspDeInit 0 */

  /* USER CODE END ADC1_MspDeInit 0 */
    /* Peripheral clock disable */
    /* Be sure that all peripheral instances that share the same clock need to be disabled */
    /**  HAL_RCC_ADC12_CLK_ENABLED--;
    *  if(HAL_RCC_ADC12_CLK_ENABLED==0){
    *    __HAL_RCC_ADC12_CLK_DISABLE();
    **/
  
    /**ADC1 GPIO Configuration    
    PC0     ------> ADC1_IN6
    PC2     ------> ADC1_IN8
    PC3     ------> ADC1_IN9
    PA0     ------> ADC1_IN1
    PA1     ------> ADC1_IN2
    PB11     ------> ADC1_IN14 
    */
    HAL_GPIO_DeInit(GPIOC, GPIO_PIN_0|GPIO_PIN_2|GPIO_PIN_3);

    HAL_GPIO_DeInit(GPIOA, GPIO_PIN_0|ADC1_IN2_Pin);

    HAL_GPIO_DeInit(GPIOB, GPIO_PIN_11);

    /* ADC1 DMA DeInit */
    HAL_DMA_DeInit(adcHandle->DMA_Handle);
  /* USER CODE BEGIN ADC1_MspDeInit 1 */

  /* USER CODE END ADC1_MspDeInit 1 */
  }
  else if(adcHandle->Instance==ADC2)
  {
  /* USER CODE BEGIN ADC2_MspDeInit 0 */

  /* USER CODE END ADC2_MspDeInit 0 */
    /* Peripheral clock disable */
    /* Be sure that all peripheral instances that share the same clock need to be disabled */
    /**  HAL_RCC_ADC12_CLK_ENABLED--;
    *  if(HAL_RCC_ADC12_CLK_ENABLED==0){
    *    __HAL_RCC_ADC12_CLK_DISABLE();
    **/
  
    /**ADC2 GPIO Configuration    
    PC1     ------> ADC2_IN7
    PA4     ------> ADC2_IN1
    PC4     ------> ADC2_IN5
    PB2     ------> ADC2_IN12 
    */
    HAL_GPIO_DeInit(GPIOC, ADC2_IN7_Pin|GPIO_PIN_4);

    HAL_GPIO_DeInit(ADC2_IN1_GPIO_Port, ADC2_IN1_Pin);

    HAL_GPIO_DeInit(GPIOB, GPIO_PIN_2);

    /* ADC2 DMA DeInit */
    HAL_DMA_DeInit(adcHandle->DMA_Handle);
  /* USER CODE BEGIN ADC2_MspDeInit 1 */

  /* USER CODE END ADC2_MspDeInit 1 */
  }
  else if(adcHandle->Instance==ADC3)
  {
  /* USER CODE BEGIN ADC3_MspDeInit 0 */

  /* USER CODE END ADC3_MspDeInit 0 */
    /* Peripheral clock disable */
    /* Be sure that all peripheral instances that share the same clock need to be disabled */
    /**  HAL_RCC_ADC34_CLK_ENABLED--;
    *  if(HAL_RCC_ADC34_CLK_ENABLED==0){
    *    __HAL_RCC_ADC34_CLK_DISABLE();
    **/
  
    /**ADC3 GPIO Configuration    
    PB0     ------> ADC3_IN12
    PB1     ------> ADC3_IN1
    PB13     ------> ADC3_IN5 
    */
    HAL_GPIO_DeInit(GPIOB, ADC3_IN12_Pin|GPIO_PIN_1|GPIO_PIN_13);

    /* ADC3 DMA DeInit */
    HAL_DMA_DeInit(adcHandle->DMA_Handle);
  /* USER CODE BEGIN ADC3_MspDeInit 1 */

  /* USER CODE END ADC3_MspDeInit 1 */
  }
  else if(adcHandle->Instance==ADC4)
  {
  /* USER CODE BEGIN ADC4_MspDeInit 0 */

  /* USER CODE END ADC4_MspDeInit 0 */
    /* Peripheral clock disable */
    /* Be sure that all peripheral instances that share the same clock need to be disabled */
    /**  HAL_RCC_ADC34_CLK_ENABLED--;
    *  if(HAL_RCC_ADC34_CLK_ENABLED==0){
    *    __HAL_RCC_ADC34_CLK_DISABLE();
    **/
  
    /**ADC4 GPIO Configuration    
    PB12     ------> ADC4_IN3
    PB14     ------> ADC4_IN4
    PB15     ------> ADC4_IN5 
    */
    HAL_GPIO_DeInit(GPIOB, VT1_Pin|VT2_Pin|VT3_Pin);

    /* ADC4 DMA DeInit */
    HAL_DMA_DeInit(adcHandle->DMA_Handle);
  /* USER CODE BEGIN ADC4_MspDeInit 1 */

  /* USER CODE END ADC4_MspDeInit 1 */
  }
} 

/* USER CODE BEGIN 1 */

//
//  Map the handle supplied by the HAL into an index we can use into our
//  datastructures.
//
static inline int handle_to_index (ADC_HandleTypeDef* hadc)
{
  if (hadc == &hadc1)
    return 0;
  if (hadc == &hadc2)
    return 1;
  if (hadc == &hadc3)
    return 2;
  return 3;
}

//
// The bulk of the ISR processing is identical whether we're half-full or full-full.
// The half Vs full distinction only impacts which half of the DMA buffer we extract from.
// The two callback routines demux that and both call here to do all the real work.
//
static inline void ConvCpltEither (ADC_HandleTypeDef* hadc, int dma_buff_index_base) {
  int adc_index;

  //
  // Ignore data for the first 10 seconds after booting, gives time
  // to arm the scope
  // 
  if ((HAL_GetTick() < 10000))
      return;

  //
  // Determine which ADC has just crossed a boundary and set up pointers to our
  // datastructures for it.
  //
  adc_index = handle_to_index(hadc);


  // 
  // We only want to cop the overhead of the for-loop dma buffer traversal once.
  // All three Current ADCs interrupt within nsecs of each other, so it doesn't
  // matter which one we choose.  Long before we get to the far end of the buffer
  // they'll have all completed... actually, they all complete before we get in here.
  // But we do support quite long delays between I and V, so it's important we don't
  // start processing the data when one of the I interrupts come in if the corresponding
  // V interrupt isn't due in for potentially msecs.  start_ADCs() has helpfully left
  // us a clue in isr_index.  It'll be set to 3 (to indicate ADC4)  when we're expecting
  // V last or we're when we're expecting all four at once (no lag), and 2 otherwise.
  // That's the one we process and we simply ignore interrupts from the remaining 3 ADCs.
  //
  if (adc_index == isr_index) {  // Voltage Vs Current, whichever comes last

    HAL_GPIO_WritePin(LD2_GPIO_Port, LD2_Pin, GPIO_PIN_RESET);   // For scope timing

    //
    // Knowing which value to get from where in the DMA buffer is all about
    // knowing how the ADC sequence slots have been configured.  In this example
    // all ADCs have been configured with 7 slots as follows:
    //
    //  slot     ADC1      ADC2      ADC3      ADC4
    //    1      ch0       VOpAmp2   ch2       V1
    //    2      ch3(CT1)  ch4(CT2)  ch5(CT3)  V2
    //    3      ch6       VOpAmp2   ch8       V3
    //    4      ch9       ch10      Vref      V1
    //    5      ch11      ch12(CT4) fill      V2
    //    6      ch13      ch14      fill      V3
    //    7      Temp      fill      fill      V1
    //
    for (int i=0; i<SEQS_PER_HALF; i++) {
      //
      // Process the first three slots which are fully populated
      //
      int chan = 0;
      for (int slot=0; slot<3; slot++)
	for (int adc=0; adc<3; adc++)
	  process_VI_pair(adc_dma_buff[3][i*CHANNELS_PER_SEQ+slot+dma_buff_index_base],  // V
			  adc_dma_buff[adc][i*CHANNELS_PER_SEQ+slot+dma_buff_index_base],  // I
			  chan++);
      //
      // Process the last three slots which are half populated
      //
      for (int slot=3; slot<6; slot++)
	for (int adc=0; adc<2; adc++)
	  process_VI_pair(adc_dma_buff[3][i*CHANNELS_PER_SEQ+slot+dma_buff_index_base],  // V
			  adc_dma_buff[adc][i*CHANNELS_PER_SEQ+slot+dma_buff_index_base],  // I
			  chan++);
    }

    HAL_GPIO_WritePin(LD2_GPIO_Port, LD2_Pin, GPIO_PIN_SET);
  }
}

void HAL_ADC_ConvHalfCpltCallback(ADC_HandleTypeDef* hadc)
{
  ConvCpltEither(hadc, 0);
}

void HAL_ADC_ConvCpltCallback(ADC_HandleTypeDef* hadc)
{
  ConvCpltEither(hadc, CHANNELS_PER_SEQ*SEQS_PER_HALF);
}

void calibrate_ADCs (void) {

  HAL_ADCEx_Calibration_Start(&hadc1, ADC_SINGLE_ENDED);
  HAL_ADCEx_Calibration_Start(&hadc2, ADC_SINGLE_ENDED);
  HAL_ADCEx_Calibration_Start(&hadc3, ADC_SINGLE_ENDED);
  HAL_ADCEx_Calibration_Start(&hadc4, ADC_SINGLE_ENDED);
}

//
// Start all four ADCs with a lag between the Voltage ADC (ADC4)
// and the three current ADCs (ADC1-ADC3).   We achieve this with
// a one-shot PWM pulse from TIM8. The natural state of the
// timer when it's not running is HIGH, so we trigger on a negative
// pulse.   Which ever one(s) starts on the negative edge will be off
// and running early.  
//
// usec_lag > 0, Current ADCs start early, Voltage brings up the rear
// usec_lag < 0, Voltage ADC starts early, Current brings up the rear.
// usec_lag = 0, both start on same edge
//
void start_ADCs (int32_t usec_lag) {

  snprintf(log_buffer, sizeof(log_buffer),
	   "ADC DMA buffs: %d bytes\n", sizeof(adc_dma_buff));
  debug_printf(log_buffer);
  
  //
  // On the first call they will already be stopped, but in case we're
  // called multiple times to change the usec_lag, make sure they're
  // all stopped.
  //
  HAL_ADC_Stop_DMA(&hadc1);
  HAL_ADC_Stop_DMA(&hadc2);
  HAL_ADC_Stop_DMA(&hadc3);
  HAL_ADC_Stop_DMA(&hadc4);
  
  if (usec_lag == 0) {
    //
    // Caller has requested no lag so set them all to the same edge.
    //
    MODIFY_REG(hadc1.Instance->CFGR, ADC_CFGR_EXTEN, ADC_EXTERNALTRIGCONVEDGE_RISING);
    MODIFY_REG(hadc2.Instance->CFGR, ADC_CFGR_EXTEN, ADC_EXTERNALTRIGCONVEDGE_RISING);
    MODIFY_REG(hadc3.Instance->CFGR, ADC_CFGR_EXTEN, ADC_EXTERNALTRIGCONVEDGE_RISING);
    MODIFY_REG(hadc4.Instance->CFGR, ADC_CFGR_EXTEN, ADC_EXTERNALTRIGCONVEDGE_RISING);
    usec_lag = 1;                                         // any pulse from the timer will do 
    isr_index = 3;                                        // process ISR when V comes in


  } else if (usec_lag < 0) {
    //
    // Caller wants -ve lag, so Currents on rising, and Voltage on falling.
    //
    MODIFY_REG(hadc1.Instance->CFGR, ADC_CFGR_EXTEN, ADC_EXTERNALTRIGCONVEDGE_RISING);
    MODIFY_REG(hadc2.Instance->CFGR, ADC_CFGR_EXTEN, ADC_EXTERNALTRIGCONVEDGE_RISING);
    MODIFY_REG(hadc3.Instance->CFGR, ADC_CFGR_EXTEN, ADC_EXTERNALTRIGCONVEDGE_RISING);
    MODIFY_REG(hadc4.Instance->CFGR, ADC_CFGR_EXTEN, ADC_EXTERNALTRIGCONVEDGE_FALLING);
    usec_lag = abs(usec_lag);                             // turn it +ve for the timer
    isr_index = 2;                                        // process ISR when I comes in
  } else {
    //
    // Caller wants a +ve lag, so Currents on falling, and Voltage on rising.
    //
    MODIFY_REG(hadc1.Instance->CFGR, ADC_CFGR_EXTEN, ADC_EXTERNALTRIGCONVEDGE_FALLING);
    MODIFY_REG(hadc2.Instance->CFGR, ADC_CFGR_EXTEN, ADC_EXTERNALTRIGCONVEDGE_FALLING);
    MODIFY_REG(hadc3.Instance->CFGR, ADC_CFGR_EXTEN, ADC_EXTERNALTRIGCONVEDGE_FALLING);
    MODIFY_REG(hadc4.Instance->CFGR, ADC_CFGR_EXTEN, ADC_EXTERNALTRIGCONVEDGE_RISING);
    isr_index = 3;                                        // process ISR when V comes in
  }
  if (usec_lag > 9999) usec_lag = 9999;                   // limit of the counter/timer

  
  //
  // These PWM timers in one-shot mode are almost like a pipeline,
  // every action causes the previous action to finally happen.
  // This call stops the last pulse - by generating a pulse with whatever
  // value is set in the GUI and appears above.  We don't want the
  // ADCs to trigger on this one, and it'll take 10 msecs to complete,
  // i.e. the timer will take 10msecs to do it's thing, the call returns
  // immediately, and it makes sure the next pulse will be of the requested
  // duration.
  //
  pulse_tim8_ch2(usec_lag);
  HAL_Delay(20);

  //
  // Now the bogus one has flown through the timer, it's safe to arm the ADCs.
  // After these calls, the next edges on the timer will start them running, forever.
  // The Voltage ADC (ADC4) will start on one edge, all the rest will start on the other.
  // In the case where the caller has requested no timeshift, all four start on the
  // same edge, and the pulse duration makes no difference.
  //
  HAL_ADC_Start_DMA(&hadc1, (uint32_t*)adc_dma_buff[0], ADC_DMA_BUFFSIZE);
  HAL_ADC_Start_DMA(&hadc2, (uint32_t*)adc_dma_buff[1], ADC_DMA_BUFFSIZE);
  HAL_ADC_Start_DMA(&hadc3, (uint32_t*)adc_dma_buff[2], ADC_DMA_BUFFSIZE);
  HAL_ADC_Start_DMA(&hadc4, (uint32_t*)adc_dma_buff[3], ADC_DMA_BUFFSIZE);

  //
  // Now force the last one through (which has the correct settings) and they all
  // kick to life on the appropriate edges.
  //
  pulse_tim8_ch2(usec_lag);
}

//
// See the ADC sequence definition above (comment in ConvCplEither()) to know
// where to get these two from.  They're being sampled every time around the
// sequence and they're slow moving, so it doesn't matter which half of the
// dma buffer we sample them from.
//
int get_cpu_temp () {
  return (COMPUTATION_TEMPERATURE(adc_dma_buff[0][6]));
}

int get_vdd () {
  return (3300 * *VREF_CAL_ADDR / adc_dma_buff[2][3]);
}

//
// Diagnostic routine to dump all 4 ADC's config registers to check edge configs
//
void dump_CFGRs() {
  snprintf(log_buffer, sizeof(log_buffer), "hadc1.CFGR: %08x\n", (unsigned int)hadc1.Instance->CFGR);
  debug_printf(log_buffer);
  snprintf(log_buffer, sizeof(log_buffer), "hadc2.CFGR: %08x\n", (unsigned int)hadc2.Instance->CFGR);
  debug_printf(log_buffer);
  snprintf(log_buffer, sizeof(log_buffer), "hadc3.CFGR: %08x\n", (unsigned int)hadc3.Instance->CFGR);
  debug_printf(log_buffer);
  snprintf(log_buffer, sizeof(log_buffer), "hadc4.CFGR: %08x\n", (unsigned int)hadc4.Instance->CFGR);
  debug_printf(log_buffer);
}


/* USER CODE END 1 */

/**
  * @}
  */

/**
  * @}
  */

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
