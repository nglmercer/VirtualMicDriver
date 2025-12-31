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
    
    // Manual safe string copy for kernel mode
    SIZE_T i;
    for (i = 0; i < destSize - 1 && src[i] != L'\0'; i++) {
        dest[i] = src[i];
    }
    dest[i] = L'\0';
}
