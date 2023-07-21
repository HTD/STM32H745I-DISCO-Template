/**
 * @file        FS.cpp
 * @author      CodeDog
 * @brief       Minimalistic file system helper.
 *
 * @remarks     WARNING: DO NOT CALL ANYTHING BUT `mount_` from ISR!
 *
 * @copyright   (c)2023 CodeDog, All rights reserved.
 */

#include "FS.hpp"
#include "Thread.hpp"
#include <memory>

void FS::init()
{
    mount_eMMC();
    MX_USB_HOST_Init();
    internal.rootPath = static_cast<char*>(MMCPath);
    external.rootPath = static_cast<char*>(USBHPath);
}

std::string FS::path(Target& target, const char* relativePath)
{
    return std::string(target.rootPath) + std::string(relativePath);
}

bool FS::dirExists(Target& target, const char* dirName)
{
    if (!target.isAvailable) return false;
    std::string sPath = path(target, dirName);
    const char* cPath = sPath.c_str();
    DIR dp;
    if (f_opendir(&dp, cPath) == FR_OK)
    {
        f_closedir(&dp);
        return true;
    }
    return false;
}

bool FS::mkdir(Target& target, const char* dirName)
{
    if (!target.isAvailable) return false;
    if (dirExists(target, dirName)) return true;
    std::string sPath = path(target, dirName);
    const char* cPath = sPath.c_str();
    return f_mkdir(cPath) == FR_OK;
}

File FS::open(Target &target, const char *fileName, File::Mode mode)
{
    if (!target.isAvailable || !target.rootPath) return File();
    std::string sPath = FS::path(target, fileName);
    const char* cPath = sPath.c_str();
    target.isIdle = 0;
    return File(cPath, mode);
}

void FS::close(Target& target, File& file)
{
    file.close();
    target.isIdle = 1;
}

FS::ReadResult FS::read(File& file, void* buffer, UINT length)
{
    if (!file.isOpen() || (file.mode() & File::read) == 0) return {};
    FRESULT result;
    UINT offset = 0;
    UINT blockLength = 0;
    UINT bytesLeft = length;
    UINT bytesRead = 0;
    do
    {
        result = f_read(file, ((uint8_t*) buffer + offset), bytesLeft, &blockLength);
        if (result != FR_OK) return {};
        bytesLeft -= blockLength;
        bytesRead += blockLength;
        offset += blockLength;
    }
    while (bytesLeft > 0 && blockLength > 0);
    return bytesRead;
}

FS::ReadResult FS::read(Target& target, const char* fileName, void* buffer, UINT length)
{
    if (!target.isAvailable) return {};
    auto file = open(target, fileName, File::read);
    FS::ReadResult result = read(file, buffer, length);
    close(target, file);
    return result;
}

bool FS::write(File& file, const void* buffer, UINT length)
{
    if (!file.isOpen() || (file.mode() & File::write) == 0) return false;
    FRESULT result;
    UINT offset = 0;
    UINT blockLength = 0;
    UINT bytesLeft = length;
    do
    {
        result = f_write(file, ((uint8_t*) buffer + offset), bytesLeft, &blockLength);
        if (result != FR_OK) return false;
        bytesLeft -= blockLength;
        offset += blockLength;
    }
    while (bytesLeft > 0);
    return true;
}

bool FS::write(Target& target, const char* fileName, const void* buffer, UINT length)
{
    if (!target.isAvailable) return false;
    auto file = open(target, fileName, File::write | File::createAlways);
    bool result = write(file, buffer, length);
    close(target, file);
    return result;
}

bool FS::rename(Target& target, const char* fileName, const char* newName, bool overwrite)
{
    if (!target.isAvailable) return false;
    std::string sPath = path(target, fileName);
    std::string sNewPath = path(target, newName);
    const char* cPath = sPath.c_str();
    const char* cNewPath = sNewPath.c_str();
    FRESULT result = f_rename(cPath, cNewPath);
    if (result != FR_OK && overwrite)
    { // oopsie, newPath exists:
        result = f_unlink(cNewPath); // right?
        if (result == FR_OK) result = f_rename(cPath, cNewPath);
    }
    return result == FR_OK;
}

