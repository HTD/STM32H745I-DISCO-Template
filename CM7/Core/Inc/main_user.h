#pragma once

#include "c_bindings.h"

EXTERN_C_BEGIN

#include "c_debug.h"
#include "usb_host.h"

//! initialize LTDC clocks
void initLtdcClocks(void);

//! initialize QSPI via BSP
void initBspQuadSpi(void (*error_handler)());

//! initialize SDRAM/FMC via BSP
void initBspSdRam(void (*error_handler)());

//! clear RAM for LTDC frame buffer
void clearLTDCRam();

EXTERN_C_END
