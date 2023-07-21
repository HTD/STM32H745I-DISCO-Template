/**
 * @file      mmc_diskio.c
 * @author    CodeDog
 * @brief     MMC Disk I/O DMA with RTOS driver template using HAL and BSPv2 API.
 *
 * @remarks   Based on MCD Application Team template:
 * @see       https://github.com/STMicroelectronics/STM32CubeH7/blob/master/Middlewares/Third_Party/FatFs/src/drivers/sd_diskio_dma_rtos_template_bspv2.c
 *
 * @copyright (c)2022 CodeDog, All rights reserved.
 */

/* Includes ------------------------------------------------------------------*/
#include "ff_gen_drv.h"
#include "mmc_diskio.h"

#include <string.h>

/* Private typedef -----------------------------------------------------------*/
/* Private define ------------------------------------------------------------*/
#define QUEUE_SIZE         (uint32_t) 10
#define READ_CPLT_MSG      (uint32_t) 1
#define WRITE_CPLT_MSG     (uint32_t) 2
#define RW_ABORT_MSG       (uint32_t) 3

#define MMC_TIMEOUT 2000  // If I/O operation don't complete in 2 seconds, something's broken.

#define MMC_DEFAULT_BLOCK_SIZE 512

#define DISABLE_MMC_INIT  // The device should be already initialized in core application (main.c).

/*
 * when using cachable memory region, it may be needed to maintain the cache
 * validity. Enable the define below to activate a cache maintenance at each
 * read and write operation.
 * Notice: This is applicable only for cortex M7 based platform.
 */

//#define ENABLE_MMC_DMA_CACHE_MAINTENANCE  1

/*
 * Some DMA requires 4-Byte aligned address buffer to correctly read/wite data,
 * in FatFs some accesses aren't thus we need a 4-byte aligned scratch buffer to correctly
 * transfer data
 */
//#define ENABLE_SCRATCH_BUFFER

/* Private variables ---------------------------------------------------------*/

#if defined(ENABLE_SCRATCH_BUFFER)
#if defined (ENABLE_MMC_DMA_CACHE_MAINTENANCE)
ALIGN_32BYTES(static uint8_t scratch[MMC_BLOCKSIZE]); // 32-Byte aligned for cache maintenance
#else
__ALIGN_BEGIN static uint8_t scratch[MMC_BLOCKSIZE] __ALIGN_END;
#endif
#endif

/* Disk status */
static volatile DSTATUS Stat = STA_NOINIT;
#if (osCMSIS <= 0x20000U)
static osMessageQId MMCQueueID = NULL;
#else
static osMessageQueueId_t MMCQueueID = NULL;
#endif
/* Private function prototypes -----------------------------------------------*/
static DSTATUS MMC_CheckStatus(BYTE lun);
DSTATUS MMC_initialize(BYTE);
DSTATUS MMC_status(BYTE);
DRESULT MMC_read(BYTE, BYTE*, DWORD, UINT);
#if _USE_WRITE == 1
DRESULT MMC_write(BYTE, const BYTE*, DWORD, UINT);
#endif /* _USE_WRITE == 1 */
#if _USE_IOCTL == 1
DRESULT MMC_ioctl(BYTE, BYTE, void*);
#endif  /* _USE_IOCTL == 1 */

const Diskio_drvTypeDef MMC_Driver =
    {
        MMC_initialize,
        MMC_status,
        MMC_read,
        #if  _USE_WRITE == 1
        MMC_write,
        #endif /* _USE_WRITE == 1 */

#if  _USE_IOCTL == 1
        MMC_ioctl,
    #endif /* _USE_IOCTL == 1 */
    };

/* Private functions ---------------------------------------------------------*/

