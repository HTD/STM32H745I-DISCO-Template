/**
 * @file      hal_ex.c
 * @author    CodeDog
 * @brief     HAL extensions for STM32 drivers.
 * @remarks
 *            Made to improve both speed and readability of GPIO related C code.
 *
 *            For GPIO_EX reference see
 *            https://embeddedbucket.wordpress.com/2017/02/19/configuring-gpios-with-hal-and-cmsis-part-1/
 *
 * @copyright (c)2022 CodeDog, All rights reserved.
 */

#include "hal_ex.h"

/**
 * @brief Contains registered callbacks for all the EXTI lines.
 */
static GPIO_EXTI_CallbackBinding EXTI_callbacks[16];

/**
 * @brief Gets the 2 bits from the 32-bit GPIO register for the pin.
 *
 * @param reg GPIO 32-bit register pointer.
 * @param m16 16-bit mask for the pin.
 * @return 2-bit value.
 */
static inline uint8_t get_32_bit_register_bits(__IO uint32_t* reg, uint16_t m16)
{
  uint8_t p; for (p = 0; p < 16; p++) if ((m16 >> p) & 1) break;
  return (*reg >> (p << 1)) & 3;
}

/**
 * @brief Sets the 2 bits in the 32-bit GPIO register for the pin.
 *
 * @param reg GPIO 32-bit register pointer.
 * @param m16 16-bit mask for the pin.
 * @param value 2-bit value to set.
 */
static inline void set_32_bit_register_bits(__IO uint32_t* reg, uint16_t m16, uint8_t value)
{
  uint8_t p; for (p = 0; p < 16; p++) if ((m16 >> p) & 1) break;
  uint32_t t = *reg; t &= ~(3 << (p << 1)); t |= ((uint32_t)value << (p << 1)); *reg = t;
}

GPIO_EX_ModeTypeDef HAL_EX_GPIO_get_mode(GPIO_PinTypeDef* pin)
{
  return get_32_bit_register_bits(&pin->Port->MODER, pin->Bit);
}

void HAL_EX_GPIO_set_mode(GPIO_PinTypeDef* pin, GPIO_EX_ModeTypeDef value)
{
  set_32_bit_register_bits(&pin->Port->MODER, pin->Bit, value);
}

GPIO_EX_PullTypeDef HAL_EX_GPIO_get_pull(GPIO_PinTypeDef* pin)
{
  return get_32_bit_register_bits(&pin->Port->PUPDR, pin->Bit);
}

void HAL_EX_GPIO_set_pull(GPIO_PinTypeDef* pin, GPIO_EX_PullTypeDef value)
{
  set_32_bit_register_bits(&pin->Port->PUPDR, pin->Bit, value);
}

GPIO_EX_OTTypeDef HAL_EX_GPIO_get_OT(GPIO_PinTypeDef* pin)
{
  return (pin->Port->OTYPER & pin->Bit) != 0;
}

void HAL_EX_GPIO_set_OT(GPIO_PinTypeDef* pin, GPIO_EX_OTTypeDef value)
{
  if (value) pin->Port->OTYPER |= pin->Bit; else pin->Port->OTYPER &= ~pin->Bit;
}

GPIO_EX_SpeedTypeDef HAL_EX_GPIO_get_speed(GPIO_PinTypeDef* pin)
{
  return get_32_bit_register_bits(&pin->Port->OSPEEDR, pin->Bit);
}

void HAL_EX_GPIO_set_speed(GPIO_PinTypeDef* pin, GPIO_EX_SpeedTypeDef value)
{
  set_32_bit_register_bits(&pin->Port->OSPEEDR, pin->Bit, value);
}

void HAL_EXTI_RegisterCallbackEx(GPIO_PinTypeDef *pin, GPIO_EXTI_CallbackTypeDef callback)
{
  int line = __builtin_ctz(pin->Bit);
  EXTI_callbacks[line].pin = pin;
  EXTI_callbacks[line].callback = callback;
}

void HAL_EXTI_UnregisterCallbackEx(GPIO_PinTypeDef *pin)
{
  int line = __builtin_ctz(pin->Bit);
  EXTI_callbacks[line].pin = NULL;
  EXTI_callbacks[line].callback = NULL;
}

/**
 * @brief Overrides `__weak void HAL_GPIO_EXTI_Callback(uint16_t pin)`.
 *        Called on EXTI interrupt.
 *
 * @param pin GPIO pin bit.
 */
void HAL_GPIO_EXTI_Callback(uint16_t pin)
{
  GPIO_EXTI_CallbackBinding* binding = &EXTI_callbacks[__builtin_ctz(pin)];
  if (binding->pin && binding->callback) binding->callback(HAL_EX_PIN_GET_STATE(binding->pin));
}
