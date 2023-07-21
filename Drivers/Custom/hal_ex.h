/**
 * @file      hal_ex.h
 * @author    CodeDog
 * @brief     HAL extensions for STM32 drivers, header file.
 * @remarks
 *            For GPIO_EX reference see
 *            https://embeddedbucket.wordpress.com/2017/02/19/configuring-gpios-with-hal-and-cmsis-part-1/
 *
 * @copyright (c)2023 CodeDog, All rights reserved.
 */

#pragma once

#include "stm32h7xx_hal.h" ///< Appopriate HAL driver header.
#include <stdbool.h>

/**
 * @brief GPIO port address and 16-bit pin mask structure.
 */
typedef struct
{
  GPIO_TypeDef* Port; // General purpose I/O port structure pointer.
  uint16_t Bit;       // Pin bit mask.
  uint8_t LastState;  // Last state value storage (optional, set in user code).
  char* Name;         // Optional pin name.
} GPIO_PinTypeDef;

/// @brief Macro setting a GPIO pin to H.
#define HAL_EX_PIN_SET_L(GPIO) (GPIO)->Port->ODR &= ~(GPIO)->Bit

/// @brief Macro setting a GPIO pin to L.
#define HAL_EX_PIN_SET_H(GPIO) (GPIO)->Port->ODR |= (GPIO)->Bit

/// @brief Macro returning a GPIO pin state.
#define HAL_EX_PIN_GET_STATE(GPIO) (((GPIO)->Port->IDR & (GPIO)->Bit) != 0)

/**
 * @brief TIM output types.
 */
typedef enum
{
  TIM_GP,   // General purpose timer.
  TIM_IRQ,  // IRQ output.
  TIM_PWM   // PWM output.
} TIM_OutputTypeDef;

/**
 * @brief TIM configuration structure.
 */
typedef struct
{
  TIM_HandleTypeDef* TIM;   // TIM handle pointer.
  uint32_t CHANNEL_DEFAULT; // TIM channel. 0 for no channel output.
  TIM_OutputTypeDef TYPE;   // TIM output type.
  char* Name;               // Optional timer name.
} TIM_ConfigTypeDef;

/**
 * @brief GPIO MODER register values.
 */
typedef enum
{
  GPIO_EX_MODE_INPUT  = 0x00u, // Input (reset state).
  GPIO_EX_MODE_OUTPUT = 0x01u, // General purpsose output mode.
  GPIO_EX_MODE_AF     = 0x10u, // Alternate function mode.
  GPIO_EX_MODE_ANALOG = 0x11u  // Analog mode.
} GPIO_EX_ModeTypeDef;

/**
 * @brief GPIO OTYPER register values.
 */
typedef enum
{
  GPIO_EX_OT_PP = 0x00, // Output push-pull (reset state).
  GPIO_EX_OT_OD = 0x01  // Output open drain.
} GPIO_EX_OTTypeDef;

/**
 * @brief GPIO pull resistor configuration values.
 */
typedef enum
{
  GPIO_EX_PULL_NONE = 0x00, // Floating (reset state).
  GPIO_EX_PULL_UP   = 0x01, // Internal weak pull-up.
  GPIO_EX_PULL_DOWN = 0x10  // Internal weak pull-down.
} GPIO_EX_PullTypeDef;

/**
 * @brief GPIO OSPEEDR register values.
 */
typedef enum
{
  GPIO_EX_SPD_LOW     = 0x00, // Low speed.
  GPIO_EX_SPD_MEDIUM  = 0x01, // Medium speed.
  GPIO_EX_SPD_HIGH    = 0x10, // High speed.
  GPIO_EX_SPD_MAX     = 0x11  // Very high speed.
} GPIO_EX_SpeedTypeDef;

/**
 * @brief A callback from the EXTI passing the pin state.
 */
typedef void(*GPIO_EXTI_CallbackTypeDef)(bool pinState);

/**
 * @brief EXTI pin binding structure.
 */
typedef struct
{
  GPIO_PinTypeDef* pin;               // GPIO pin configuration pointer.
  GPIO_EXTI_CallbackTypeDef callback; // A function receiving the pin state.
} GPIO_EXTI_CallbackBinding;

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Gets the GPIO mode (input / output / AF / analog).
 *
 * @param pin GPIO pin configuration pointer.
 * @return GPIO mode register bits as enumeration.
 */
GPIO_EX_ModeTypeDef HAL_EX_GPIO_get_mode(GPIO_PinTypeDef* pin);

/**
 * @brief Sets the GPIO mode (input / output / AF / analog).
 *
 * @param pin GPIO pin configuration pointer.
 * @param value GPIO mode register bits as enumeration.
 */
void HAL_EX_GPIO_set_mode(GPIO_PinTypeDef* pin, GPIO_EX_ModeTypeDef value);

/**
 * @brief Gets the GPIO mode pull-up / pull-down register value.
 *
 * @param pin GPIO pin configuration pointer.
 * @return GPIO PUPDR register bits as enumeration.
 */
GPIO_EX_PullTypeDef HAL_EX_GPIO_get_pull(GPIO_PinTypeDef* pin);

/**
 * @brief Sets the GPIO mode pull-up / pull-down register value.
 *
 * @param pin GPIO pin configuration pointer.
 * @param value PUPDR register bits as enumeration.
 */

void HAL_EX_GPIO_set_pull(GPIO_PinTypeDef* pin, GPIO_EX_PullTypeDef value);

/**
 * @brief Gets the GPIO output type register value.
 *
 * @param pin GPIO pin configuration pointer.
 * @return OTYPER register bits as enumeration.
 */
GPIO_EX_OTTypeDef HAL_EX_GPIO_get_OT(GPIO_PinTypeDef* pin);

/**
 * @brief Gets the GPIO output type register value.

 * @param pin GPIO pin configuration pointer.
 * @param value OTYPER register bits as enumeration.
 */
void HAL_EX_GPIO_set_OT(GPIO_PinTypeDef* pin, GPIO_EX_OTTypeDef value);

/**
 * @brief Gets the GPIO output speed register value.
 *
 * @param pin GPIO pin configuration pointer.
 * @return OSPEED register bits as enumeration.
 */
GPIO_EX_SpeedTypeDef HAL_EX_GPIO_get_speed(GPIO_PinTypeDef* pin);

/**
 * @brief Sets the GPIO output speed register value.
 *
 * @param pin GPIO pin configuration pointer.
 * @param value OSPEED register bits as enumeration.
 */
void HAL_EX_GPIO_set_speed(GPIO_PinTypeDef* pin, GPIO_EX_SpeedTypeDef value);

/**
 * @brief Registers a callback assigned to the specified pin / EXTI line.
 *        There can be only one callback per EXTI line.
 *
 * @param pin GPIO pin configuration pointer.
 * @param callback A function receiving the pin state when the interrupt occurs.
 */
void HAL_EXTI_RegisterCallbackEx(GPIO_PinTypeDef* pin, GPIO_EXTI_CallbackTypeDef callback);

/**
 * @brief Unregisters a callback assigned to the specified pin / EXTI line.
 *
 * @param pin GPIO pin configuration pointer.
 */
void HAL_EXTI_UnregisterCallbackEx(GPIO_PinTypeDef* pin);

#ifdef __cplusplus
}
#endif