/* Private functions ---------------------------------------------------------*/
static int MMC_CheckStatusWithTimeout(uint32_t timeout)
{
  uint32_t timer;
#if (osCMSIS < 0x20000U)
  timer = osKernelSysTick();
  /* block until SDIO IP is ready or a timeout occur */
  while(osKernelSysTick() - timer <timeout)
#else
  timer = osKernelGetTickCount();
  /* block until SDIO IP is ready or a timeout occur */
  while (osKernelGetTickCount() - timer < timeout)
  #endif
  {
    if (HAL_MMC_GetCardState(&EMMC) == HAL_MMC_CARD_TRANSFER)
    {
      return 0;
    }
  }
  return -1;
}

static DSTATUS MMC_CheckStatus(BYTE lun)
{
  Stat = STA_NOINIT;
  if (HAL_MMC_GetCardState(&EMMC) == HAL_MMC_CARD_TRANSFER)
  {
    Stat &= ~STA_NOINIT;
  }
  return Stat;
}

/**
 * @brief  Initializes a Drive
 * @param  lun : not used
 * @retval DSTATUS: Operation status
 */
DSTATUS MMC_initialize(BYTE lun)
{
  Stat = STA_NOINIT;
  /*
   * check that the kernel has been started before continuing
   * as the osMessage API will fail otherwise
   */
#if (osCMSIS <= 0x20000U)
  if(osKernelRunning())
#else
  if (osKernelGetState() == osKernelRunning)
  #endif
  {
#if !defined(DISABLE_MMC_INIT)

    if(HAL_MMC_Init(&EMMC) == HAL_OK)
    {
      Stat = MMC_CheckStatus(lun);
    }

#else
    Stat = MMC_CheckStatus(lun);
#endif

    /*
     * if the MMC is correctly initialized, create the operation queue
     */

    if (Stat != STA_NOINIT)
    {
      if (MMCQueueID == NULL)
      {
#if (osCMSIS <= 0x20000U)
      osMessageQDef(MMC_Queue, QUEUE_SIZE, uint16_t);
      MMCQueueID = osMessageCreate (osMessageQ(MMC_Queue), NULL);
#else
        MMCQueueID = osMessageQueueNew(QUEUE_SIZE, 2, NULL);
#endif
      }

      if (MMCQueueID == NULL)
      {
        Stat |= STA_NOINIT;
      }
    }
  }

  return Stat;
}

/**
 * @brief  Gets Disk Status
 * @param  lun : not used
 * @retval DSTATUS: Operation status
 */
DSTATUS MMC_status(BYTE lun)
{
  return MMC_CheckStatus(lun);
}

/**
 * @brief  Reads Sector(s)
 * @param  lun : not used
 * @param  *buff: Data buffer to store read data
 * @param  sector: Sector address (LBA)
 * @param  count: Number of sectors to read (1..128)
 * @retval DRESULT: Operation result
 */
