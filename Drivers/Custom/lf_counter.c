/**
 * @file		  lf_counter.c
 * @author		CodeDog
 * @brief		  Low Frequency Counter with a digital filter for high frequency noise rejection.
 * @remarks
 * 				    Uses a TIM timer to sample the signal.
 *
 * @copyright	(c)2022 CodeDog, All rights reserved.
 */

#include "lf_counter.h"

/**
 * @brief Available handle pointers.
 */
static LFC_HandleTypeDef* handles[LFC_HCOUNT];

/**
 * @brief Initialized timers table.
 * @remarks A single timer can be reused for multiple counters sharing the same timing parameters.
 *          This table is used to find if a specified TIM handle was already initialized to use with the counter.
 */
static TIM_HandleTypeDef* timers[LFC_HCOUNT];

/**
 * @fn void return_result(LFC_HandleTypeDef*, double)
 * @brief Detects the result change, updates the handle properties on changes and calls T_Changed() event.
 * @param h LFC handle pointer.
 * @param t Time measured.
 */
static void return_result(LFC_HandleTypeDef* h, double t)
{
  if (t != h->T)
  {
    h->T = t;
    h->F = 1 / (double) t;
    if (h->T_changed) h->T_changed(h);
  }
}

/**
 * @fn void process(LFC_HandleTypeDef*, double)
 * @brief Applies optional postprocessing (average) to the results.
 * @param h LFC handle pointer.
 * @param t Time measured.
 */
static void process(LFC_HandleTypeDef* h, double t)
{
  if (!h->filter.AE) return return_result(h, t);
  h->filter.AVG[h->filter.AI] = t;
  if (h->filter.AI == LFC_AVG_COUNT - 1) // the last sample
  {
    double avg = 0;
    for (uint8_t i = 0; i < LFC_AVG_COUNT; i++)
      avg += h->filter.AVG[i];
    avg /= (double) LFC_AVG_COUNT;
    return_result(h, avg);
  }
  if (++h->filter.AI >= LFC_AVG_COUNT) h->filter.AI = 0;
}

/**
 * @fn void count(LFC_HandleTypeDef*)
 * @brief Counts one sample of the LFC.
 * @param h LFC handle pointer.
 */
static void count(LFC_HandleTypeDef* h)
{
  double t;
  uint8_t cs = (h->pin->Port->IDR & h->pin->Bit) != 0; // current state
  uint8_t cd = cs != h->LS; // change detected
  h->LS = cs;

  if (h->ST) // digital filter enabled
  {
    uint8_t f_offset = h->SC % h->ST; // filter offset
    if (cd)
    {
      if (!h->filter.CC) h->filter.T0 = h->SC;
      ++h->filter.CC;
    }
    if (!f_offset) // first filter sample
    {
      h->filter.S0 = cd ? !cs : cs; // edge case when beginning of the sample window is a new value
    }
    else if (f_offset == h->ST - 1) // last filer sample
    {
      h->filter.S1 = cs;
      if (h->filter.CC & 1) // odd number of changes in the filter sample window
      {
        if ((h->edge && !h->filter.S0 && h->filter.S1) || (!h->edge && h->filter.S0 && !h->filter.S1)) // valid edge detected
        {
          if (h->RTM) // ready to measue
          {
            uint16_t sc = h->filter.T0 - h->filter.T1;
            double t = sc / (double) h->SR;
            process(h, t);
            h->filter.T1 = h->filter.T0 - h->SC;
            h->SC = 0;
          }
          else
          {
            h->RTM = 1;
            h->filter.T1 = h->filter.T0;
          }
          if (h->edge_detected) h->edge_detected(h);
        }
        if ((h->edge && h->filter.S0 && !h->filter.S1) || (!h->edge && !h->filter.S0 && h->filter.S1)) // second edge detected
        {
          uint16_t sc = h->SC - h->filter.T1;
          h->t = sc / (double) h->SR;
          if (h->second_edge_detected) h->second_edge_detected(h);
        }
      }
      h->filter.CC = 0;
    }
  }

  else // no filtering
  {
    if (cd && cs == h->edge) // edge detected
    {
      if (h->RTM)
      {
        t = h->SC / (double) h->SR;
        process(h, t);
      }
      else h->RTM = 1;
      if (h->edge_detected) h->edge_detected(h);
      h->SC = 0;
    }
  }

  ++h->SC;
  if (h->SC > h->SR)
  {
    h->SC = 0;
    h->RTM = 0;
    h->filter.CC = 0;
    h->filter.T0 = 0;
    h->filter.T1 = 0;
    h->filter.AI = 0;
    if (h->T)
    {
      h->T = 0;
      h->F = 0;
      if (h->T_changed) h->T_changed(h);
    }
  }
}

