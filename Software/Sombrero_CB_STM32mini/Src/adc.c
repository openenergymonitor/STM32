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
  * COPYRIGHT(c) 2021 STMicroelectronics
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
#include "tim.h"
#include "usart.h"
#include <stdbool.h>
#include <stdlib.h>
int32_t usec_lag = 95;  // how many microseconds to pull the voltage channel back by.
                    // this is a single value for phase correction for both voltage and current ADCs, effecting all CT and VT channels.
uint16_t const adc_buff_size = CTn * ADC_DMA_BUFFSIZE_PERCHANNEL;
uint16_t const adc_buff_half_size = (CTn * ADC_DMA_BUFFSIZE_PERCHANNEL) / 2;
uint16_t adcV_dma_buff[CTn * ADC_DMA_BUFFSIZE_PERCHANNEL];
uint16_t adcI_dma_buff[CTn * ADC_DMA_BUFFSIZE_PERCHANNEL];

bool adc_buffer_overflow = 0;
// volatile uint16_t adcV_dma_buff[ADC_DMA_BUFFSIZE];
// volatile uint16_t adcI_dma_buff[ADC_DMA_BUFFSIZE];
//bool adc_conv_halfcplt_flag = 0;
//bool adc_conv_cplt_flag = 0;
/* USER CODE END 0 */

ADC_HandleTypeDef hadc2;
ADC_HandleTypeDef hadc4;
DMA_HandleTypeDef hdma_adc2;
DMA_HandleTypeDef hdma_adc4;

/* ADC2 init function */
void MX_ADC2_Init(void)
{
  ADC_ChannelConfTypeDef sConfig;

    /**Common config 
    */
  hadc2.Instance = ADC2;
  hadc2.Init.ClockPrescaler = ADC_CLOCK_ASYNC_DIV1;
  hadc2.Init.Resolution = ADC_RESOLUTION_12B;
  hadc2.Init.ScanConvMode = ADC_SCAN_DISABLE;
  hadc2.Init.ContinuousConvMode = ENABLE;
  hadc2.Init.DiscontinuousConvMode = DISABLE;
  hadc2.Init.ExternalTrigConvEdge = ADC_EXTERNALTRIGCONVEDGE_RISING;
  hadc2.Init.ExternalTrigConv = ADC_EXTERNALTRIGCONV_T8_TRGO2;
  hadc2.Init.DataAlign = ADC_DATAALIGN_RIGHT;
  hadc2.Init.NbrOfConversion = 1;
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
  sConfig.Channel = ADC_CHANNEL_12;
  sConfig.Rank = ADC_REGULAR_RANK_1;
  sConfig.SingleDiff = ADC_SINGLE_ENDED;
  sConfig.SamplingTime = ADC_SAMPLETIME_181CYCLES_5;
  sConfig.OffsetNumber = ADC_OFFSET_NONE;
  sConfig.Offset = 0;
  if (HAL_ADC_ConfigChannel(&hadc2, &sConfig) != HAL_OK)
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
  hadc4.Init.ClockPrescaler = ADC_CLOCK_ASYNC_DIV1;
  hadc4.Init.Resolution = ADC_RESOLUTION_12B;
  hadc4.Init.ScanConvMode = ADC_SCAN_ENABLE;
  hadc4.Init.ContinuousConvMode = ENABLE;
  hadc4.Init.DiscontinuousConvMode = DISABLE;
  hadc4.Init.ExternalTrigConvEdge = ADC_EXTERNALTRIGCONVEDGE_NONE;
  hadc4.Init.ExternalTrigConv = ADC_SOFTWARE_START;
  hadc4.Init.DataAlign = ADC_DATAALIGN_RIGHT;
  hadc4.Init.NbrOfConversion = 3;
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
  sConfig.SamplingTime = ADC_SAMPLETIME_181CYCLES_5;
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

}

