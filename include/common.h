#ifndef COMMON_H
#define COMMON_H

#include <ntddk.h>

// Macros de utilidad
#define DEBUG_PRINT(fmt, ...) DbgPrint("VirtualMic: " fmt "\n", ##__VA_ARGS__)
#define ERROR_PRINT(fmt, ...) DbgPrint("VirtualMic ERROR: " fmt "\n", ##__VA_ARGS__)

// Validaciones
#define IS_VALID_SAMPLE_RATE(rate) ((rate) >= 8000 && (rate) <= 192000)
#define IS_VALID_CHANNELS(ch) ((ch) >= 1 && (ch) <= 8)
#define IS_VALID_BITS_PER_SAMPLE(bits) ((bits) == 16 || (bits) == 24 || (bits) == 32)

// Constantes de tiempo
#define MS_TO_100NS(ms) ((ms) * 10000LL)
#define SECONDS_TO_MS(sec) ((sec) * 1000)

// Funciones de utilidad
ULONG64 GetSystemUptimeMs(VOID);
VOID SafeStringCopy(
    _Out_writes_(destSize) PWSTR dest,
    _In_ SIZE_T destSize,
    _In_ PCWSTR src
);

// Manejo de errores
#define RETURN_IF_NT_ERROR(status) \
    if (!NT_SUCCESS(status)) { \
        ERROR_PRINT("Operation failed with status: 0x%X", status); \
        return status; \
    }

#endif // COMMON_H