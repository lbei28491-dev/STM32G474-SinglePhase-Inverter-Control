/*
 * Compatibility shim for STM32 HAL configuration header.
 * The CubeMX-generated config lives in Core/Inc, so this wrapper keeps
 * driver-layer includes working even when the project include paths are
 * resolved differently by the editor or build tools.
 */
#include "../../../Core/Inc/stm32g4xx_hal_conf.h"
