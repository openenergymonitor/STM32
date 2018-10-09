## STM32 F103 (Blue Pill)

Initial project build using STM32CubeMX:

1. Start STM32CubeMX
2. Select STM32 F103 C8T
3. SET PIN PC13 to GPIO_Output
4. Select Makefile in project settings.
3. Generate Code

Fix Makefile (remove duplicate entries set BINPATH = /usr/bin)

Add blink code to main.c:

    /* Infinite loop */
    /* USER CODE BEGIN WHILE */
    while (1)
    {
      HAL_GPIO_TogglePin(GPIOC, GPIO_PIN_13);
      /* Insert delay 100 ms */
      HAL_Delay(500);

Compile:

    make

Upload using st-flash:
    
    st-flash write build/Blink.bin 0x8000000

