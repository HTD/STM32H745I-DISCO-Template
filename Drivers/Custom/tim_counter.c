/**
 * @file      tim_counter.c
 * @author    CodeDog
 * @brief     TIM based frequency counter.
 *
 * @copyright (c)2022 CodeDog, All rights reserved.
 */

#include "tim_counter.h"

static TC_HandleTypeDef* handles[TC_HCOUNT]; ///< TIM counter handles.

/**
 * @fn TC_HandleTypeDef get_handle*(TIM_HandleTypeDef*)
 * @brief Gets the TIM counter handle for the specified TIM handle.
 * @param htim TIM handle pointer.
 * @return TIM counter handle pointer or NULL if not found.
 */
static TC_HandleTypeDef* get_handle(TIM_HandleTypeDef* htim)
{
    for (uint8_t i = 0; i < TC_HCOUNT; i++) if (handles[i] && handles[i]->htim == htim) return handles[i];
    return NULL;
}

/**
 * @fn volatile uint32_t get_CCR*(TC_HandleTypeDef*)
 * @brief Gets the CCR register for the TIM counter.
 * @param htc TIM counter handle pointer.
 * @return CCR register pointer.
 */
static volatile uint32_t* get_CCR(TC_HandleTypeDef* htc)
{
    if (!htc) return NULL;
    switch (htc->channel)
    {
    case TIM_CHANNEL_1: return &htc->htim->Instance->CCR1;
    case TIM_CHANNEL_2: return &htc->htim->Instance->CCR2;
    case TIM_CHANNEL_3: return &htc->htim->Instance->CCR3;
    case TIM_CHANNEL_4: return &htc->htim->Instance->CCR4;
    case TIM_CHANNEL_5: return &htc->htim->Instance->CCR5;
    case TIM_CHANNEL_6: return &htc->htim->Instance->CCR6;
    default: return NULL;
    }
}

/**
 * @fn void ic_capture_callback(TIM_HandleTypeDef*)
 * @brief Called when TIM counter captures input signal.
 * @param htim TIM handle pointer.
 */
static void ic_capture_callback(TIM_HandleTypeDef* htim)
{
    TC_HandleTypeDef* htc = get_handle(htim);
    if (!htc) return;
    if (!htc->state)
    {
        htc->T1 = *get_CCR(htc);
        htc->PC = 0;
        htc->state = 1;
    }
    else
    {
        htc->T2 = *get_CCR(htc);
        htc->TC = htc->PC * (htim->Instance->ARR + 1) + htc->T2 - htc->T1;
        htc->F =  htc->TC ? (htc->timeBase * 1000000.0 / (double)(htim->Instance->PSC + 1) / (double)htc->TC) : 0;
        uint32_t tick = HAL_GetTick();
        if (htc->F != htc->LF && tick != htc->LT)
        {
            htc->LT = tick;
            htc->LF = htc->F;
            if (htc->frequency_changed) htc->frequency_changed(htc);
        }
        htc->state = 0;
    }
}

/**
 * @fn void period_elapsed_callback(TIM_HandleTypeDef*)
 * @brief Called when TIM counter period elapses.
 * @param htim TIM handle pointer.
 */
static void period_elapsed_callback(TIM_HandleTypeDef* htim)
{
    TC_HandleTypeDef* htc = get_handle(htim);
    if (!htc) return;
    htc->PC++;
    if (htc->F && htc->PC > htc->PTR) // zero if no input during PTR number of periods
    {
        htc->LT = HAL_GetTick();
        htc->T1 = htc->T2 = 0;
        htc->F = htc->LF = 0;
        if (htc->frequency_changed) htc->frequency_changed(htc);
    }
}

/**
 * @fn HAL_StatusTypeDef TC_Init_IT(TC_HandleTypeDef*, TC_CallbackTypeDef)
 * @brief Initializes and starts the TIM counter in interrupt mode.
 * @param htc TIM counter handle pointer.
 * @param frequency_changed Frequency changed callback.
 * @return HAL status.
 */
HAL_StatusTypeDef TC_Init_IT(TC_HandleTypeDef* htc, TC_CallbackTypeDef frequency_changed)
{
    HAL_StatusTypeDef status;
    for (uint8_t i = 0; i < TC_HCOUNT; i++)
    {
        if (handles[i] == htc) return HAL_ERROR; // already initialized.
        if (!handles[i])
        {
            htc->htim->Instance->ARR = 0xffff; // the only ARR value that makes 16-bit T2-T1 work correctly when they overflow.
            double pms = htc->timeBase * 1000u / (double)(htc->htim->Instance->PSC + 1) / (double)(htc->htim->Instance->ARR + 1);
            htc->PTR = (uint16_t)(TC_T_MAX * pms);
            htc->frequency_changed = frequency_changed;
            handles[i] = htc;
            status = HAL_TIM_RegisterCallback(htc->htim, HAL_TIM_IC_CAPTURE_CB_ID, ic_capture_callback);
            if (status != HAL_OK) return status;
            status = HAL_TIM_RegisterCallback(htc->htim, HAL_TIM_PERIOD_ELAPSED_CB_ID, period_elapsed_callback);
            if (status != HAL_OK) return status;
            status = HAL_TIM_Base_Start_IT(htc->htim);
            if (status != HAL_OK) return status;
            return HAL_TIM_IC_Start_IT(htc->htim, htc->channel);
        }
    }
    return HAL_ERROR; // out of handles.
}
