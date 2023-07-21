/**
 * @file        FP.hpp
 * @author      CodeDog
 * @brief       Defines a single `eq` function that tests if 2 double arguments are approximately equal.
 *
 * @copyright	(c)2023 CodeDog, All rights reserved.
 */

#pragma once

#include <cmath>
#include <limits>
#include <type_traits>

/**
 * @brief Tests if the 2 numbers are approximately equal, with the floating point rounding error taken into account.
 *
 * @tparam T double or float.
 * @param a Compared value.
 * @param b Reference value.
 * @returns 1: Approximately equal. 0: Not approximately equal.
 */
template<typename T = double>
inline bool eq(T a, T b)
{
    static_assert(
        std::is_same<T, float>::value || std::is_same<T, double>::value,
        "Approximate equality works only on floating point numbers."
    );
    T _a = fabs(a), _b = fabs(b);
    return fabs(a - b) <= ((_a < _b ? _b : _a) * std::numeric_limits<T>::epsilon());
}

/**
 * @brief Tests if the 2 numbers are approximately equal, up to the constant deviation.
 * @remarks Rounding errors are still taken into account.
 *
 * @tparam T double or float.
 * @param a Compared value.
 * @param b Reference value.
 * @param d Maximal absolute difference between the numbers.
 * @returns 1: Approximately equal. 0: Not approximately equal.
 */
template<typename T = double>
inline bool eq(T a, T b, T d)
{
    static_assert(
        std::is_same<T, float>::value || std::is_same<T, double>::value,
        "Approximate equality works only on floating point numbers."
    );
    T _a = fabs(a), _b = fabs(b);
    return fabs(a - b) <= d + ((_a < _b ? _b : _a) * std::numeric_limits<T>::epsilon());
}
