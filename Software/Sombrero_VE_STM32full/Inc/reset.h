//---------------------------------------
// RESET CAUSE (aka BOOT REASON)
//---------------------------------------
#ifndef RST_CAUSE_H
#define RST_CAUSE_H

#include "usart.h"

/// @brief  Possible STM32 system reset causes
typedef enum reset_cause_e
{
    RESET_CAUSE_UNKNOWN = 0,
    RESET_CAUSE_LOW_POWER_RESET,
    RESET_CAUSE_WINDOW_WATCHDOG_RESET,
    RESET_CAUSE_INDEPENDENT_WATCHDOG_RESET,
    RESET_CAUSE_SOFTWARE_RESET,
    RESET_CAUSE_POWER_ON_POWER_DOWN_RESET,
    RESET_CAUSE_EXTERNAL_RESET_PIN_RESET,
    RESET_CAUSE_EXTERNAL_OPTION_BYTE_LOAD_RESET,
} reset_cause_t;

/// @brief      Obtain the STM32 system reset cause
/// @param      None
/// @return     The system reset cause
reset_cause_t reset_cause_get(void);

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
const char *reset_cause_get_name(reset_cause_t reset_cause);

reset_cause_t reset_cause_store;

/*
Example usage:
reset_cause_t reset_cause = reset_cause_get();
printf("The system reset cause is \"%s\"\n", reset_cause_get_name(reset_cause));
*/


#endif