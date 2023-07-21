/**
 * @file      lf_filter.c
 * @author    CodeDog
 * @brief     A simple digital filter that requires a digital pin state to be stable for a certain time to trigger an event.
 * @remarks
 *            Using delay_async timer (with usually 1kHz time source) to sample the input pin state.
 *
 * @copyright	(c)2022 CodeDog, All rights reserved.
 */

#include "lf_filter.h"

static LFF_HandleTypeDef handles[LFF_HCOUNT];

/**
 * @fn void callback(LFF_HandleTypeDef*)
 * @brief The function called on each sample interval.
 * @param h LF Filter handle.
 */
static void callback(LFF_HandleTypeDef* h)
{
  if (!h || !h->state_changed) return;
  uint8_t cs = (h->pin->Port->IDR & h->pin->Bit) != 0; // Current sample.
  uint8_t sc = cs != h->last_sample; // Sample value changed.
  h->last_sample = cs;
  if (cs)
  {
    if (sc && h->counter > h->current_min)
    {
      h->current_min = h->counter;
      if (h->stats_changed) h->stats_changed(h);
    }
    if (h->counter < h->sample_threshold) h->counter++;
    else if (cs != h->current_state)
    {
      h->current_state = cs;
      h->current_min = 0;
      h->current_max = 0;
      h->state_changed(h);
    }
  }
  else
  {
    if (sc && h->counter > h->current_max)
    {
      h->current_max = h->counter;
      if (h->stats_changed) h->stats_changed(h);
    }
    if (h->counter > 0) h->counter--;
    else if (cs != h->current_state)
    {
      h->current_state = cs;
      h->current_min = 0;
      h->current_max = 0;
      h->state_changed(h);
    }
  }
}

/**
 * @fn HAL_StatusTypeDef LFF_RegisterPin(GPIO_TypeDef*, uint16_t, float, uint16_t, LFFilter_CallbackTypeDef)
 * @brief Registers a pin filter for the specified pin.
 * @param pin GPIO pin structure pointer.
 * @param sampleInterval Time in milliseconds between the samples.
 * @param sampleThreshold The number of samples of the same value to trigger the event.
 * @param stateChanged A function to call when the pin state changes.
 * @return HAL status.
 */
HAL_StatusTypeDef LFF_RegisterPin(GPIO_PinTypeDef* pin, float sampleInterval, uint16_t sampleThreshold,
    LFF_CallbackTypeDef stateChanged, const void* binding)
{
  if (!stateChanged || !sampleInterval || !sampleThreshold) return HAL_ERROR;
  for (uint8_t i = 0; i < LFF_HCOUNT; i++)
  {
    if (!handles[i].state_changed)
    {
      LFF_HandleTypeDef* h = &handles[i];
      h->pin = pin;
      h->counter = 0;
      h->current_state = h->last_sample = (h->pin->Port->IDR & h->pin->Bit) == 0; // inverted to force the first event
      h->sample_interval = sampleInterval;
      h->sample_threshold = sampleThreshold;
      h->state_changed = stateChanged;
      h->stats_changed = NULL;
      h->binding = binding;
      h->wait_handle = repeat_async(h->sample_interval, h, (TIM_EX_CallbackTypeDef) callback);
      return HAL_OK;
    }
  }
  return HAL_ERROR; // out of handles.
}

/**
 * @fn LFF_HandleTypeDef LFF_GetHandle*(GPIO_PinTypeDef*)
 * @brief Gets the LFF handle pointer for a registered pin.
 * @param pin GPIO pin structure pointer.
 * @return LFF handle pointer if the pin pointer was registered, NULL otherwise.
 */
LFF_HandleTypeDef* LFF_GetHandle(GPIO_PinTypeDef* pin)
{
    if (!pin) return NULL;
    for (uint8_t i = 0; i < LFF_HCOUNT; i++)
    {
        LFF_HandleTypeDef* h = &handles[i];
        if (h->pin == pin) return h;
    }
    return NULL;
}

/**
 * @fn HAL_StatusTypeDef LFF_ChangeInterval(GPIO_PinTypeDef*, float)
 * @brief Changes registered pin polling interval.
 * @param pin Registered GPIO pin structure pointer.
 * @param interval New interval value in milliseconds.
 * @return HAL_OK: Pin matched. HAL_ERROR: Pin not matched.
 */
HAL_StatusTypeDef LFF_ChangeInterval(GPIO_PinTypeDef* pin, float interval)
{
    LFF_HandleTypeDef* h = LFF_GetHandle(pin);
    if (!h || !h->wait_handle) return HAL_ERROR;
    wait_handle_reset(h->wait_handle, h->sample_interval = interval);
    return HAL_OK;
}

/**
 * @fn HAL_StatusTypeDef LFF_ChangeThreshold(GPIO_PinTypeDef*, float)
 * @brief Changes registered pin required same value samples threshold.
 * @param pin Registered GPIO pin structure pointer.
 * @param threshold Required same value samples threshold.
 * @return HAL_OK: Pin matched. HAL_ERROR: Pin not matched.
 */
HAL_StatusTypeDef LFF_ChangeThreshold(GPIO_PinTypeDef* pin, float threshold)
{
    LFF_HandleTypeDef* h = LFF_GetHandle(pin);
    if (!h) return HAL_ERROR;
    h->sample_threshold = threshold;
    return HAL_OK;
}

/**
 * @fn HAL_StatusTypeDef LFF_UnregisterPin(GPIO_PinTypeDef*)
 * @brief Unregisters previously registered pin and returns its resources to the pool.
 * @param pin Registered GPIO pin structure pointer.
 * @return HAL_OK: Pin matched. HAL_ERROR: Pin not matched.
 */
HAL_StatusTypeDef LFF_UnregisterPin(GPIO_PinTypeDef* pin)
{
    LFF_HandleTypeDef* h = LFF_GetHandle(pin);
    if (!h || !h->wait_handle) return HAL_ERROR;
    wait_handle_dispose(h->wait_handle);
    h->wait_handle = NULL;
    h->state_changed = NULL;
    h->stats_changed = NULL;
    h->pin = NULL;
    h->current_state = 0;
    h->sample_interval = 0;
    h->sample_threshold = 0;
    h->binding = NULL;
    return HAL_OK;
}
