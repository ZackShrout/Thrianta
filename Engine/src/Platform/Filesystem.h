#pragma once
#include "Defines.h"

// Holds a handle to a file.
typedef struct file_handle
{
    // Opaque handle to internal file handle.
    void* handle;
    b8 isValid;
} file_handle;

typedef enum file_modes
{
    FILE_MODE_READ = 0x1,
    FILE_MODE_WRITE = 0x2
} file_modes;

/**
 * Checks if a file with the given path exists.
 * @param path The path of the file to be checked.
 * @returns True if exists; otherwise false.
 */
TAPI b8 FilesystemExists(const char* path);

/** 
 * Attempt to open file located at path.
 * @param path The path of the file to be opened.
 * @param mode Mode flags for the file when opened (read/write). See file_modes enum in filesystem.h.
 * @param binary Indicates if the file should be opened in binary mode.
 * @param outHandle A pointer to a file_handle structure which holds the handle information.
 * @returns True if opened successfully; otherwise false.
 */
TAPI b8 FilesystemOpen(const char* path, file_modes mode, b8 binary, file_handle* outHandle);

/** 
 * Closes the provided handle to a file.
 * @param handle A pointer to a file_handle structure which holds the handle to be closed.
 */
TAPI void FilesystemClose(file_handle* handle);

/** 
 * Reads up to a newline or EOF. Allocates *lineBuf, which must be freed by the caller.
 * @param handle A pointer to a file_handle structure.
 * @param lineBuf A pointer to a character array which will be allocated and populated by this method.
 * @returns True if successful; otherwise false.
 */
TAPI b8 FilesystemReadLine(file_handle* handle, char** lineBuf);

/** 
 * Writes text to the provided file, appending a '\n' afterward.
 * @param handle A pointer to a file_handle structure.
 * @param text The text to be written.
 * @returns True if successful; otherwise false.
 */
TAPI b8 FilesystemWriteLine(file_handle* handle, const char* text);

/** 
 * Reads up to dataSize bytes of data into outBytesRead. 
 * Allocates *outData, which must be freed by the caller.
 * @param handle A pointer to a file_handle structure.
 * @param dataSize The number of bytes to read.
 * @param outData A pointer to a block of memory to be populated by this method.
 * @param outBytesRead A pointer to a number which will be populated with the number of bytes actually read from the file.
 * @returns True if successful; otherwise false.
 */
TAPI b8 FilesystemRead(file_handle* handle, u64 dataSize, void* outData, u64* outBytesRead);

/** 
 * Reads up to dataSize bytes of data into outBytesRead. 
 * Allocates *outBytes, which must be freed by the caller.
 * @param handle A pointer to a file_handle structure.
 * @param outBytes A pointer to a byte array which will be allocated and populated by this method.
 * @param outBytesRead A pointer to a number which will be populated with the number of bytes actually read from the file.
 * @returns True if successful; otherwise false.
 */
TAPI b8 FilesystemReadAllBytes(file_handle* handle, u8** outBytes, u64* outBytesRead);

/** 
 * Writes provided data to the file.
 * @param handle A pointer to a file_handle structure.
 * @param dataSize The size of the data in bytes.
 * @param data The data to be written.
 * @param outBytesWritten A pointer to a number which will be populated with the number of bytes actually written to the file.
 * @returns True if successful; otherwise false.
 */
TAPI b8 FilesystemWrite(file_handle* handle, u64 dataSize, const void* data, u64* outBytesWritten);