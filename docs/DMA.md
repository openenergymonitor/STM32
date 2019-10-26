## DMA

Starting from where we left off in the previous example with ADC1 channel 1 and 2 enabled and configured.

- Enable Continuous Conversions
- Enable DMA Continuous Requests
- End of Conversion to be End of Sequence
- Add DMA channel in the DMA Settings tab
- Select Circular mode
- NVIC Settings: enable 'DMA1 channel1 global interrupt'
- NVIC Settings: disable 'ADC1 and ADC2 interrupts'
- NVIC Code generation, tick in generate IRQ handler for Time base and DMA1.

Project > Generate Code

**User source code edits**

In adc.c the main addition is the callback function to mark half way through block of conversions (0-2000) and callback to mark complete conversion (2000-4000). There are also variables defined at the top and in adc.h.

adc.c:

View sources: [../DMA/Src/adc.c](../DMA/Src/adc.c) [../DMA/Src/adc.h](../DMA/Src/adc.h)

    /* USER CODE BEGIN 1 */
    void HAL_ADC_ConvHalfCpltCallback(ADC_HandleTypeDef* hadc)
    {
      if (adc1_half_conv_complete) {
        adc1_half_conv_overrun = true;
        adc1_half_conv_complete = false;
      } else
        adc1_half_conv_complete = true;
    }

    void HAL_ADC_ConvCpltCallback(ADC_HandleTypeDef* hadc)
    {
      if (adc1_full_conv_complete) {
        adc1_full_conv_overrun = true;
        adc1_full_conv_complete = false;
      } else
        adc1_full_conv_complete = true;
    }

    void start_ADC1 (void) {
      HAL_ADC_Start_DMA(&hadc1, (uint32_t*)adc1_dma_buff, ADC1_DMA_BUFFSIZE);
    }
    /* USER CODE END 1 */

In main.c there are a few additions:

View source: [../DMA/Src/main.c](../DMA/Src/main.c)

A few variables:

    /* USER CODE BEGIN PV */
    char log_buffer[100];
    #define true 1
    #define false 0
    int sampleA0 = 0;
    int sampleA1 = 0;
    long sumA0 = 0;
    long sumA1 = 0;
    /* USER CODE END PV */

A function for processing a frame/block of samples:

    /* USER CODE BEGIN 0 */
    void process_frame(int offset)
    {
      for(int i=0; i<2000; i+=2) 
      {
        sampleA0 = adc1_dma_buff[offset+i];
        sumA0 += sampleA0;
        sampleA1 = adc1_dma_buff[offset+i+1];
        sumA1 += sampleA1;
      }
    }
    /* USER CODE END 0 */
    
Start the conversions:

    /* USER CODE BEGIN 2 */
      start_ADC1();
    /* USER CODE END 2 */

The main loop:

    /* Infinite loop */
    /* USER CODE BEGIN WHILE */
    while (1)
    {
      if (adc1_half_conv_complete && !adc1_half_conv_overrun) 
      {
        HAL_GPIO_WritePin(GPIOA, GPIO_PIN_5, GPIO_PIN_RESET);      // LED off
        adc1_half_conv_complete = false;
        process_frame(0);  // 0 to 2000
      }

      if (adc1_full_conv_complete && !adc1_full_conv_overrun) 
      {
        HAL_GPIO_WritePin(GPIOA, GPIO_PIN_5, GPIO_PIN_SET);        // LED on
        adc1_full_conv_complete = false;
        process_frame(2000); // 2000 to 4000
        
        // 4000 Samples all together divided by 2 inputs = 2000 samples per input
        int meanA0 = sumA0 / 2000;
        int meanA1 = sumA1 / 2000;
        
        sprintf(log_buffer,"Mean A0 %d\r\n", meanA0);
        debug_printf(log_buffer);

        sprintf(log_buffer,"Mean A1 %d\r\n", meanA1);
        debug_printf(log_buffer);
        
        sumA0 = 0;
        sumA1 = 0;
      }
    /* USER CODE END WHILE */


Makefile flags -c99, change line:

    CFLAGS += -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@:%.o=%.d)"

to

    CFLAGS += -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@:%.o=%.d)" -std=c99
    

