/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file   fatfs.c
  * @brief  Code for fatfs applications
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2022 STMicroelectronics.
  * All rights reserved.
  *
  * This software is licensed under terms that can be found in the LICENSE file
  * in the root directory of this software component.
  * If no LICENSE file comes with this software, it is provided AS-IS.
  *
  ******************************************************************************
  */

#include "c_datetime.h"
#include "c_debug.h"
#include "FS_C.h"
/* USER CODE END Header */
#include "fatfs.h"

uint8_t retUSBH;    /* Return value for USBH */
char USBHPath[4];   /* USBH logical drive path */
FATFS USBHFatFS;    /* File system object for USBH logical drive */
FIL USBHFile;       /* File object for USBH */

/* USER CODE BEGIN Variables */
uint8_t retMMC; /* Return value for eMMC */
TCHAR MMCPath[4]; /* eMMC logical drive path */
FATFS MMCFatFS; /* File system object for eMMC logical drive */
FIL MMCFile; /* File object for eMMC */

/* USER CODE END Variables */

void MX_FATFS_Init(void)
{
  /*## FatFS: Link the USBH driver ###########################*/
  retUSBH = FATFS_LinkDriver(&USBH_Driver, USBHPath);

  /* USER CODE BEGIN Init */
  /*## FatFS: Link the MMC driver ###########################*/
  retMMC = FATFS_LinkDriver(&MMC_Driver, MMCPath);
  /* additional user code for init */
  if (retUSBH) debug(0, "Linking the USBH driver FAILED!");
  if (retMMC) debug(0, "Linking the MMC driver FAILED!");
  /* USER CODE END Init */
}

/**
  * @brief  Gets Time from RTC
  * @param  None
  * @retval Time in DWORD
  */
DWORD get_fattime(void)
{
  /* USER CODE BEGIN get_fattime */
  return RTC_GetFATTime();
  /* USER CODE END get_fattime */
}

/* USER CODE BEGIN Application */

/* USER CODE END Application */