DRESULT MMC_read(BYTE lun, BYTE* buff, DWORD sector, UINT count)
{
  DRESULT res = RES_ERROR;
  uint32_t timer;

#if (osCMSIS < 0x20000U)
  osEvent event;
#else
  uint16_t event;
  osStatus_t status;
#endif

#if defined(ENABLE_SCRATCH_BUFFER)
  int32_t ret;
#endif

#if (ENABLE_MMC_DMA_CACHE_MAINTENANCE == 1)
  uint32_t alignedAddr;
#endif

  /*
   * ensure the MMCCard is ready for a new operation
   */

  if (MMC_CheckStatusWithTimeout(MMC_TIMEOUT) < 0)
  {
    return res;
  }

#if defined(ENABLE_SCRATCH_BUFFER)
  if (!((uint32_t)buff & 0x3))
  {
#endif
  if (HAL_MMC_ReadBlocks_DMA(&EMMC, buff,
      (uint32_t) (sector),
      count) == HAL_OK)
  {
#if (osCMSIS < 0x20000U)
    /* wait for a message from the queue or a timeout */
    event = osMessageGet(MMCQueueID, MMC_TIMEOUT);

    if (event.status == osEventMessage)
    {
      if (event.value.v == READ_CPLT_MSG)
      {
        timer = osKernelSysTick();
        /* block until SDIO IP is ready or a timeout occur */
        while(osKernelSysTick() - timer <MMC_TIMEOUT)
#else
    status = osMessageQueueGet(MMCQueueID, (void*) &event, NULL, MMC_TIMEOUT);
    if ((status == osOK) && (event == READ_CPLT_MSG ))
    {
      timer = osKernelGetTickCount();
      /* block until SDIO IP is ready or a timeout occur */
      while (osKernelGetTickCount() - timer < MMC_TIMEOUT)
      #endif
      {
        if (HAL_MMC_GetCardState(&EMMC) == HAL_MMC_CARD_TRANSFER)
        {
          res = RES_OK;
#if (ENABLE_MMC_DMA_CACHE_MAINTENANCE == 1)
            /*
               the SCB_InvalidateDCache_by_Addr() requires a 32-Byte aligned address,
               adjust the address and the D-Cache size to invalidate accordingly.
             */
            alignedAddr = (uint32_t)buff & ~0x1F;
            SCB_InvalidateDCache_by_Addr((uint32_t*)alignedAddr, count*MMC_BLOCKSIZE + ((uint32_t)buff - alignedAddr));
#endif
          break;
        }
      }
#if (osCMSIS < 0x20000U)
      }
    }
#else
    }
#endif
  }
#if defined(ENABLE_SCRATCH_BUFFER)
  else {
    /* Slow path, fetch each sector a part and memcpy to destination buffer */
    int i;

    for (i = 0; i < count; i++)
    {
      ret = HAL_MMC_ReadBlocks_DMA(&EMMC, (uint32_t*)scratch, (uint32_t)sector++, 1);
      if (ret == HAL_OK)
      {
        /* wait until the read is successful or a timeout occurs */
#if (osCMSIS < 0x20000U)
        /* wait for a message from the queue or a timeout */
        event = osMessageGet(MMCQueueID, MMC_TIMEOUT);

        if (event.status == osEventMessage)
        {
          if (event.value.v == READ_CPLT_MSG)
          {
            timer = osKernelSysTick();
            /* block until SDIO IP is ready or a timeout occur */
            while(osKernelSysTick() - timer <MMC_TIMEOUT)
#else
          status = osMessageQueueGet(MMCQueueID, (void *)&event, NULL, MMC_TIMEOUT);
          if ((status == osOK) && (event == READ_CPLT_MSG))
          {
            timer = osKernelGetTickCount();
            /* block until SDIO IP is ready or a timeout occur */
            ret = HAL_BUSY;
            while(osKernelGetTickCount() - timer < MMC_TIMEOUT)
#endif
            {
              ret = HAL_MMC_GetCardState(&EMMC);

              if (ret == HAL_MMC_CARD_TRANSFER)
              {
                break;
              }
            }

            if (ret != HAL_MMC_CARD_TRANSFER)
            {
              break;
            }
#if (osCMSIS < 0x20000U)
      }
    }
#else
    }
#endif
#if (ENABLE_MMC_DMA_CACHE_MAINTENANCE == 1)
          /*
          *
          * invalidate the scratch buffer before the next read to get the actual data instead of the cached one
          */
          SCB_InvalidateDCache_by_Addr((uint32_t*)scratch, MMC_BLOCKSIZE);
#endif
          memcpy(buff, scratch, MMC_BLOCKSIZE);
          buff += MMC_BLOCKSIZE;
        }
        else
        {
          break;
        }
      }

      if ((i == count) && (ret == HAL_MMC_CARD_TRANSFER))
        res = RES_OK;
    }

  }
#endif
  return res;
}

/**
 * @brief  Writes Sector(s)
 * @param  lun : not used
 * @param  *buff: Data to be written
 * @param  sector: Sector address (LBA)
 * @param  count: Number of sectors to write (1..128)
 * @retval DRESULT: Operation result
 */
