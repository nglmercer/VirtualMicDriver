#pragma once

// Mock implementation of ntstrsafe.h for CI/testing without WDK

#include "ntddk.h"
#include <stdarg.h>

#ifdef __cplusplus
extern "C" {
#endif

// String safe functions - mock implementations
static inline HRESULT RtlStringCbCopyW(wchar_t *pszDest, size_t cbDest, const wchar_t *pszSrc) {
    if (pszDest == NULL || pszSrc == NULL || cbDest == 0) {
        return -1; // ERROR_INVALID_PARAMETER
    }
    
    size_t srcLen = wcslen(pszSrc);
    size_t bytesNeeded = (srcLen + 1) * sizeof(wchar_t);
    
    if (bytesNeeded > cbDest) {
        return -1; // ERROR_INSUFFICIENT_BUFFER
    }
    
    wcscpy_s(pszDest, cbDest / sizeof(wchar_t), pszSrc);
    return 0; // S_OK
}

static inline HRESULT RtlStringCbCopyA(char *pszDest, size_t cbDest, const char *pszSrc) {
    if (pszDest == NULL || pszSrc == NULL || cbDest == 0) {
        return -1; // ERROR_INVALID_PARAMETER
    }
    
    size_t srcLen = strlen(pszSrc);
    size_t bytesNeeded = srcLen + 1;
    
    if (bytesNeeded > cbDest) {
        return -1; // ERROR_INSUFFICIENT_BUFFER
    }
    
    strcpy_s(pszDest, cbDest, pszSrc);
    return 0; // S_OK
}

static inline HRESULT RtlStringCbCatW(wchar_t *pszDest, size_t cbDest, const wchar_t *pszSrc) {
    if (pszDest == NULL || pszSrc == NULL || cbDest == 0) {
        return -1; // ERROR_INVALID_PARAMETER
    }
    
    size_t destLen = wcslen(pszDest);
    size_t srcLen = wcslen(pszSrc);
    size_t bytesAvailable = (cbDest / sizeof(wchar_t)) - destLen;
    size_t bytesNeeded = srcLen + 1;
    
    if (bytesNeeded > bytesAvailable) {
        return -1; // ERROR_INSUFFICIENT_BUFFER
    }
    
    wcscat_s(pszDest, cbDest / sizeof(wchar_t), pszSrc);
    return 0; // S_OK
}

static inline HRESULT RtlStringCbCatA(char *pszDest, size_t cbDest, const char *pszSrc) {
    if (pszDest == NULL || pszSrc == NULL || cbDest == 0) {
        return -1; // ERROR_INVALID_PARAMETER
    }
    
    size_t destLen = strlen(pszDest);
    size_t srcLen = strlen(pszSrc);
    size_t bytesAvailable = cbDest - destLen;
    size_t bytesNeeded = srcLen + 1;
    
    if (bytesNeeded > bytesAvailable) {
        return -1; // ERROR_INSUFFICIENT_BUFFER
    }
    
    strcat_s(pszDest, cbDest, pszSrc);
    return 0; // S_OK
}

static inline HRESULT RtlStringCbLengthW(const wchar_t *pszString, size_t cbMax, size_t *pcbLength) {
    if (pszString == NULL || pcbLength == NULL) {
        return -1; // ERROR_INVALID_PARAMETER
    }
    
    size_t stringLen = wcslen(pszString);
    size_t byteLength = stringLen * sizeof(wchar_t);
    
    if (byteLength >= cbMax) {
        return -1; // ERROR_INVALID_PARAMETER
    }
    
    *pcbLength = byteLength;
    return 0; // S_OK
}

static inline HRESULT RtlStringCbLengthA(const char *pszString, size_t cbMax, size_t *pcbLength) {
    if (pszString == NULL || pcbLength == NULL) {
        return -1; // ERROR_INVALID_PARAMETER
    }
    
    size_t stringLen = strlen(pszString);
    size_t byteLength = stringLen;
    
    if (byteLength >= cbMax) {
        return -1; // ERROR_INVALID_PARAMETER
    }
    
    *pcbLength = byteLength;
    return 0; // S_OK
}

static inline HRESULT RtlStringCchCopyW(wchar_t *pszDest, size_t cchDest, const wchar_t *pszSrc) {
    if (pszDest == NULL || pszSrc == NULL || cchDest == 0) {
        return -1; // ERROR_INVALID_PARAMETER
    }
    
    size_t srcLen = wcslen(pszSrc);
    
    if (srcLen >= cchDest) {
        return -1; // ERROR_INSUFFICIENT_BUFFER
    }
    
    wcscpy_s(pszDest, cchDest, pszSrc);
    return 0; // S_OK
}

static inline HRESULT RtlStringCchCopyA(char *pszDest, size_t cchDest, const char *pszSrc) {
    if (pszDest == NULL || pszSrc == NULL || cchDest == 0) {
        return -1; // ERROR_INVALID_PARAMETER
    }
    
    size_t srcLen = strlen(pszSrc);
    
    if (srcLen >= cchDest) {
        return -1; // ERROR_INSUFFICIENT_BUFFER
    }
    
    strcpy_s(pszDest, cchDest, pszSrc);
    return 0; // S_OK
}

static inline HRESULT RtlStringCchCatW(wchar_t *pszDest, size_t cchDest, const wchar_t *pszSrc) {
    if (pszDest == NULL || pszSrc == NULL || cchDest == 0) {
        return -1; // ERROR_INVALID_PARAMETER
    }
    
    size_t destLen = wcslen(pszDest);
    size_t srcLen = wcslen(pszSrc);
    size_t charsAvailable = cchDest - destLen;
    
    if (srcLen >= charsAvailable) {
        return -1; // ERROR_INSUFFICIENT_BUFFER
    }
    
    wcscat_s(pszDest, cchDest, pszSrc);
    return 0; // S_OK
}

static inline HRESULT RtlStringCchCatA(char *pszDest, size_t cchDest, const char *pszSrc) {
    if (pszDest == NULL || pszSrc == NULL || cchDest == 0) {
        return -1; // ERROR_INVALID_PARAMETER
    }
    
    size_t destLen = strlen(pszDest);
    size_t srcLen = strlen(pszSrc);
    size_t charsAvailable = cchDest - destLen;
    
    if (srcLen >= charsAvailable) {
        return -1; // ERROR_INSUFFICIENT_BUFFER
    }
    
    strcat_s(pszDest, cchDest, pszSrc);
    return 0; // S_OK
}

static inline HRESULT RtlStringCchLengthW(const wchar_t *pszString, size_t cchMax, size_t *pcchLength) {
    if (pszString == NULL || pcchLength == NULL) {
        return -1; // ERROR_INVALID_PARAMETER
    }
    
    size_t stringLen = wcslen(pszString);
    
    if (stringLen >= cchMax) {
        return -1; // ERROR_INVALID_PARAMETER
    }
    
    *pcchLength = stringLen;
    return 0; // S_OK
}

static inline HRESULT RtlStringCchLengthA(const char *pszString, size_t cchMax, size_t *pcchLength) {
    if (pszString == NULL || pcchLength == NULL) {
        return -1; // ERROR_INVALID_PARAMETER
    }
    
    size_t stringLen = strlen(pszString);
    
    if (stringLen >= cchMax) {
        return -1; // ERROR_INVALID_PARAMETER
    }
    
    *pcchLength = stringLen;
    return 0; // S_OK
}

// HRESULT definitions for string functions
#define S_OK 0
#define STRSAFE_E_INSUFFICIENT_BUFFER -1
#define STRSAFE_E_INVALID_PARAMETER -1

#ifdef __cplusplus
}
#endif