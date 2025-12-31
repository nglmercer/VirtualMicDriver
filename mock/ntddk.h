#pragma once

// Mock implementation of Windows Driver Kit headers for CI/testing
// This allows the code to compile without WDK installed

#ifdef __cplusplus
extern "C" {
#endif

// Basic Windows types
typedef void *PVOID;
typedef void *HANDLE;
typedef unsigned char UCHAR;
typedef unsigned short USHORT;
typedef unsigned long ULONG;
typedef unsigned long long ULONG64;
typedef int BOOL;
typedef wchar_t WCHAR;

// NTSTATUS
typedef long NTSTATUS;
#define STATUS_SUCCESS ((NTSTATUS)0x00000000L)
#define STATUS_UNSUCCESSFUL ((NTSTATUS)0xC0000001L)
#define STATUS_INSUFFICIENT_RESOURCES ((NTSTATUS)0xC000009AL)
#define STATUS_INVALID_PARAMETER ((NTSTATUS)0xC000000DL)
#define STATUS_INVALID_DEVICE_REQUEST ((NTSTATUS)0xC0000010L)
#define STATUS_DEVICE_NOT_READY ((NTSTATUS)0xC00000A1L)
#define STATUS_BUFFER_TOO_SMALL ((NTSTATUS)0xC0000023L)
#define STATUS_INVALID_BUFFER_SIZE ((NTSTATUS)0xC0000203L)

#define NT_SUCCESS(Status) (((NTSTATUS)(Status)) >= 0)

// String types
typedef struct _UNICODE_STRING {
    USHORT Length;
    USHORT MaximumLength;
    PWSTR Buffer;
} UNICODE_STRING, *PUNICODE_STRING;

#define RTL_CONSTANT_STRING(s) { sizeof(s) - sizeof(WCHAR), sizeof(s), s }

// IRP and device structures (simplified)
typedef struct _IRP IRP;
typedef struct _DEVICE_OBJECT DEVICE_OBJECT, *PDEVICE_OBJECT;
typedef struct _DRIVER_OBJECT DRIVER_OBJECT, *PDRIVER_OBJECT;

// Function types
typedef NTSTATUS (*DRIVER_DISPATCH)(PDEVICE_OBJECT DeviceObject, PIRP Irp);
typedef void (*DRIVER_UNLOAD)(PDRIVER_OBJECT DriverObject);

// Driver object
typedef struct _DRIVER_OBJECT {
    PDEVICE_OBJECT DeviceObject;
    DRIVER_UNLOAD DriverUnload;
    DRIVER_DISPATCH MajorFunction[28];
} DRIVER_OBJECT;

// Device object (simplified)
typedef struct _DEVICE_OBJECT {
    PDRIVER_OBJECT DriverObject;
    PVOID DeviceExtension;
} DEVICE_OBJECT;

// IRP (simplified)
typedef struct _IRP {
    PVOID AssociatedIrp;
    PVOID Tail;
} IRP;

// IO_STACK_LOCATION (simplified)
typedef struct _IO_STACK_LOCATION {
    ULONG MajorFunction;
    ULONG Parameters;
} IO_STACK_LOCATION, *PIO_STACK_LOCATION;

// Macros
#define IoGetCurrentIrpStackLocation(Irp) ((PIO_STACK_LOCATION)0)
#define IoCompleteRequest(Irp, PriorityBoost) 
#define IoCreateDevice(DriverObject, DeviceExtensionSize, DeviceName, DeviceType, DeviceCharacteristics, Exclusive, DeviceObject) STATUS_SUCCESS
#define IoDeleteDevice(DeviceObject)
#define IoCreateSymbolicLink(SymbolicLinkName, DeviceName) STATUS_SUCCESS
#define IoDeleteSymbolicLink(SymbolicLinkName) STATUS_SUCCESS

// Memory management
#define NonPagedPool 0
#define POOL_TAG 'VMic'

static inline PVOID ExAllocatePoolWithTag(int PoolType, SIZE_T NumberOfBytes, ULONG Tag) {
    return malloc(NumberOfBytes);
}

static inline void ExFreePoolWithTag(PVOID P, ULONG Tag) {
    free(P);
}

// String functions
static inline void RtlZeroMemory(PVOID Destination, SIZE_T Length) {
    memset(Destination, 0, Length);
}

static inline void RtlCopyMemory(PVOID Destination, const void *Source, SIZE_T Length) {
    memcpy(Destination, Source, Length);
}

// Debug output
#define DbgPrint printf

// Spinlocks (simplified)
typedef struct _KSPIN_LOCK {
    int dummy;
} KSPIN_LOCK, *PKSPIN_LOCK;

static inline void KeInitializeSpinLock(PKSPIN_LOCK SpinLock) {
    // Mock implementation
}

static inline void KeAcquireSpinLock(PKSPIN_LOCK SpinLock, int *OldIrql) {
    // Mock implementation
}

static inline void KeReleaseSpinLock(PKSPIN_LOCK SpinLock, int OldIrql) {
    // Mock implementation
}

// WDF (Windows Driver Framework) - simplified
typedef void* WDFDRIVER;
typedef void* WDFDEVICE;
typedef void* WDF_OBJECT_ATTRIBUTES;
typedef void* WDF_DRIVER_CONFIG;
typedef void* WDF_NO_HANDLE;

#define WDF_NO_EVENT_CALLBACK NULL
#define WdfDriverInitNonPnpDriver 0x00000001

// WDF functions (mock)
static inline void WDF_DRIVER_CONFIG_INIT(WDF_DRIVER_CONFIG *Config, void *EvtDriverDeviceAdd) {
    // Mock implementation
}

static inline NTSTATUS WdfDriverCreate(PDRIVER_OBJECT DriverObject, void *RegistryPath, 
                                     WDF_OBJECT_ATTRIBUTES *DriverAttributes, 
                                     WDF_DRIVER_CONFIG *DriverConfig, 
                                     WDFDRIVER *Driver) {
    return STATUS_SUCCESS;
}

// System time
typedef struct _LARGE_INTEGER {
    long long QuadPart;
} LARGE_INTEGER, *PLARGE_INTEGER;

static inline void KeQuerySystemTime(LARGE_INTEGER *CurrentTime) {
    CurrentTime->QuadPart = 0; // Mock implementation
}

// String safe functions
static inline int RtlStringCchCopyW(wchar_t *dest, size_t destSize, const wchar_t *src) {
    wcscpy_s(dest, destSize, src);
    return 0;
}

// Constants
#define FILE_DEVICE_UNKNOWN 0x00000022
#define FILE_DEVICE_SECURE_OPEN 0x00000100
#define METHOD_BUFFERED 0
#define FILE_ANY_ACCESS 0
#define FILE_READ_ACCESS 1
#define FILE_WRITE_ACCESS 2

#define CTL_CODE(DeviceType, Function, Method, Access) \
    (((DeviceType) << 16) | ((Access) << 14) | ((Function) << 2) | (Method))

#define IRP_MJ_CREATE 0x00
#define IRP_MJ_CLOSE 0x02
#define IRP_MJ_READ 0x03
#define IRP_MJ_DEVICE_CONTROL 0x0E

#define IO_NO_INCREMENT 1

#ifdef __cplusplus
}
#endif