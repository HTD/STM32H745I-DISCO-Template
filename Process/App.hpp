/**
 * @file        App.hpp
 * @author      CodeDog
 * @brief       Main application process code. Header file.
 *
 * @remarks     Contains initialization code for the main application process.
 *
 * @copyright   (c)2023 CodeDog, All rights reserved.
 */

#pragma once

#include "Thread.hpp"

class App
{
public:

    // This is a "static / namespace" class, no instance allowed:

    App() = delete;
    App(const App& other) = delete;
    App(App&& other) = delete;

    /// @brief Performs final hardware initialization and starts application code.
    static void init(void);

protected:

    /// @brief Called when USB disk is inserted or removed.
    static void fsAvailableChanged();

};
