/**
 * @file        DateTimeEx.hpp
 * @author      CodeDog
 * @brief       DateTime class extended with RTC / FAT methods. Header file.
 *
 * @copyright   (c)2023 CodeDog, All rights reserved.
 */

#pragma once

#include "DateTime.hpp"
#include "c_bindings.h"
EXTERN_C_BEGIN
#include "c_datetime.h"
EXTERN_C_END

class __attribute__((__packed__)) DateTimeEx : public DateTime
{
public:
    DateTimeEx();
    DateTimeEx(bool initializeFromRTC);
    DateTimeEx(DateTimeTypeDef& dt);
    DateTimeTypeDef* c_ptr();
    bool getRTC();
    bool setRTC();
    void getFAT(DWORD ftd);
    void setFAT(DWORD& ftd);
};
