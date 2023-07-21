/**
 * @file        BitFlags.hpp
 * @author      CodeDog
 * @brief       Bit flag macros and templates.
 *
 * @copyright	(c)2023 CodeDog, All rights reserved.
 */

#pragma once

#include <type_traits>

/**
 * @brief Counter base for generating subsequent bit flags.
 *
 * @param base A unique indentifier related to the enumeration.
 */
#define BIT_FLAG_BASE(base) enum { base = __COUNTER__ + 1U }

/**
 * @brief Generates subsequent bit flag for the specified base.
 *
 * @param base A unique indentifier related to the enumeration.
 */
#define BIT_FLAG(base) 1U << (__COUNTER__ - base)

#if !BIT_FLAG_OPERATORS_DISABLE // In case of conflicting with some pieces of code...

template<typename TEnum>
TEnum operator~(const TEnum a)
{
    using TValue = typename std::underlying_type<TEnum>::type;
    return static_cast<TEnum>(~static_cast<TValue>(a));
}

template<typename TEnum>
TEnum operator&(const TEnum a, const TEnum b)
{
    using TValue = typename std::underlying_type<TEnum>::type;
    return static_cast<TEnum>(static_cast<TValue>(a) & static_cast<TValue>(b));
}

template<typename TEnum>
TEnum operator|(const TEnum a, const TEnum b)
{
    using TValue = typename std::underlying_type<TEnum>::type;
    return static_cast<TEnum>(static_cast<TValue>(a) | static_cast<TValue>(b));
}

template<typename TEnum>
TEnum operator^(const TEnum a, const TEnum b)
{
    using TValue = typename std::underlying_type<TEnum>::type;
    return static_cast<TEnum>(static_cast<TValue>(a) ^ static_cast<TValue>(b));
}

template<typename TEnum>
TEnum operator&=(TEnum& a, const TEnum b)
{
    using TValue = typename std::underlying_type<TEnum>::type;
    return a = static_cast<TEnum>(static_cast<TValue>(a) & static_cast<TValue>(b));
}

template<typename TEnum>
TEnum operator|=(TEnum& a, const TEnum b)
{
    using TValue = typename std::underlying_type<TEnum>::type;
    return a = static_cast<TEnum>(static_cast<TValue>(a) | static_cast<TValue>(b));
}

template<typename TEnum>
TEnum operator^=(TEnum& a, const TEnum b)
{
    using TValue = typename std::underlying_type<TEnum>::type;
    return a = static_cast<TEnum>(static_cast<TValue>(a) ^ static_cast<TValue>(b));
}

/**
 * @brief Bit flags name space.
 */
namespace BF
{

/**
 * @brief Sets bit flags.
 *
 * @tparam TEnum Bit flags enumeration type.
 * @param what Bits.
 * @param where Target.
 */
template<typename TEnum>
void set(TEnum what, TEnum& where)
{
    where |= what;
}

/**
 * @brief Clears bit flags.
 *
 * @tparam TEnum Bit flags enumeration type.
 * @param what Bits.
 * @param where Target.
 */
template<typename TEnum>
void clear(TEnum what, TEnum& where)
{
    where &= ~what;
}

/**
 * @brief Tests if at least one of the bits is set in the target.
 *
 * @tparam TEnum Bit flags enumeration type.
 * @param what Bits.
 * @param where Target.
 * @param clear 0: Leave target alone. 1: Clear flags.
 * @return 1: At least flag exists in the target.
 * @return 0: Target does not contain any specified flags.
 */
template<typename TEnum>
bool isSet(TEnum what, TEnum& where, bool clear = false)
{
    if (!clear) return (where & what) != 0;
    bool result = (where & what) != 0;
    where &= ~what;
    return result;
}

}
#endif
