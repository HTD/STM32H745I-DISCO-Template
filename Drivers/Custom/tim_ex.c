/**
 * @file        delay_async.c
 * @author      CodeDog
 * @brief       HAL TIM extensions.
 * @remarks
 *              Uses HAL TIM interrupt to schedule events.
 *
 * @copyright   (c)2022 CodeDog, All rights reserved.
 */

#include "tim_ex.h"

#if (defined(HAL_TIM_MODULE_ENABLED) && USE_HAL_TIM_REGISTER_CALLBACKS > 0)

#ifdef __cplusplus
extern "C" {
#endif

static void TIM_callback(TIM_HandleTypeDef* htim);

#ifdef __cplusplus
}
#endif

/**
 * Timer configuration.
 */
static TIM_EX_ConfigTypeDef config = { };

/**
 * Stopwatch handle.
 */
static StopwatchHandleTypeDef sw_handle = { };

/**
 * Events queue AKA wait handles.
 */
static WaitHandleTypeDef wait_handles[WAIT_HANDLES_N];

/**
 * Wait handles pool current size.
 */
static uint8_t pool_size = 0;

/**
 * @fn void _delay_async_tim_callback(TIM_HandleTypeDef*)
 * @brief Callback for TIM period elapsed.
 * @param htim TIM handle pointer.
 */
static void TIM_callback(TIM_HandleTypeDef* htim)
{
  if (sw_handle.active)
  {
    sw_handle.ticks++;
    if ((sw_handle.ticks % sw_handle.ticks_per_second) == 0 && sw_handle.second_tick)
      sw_handle.second_tick(sw_handle.ticks / (double)sw_handle.ticks_per_second);
  }
  for (uint8_t i = 0; i < pool_size; i++)
  {
    WaitHandleTypeDef* event = &wait_handles[i];
    if (event->active && event->callback)
    {
      if (event->ticks_left > 0)
      {
        event->ticks_left--;
        continue;
      }
      if (event->reset)
      {
        event->ticks_left = event->reset - 1;
        event->callback(event->binding);
      }
      else
      {
        event->active = 0;
        if (i == pool_size - 1) --pool_size;
        event->callback(event->binding);
      }
    }
  }
}

/**
 * @fn uint16_t _delay_get_ticks_per_ms(double)
 * @brief Calculates the number of ticks for t milliseconds.
 * @param t Number of milliseconds.
 * @return Number of ticks.
 */
static inline uint32_t get_ticks(double t)
{
  return round(t * pow(10, config.tick_time_exp - 6));
}

/**
 * @fn HAL_StatusTypeDef _delay_async(double, uint32_t, void*, void(*)(void*))
 * @brief Delays or repeats the callback in interrupt mode.
 * @param ms Number of millisecods to wait. Fractions are valid if configured tick time is small enough.
 * @param reset_ms When time elapses the delay will be restarted after this number of millseconds.
 * @param binding A pointer that is passed as the callback argument.
 * @param callback A function to call when the time elapses.
 * @return Wait handle pointer or NULL on error.
 */
inline static WaitHandleTypeDef* _delay_async(double ms, double reset_ms, void* binding, TIM_EX_CallbackTypeDef callback)
{
  uint32_t ticks = get_ticks(ms);
  uint32_t reset = reset_ms == 0 ? 0 : get_ticks(reset_ms);
  if (ticks < 1) return NULL;
  for (uint8_t i = 0; i < WAIT_HANDLES_N; i++)
  {
    WaitHandleTypeDef* event = &wait_handles[i];
    if (!event->active)
    {
      event->ticks_left = ticks;
      event->reset = reset;
      event->binding = binding;
      event->callback = callback;
      event->active = 1;
      if (i + 1 > pool_size) pool_size = i + 1;
      return event;
    }
  }
  return NULL;
}

