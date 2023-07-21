/**
 * @file        FS.hpp
 * @author      CodeDog
 * @brief       Minimalistic file system helper header file.
 *
 * @remarks     WARNING: DO NOT CALL ANYTHING BUT `mount_` from ISR!
 *
 * @copyright   (c)2023 CodeDog, All rights reserved.
 */

#pragma once

#include <FS_C.h>
#include <memory>
#include <optional>
#include <string>

#include "c_bindings.h"
EXTERN_C_BEGIN
#include "hal_ex.h"
#include "fatfs.h"
#include "usb_host.h"
EXTERN_C_END

#include "Action.hpp"
#include "DateTimeEx.hpp"
#include "File.hpp"

class FS
{
public:

    /**
     * @brief FATFS target definition.
     */
    struct Target final
    {
        const TCHAR* rootPath; // Root path of the file system.
        bool isAvailable;      // True if the specified file system is available.
        bool isIdle;           // True if the specified file system is idle.
        bool equals(Target& other) { return this == &other; }
        bool operator==(Target& other) { return this == &other; }
        bool operator!=(Target& other) { return this != &other; }
        Target() : rootPath(), isAvailable(0), isIdle(1) { }
    };

    using ReadResult = std::optional<UINT>; // A read result of the file read operation.
    using TimeResult = std::optional<std::unique_ptr<DateTime>>; // A result of the getTimestamp() operation.

    FS() = delete;
    FS(const FS&) = delete;
    FS(const FS&&) = delete;

    /// @brief Initializes the file systems.
    static void init();

    /**
     * @brief Gets the absolute path to the file system entry.
     *
     * @param target Target file system.
     * @param relativePath Relative path.
     * @return Absolute path to the file system entry.
     */
    static std::string path(Target& target, const char* fileName);

    /**
     * @brief Checks if the specified directory exist on the target file system.
     *
     * @param target Target file system.
     * @param dirName Relative path.
     * @returns 1: Directory exists. 0: Directory does not exist.
     */
    static bool dirExists(Target& target, const char* dirName);

    /**
     * @brief Creates a directory on the target file system if it doesn't exist already.
     *
     * @param target Target file system.
     * @param dirName Relative directory path.
     * @returns 1: Directory already exists or has been created successfully. 0: Could not create directory.
     */
    static bool mkdir(Target& target, const char* dirName);

    /**
     * @brief Opens a file for `read` and `write` operations. Clears the `isIdle` member of the target.
     * @remarks The `isIdle` member of the target must be cleared manually after multiple I/O access.
     *
     * @param target Target file system.
     * @param fileName File name (relative path).
     * @param mode File access mode.
     * @returns File object, check with `isOpen()` if it's ready to use.
     */
    static File open(Target& target, const char* fileName, File::Mode mode);

    /**
     * @brief Closes a file if it's open.
     *
     * @param Target file system.
     * @param file File object reference.
     */
    static void close(Target& target, File& file);

    /**
     * @brief Reads a block of data from a file.
     *
     * @param file Open file object.
     * @param buffer Data pointer.
     * @param length Data length.
     * @return Optional bytes read, no value on error.
     */
    static ReadResult read(File& file, void* buffer, UINT length);

    /**
     * @brief Reads data from a file.
     *
     * @param target Target file system.
     * @param fileName File name (relative path).
     * @param buffer Data pointer.
     * @param length Data length.
     * @return Optional bytes read, no value on error.
     */
    static ReadResult read(Target& target, const char* fileName, void* buffer, UINT length);

    /**
     * @brief Writes a block of data to a file.
     *
     * @param file Open file object.
     * @param buffer Data pointer.
     * @param length Data length.
     * @return 1: Success. 0: Failure.
     */
    static bool write(File& file, const void* buffer, UINT length);

    /**
     * @brief Writes data to a file.
     *
     * @param target Target file system.
     * @param fileName File name (relative path).
     * @param buffer Data pointer.
     * @param length Data length.
     * @return 1: Success. 0: Failure.
     */
    static bool write(Target& target, const char* fileName, const void* buffer, UINT length);

    /**
     * @brief Renames a file.
     *
     * @param target Target file system.
     * @param fileName Existing file name.
     * @param newName New file name.
     * @param overwrite Set to overwrite the existing file with the new name. Default true.
     * @return 1: Success. 0: Failure.
     */
    static bool rename(Target& target, const char* fileName, const char* newName, bool overwrite = true);

    /**
     * @brief Deletes a file.
     *
     * @param target Target file system.
     * @param fileName File name.
     * @return 1: File deleted sucessfully. 0: Something went wrong.
     */
    static bool unlink(Target& target, const char* fileName);

    /**
     * @brief Renames a file by adding a prefix before its name.
     *
     * @param target Target file system.
     * @param dirName Directory name. Use nullptr for current directory.
     * @param fileName File name.
     * @param prefix Prefix string.
     * @param overwrite Set to overwrite the existing file with the new name. Default true.
     * @return 1: File renamed successfully. 0: Something went wrong.
     */
    static bool prefix(Target& target, const char* dirName, const char* fileName, const char* prefix = ".", bool overwrite = true);

    /**
     * @brief Gets the timestap of a file if it exists.
     *
     * @param target Target file system.
     * @param fileName File name.
     * @param timestamp A timestamp instance reference.
     * @return 1: File exists, timestamp read. 0: No file or other error.
     */
    static bool getTimestamp(Target& target, const char* fileName, DateTime& timestamp);

    /**
     * @brief Gets the timestap of a file if it exists.
     *
     * @param target Target file system.
     * @param fileName File name.
     * @return FS::TimeResult A timestamp if file exists or no value otherwise.
     */
    static TimeResult getTimestamp(Target& target, const char* fileName);

    /**
     * @brief Tests if the storage is capable of writing, reading and deleting a small test file.
     *
     * @param target Target file system.
     * @return 1: Success, 0: Failure.
     */
     static bool test(Target& target);

    /**
     * @brief Formats eMMC chip with ExFAT.
     *
     * @returns Operation result.
     */
    static FRESULT format_eMMC();

protected:

    /**
     * @brief Mounts the eMMC file system as internal storage.
     */
    static void mount_eMMC();

public:

    /**
     * @brief Mounts the USB disk file system when the media presence is detected.
     */
    static void mount_USB();

    /**
     * @brief Unmounts the USB disk file system when the media removal is detected.
     */
    static void umount_USB();

public:
    static inline Target internal{}; // Internal storage file system.
    static inline Target external{}; // External storage file system.
    static inline Action availableChanged{}; // Available file system changed (media inserted / removed).

};
