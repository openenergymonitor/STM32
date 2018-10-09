## STMCubeMX 101: Serial Output

To enable USART output enable the RX and TX pins for USART2 (PA2 & PA3) using the pinout viewer:

![STMCube5a.png](images/STMCube5a.png)

Click on Configuration, USART2 and then Parameter Settings and enter your desired baud rate, worth length:

![STMCube5b.png](images/STMCube5b.png)

Click on 'Generate Code' again and open main.c in an editor again. Add a char array to hold output contents at the top of the file in the private variables section:

In main.c:

    /* USER CODE BEGIN PV */
    /* Private variables ---------------------------------------------------------*/
    char log_buffer[100];

and then the following lines in the loop to print 'Hello World':

    /* Infinite loop */
    /* USER CODE BEGIN WHILE */
    while (1)
    {
      HAL_GPIO_TogglePin(GPIOA, GPIO_PIN_5);
      
      sprintf(log_buffer,"Hello World\r\n");
      debug_printf(log_buffer);
      
      HAL_Delay(200);
    /* USER CODE END WHILE */

    /* USER CODE BEGIN 3 */

    }
    /* USER CODE END 3 */
    
At the bottom of usart.c, add the following function to print the string:

    /* USER CODE BEGIN 1 */
    void debug_printf (char* p) {
      HAL_UART_Transmit(&huart2, (uint8_t*)p, strlen(p), 1000);
    }
    /* USER CODE END 1 */

Compile and upload again.
    
Serial monitor with minicom:

    minicom -b115200 -D/dev/ttyACM0
