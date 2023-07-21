/**
 * @file        FS_C.h
 * @author      CodeDog
 * @brief       C binding for FS class.
 * @remarks     Used to call mounting and unmounting file systems.
 *
 * @copyright   (c)2022 CodeDog, All rights reserved.
 */

#pragma once

#include <stdint.h>
#include "c_bindings.h"

typedef enum
{
    FS_TYPE_MMC,    ///< Internal eMMC memory.
    FS_TYPE_USB     ///< External USB disk.
} FS_TYPE;

EXTERN_C_BEGIN

/// @brief Mounts USB file system. ISR safe.
void FS_mount_USB();

/// @brief Unmounts USB file system. ISR safe.
void FS_umount_USB();

EXTERN_C_END