void HAL_ADC_MspInit(ADC_HandleTypeDef* adcHandle)
{

  GPIO_InitTypeDef GPIO_InitStruct;
  if(adcHandle->Instance==ADC2)
  {
  /* USER CODE BEGIN ADC2_MspInit 0 */

  /* USER CODE END ADC2_MspInit 0 */
    /* ADC2 clock enable */
    __HAL_RCC_ADC12_CLK_ENABLE();
  
    /**ADC2 GPIO Configuration    
    PB2     ------> ADC2_IN12 
    */
    GPIO_InitStruct.Pin = VSIG_Pin;
    GPIO_InitStruct.Mode = GPIO_MODE_ANALOG;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    HAL_GPIO_Init(VSIG_GPIO_Port, &GPIO_InitStruct);

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
  else if(adcHandle->Instance==ADC4)
  {
  /* USER CODE BEGIN ADC4_MspInit 0 */

  /* USER CODE END ADC4_MspInit 0 */
    /* ADC4 clock enable */
    __HAL_RCC_ADC34_CLK_ENABLE();
  
    /**ADC4 GPIO Configuration    
    PB12     ------> ADC4_IN3
    PB14     ------> ADC4_IN4
    PB15     ------> ADC4_IN5 
    */
    GPIO_InitStruct.Pin = GPIO_PIN_12|GPIO_PIN_14|GPIO_PIN_15;
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
    hdma_adc4.Init.Priority = DMA_PRIORITY_LOW;
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

  if(adcHandle->Instance==ADC2)
  {
  /* USER CODE BEGIN ADC2_MspDeInit 0 */

  /* USER CODE END ADC2_MspDeInit 0 */
    /* Peripheral clock disable */
    __HAL_RCC_ADC12_CLK_DISABLE();
  
    /**ADC2 GPIO Configuration    
    PB2     ------> ADC2_IN12 
    */
    HAL_GPIO_DeInit(VSIG_GPIO_Port, VSIG_Pin);

    /* ADC2 DMA DeInit */
    HAL_DMA_DeInit(adcHandle->DMA_Handle);
  /* USER CODE BEGIN ADC2_MspDeInit 1 */

  /* USER CODE END ADC2_MspDeInit 1 */
  }
  else if(adcHandle->Instance==ADC4)
  {
  /* USER CODE BEGIN ADC4_MspDeInit 0 */

  /* USER CODE END ADC4_MspDeInit 0 */
    /* Peripheral clock disable */
    __HAL_RCC_ADC34_CLK_DISABLE();
  
    /**ADC4 GPIO Configuration    
    PB12     ------> ADC4_IN3
    PB14     ------> ADC4_IN4
    PB15     ------> ADC4_IN5 
    */
    HAL_GPIO_DeInit(GPIOB, GPIO_PIN_12|GPIO_PIN_14|GPIO_PIN_15);

    /* ADC4 DMA DeInit */
    HAL_DMA_DeInit(adcHandle->DMA_Handle);
  /* USER CODE BEGIN ADC4_MspDeInit 1 */

  /* USER CODE END ADC4_MspDeInit 1 */
  }
} 

/* USER CODE BEGIN 1 */
void HAL_ADC_ConvHalfCpltCallback(ADC_HandleTypeDef* hadc)
{
  if(hadc == &hadc2) process_frame(0);
  //if(hadc == &hadc1) adc_conv_halfcplt_flag = true;
}

void HAL_ADC_ConvCpltCallback(ADC_HandleTypeDef* hadc)
{
 if(hadc == &hadc2) process_frame(3000);
 // if(hadc == &hadc1) adc_conv_cplt_flag = true;
}
void start_ADCs (int32_t usec_lag) {

  // while (!usart_tx_ready); // force wait while usart Tx finishes.
  snprintf(log_buffer, sizeof(log_buffer),
	   "ADC DMA buffs: %d bytes\n", sizeof(adcV_dma_buff));
  debug_printf(log_buffer);
  
  //
  // On the first call they will already be stopped, but in case we're
  // called multiple times to change the usec_lag, make sure they're
  // all stopped.
  //
  HAL_ADC_Stop_DMA(&hadc2);
  HAL_ADC_Stop_DMA(&hadc4);
  
  if (usec_lag == 0) {
    //
    // Caller has requested no lag so set them all to the same edge.
    //
    MODIFY_REG(hadc2.Instance->CFGR, ADC_CFGR_EXTEN, ADC_EXTERNALTRIGCONVEDGE_RISING);
    MODIFY_REG(hadc4.Instance->CFGR, ADC_CFGR_EXTEN, ADC_EXTERNALTRIGCONVEDGE_RISING);
    usec_lag = 1;                                         // any pulse from the timer will do 
    //isr_index = 3;                                        // process ISR when V comes in


  } else if (usec_lag < 0) {
    //
    // Caller wants -ve lag, so Currents on rising, and Voltage on falling.
    //
    MODIFY_REG(hadc2.Instance->CFGR, ADC_CFGR_EXTEN, ADC_EXTERNALTRIGCONVEDGE_RISING);
    MODIFY_REG(hadc4.Instance->CFGR, ADC_CFGR_EXTEN, ADC_EXTERNALTRIGCONVEDGE_FALLING);
    usec_lag = abs(usec_lag);                             // turn it +ve for the timer
    //isr_index = 2;                                        // process ISR when I comes in
  } else {
    //
    // Caller wants a +ve lag, so Currents on falling, and Voltage on rising.
    //
    MODIFY_REG(hadc2.Instance->CFGR, ADC_CFGR_EXTEN, ADC_EXTERNALTRIGCONVEDGE_FALLING);
    MODIFY_REG(hadc4.Instance->CFGR, ADC_CFGR_EXTEN, ADC_EXTERNALTRIGCONVEDGE_RISING);
    //isr_index = 3;                                        // process ISR when V comes in
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
  HAL_Delay(100);

  //
  // Now the bogus one has flown through the timer, it's safe to arm the ADCs.
  // After these calls, the next edges on the timer will start them running, forever.
  // The Voltage ADC (ADC4) will start on one edge, all the rest will start on the other.
  // In the case where the caller has requested no timeshift, all four start on the
  // same edge, and the pulse duration makes no difference.
  //
  HAL_ADC_Start_DMA(&hadc2, (uint16_t*)adcV_dma_buff, adc_buff_size);
  HAL_ADC_Start_DMA(&hadc4, (uint16_t*)adcI_dma_buff, adc_buff_size);

  HAL_Delay(100);
  //
  // Now force the last one through (which has the correct settings) and they all
  // kick to life on the appropriate edges.
  //
  pulse_tim8_ch2(usec_lag);
}

// void start_ADCs (void) {
//   HAL_ADC_Stop_DMA(&hadc2);
//   HAL_ADC_Stop_DMA(&hadc4);
  
//   pulse_tim8_ch2(10);
//   HAL_Delay(100);
  
//   HAL_ADC_Start_DMA(&hadc2, (uint32_t*)adcV_dma_buff, ADC_DMA_BUFFSIZE);
//   HAL_ADC_Start_DMA(&hadc4, (uint32_t*)adcI_dma_buff, ADC_DMA_BUFFSIZE);
  
//   pulse_tim8_ch2(10);
// }
/* USER CODE END 1 */

/**
  * @}
  */

/**
  * @}
  */

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