#if _USE_WRITE == 1
DRESULT MMC_write(BYTE lun, const BYTE* buff, DWORD sector, UINT count)
{
  DRESULT res = RES_ERROR;
  uint32_t timer;

#if (osCMSIS < 0x20000U)
  osEvent event;
#else
  uint16_t event;
  osStatus_t status;
#endif

#if defined(ENABLE_SCRATCH_BUFFER)
  int32_t ret;
#endif

  /*
   * ensure the MMCCard is ready for a new operation
   */

  if (MMC_CheckStatusWithTimeout(MMC_TIMEOUT) < 0)
  {
    return res;
  }

#if defined(ENABLE_SCRATCH_BUFFER)
  if (!((uint32_t)buff & 0x3))
  {
#endif
#if (ENABLE_MMC_DMA_CACHE_MAINTENANCE == 1)
  uint32_t alignedAddr;
  /*
    the SCB_CleanDCache_by_Addr() requires a 32-Byte aligned address
    adjust the address and the D-Cache size to clean accordingly.
  */
  alignedAddr = (uint32_t)buff & ~0x1F;
  SCB_CleanDCache_by_Addr((uint32_t*)alignedAddr, count*MMC_BLOCKSIZE + ((uint32_t)buff - alignedAddr));
#endif

  if (HAL_MMC_WriteBlocks_DMA(&EMMC, (uint8_t*)buff, sector, count) == HAL_OK)
  {
#if (osCMSIS < 0x20000U)
    /* Get the message from the queue */
    event = osMessageGet(MMCQueueID, MMC_TIMEOUT);

    if (event.status == osEventMessage)
    {
      if (event.value.v == WRITE_CPLT_MSG)
      {
#else
    status = osMessageQueueGet(MMCQueueID, (void*) &event, NULL, MMC_TIMEOUT);
    if ((status == osOK) && (event == WRITE_CPLT_MSG ))
    {
#endif
#if (osCMSIS < 0x20000U)
        timer = osKernelSysTick();
        /* block until SDIO IP is ready or a timeout occur */
        while(osKernelSysTick() - timer  < MMC_TIMEOUT)
#else
      timer = osKernelGetTickCount();
      /* block until SDIO IP is ready or a timeout occur */
      while (osKernelGetTickCount() - timer < MMC_TIMEOUT)
      #endif
      {
        if (HAL_MMC_GetCardState(&EMMC) == HAL_MMC_CARD_TRANSFER)
        {
          res = RES_OK;
          break;
        }
      }
#if (osCMSIS < 0x20000U)
      }
    }
#else
    }
#endif
  }
#if defined(ENABLE_SCRATCH_BUFFER)
  }
  else
  {
    /* Slow path, fetch each sector a part and memcpy to destination buffer */
    int i;

#if (ENABLE_MMC_DMA_CACHE_MAINTENANCE == 1)
    /*
     * invalidate the scratch buffer before the next write to get the actual data instead of the cached one
     */
     SCB_InvalidateDCache_by_Addr((uint32_t*)scratch, MMC_BLOCKSIZE);
#endif
    for (i = 0; i < count; i++)
    {
      memcpy((void *)scratch, buff, MMC_BLOCKSIZE);
      buff += MMC_BLOCKSIZE;

      ret = HAL_MMC_WriteBlocks_DMA(&EMMC, scratch, (uint32_t)sector++, 1);
      if (ret == HAL_MMC_CARD_TRANSFER)
      {
        /* wait until the read is successful or a timeout occurs */
#if (osCMSIS < 0x20000U)
        /* wait for a message from the queue or a timeout */
        event = osMessageGet(MMCQueueID, MMC_TIMEOUT);

        if (event.status == osEventMessage)
        {
          if (event.value.v == READ_CPLT_MSG)
          {
            timer = osKernelSysTick();
            /* block until SDIO IP is ready or a timeout occur */
            while(osKernelSysTick() - timer <MMC_TIMEOUT)
#else
          status = osMessageQueueGet(MMCQueueID, (void *)&event, NULL, MMC_TIMEOUT);
          if ((status == osOK) && (event == READ_CPLT_MSG))
          {
            timer = osKernelGetTickCount();
            /* block until SDIO IP is ready or a timeout occur */
            ret = HAL_MMC_CARD_STANDBY;
            while(osKernelGetTickCount() - timer < MMC_TIMEOUT)
#endif
            {
              ret = HAL_MMC_GetCardState(&EMMC);

              if (ret == HAL_MMC_CARD_TRANSFER)
              {
                break;
              }
            }

            if (ret != HAL_MMC_CARD_TRANSFER)
            {
              break;
            }
#if (osCMSIS < 0x20000U)
      }
    }
#else
    }
#endif
        }
        else
        {
          break;
        }
      }

      if ((i == count) && (ret == HAL_MMC_CARD_TRANSFER))
        res = RES_OK;
    }
#endif

  return res;
}
#endif /* _USE_WRITE == 1 */

