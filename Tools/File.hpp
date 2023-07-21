/**
 * @file        File.hpp
 * @author      CodeDog
 * @brief       Provides access to a FATFS file.
 * @remarks     Implements Resource Acquisition Is Initialization principle with the FATFS.
 *
 * @copyright	(c)2022 CodeDog, All rights reserved.
 */

#pragma once

#include "BitFlags.hpp"
#include "Debug.hpp"
#include "ThreadSafeAllocator.hpp"

class File final : public ThreadSafeAllocator
{
public:

    /**
     * @brief File access mode.
     */
    enum Mode : BYTE
    {
        openExisting    = 0x00, // Opens a file. The function fails if the file is not existing. (Default)
        read            = 0x01, // Specifies read access to the file. Data can be read from the file.
        write           = 0x02, // Specifies write access to the file. Data can be written to the file. Combine with `read` for read-write access.
        createNew       = 0x04, // Creates a new file. The function fails with `FR_EXIST` if the file is existing.
        createAlways    = 0x08, // Creates a new file. If the file is existing, it will be truncated and overwritten.
        openAlways      = 0x10, // Opens the file if it is existing. If not, a new file will be created.
        openAppend      = 0x30  // Same as `openAlways` except the read/write pointer is set end of the file.
    };

    /**
     * @brief Creates invalid / empty file object.
     */
    File() : m_handle(), m_mode(), m_isOpen() { }

    /**
     * @brief Opens a file.
     *
     * @remarks The file (if opened) will be closed when the File object goes out of scope.
     * @param target File system target.
     * @param path File path (relative to the file system root).
     * @param mode File access mode.
     */
    File(const char* name, Mode mode) : m_handle(), m_mode(mode), m_isOpen()
    {
        m_handle = allocate<FIL>();
        memset(m_handle, 0, sizeof(FIL));
        FRESULT fr = f_open(m_handle, name, mode);
        if (fr == FR_OK) m_isOpen = true;
        else
        {
            if (fr == FR_NO_FILE)
                Debug::log(LogMessage::spam, "File '%s' not found.", name);
            else
                Debug::log(LogMessage::error, "Could not open file '%s', code %i.", name, (uint32_t)fr);
            close();
        }
    }

    /**
     * @brief File object cannot be copied.
     */
    File(const File&) = delete;

    File(File&& other) noexcept : m_handle(other.m_handle), m_mode(other.m_mode), m_isOpen(other.m_isOpen)
    {
        other.m_handle = nullptr;
        other.m_mode = openExisting;
        other.m_isOpen = false;
    }

    File& operator=(File&& other) noexcept
    {
        if (this != &other)
        {
            this->close(); // Should do nothing for empty objects.
            m_handle = other.m_handle;
            m_mode = other.m_mode;
            m_isOpen = other.m_isOpen;
            other.m_handle = nullptr;
            other.m_mode = openExisting;
            other.m_isOpen = false;
        }
        return *this;
    }

    /**
     * @brief Closes the file if it was successfully opened in constructor.
     */
    ~File() {
        close();
    }

    /**
     * @brief Closes the file if it is open. The file will be closed automatically when this object goes out of scope.
     */
    void close()
    {
        if (!m_handle) return;
        if (m_isOpen && m_handle)
        {
            FRESULT result = f_close(m_handle);
            if (result == FR_OK) m_isOpen = false;
            else Debug::log(LogMessage::error, "Couldn't close a file, code %i.", result);
        }
        deallocate<FIL>(m_handle);
        m_handle = nullptr;
        m_mode = openExisting;
    }

    /**
     * @returns True if the file is open.
     */
    bool isOpen() { return m_handle != nullptr; }

    /**
     * @returns The file access mode.
     */
    Mode mode() { return m_mode; }

    /**
     * @returns Initialized file handle or `nullptr` if the file is not open.
     */
    operator FIL*() { return m_handle; }

private:
    FIL* m_handle;
    Mode m_mode;
    bool m_isOpen;
};