/**
 * @fn HAL_StatusTypeDef delay_async_init(TIM_HandleTypeDef*, uint16_t, uint8_t)
 * @brief Configure one system timer as a time base source for precise delays.
 * @remarks Reconfigures hardware timer, use with caution.
 * @param htim Timer not used for anything else, but enabled with IRQ.
 * @param time_base Timer clock in MHz.
 * @param tick_time_exp 0: 1ns, 3: 1μs, 6: 1ms, 9: 1s.
 * @return HAL status, error on invalid values.
 */
HAL_StatusTypeDef TIM_EX_init(TIM_HandleTypeDef* htim, uint16_t time_base, uint8_t tick_time_exp)
{
  if (!htim || !time_base || config.htim)
    return HAL_ERROR;
  config.htim = htim;
  config.time_base = time_base;
  config.tick_time_exp = tick_time_exp;
  double d = (double) time_base * pow(10, tick_time_exp - 3); // time base division
  if (d < 1)
    return HAL_ERROR;
  double m = floor(log10(d) / 2); // division magnitude
  uint16_t arr = pow(10, m); // auto reload register value
  uint16_t psc = d / arr; // prescaler register value
  if (arr < 1 || psc < 1)
    return HAL_ERROR;
  htim->Instance->ARR = arr - 1;
  htim->Instance->PSC = psc - 1;
  sw_handle.ticks_per_second = get_ticks(1000.0);
  HAL_StatusTypeDef status = HAL_TIM_RegisterCallback(htim, HAL_TIM_PERIOD_ELAPSED_CB_ID, TIM_callback);
  if (status != HAL_OK)
    return status;
  status = HAL_TIM_Base_Start_IT(htim);
  if (status != HAL_OK)
    return status;
  return HAL_OK;
}

/**
 * @fn HAL_StatusTypeDef delay_async(double, void*, TIM_EX_CallbackTypeDef)
 * @brief Delays the callback in interrupt mode.
 * @param ms Number of millisecods to wait. Fractions are valid if configured tick time is small enough.
 * @param binding A pointer that is passed as the callback argument.
 * @param callback A function to call when the time elapses.
 * @return Wait handle pointer or NULL on error.
 */
WaitHandleTypeDef* delay_async(double ms, void* binding, TIM_EX_CallbackTypeDef callback)
{
  return _delay_async(ms, 0, binding, callback);
}

/**
 * @fn HAL_StatusTypeDef repeat_async(double, void*, TIM_EX_CallbackTypeDef)
 * @brief Repeats the callback in interrupt mode.
 * @param ms Number of millisecods to wait. Fractions are valid if configured tick time is small enough.
 * @param binding A pointer that is passed as the callback argument.
 * @param callback A function to call when the time elapses.
 * @return Wait handle pointer or NULL on error.
 */
WaitHandleTypeDef* repeat_async(double ms, void* binding, TIM_EX_CallbackTypeDef callback)
{
  return _delay_async(ms, ms, binding, callback);
}

/**
 * @fn void wait_handle_dispose(WaitHandleTypeDef*)
 * @brief Immediately disposes the wait handle and returs it to the pool.
 * @remarks Next timer tick should not trigger the current handle.
 * @param wh Wait handle pointer.
 */
void wait_handle_dispose(WaitHandleTypeDef* wh)
{
  if (!wh) return;
  wh->active = 0;
  wh->reset = 0;
  wh->binding = NULL;
  wh->callback = NULL;
  wh->ticks_left = 0;
  for (uint8_t i = 0; i < pool_size; i++)
    if (&wait_handles[i] == wh && i == pool_size - 1) --pool_size;
}

/**
 * @fn void wait_handle_reset(WaitHandleTypeDef*, double)
 * @brief Resets the timing of the repeat events.
 * @param wh Wait handle pointer.
 * @param ms A number of milliseconds to reset.
 */
void wait_handle_reset(WaitHandleTypeDef* wh, double ms)
{
    wh->reset = get_ticks(ms);
}

/**
 * @fn void stopwatch_start()
 * @brief Starts the stopwatch.
 */
void stopwatch_start()
{
  sw_handle.active = 1;
}

