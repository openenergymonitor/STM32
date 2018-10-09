## Analog Read

**Basic example of reading from a single analog input**

Start by setting PA0 to a single-ended ADC1 analog input.

![STMCubeADC1.png](images/STMCubeADC1.png)

Navigate to the configuration tab for ADC1 and configure as follows:

![STMCubeADC2.png](images/STMCubeADC2.png)

Project > Generate Code

Towards the bottom of the file adc.c, in the 'user code begin 1' section add the following function:

    uint32_t analogRead()
    {
      uint32_t ADCValue = 0;
      HAL_ADC_Start(&hadc1);
      if (HAL_ADC_PollForConversion(&hadc1, 1000000) == HAL_OK) {
          ADCValue = HAL_ADC_GetValue(&hadc1);
      }
      HAL_ADC_Stop(&hadc1);
      
      return ADCValue;
    }
    
Call the function and print the result in main.c:

    /* Infinite loop */
    /* USER CODE BEGIN WHILE */
    while (1)
    {
      sprintf(log_buffer, "ADCValue: %d\r\n", analogRead());
      debug_printf(log_buffer);
      HAL_Delay(200);
          
      HAL_GPIO_TogglePin(GPIOA, GPIO_PIN_5);
      HAL_Delay(100);
    /* USER CODE END WHILE */
    
Compile and Upload, see the result with minicom:

    minicom -b115200 -D/dev/ttyACM0
    
Example result:
                                                               
    ADCValue: 0
    ADCValue: 0
    ADCValue: 0
    ADCValue: 0
    ADCValue: 0
    ADCValue: 0
    ADCValue: 4038
    ADCValue: 4038
    ADCValue: 4037
    ADCValue: 4038
    ADCValue: 4037
    
**Reading from two analog inputs**

Using STM32CubeMX, configure PA1 as a single ended analog input as well and click on 'Generate Code' again.

Modify the analagRead function to include the following channel configuration section:

    uint32_t analogRead(uint32_t channel)
    {

      ADC_ChannelConfTypeDef sConfig;
      sConfig.Channel = channel;
      sConfig.Rank = ADC_REGULAR_RANK_1;
      sConfig.SingleDiff = ADC_SINGLE_ENDED;
      sConfig.SamplingTime = ADC_SAMPLETIME_601CYCLES_5;
      sConfig.OffsetNumber = ADC_OFFSET_NONE;
      sConfig.Offset = 0;

      if (HAL_ADC_ConfigChannel(&hadc1, &sConfig) != HAL_OK) {
        _Error_Handler(__FILE__, __LINE__);
      }

      uint32_t ADCValue = 0;
      HAL_ADC_Start(&hadc1);
      if (HAL_ADC_PollForConversion(&hadc1, 1000000) == HAL_OK) {
          ADCValue = HAL_ADC_GetValue(&hadc1);
      }
      HAL_ADC_Stop(&hadc1);
      
      return ADCValue;
    }
    
This configuration code is copied straight from the bottom of the MX_ADC1_Init function.
    
Call the modified function from main.c:

    /* Infinite loop */
    /* USER CODE BEGIN WHILE */
    while (1)
    {
      sprintf(log_buffer, "ADCValue 1: %d\r\n", analogRead(1));
      debug_printf(log_buffer);
      HAL_Delay(200);
      
      sprintf(log_buffer, "ADCValue 2: %d\r\n", analogRead(2));
      debug_printf(log_buffer);
      HAL_Delay(200);
          
      HAL_GPIO_TogglePin(GPIOA, GPIO_PIN_5);
      HAL_Delay(100);
    /* USER CODE END WHILE */
    
Example output with PA0 connected to ground and PA1 connected to 3.3V:

    ADCValue 1: 0
    ADCValue 2: 4037
    ADCValue 1: 0
    ADCValue 2: 4038
    ADCValue 1: 0
    ADCValue 2: 4037
    ADCValue 1: 0
    ADCValue 2: 4038
    ADCValue 1: 0
    ADCValue 2: 4038
    ADCValue 1: 0
    ADCValue 2: 4037
    ADCValue 1: 0
    ADCValue 2: 4037
    
**Reading from two analog inputs sequentially with scan mode**

The last example configure's, starts, reads and then stops the ADC for each channel. With scan conversion mode its possible to automatically read through a configured list of channels sequentially.

Enable 'Scan Conversion Mode' in the ADC configuration window. 

Keep End of Conversion Selection on 'End of single conversion'.

![STMCubeADC3.png](images/STMCubeADC3.png)

Remove the analogRead function from adc.c that we added in the previous examples. Replace the code in the main.c loop with:

    /* Infinite loop */
    /* USER CODE BEGIN WHILE */
    while (1)
    {

      HAL_ADC_Start(&hadc1);
      
      if (HAL_ADC_PollForConversion(&hadc1, 1000) == HAL_OK) {
          ADCValue = HAL_ADC_GetValue(&hadc1);
          sprintf(log_buffer, "ADCValue 1: %d\r\n", ADCValue);
          debug_printf(log_buffer);
          
          ADCValue = HAL_ADC_GetValue(&hadc1);
          sprintf(log_buffer, "ADCValue 2: %d\r\n", ADCValue);
          debug_printf(log_buffer);
      }
      
      HAL_ADC_Stop(&hadc1);
      HAL_Delay(200);
          
      HAL_GPIO_TogglePin(GPIOA, GPIO_PIN_5);
      HAL_Delay(100);
    /* USER CODE END WHILE */
    
**Analog read on completion interrupt**

This example replaces the polling for completed ADC conversions with an callback function called via an interrupt.

- Enable 'Scan Conversion Mode' in the ADC configuration window. 
- Keep End of Conversion Selection on 'End of single conversion'.
- NVIC Settings: Enable ADC1 and ADC2 interrupts.

In adc.c:

    /* USER CODE BEGIN 0 */
    char log_buffer[100];

    /* USER CODE END 0 */
    
    ---

    /* USER CODE BEGIN 1 */
    void HAL_ADC_ConvCpltCallback(ADC_HandleTypeDef* hadc1)
    {
      uint32_t ADCValue = HAL_ADC_GetValue(hadc1);
      sprintf(log_buffer, "ADCValue 1: %d\r\n", ADCValue);
      debug_printf(log_buffer);

      ADCValue = HAL_ADC_GetValue(hadc1);
      sprintf(log_buffer, "ADCValue 2: %d\r\n", ADCValue);
      debug_printf(log_buffer);
    }
    /* USER CODE END 1 */
    
In main.c:

    /* Infinite loop */
    /* USER CODE BEGIN WHILE */
    while (1)
    {     
      HAL_ADC_Start_IT(&hadc1);
      HAL_GPIO_TogglePin(GPIOA, GPIO_PIN_5);
      HAL_Delay(500);
    /* USER CODE END WHILE */