bool FS::unlink(Target& target, const char* fileName)
{
    if (!target.isAvailable) return false;
    std::string sPath = path(target, fileName);
    const char* cPath = sPath.c_str();
    bool result = f_unlink(cPath) == FR_OK;
    return result;
}

bool FS::prefix(Target& target, const char* dirName, const char* fileName, const char* prefix, bool overwrite)
{
    if (!target.isAvailable) return false;
    std::string sDirName = std::string(dirName);
    std::string sFileName = std::string(fileName);
    std::string sPrefix = std::string(prefix);
    std::string sMerged = dirName ? (sDirName + '/' + sFileName) : sFileName;
    std::string sPrefixed = dirName ? (sDirName + '/' + sPrefix + sFileName) : (sPrefix + sFileName);
    const char* cPrefixed = sPrefixed.c_str();
    const char* cMerged = sMerged.c_str();
    bool result = rename(target, cMerged, cPrefixed, overwrite);
    return result;
}

bool FS::getTimestamp(Target& target, const char* fileName, DateTime& timestamp)
{
    if (!target.isAvailable) return false;
    std::string sPath = path(target, fileName);
    const char* cPath = sPath.c_str();
    FILINFO stat;
    if (f_stat(cPath, &stat) != FR_OK) return false;
    reinterpret_cast<DateTimeEx&>(timestamp).getFAT(0x10000 * stat.fdate + stat.ftime);
    return true;
}

FS::TimeResult FS::getTimestamp(Target &target, const char *fileName)
{
    if (!target.isAvailable) return {};
    std::string sPath = path(target, fileName);
    const char* cPath = sPath.c_str();
    FILINFO stat;
    if (f_stat(cPath, &stat) != FR_OK) return {};
    auto timestamp = std::make_unique<DateTimeEx>();
    timestamp->getFAT(0x10000 * stat.fdate + stat.ftime);
    return timestamp;
}

bool FS::test(Target& target)
{
    if (!target.isAvailable) return false;
    const char* fileName = "_test.dat";
    const uint32_t s1 = 0x1337;
    uint32_t s2 = 0;
    bool writeResult = write(target, fileName, (void*) &s1, sizeof(s1));
    if (!writeResult) return false;
    bool readResult = read(target, fileName, (void*) &s2, sizeof(s2)).has_value();
    if (writeResult) unlink(target, fileName);
    return readResult && s1 == s2;
}

FRESULT FS::format_eMMC()
{
    if (Thread::fromISR())
    {
        Thread::sync([]{ format_eMMC(); });
        return FR_OK;
    }
    Debug::log(LogMessage::info, "Formatting...");
    TCHAR workBuffer[_MAX_SS];
    FRESULT fr = f_mkfs(MMCPath, FM_EXFAT, 0, workBuffer, sizeof(workBuffer));
    if (fr == FR_OK)
    {
        Debug::log(LogMessage::info, "Formatted successfully. Mounting...");
        fr = f_mount(&MMCFatFS, MMCPath, 0x1);
        if (fr == FR_OK) Debug::log(LogMessage::info, "Mounted successfully.");
        else Debug::log(LogMessage::error, "Mount FAILED!");
    }
    else Debug::log(LogMessage::error, "Format FAILED!");
    return fr;
}

void FS::mount_eMMC()
{
    if (Thread::fromISR())
    {
        Thread::sync(mount_eMMC);
        return;
    }
    FRESULT fr = f_mount(&MMCFatFS, MMCPath, 0x1);
    if (fr != FR_OK) fr = format_eMMC();
    if (fr == FR_OK)
    {
        internal.isAvailable = true;
        if (availableChanged) availableChanged();
    }
    else Debug::log(LogMessage::error, "MMC initialization FAILED!");
}

void FS::mount_USB()
{
    if (Thread::fromISR())
    {
        Thread::sync(mount_USB);
        return;
    }
    FRESULT fr = f_mount(&USBHFatFS, USBHPath, 0x1);
    if (fr == FR_OK) external.isAvailable = true;
    else Debug::log(LogMessage::error, "Could not mount USB drive, code %i.", fr);
    if (availableChanged) availableChanged();
}

void FS::umount_USB()
{
    external.isAvailable = false;
    if (availableChanged) availableChanged();
}

void FS_mount_USB() { FS::mount_USB(); }
void FS_umount_USB() { FS::umount_USB(); }