/**
 * @fn void tim_callback(TIM_HandleTypeDef*)
 * @brief Calls counting for the defined LFC / TIM bindings.
 * @param htim TIM handle pointer.
 */
static void tim_callback(TIM_HandleTypeDef* htim)
{
  for (uint8_t i = 0; i < LFC_HCOUNT; i++)
    if (handles[i] && handles[i]->htim == htim) count(handles[i]);
}

/**
 * @fn LFC_HandleTypeDef get_handle*()
 * @brief Gets the next available handle for the LFC initialization.
 * @return LFC handle pointer or NULL if no handles available.
 */
static LFC_HandleTypeDef** get_handle_ptr()
{
  for (uint8_t i = 0; i < LFC_HCOUNT; i++)
    if (!handles[i]) return &handles[i];
  return NULL;
}

/**
 * @fn uint8_t is_tim_initialized(TIM_HandleTypeDef*)
 * @brief Tests if the specified timer has already been initialized with another counter instance.
 * @param htim TIM handle pointer.
 * @return 1: Already initialized. 0: Not initialized.
 */
static uint8_t is_tim_initialized(TIM_HandleTypeDef* htim)
{
  for (uint8_t i = 0; i < LFC_HCOUNT; i++)
    if (timers[i] == htim) return 1;
  return 0;
}

/**
 * @fn HAL_StatusTypeDef add_tim(TIM_HandleTypeDef*)
 * @brief Add the TIM handle pointer to the register timers table.
 * @param htim TIM handle pointer.
 * @return HAL_OK: Added successfully, HAL_ERROR: Out of handles.
 */
static HAL_StatusTypeDef add_tim(TIM_HandleTypeDef* htim)
{
  for (uint8_t i = 0; i < LFC_HCOUNT; i++)
    if (!timers[i])
    {
      timers[i] = htim;
      return HAL_OK;
    }
  return HAL_ERROR;
}

/**
 * @fn HAL_StatusTypeDef LFC_Init(LFC_HandleTypeDef*, TIM_HandleTypeDef*, uint16_t, uint16_t, uint16_t)
 * @brief Initializes the low frequency counter.
 * @remarks The timer is initialized only when used for the first time.
 *          When the timer is reused, the new counter should use the same timing.
 * @param h LFC handle pointer.
 * @param htim TIM handle pointer.
 * @param tb Time base in MHz.
 * @param sr Sample rate in samples per second.
 * @param st Stable time in samples.
 * @param pin GPIO pin.
 * @param edge Signal edge to detect.
 * @return HAL_OK: success, HAL_ERROR: invalid parameters, out of handles.
 */
HAL_StatusTypeDef LFC_Init(LFC_HandleTypeDef* h, TIM_HandleTypeDef* htim, uint16_t tb, uint16_t sr, uint16_t st,
                                  GPIO_PinTypeDef* pin, LFC_EdgeTypeDef edge)
{
  if (!h || !htim || !tb || sr < 1000) return HAL_ERROR;
  h->htim = htim;
  h->TB = tb;
  h->SR = sr;
  h->ST = st;
  h->pin = pin;
  h->edge = edge;
  double arr = h->TB * 1000000.0 / (h->htim->Instance->PSC + 1) / (double) h->SR - 1.0;
  if (arr < 0 || arr > 0xffff) return HAL_ERROR;
  LFC_HandleTypeDef** ptr = get_handle_ptr();
  if (!ptr) return HAL_ERROR;
  *ptr = h; // sets the handle pointer to this instance.
  h->is_initialized = is_tim_initialized(h->htim);
  HAL_StatusTypeDef status;
  if (!h->is_initialized)
  {
    h->htim->Instance->ARR = arr;
    status = HAL_TIM_RegisterCallback(h->htim, HAL_TIM_PERIOD_ELAPSED_CB_ID, tim_callback);
    if (status != HAL_OK) return status;
    add_tim(h->htim);
    h->is_initialized = 1;
    if (!h->is_enabled)
    {
      status = HAL_TIM_Base_Start_IT(h->htim);
      if (status != HAL_OK) return status;
      h->is_enabled = 1;
    }
  }
  return HAL_OK;
}
