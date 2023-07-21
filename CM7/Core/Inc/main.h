/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.h
  * @brief          : Header for main.c file.
  *                   This file contains the common defines of the application.
  ******************************************************************************
  * @attention
  *
  * <h2><center>&copy; Copyright (c) 2021 STMicroelectronics.
  * All rights reserved.</center></h2>
  *
  * This software component is licensed by ST under Ultimate Liberty license
  * SLA0044, the "License"; You may not use this file except in compliance with
  * the License. You may obtain a copy of the License at:
  *                             www.st.com/SLA0044
  *
  ******************************************************************************
  */

/* USER CODE END Header */

/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __MAIN_H
#define __MAIN_H

#ifdef __cplusplus
extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include "stm32h7xx_hal.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */

/* USER CODE END Includes */

/* Exported types ------------------------------------------------------------*/
/* USER CODE BEGIN ET */

/* USER CODE END ET */

/* Exported constants --------------------------------------------------------*/
/* USER CODE BEGIN EC */
extern char* debugMessage;
extern uint8_t debugMessageChanged;

/* USER CODE END EC */

/* Exported macro ------------------------------------------------------------*/
/* USER CODE BEGIN EM */

/* USER CODE END EM */

void HAL_TIM_MspPostInit(TIM_HandleTypeDef *htim);

/* Exported functions prototypes ---------------------------------------------*/
void Error_Handler(void);

/* USER CODE BEGIN EFP */
/* USER CODE END EFP */

/* Private defines -----------------------------------------------------------*/
#define COUNTER_TIM htim5
#define COUNTER_1S 100000
#define DEBUG_PORT huart3
#define PRINTER_PORT huart1
#define EMMC hmmc1
#define SLAVE_LINK hspi2
#define MICROTIME_TIM htim7
#define DELAY_TIM htim17
#define APB_CLOCK 200
#define DELAY_RESOLUTION 6
#define VBUS_OTG_FS_Pin GPIO_PIN_9
#define VBUS_OTG_FS_GPIO_Port GPIOA
#define D1_Pin GPIO_PIN_6
#define D1_GPIO_Port GPIOE
#define D2_Pin GPIO_PIN_8
#define D2_GPIO_Port GPIOA
#define SPI_CS_Pin GPIO_PIN_11
#define SPI_CS_GPIO_Port GPIOI
#define LD6_Pin GPIO_PIN_13
#define LD6_GPIO_Port GPIOI
#define LCD_BL_Pin GPIO_PIN_0
#define LCD_BL_GPIO_Port GPIOK
#define SPI_HS_Pin GPIO_PIN_4
#define SPI_HS_GPIO_Port GPIOH
#define SPI_HS_EXTI_IRQn EXTI4_IRQn
#define A2_Pin GPIO_PIN_0
#define A2_GPIO_Port GPIOA
#define A3_Pin GPIO_PIN_1
#define A3_GPIO_Port GPIOA
#define OTG_FS2_PSO_Pin GPIO_PIN_5
#define OTG_FS2_PSO_GPIO_Port GPIOA
#define LD7_Pin GPIO_PIN_2
#define LD7_GPIO_Port GPIOJ

/* USER CODE BEGIN Private defines */

/* USER CODE END Private defines */

#ifdef __cplusplus
}
#endif

#endif /* __MAIN_H */
