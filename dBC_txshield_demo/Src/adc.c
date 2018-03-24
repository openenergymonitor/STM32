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

#include "power.h"
#include "tim.h"

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
  hadc1.Init.ClockPrescaler = ADC_CLOCK_SYNC_PCLK_DIV4;
  hadc1.Init.Resolution = ADC_RESOLUTION_12B;
  hadc1.Init.ScanConvMode = ADC_SCAN_ENABLE;
  hadc1.Init.ContinuousConvMode = ENABLE;
  hadc1.Init.DiscontinuousConvMode = DISABLE;
  hadc1.Init.ExternalTrigConvEdge = ADC_EXTERNALTRIGCONVEDGE_FALLING;
  hadc1.Init.ExternalTrigConv = ADC_EXTERNALTRIGCONV_T8_TRGO2;
  hadc1.Init.DataAlign = ADC_DATAALIGN_RIGHT;
  hadc1.Init.NbrOfConversion = 2;
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
  sConfig.Channel = ADC_CHANNEL_2;
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
  sConfig.Channel = ADC_CHANNEL_TEMPSENSOR;
  sConfig.Rank = ADC_REGULAR_RANK_2;
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
  hadc2.Init.ClockPrescaler = ADC_CLOCK_SYNC_PCLK_DIV4;
  hadc2.Init.Resolution = ADC_RESOLUTION_12B;
  hadc2.Init.ScanConvMode = ADC_SCAN_ENABLE;
  hadc2.Init.ContinuousConvMode = ENABLE;
  hadc2.Init.DiscontinuousConvMode = DISABLE;
  hadc2.Init.ExternalTrigConvEdge = ADC_EXTERNALTRIGCONVEDGE_FALLING;
  hadc2.Init.ExternalTrigConv = ADC_EXTERNALTRIGCONV_T8_TRGO2;
  hadc2.Init.DataAlign = ADC_DATAALIGN_RIGHT;
  hadc2.Init.NbrOfConversion = 2;
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
  sConfig.Channel = ADC_CHANNEL_1;
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
  sConfig.Channel = ADC_CHANNEL_7;
  sConfig.Rank = ADC_REGULAR_RANK_2;
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
  hadc3.Init.ClockPrescaler = ADC_CLOCK_SYNC_PCLK_DIV4;
  hadc3.Init.Resolution = ADC_RESOLUTION_12B;
  hadc3.Init.ScanConvMode = ADC_SCAN_ENABLE;
  hadc3.Init.ContinuousConvMode = ENABLE;
  hadc3.Init.DiscontinuousConvMode = DISABLE;
  hadc3.Init.ExternalTrigConvEdge = ADC_EXTERNALTRIGCONVEDGE_FALLING;
  hadc3.Init.ExternalTrigConv = ADC_EXTERNALTRIGCONV_T8_TRGO2;
  hadc3.Init.DataAlign = ADC_DATAALIGN_RIGHT;
  hadc3.Init.NbrOfConversion = 2;
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
  sConfig.Channel = ADC_CHANNEL_12;
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
  sConfig.Channel = ADC_CHANNEL_VREFINT;
  sConfig.Rank = ADC_REGULAR_RANK_2;
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
  hadc4.Init.ClockPrescaler = ADC_CLOCK_SYNC_PCLK_DIV4;
  hadc4.Init.Resolution = ADC_RESOLUTION_12B;
  hadc4.Init.ScanConvMode = ADC_SCAN_ENABLE;
  hadc4.Init.ContinuousConvMode = ENABLE;
  hadc4.Init.DiscontinuousConvMode = DISABLE;
  hadc4.Init.ExternalTrigConvEdge = ADC_EXTERNALTRIGCONVEDGE_RISING;
  hadc4.Init.ExternalTrigConv = ADC_EXTERNALTRIGCONV_T8_TRGO2;
  hadc4.Init.DataAlign = ADC_DATAALIGN_RIGHT;
  hadc4.Init.NbrOfConversion = 2;
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
  sConfig.Channel = ADC_CHANNEL_4;
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
  sConfig.Rank = ADC_REGULAR_RANK_2;
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
    PC1     ------> ADC1_IN7
    PA1     ------> ADC1_IN2 
    */
    GPIO_InitStruct.Pin = CT1_4_Pin;
    GPIO_InitStruct.Mode = GPIO_MODE_ANALOG;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    HAL_GPIO_Init(CT1_4_GPIO_Port, &GPIO_InitStruct);

    GPIO_InitStruct.Pin = CT1_1_Pin;
    GPIO_InitStruct.Mode = GPIO_MODE_ANALOG;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    HAL_GPIO_Init(CT1_1_GPIO_Port, &GPIO_InitStruct);

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
    */
    GPIO_InitStruct.Pin = CT1_4_Pin;
    GPIO_InitStruct.Mode = GPIO_MODE_ANALOG;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    HAL_GPIO_Init(CT1_4_GPIO_Port, &GPIO_InitStruct);

    GPIO_InitStruct.Pin = CT1_2_Pin;
    GPIO_InitStruct.Mode = GPIO_MODE_ANALOG;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    HAL_GPIO_Init(CT1_2_GPIO_Port, &GPIO_InitStruct);

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
    */
    GPIO_InitStruct.Pin = CT1_3_Pin;
    GPIO_InitStruct.Mode = GPIO_MODE_ANALOG;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    HAL_GPIO_Init(CT1_3_GPIO_Port, &GPIO_InitStruct);

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
    PB14     ------> ADC4_IN4 
    */
    GPIO_InitStruct.Pin = VT1_Pin;
    GPIO_InitStruct.Mode = GPIO_MODE_ANALOG;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    HAL_GPIO_Init(VT1_GPIO_Port, &GPIO_InitStruct);

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
    PC1     ------> ADC1_IN7
    PA1     ------> ADC1_IN2 
    */
    HAL_GPIO_DeInit(CT1_4_GPIO_Port, CT1_4_Pin);

    HAL_GPIO_DeInit(CT1_1_GPIO_Port, CT1_1_Pin);

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
    */
    HAL_GPIO_DeInit(CT1_4_GPIO_Port, CT1_4_Pin);

    HAL_GPIO_DeInit(CT1_2_GPIO_Port, CT1_2_Pin);

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
    */
    HAL_GPIO_DeInit(CT1_3_GPIO_Port, CT1_3_Pin);

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
    PB14     ------> ADC4_IN4 
    */
    HAL_GPIO_DeInit(VT1_GPIO_Port, VT1_Pin);

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
    // knowing how the ADC sequence slots have been configured.  In this case all
    // four have been configured for just two channels per sequence.  To
    // further complicate things, channels mostly correspond with CTs (apart from
    // the internal channels Temp and Vref) but channels are numbered from 0, while
    // CTs are numbered from 1.  Our sequence is defined as follows:
    //
    // ADC1 (buff[0]) : CT1, Temp, CT1, Temp, CT1, Temp...
    // ADC2 (buff[1]) : CT2, CT4, CT2, CT4....
    // ADC3 (buff[2]) : CT3, Vref, CT3, Vref....
    // ADC4 (buff[3]) : V1, V1, V1, V1
    //
    for (int i=0; i<SEQS_PER_HALF; i++) {
      process_VI_pair(adc_dma_buff[3][i*CHANNELS_PER_SEQ+dma_buff_index_base],  // V for Ch0 (CT1)
		      adc_dma_buff[0][i*CHANNELS_PER_SEQ+dma_buff_index_base],  // I for Ch0 (CT1)
		      0);                                                       // Chan num
      process_VI_pair(adc_dma_buff[3][i*CHANNELS_PER_SEQ+dma_buff_index_base],  // V for Ch1 (CT2)
		      adc_dma_buff[1][i*CHANNELS_PER_SEQ+dma_buff_index_base],  // I for Ch1 (CT2)
		      1);                                                       // Chan num
      process_VI_pair(adc_dma_buff[3][i*CHANNELS_PER_SEQ+dma_buff_index_base],  // V for Ch2 (CT3)
		      adc_dma_buff[2][i*CHANNELS_PER_SEQ+dma_buff_index_base],  // I for Ch2 (CT3)
		      2);                                                       // Chan num
      process_VI_pair(adc_dma_buff[3][i*CHANNELS_PER_SEQ+1+dma_buff_index_base],// V for Ch3 (CT4)
		      adc_dma_buff[1][i*CHANNELS_PER_SEQ+1+dma_buff_index_base],// I for Ch3 (CT4)
		      3);                                                       // Chan num
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
// XXX - how do you implement these without cutting-n-pasting all that
// MX_ADCn_Init() code above in order to parameterise ExternalTrigConvEdge?
// Doing that would be very problematic because when someone re-configures
// the ADC in the GUI (which is pretty much the only way to configure them),
// their changes would be reflected above, but not here and so would get
// clobbered.  --- needs further investigation.
//
void config_voltage_ADC_edge (uint32_t direction) {
};
void config_current_ADC_edge (uint32_t direction) {
};


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


  //
  // XXX eventually we need to handle +ve and -ve lags, and no lags
  // But for now we don't, so
  // condition the input appropriately.
  //
  usec_lag = abs(usec_lag);    
  if (!usec_lag) usec_lag = 1; 
  if (usec_lag > 9999) usec_lag = 9999;

  if (usec_lag == 0) {
    config_voltage_ADC_edge(ADC_EXTERNALTRIGCONVEDGE_FALLING);
    config_current_ADC_edge(ADC_EXTERNALTRIGCONVEDGE_FALLING);
    isr_index = 3;                                        // process ISR when V comes in
  } else if (usec_lag < 0) {
    config_voltage_ADC_edge(ADC_EXTERNALTRIGCONVEDGE_FALLING);
    config_current_ADC_edge(ADC_EXTERNALTRIGCONVEDGE_RISING);
    isr_index = 2;                                        // process ISR when I comes in
  } else {
    config_voltage_ADC_edge(ADC_EXTERNALTRIGCONVEDGE_RISING);
    config_current_ADC_edge(ADC_EXTERNALTRIGCONVEDGE_FALLING);
    isr_index = 3;                                        // process ISR when V comes in
  }
 
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


/* USER CODE END 1 */

/**
  * @}
  */

/**
  * @}
  */

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
