#ifndef DRIVER_CORE_H
#define DRIVER_CORE_H

#include "virtual_mic.h"

// Estructura de extensión del dispositivo
typedef struct _DEVICE_EXTENSION {
    PDEVICE_OBJECT DeviceObject;
    UNICODE_STRING DeviceName;
    UNICODE_STRING SymbolicLinkName;
    BOOLEAN IsInitialized;
    PVOID AudioBuffer;
    KSPIN_LOCK BufferLock;
    ULONG BufferSize;
    ULONG WritePosition;
    ULONG ReadPosition;
} DEVICE_EXTENSION, *PDEVICE_EXTENSION;

// Variables globales del driver
extern UNICODE_STRING g_DeviceName;
extern UNICODE_STRING g_SymbolicLinkName;

// Funciones del núcleo del driver
NTSTATUS InitializeDevice(
    _In_ PDRIVER_OBJECT DriverObject,
    _In_ PUNICODE_STRING RegistryPath
);

VOID CleanupDevice(
    _In_ PDEVICE_OBJECT DeviceObject
);

NTSTATUS AllocateAudioBuffer(
    _In_ PDEVICE_EXTENSION DeviceExtension
);

VOID FreeAudioBuffer(
    _In_ PDEVICE_EXTENSION DeviceExtension
);

#endif // DRIVER_CORE_H