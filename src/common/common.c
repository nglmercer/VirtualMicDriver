#include "common.h"

ULONG64 GetSystemUptimeMs(VOID)
{
    LARGE_INTEGER currentTime;
    KeQuerySystemTime(&currentTime);
    
    // Convertir de 100-nanosecond intervals a milisegundos
    return (ULONG64)(currentTime.QuadPart / 10000);
}

VOID SafeStringCopy(
    _Out_writes_(destSize) PWSTR dest,
    _In_ SIZE_T destSize,
    _In_ PCWSTR src
)
{
    if (dest == NULL || src == NULL || destSize == 0) {
        return;
    }
    
    // Usar RtlStringCchCopyW para copia segura
    RtlStringCchCopyW(dest, destSize, src);
}