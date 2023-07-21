/**
 * @file        c_bindings.h
 * @author      CodeDog
 * @brief       Defines EXTERN_C C/C++ interoperaion macros.
 *
 * @copyright   (c)2023 CodeDog, All rights reserved.
 */

#pragma once

#ifdef __cplusplus
#define EXTERN_C extern "C"
#define EXTERN_C_BEGIN extern "C" {
#define EXTERN_C_END }
#else
#define EXTERN_C
#define EXTERN_C_BEGIN
#define EXTERN_C_END
#endif
