//---------------------------------------
// RESET CAUSE (aka BOOT REASON)
//---------------------------------------


#include "usart.h"
#include "reset.h"

/// @brief      Obtain the STM32 system reset cause
/// @param      None
/// @return     The system reset cause
reset_cause_t reset_cause_get(void)
{
    reset_cause_t reset_cause;
    /*
    *            @arg @ref RCC_FLAG_OBLRST    Option Byte Load reset
    *            @arg @ref RCC_FLAG_PINRST    Pin reset.
    *            @arg @ref RCC_FLAG_PORRST    POR/PDR reset.
    *            @arg @ref RCC_FLAG_SFTRST    Software reset.
    *            @arg @ref RCC_FLAG_IWDGRST   Independent Watchdog reset.
    *            @arg @ref RCC_FLAG_WWDGRST   Window Watchdog reset.
    *            @arg @ref RCC_FLAG_LPWRRST   Low Power reset.
    * */
    if (__HAL_RCC_GET_FLAG(RCC_FLAG_LPWRRST))
    {
        reset_cause = RESET_CAUSE_LOW_POWER_RESET;
    }
    else if (__HAL_RCC_GET_FLAG(RCC_FLAG_WWDGRST))
    {
        reset_cause = RESET_CAUSE_WINDOW_WATCHDOG_RESET;
    }
    else if (__HAL_RCC_GET_FLAG(RCC_FLAG_IWDGRST))
    {
        reset_cause = RESET_CAUSE_INDEPENDENT_WATCHDOG_RESET;
    }
    else if (__HAL_RCC_GET_FLAG(RCC_FLAG_SFTRST))
    {
        reset_cause = RESET_CAUSE_SOFTWARE_RESET; // This reset is induced by calling the ARM CMSIS `NVIC_SystemReset()` function!
    }
    else if (__HAL_RCC_GET_FLAG(RCC_FLAG_PORRST))
    {
        reset_cause = RESET_CAUSE_POWER_ON_POWER_DOWN_RESET;
    }
    else if (__HAL_RCC_GET_FLAG(RCC_FLAG_PINRST))
    {
        reset_cause = RESET_CAUSE_EXTERNAL_RESET_PIN_RESET;
    }
    else if (__HAL_RCC_GET_FLAG(RCC_FLAG_OBLRST))
    {
        reset_cause = RESET_CAUSE_EXTERNAL_OPTION_BYTE_LOAD_RESET;
    }
    else
    {
        reset_cause = RESET_CAUSE_UNKNOWN;
    }

    // Clear all the reset flags or else they will remain set during future resets until system power is fully removed.
    __HAL_RCC_CLEAR_RESET_FLAGS();

    return reset_cause;
}

// Note: any of the STM32 Hardware Abstraction Layer (HAL) Reset and Clock Controller (RCC) header
// files, such as "STM32Cube_FW_F7_V1.12.0/Drivers/STM32F7xx_HAL_Driver/Inc/stm32f7xx_hal_rcc.h",
// "STM32Cube_FW_F2_V1.7.0/Drivers/STM32F2xx_HAL_Driver/Inc/stm32f2xx_hal_rcc.h", etc., indicate that the
// brownout flag, `RCC_FLAG_BORRST`, will be set in the event of a "POR/PDR or BOR reset". This means that a
// Power-On Reset (POR), Power-Down Reset (PDR), OR Brownout Reset (BOR) will trip this flag. See the
// doxygen just above their definition for the `__HAL_RCC_GET_FLAG()` macro to see this:
// "@arg RCC_FLAG_BORRST: POR/PDR or BOR reset." <== indicates the Brownout Reset flag will *also* be set in
// the event of a POR/PDR.
// Therefore, you must check the Brownout Reset flag, `RCC_FLAG_BORRST`, *after* first checking the
// `RCC_FLAG_PORRST` flag in order to ensure first that the reset cause is NOT a POR/PDR reset.

/// @brief      Obtain the system reset cause as an ASCII-printable name string from a reset cause type
/// @param[in]  reset_cause     The previously-obtained system reset cause
/// @return     A null-terminated ASCII name string describing the system reset cause
const char *reset_cause_get_name(reset_cause_t reset_cause)
{
    const char *reset_cause_name = "TBD";

    switch (reset_cause)
    {
    case RESET_CAUSE_UNKNOWN:
        reset_cause_name = "UNKNOWN";
        break;
    case RESET_CAUSE_LOW_POWER_RESET:
        reset_cause_name = "LOW_POWER_RESET";
        break;
    case RESET_CAUSE_WINDOW_WATCHDOG_RESET:
        reset_cause_name = "WINDOW_WATCHDOG_RESET";
        break;
    case RESET_CAUSE_INDEPENDENT_WATCHDOG_RESET:
        reset_cause_name = "INDEPENDENT_WATCHDOG_RESET";
        break;
    case RESET_CAUSE_SOFTWARE_RESET:
        reset_cause_name = "SOFTWARE_RESET";
        break;
    case RESET_CAUSE_POWER_ON_POWER_DOWN_RESET:
        reset_cause_name = "POWER-ON_RESET (POR) / POWER-DOWN_RESET (PDR)";
        break;
    case RESET_CAUSE_EXTERNAL_RESET_PIN_RESET:
        reset_cause_name = "EXTERNAL_RESET_PIN_RESET";
        break;
    case RESET_CAUSE_EXTERNAL_OPTION_BYTE_LOAD_RESET:
        reset_cause_name = "OPTION_BYTE_LOAD_RESET";
        break;
    }

    return reset_cause_name;
}

reset_cause_t reset_cause_store;

/*
Example usage:
reset_cause_t reset_cause = reset_cause_get();
printf("The system reset cause is \"%s\"\n", reset_cause_get_name(reset_cause));
*/


