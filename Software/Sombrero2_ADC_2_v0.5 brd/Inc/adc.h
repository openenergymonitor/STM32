/**
  ******************************************************************************
  * File Name          : ADC.h
  * Description        : This file provides code for the configuration
  *                      of the ADC instances.
  ******************************************************************************
  * @attention
  *
  * <h2><center>&copy; Copyright (c) 2019 STMicroelectronics.
  * All rights reserved.</center></h2>
  *
  * This software component is licensed by ST under BSD 3-Clause license,
  * the "License"; You may not use this file except in compliance with the
  * License. You may obtain a copy of the License at:
  *                        opensource.org/licenses/BSD-3-Clause
  *
  ******************************************************************************
  */
/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __adc_H
#define __adc_H
#ifdef __cplusplus
 extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include "main.h"

/* USER CODE BEGIN Includes */

/* USER CODE END Includes */

extern ADC_HandleTypeDef hadc1;
extern ADC_HandleTypeDef hadc2;
extern ADC_HandleTypeDef hadc3;
extern ADC_HandleTypeDef hadc4;

/* USER CODE BEGIN Private defines */
#define ADC_DMA_BUFFSIZE 6000    // must me integer multiple of number of channels?
extern volatile uint16_t adc1_dma_buff[ADC_DMA_BUFFSIZE];
extern volatile uint16_t adc3_dma_buff[ADC_DMA_BUFFSIZE];
/*
#define ADC1_DMA_BUFFSIZE 3000    // must me integer multiple of number of channels?
extern volatile uint16_t adc1_dma_buff[ADC1_DMA_BUFFSIZE];
extern volatile uint8_t adc1_half_conv_complete, adc1_full_conv_complete;
extern volatile uint8_t adc1_half_conv_overrun, adc1_full_conv_overrun;
#define ADC3_DMA_BUFFSIZE 9000    // must me integer multiple of number of channels?
extern volatile uint16_t adc3_dma_buff[ADC3_DMA_BUFFSIZE];
extern volatile uint8_t adc3_half_conv_complete, adc3_full_conv_complete;
extern volatile uint8_t adc3_half_conv_overrun, adc3_full_conv_overrun;
*/
/* USER CODE END Private defines */

void MX_ADC1_Init(void);
void MX_ADC2_Init(void);
void MX_ADC3_Init(void);
void MX_ADC4_Init(void);

/* USER CODE BEGIN Prototypes */

/* USER CODE END Prototypes */

#ifdef __cplusplus
}
#endif
#endif /*__ adc_H */

/**
  * @}
  */

/**
  * @}
  */

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
