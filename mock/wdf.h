#pragma once

// Mock implementation of Windows Driver Framework headers
// This allows WDF code to compile without WDK installed

#include "ntddk.h"

#ifdef __cplusplus
extern "C" {
#endif

// WDF types
typedef void* WDFDRIVER;
typedef void* WDFDEVICE;
typedef void* WDF_OBJECT_ATTRIBUTES;
typedef void* WDF_DRIVER_CONFIG;
typedef void* WDF_NO_HANDLE;
typedef void* WDFREQUEST;
typedef void* WDFQUEUE;
typedef void* WDFTIMER;

// WDF constants
#define WDF_NO_EVENT_CALLBACK NULL
#define WdfDriverInitNonPnpDriver 0x00000001
#define WDF_NO_OBJECT_ATTRIBUTES NULL
#define WDF_NO_HANDLE NULL

// WDF function prototypes
typedef NTSTATUS (*PFN_WDF_DRIVER_DEVICE_ADD)(WDFDRIVER Driver, WDFDEVICE Device);
typedef void (*PFN_WDF_DRIVER_UNLOAD)(WDFDRIVER Driver);

// WDF configuration
typedef struct _WDF_DRIVER_CONFIG {
    ULONG Size;
    PFN_WDF_DRIVER_DEVICE_ADD EvtDriverDeviceAdd;
    PFN_WDF_DRIVER_UNLOAD EvtDriverUnload;
    ULONG DriverInitFlags;
    ULONG DriverPoolTag;
} WDF_DRIVER_CONFIG, *PWDF_DRIVER_CONFIG;

static inline void WDF_DRIVER_CONFIG_INIT(PWDF_DRIVER_CONFIG Config, PFN_WDF_DRIVER_DEVICE_ADD EvtDriverDeviceAdd) {
    Config->Size = sizeof(WDF_DRIVER_CONFIG);
    Config->EvtDriverDeviceAdd = EvtDriverDeviceAdd;
    Config->EvtDriverUnload = NULL;
    Config->DriverInitFlags = 0;
    Config->DriverPoolTag = 0;
}

// WDF object attributes
typedef struct _WDF_OBJECT_ATTRIBUTES {
    ULONG Size;
    void *EvtCleanupCallback;
    void *EvtDestroyCallback;
    ULONG ExecutionLevel;
    ULONG SynchronizationScope;
} WDF_OBJECT_ATTRIBUTES, *PWDF_OBJECT_ATTRIBUTES;

static inline void WDF_OBJECT_ATTRIBUTES_INIT(PWDF_OBJECT_ATTRIBUTES Attributes) {
    Attributes->Size = sizeof(WDF_OBJECT_ATTRIBUTES);
    Attributes->EvtCleanupCallback = NULL;
    Attributes->EvtDestroyCallback = NULL;
    Attributes->ExecutionLevel = 0;
    Attributes->SynchronizationScope = 0;
}

// WDF driver functions
static inline NTSTATUS WdfDriverCreate(
    PDRIVER_OBJECT DriverObject,
    PVOID RegistryPath,
    PWDF_OBJECT_ATTRIBUTES DriverAttributes,
    PWDF_DRIVER_CONFIG DriverConfig,
    WDFDRIVER *Driver
) {
    // Mock implementation - just return success
    if (Driver) {
        *Driver = (WDFDRIVER)0x12345678; // Mock handle
    }
    return STATUS_SUCCESS;
}

// WDF device functions
static inline NTSTATUS WdfDeviceCreate(
    PWDFDEVICE_INIT *DeviceInit,
    PWDF_OBJECT_ATTRIBUTES DeviceAttributes,
    WDFDEVICE *Device
) {
    // Mock implementation
    if (Device) {
        *Device = (WDFDEVICE)0x87654321; // Mock handle
    }
    return STATUS_SUCCESS;
}

// WDF request functions
static inline PVOID WdfRequestGetInformation(WDFREQUEST Request) {
    return NULL; // Mock implementation
}

static inline void WdfRequestComplete(WDFREQUEST Request, NTSTATUS Status) {
    // Mock implementation
}

static inline void WdfRequestCompleteWithInformation(WDFREQUEST Request, NTSTATUS Status, ULONG_PTR Information) {
    // Mock implementation
}

// WDF queue functions
static inline NTSTATUS WdfIoQueueCreate(
    WDFDEVICE Device,
    void *Config,
    PWDF_OBJECT_ATTRIBUTES QueueAttributes,
    WDFQUEUE *Queue
) {
    // Mock implementation
    if (Queue) {
        *Queue = (WDFQUEUE)0x11111111; // Mock handle
    }
    return STATUS_SUCCESS;
}

// WDF timer functions
static inline NTSTATUS WdfTimerCreate(
    void *Config,
    PWDF_OBJECT_ATTRIBUTES TimerAttributes,
    WDFTIMER *Timer
) {
    // Mock implementation
    if (Timer) {
        *Timer = (WDFTIMER)0x22222222; // Mock handle
    }
    return STATUS_SUCCESS;
}

// WDF memory functions
static inline NTSTATUS WdfMemoryCreate(
    PWDF_OBJECT_ATTRIBUTES Attributes,
    POOL_TYPE PoolType,
    ULONG PoolTag,
    SIZE_T BufferSize,
    void **Memory,
    PVOID *Buffer
) {
    // Mock implementation
    if (Buffer) {
        *Buffer = malloc(BufferSize);
        if (*Buffer == NULL) {
            return STATUS_INSUFFICIENT_RESOURCES;
        }
    }
    return STATUS_SUCCESS;
}

// WDF string functions
static inline NTSTATUS WdfStringCreate(
    PCWSTR UnicodeString,
    PWDF_OBJECT_ATTRIBUTES StringAttributes,
    void **String
) {
    // Mock implementation
    if (String) {
        *String = (void*)UnicodeString; // Just return the input string
    }
    return STATUS_SUCCESS;
}

// Other WDF types and constants
typedef void* WDFDEVICE_INIT;
typedef void* WDFQUEUE_CONFIG;
typedef void* WDF_TIMER_CONFIG;
typedef void* WDF_MEMORY_DESCRIPTOR;
typedef void* WDFSTRING;

#ifdef __cplusplus
}
#endif