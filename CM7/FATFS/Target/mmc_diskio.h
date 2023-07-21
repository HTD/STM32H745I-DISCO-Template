/**
 * @file      mmc_diskio.h
 * @author    CodeDog
 * @brief     MMC Disk I/O DMA with RTOS driver template using HAL and BSPv2 API, header file.
 *
 * @remarks   Based on MCD Application Team template:
 * @see       https://github.com/STMicroelectronics/STM32CubeH7/blob/master/Middlewares/Third_Party/FatFs/src/drivers/sd_diskio_dma_rtos_template_bspv2.c
 *
 * @copyright (c)2022 CodeDog, All rights reserved.
 */

#pragma once

//#include "stm32h745i_discovery_mmc.h"
//#include "stm32h7xx_hal.h"

#include "main.h"

extern const Diskio_drvTypeDef MMC_Driver;
extern MMC_HandleTypeDef EMMC; // initialized in main.c