/**
 * @fn void stopwatch_stop()
 * @brief Stops the stopwatch.
 */
void stopwatch_stop()
{
  sw_handle.active = 0;
}

/**
 * @fn void stopwatch_reset()
 * @brief Resets the stopwatch.
 */
void stopwatch_reset()
{
  sw_handle.ticks = 0;
}

/**
 * @fn double stopwatch_get_time()
 * @brief Gets the time in seconds that elapsed since the stopwatch was started.
 * @remarks 2 ticks error is expected since the time base timer is shared, it is already running while the stopwatch is started and stopped.
 * @return Time in seconds.
 */
double stopwatch_get_time()
{
  return sw_handle.ticks / (double) sw_handle.ticks_per_second;
}

/**
 * @fn void stopwatch_register_callback(StopwatchCallbackTypeDef)
 * @brief Registers a callback that is called every second when the stopwatch is running.
 * @param callback A callback accepting the time in seconds.
 */
void stopwatch_register_callback(StopwatchCallbackTypeDef callback)
{
  sw_handle.second_tick = callback;
}

/**
 * @fn uint8_t get_TIM_PSC_ARR(uint16_t, double, float*, uint16_t*, uint16_t*)
 * @brief Calculates TIM PSC and ARR register values for the minimal prescaler being multiple power of 10.
 * @remarks Also calculates the frequency settings error.
 * @param tb Time base in MHz.
 * @param f Frequency setting in Hz.
 * @param psc Prescaler pointer.
 * @param arr Auto-reload register pointer.
 * @param e Error pointer in %.
 * @return 1: success, 0: invalid parameters.
 */
uint8_t get_TIM_PSC_ARR(uint16_t tb, double f, volatile uint32_t* psc, volatile uint32_t* arr, float* e)
{
  double d_arr, d_psc;
  uint8_t x1 = floor(log10(tb));
  uint8_t x2 = floor(log10(f));
  int8_t d = 6 + x1 - x2;
  if (d < 0) return 0;
  uint16_t m = pow(10, x1);
  uint8_t k = (tb % m) ? 1 : tb / m;
  for (uint8_t i = 0; i < 5; i++)
  {
    d_psc = pow(10, i);
    d_arr = tb / (double) f * pow(10, 6 - i);
    if (k < 2 && d_psc >= 1 && d_psc <= 0xffff && d_arr >= 1 && d_arr <= 0xffff)
    {
      *psc = (uint32_t) d_psc - 1;
      *arr = (uint32_t) d_arr - 1;
      if (e)
        *e = fabs(f - (tb * 1000000.0 / (double) (*psc + 1) / (double) (*arr + 1))) / f * 100.0;
      return 1;
    }
    if (d_psc * k >= 1 && d_psc * k <= 0xffff && d_arr / k >= 1 && d_arr / k < 0xffff)
    {
      *psc = (uint32_t) (d_psc * k - 1);
      *arr = (uint32_t) (d_arr / k - 1);
      if (e)
        *e = fabs(f - (tb * 1000000.0 / (double) (*psc + 1) / (double) (*arr + 1))) / f * 100.0;
      return 1;
    }
  }
  return 0;
}

/**
 * @fn HAL_StatusTypeDef set_TIM_PSC_ARR(TIM_HandleTypeDef*, double, uint16_t)
 * @brief Sets up a TIM timer for specified event frequency having prescaler set to a minimal power of 10 and high ARR value.
 * @param htim TIM handle pointer.
 * @param f Frequency setting in Hz.
 * @param tb Time base frequency in MHz.
 * @return HAL status: HAL_OK if set, HAL_ERROR on invalid parameters.
 */
HAL_StatusTypeDef set_TIM_PSC_ARR(TIM_HandleTypeDef* htim, double f, uint16_t tb)
{
  return get_TIM_PSC_ARR(tb, f, &htim->Instance->PSC, &htim->Instance->ARR, NULL) ? HAL_OK : HAL_ERROR;
}

#endif