/**
 * @brief  I/O control operation
 * @param  lun : not used
 * @param  cmd: Control code
 * @param  *buff: Buffer to send/receive control data
 * @retval DRESULT: Operation result
 */
#if _USE_IOCTL == 1
DRESULT MMC_ioctl(BYTE lun, BYTE cmd, void* buff)
{
  DRESULT res = RES_ERROR;
  HAL_MMC_CardInfoTypeDef CardInfo;

  if (Stat & STA_NOINIT) return RES_NOTRDY;

  switch (cmd)
  {
  /* Make sure that no pending write process */
  case CTRL_SYNC:
    res = RES_OK;
    break;

    /* Get number of sectors on the disk (DWORD) */
  case GET_SECTOR_COUNT:
    HAL_MMC_GetCardInfo(&EMMC, &CardInfo);
    *(DWORD*) buff = CardInfo.LogBlockNbr;
    res = RES_OK;
    break;

    /* Get R/W sector size (WORD) */
  case GET_SECTOR_SIZE:
    HAL_MMC_GetCardInfo(&EMMC, &CardInfo);
    *(WORD*) buff = CardInfo.LogBlockSize;
    res = RES_OK;
    break;

    /* Get erase block size in unit of sector (DWORD) */
  case GET_BLOCK_SIZE:
    HAL_MMC_GetCardInfo(&EMMC, &CardInfo);
    *(DWORD*) buff = CardInfo.LogBlockSize / MMC_DEFAULT_BLOCK_SIZE;
    res = RES_OK;
    break;

  default:
    res = RES_PARERR;
  }

  return res;
}
#endif /* _USE_IOCTL == 1 */

/**
 * @brief Tx Transfer completed callbacks
 * @param hsd: MMC handle
 * @retval None
 */
void BSP_MMC_WriteCpltCallback(uint32_t Instance)
{
  /*
   * No need to add an "osKernelRunning()" check here, as the MMC_initialize()
   * is always called before any MMC_Read()/MMC_Write() call
   */
  UNUSED(Instance);
#if (osCMSIS < 0x20000U)
   osMessagePut(MMCQueueID, WRITE_CPLT_MSG, 0);
#else
    const uint16_t msg = WRITE_CPLT_MSG;
    osMessageQueuePut(MMCQueueID, (const void*)&msg, 0, 0);
#endif
}

/**
 * @brief Rx Transfer completed callbacks
 * @param hsd: MMC handle
 * @retval None
 */
void BSP_MMC_ReadCpltCallback(uint32_t Instance)
{
  /*
   * No need to add an "osKernelRunning()" check here, as the MMC_initialize()
   * is always called before any MMC_Read()/MMC_Write() call
   */
  UNUSED(Instance);
#if (osCMSIS < 0x20000U)
    osMessagePut(MMCQueueID, READ_CPLT_MSG, 0);
#else
    const uint16_t msg = READ_CPLT_MSG;
    osMessageQueuePut(MMCQueueID, (const void*)&msg, 0, 0);
#endif
}

void BSP_MMC_AbortCallback(uint32_t Instance)
{
  UNUSED(Instance);
#if (osCMSIS < 0x20000U)
    osMessagePut(MMCQueueID, RW_ABORT_MSG, 0);
#else
    const uint16_t msg = RW_ABORT_MSG;
    osMessageQueuePut(MMCQueueID, (const void*)&msg, 0, 0);
#endif
}
