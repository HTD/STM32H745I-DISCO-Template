/**
 * @file        Action.hpp
 * @author      CodeDog
 * @brief       Basic callback actions and functions signatures.
 * @remarks     Just for the sake of code readability.
 *
 * @copyright   (c)2023 CodeDog, All rights reserved.
 */

#pragma once

using Action = void(*)(void); // Basic action pointer.
template<typename TArg1> using Ac1 = void(*)(TArg1); // An action taking one argument.
template<typename TArg1, typename TArg2> using Ac2 = void(*)(TArg1, TArg2); // An action taking 2 arguments. 
template<typename TArg1, typename TArg2, typename TArg3> using Ac3 = void(*)(TArg1, TArg2, TArg3); // An action taking 3 arguments.
template<typename TArg1, typename TArg2, typename TArg3, typename TArg4> using Ac4 = void(*)(TArg1, TArg2, TArg3, TArg4); // An action taking 4 arguments.
template<typename TResult> using Func = TResult(*)(void); // Basic function pointer.
template<typename TArg1, typename TResult> using Fn1 = TResult(*)(TArg1); // A function taking one argument.
template<typename TArg1, typename TArg2, typename TResult> using Fn2 = TResult(*)(TArg1, TArg2); // A function taking 2 arguments.
template<typename TArg1, typename TArg2, typename TArg3, typename TResult> using Fn3 = TResult(*)(TArg1, TArg2, TArg3); // A function taking 3 arguments.
template<typename TArg1, typename TArg2, typename TArg3, typename TArg4, typename TResult> using Fn4 = TResult(*)(TArg1, TArg2, TArg3, TArg4); // A function taking 4 arguments.
