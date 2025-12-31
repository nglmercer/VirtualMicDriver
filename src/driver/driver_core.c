#include "driver_core.h"
#include "common.h"

// Variables globales
UNICODE_STRING g_DeviceName = RTL_CONSTANT_STRING(L"\\Device\\VirtualMicrophone");
UNICODE_STRING g_SymbolicLinkName = RTL_CONSTANT_STRING(L"\\DosDevices\\VirtualMicrophone");

NTSTATUS InitializeDevice(
    _In_ PDRIVER_OBJECT DriverObject,
    _In_ PUNICODE_STRING RegistryPath
)
{
    NTSTATUS status;
    PDEVICE_OBJECT deviceObject = NULL;
    PDEVICE_EXTENSION deviceExtension;
    
    UNREFERENCED_PARAMETER(RegistryPath);
    
    DEBUG_PRINT("DriverEntry called");
    
    // Crear dispositivo
    status = IoCreateDevice(
        DriverObject,
        sizeof(DEVICE_EXTENSION),
        &g_DeviceName,
        FILE_DEVICE_UNKNOWN,
        FILE_DEVICE_SECURE_OPEN,
        FALSE,
        &deviceObject
    );
    
    if (!NT_SUCCESS(status)) {
        ERROR_PRINT("Failed to create device: 0x%X", status);
        return status;
    }
    
    // Inicializar extensión del dispositivo
    deviceExtension = (PDEVICE_EXTENSION)deviceObject->DeviceExtension;
    RtlZeroMemory(deviceExtension, sizeof(DEVICE_EXTENSION));
    
    deviceExtension->DeviceObject = deviceObject;
    deviceExtension->DeviceName = g_DeviceName;
    deviceExtension->IsInitialized = FALSE;
    deviceExtension->BufferSize = DEFAULT_BUFFER_SIZE;
    
    // Crear enlace simbólico
    status = IoCreateSymbolicLink(&g_SymbolicLinkName, &g_DeviceName);
    if (!NT_SUCCESS(status)) {
        ERROR_PRINT("Failed to create symbolic link: 0x%X", status);
        IoDeleteDevice(deviceObject);
        return status;
    }
    
    deviceExtension->SymbolicLinkName = g_SymbolicLinkName;
    
    // Inicializar spinlock para el buffer
    KeInitializeSpinLock(&deviceExtension->BufferLock);
    
    // Asignar memoria para el buffer de audio
    status = AllocateAudioBuffer(deviceExtension);
    if (!NT_SUCCESS(status)) {
        ERROR_PRINT("Failed to allocate audio buffer");
        IoDeleteSymbolicLink(&g_SymbolicLinkName);
        IoDeleteDevice(deviceObject);
        return status;
    }
    
    deviceExtension->IsInitialized = TRUE;
    
    DEBUG_PRINT("Driver initialized successfully");
    return STATUS_SUCCESS;
}

VOID CleanupDevice(
    _In_ PDEVICE_OBJECT DeviceObject
)
{
    PDEVICE_EXTENSION deviceExtension;
    
    if (DeviceObject != NULL) {
        deviceExtension = (PDEVICE_EXTENSION)DeviceObject->DeviceExtension;
        
        if (deviceExtension->IsInitialized) {
            // Liberar recursos
            FreeAudioBuffer(deviceExtension);
            
            // Eliminar enlace simbólico
            IoDeleteSymbolicLink(&deviceExtension->SymbolicLinkName);
        }
        
        IoDeleteDevice(DeviceObject);
    }
    
    DEBUG_PRINT("Device cleanup completed");
}

NTSTATUS AllocateAudioBuffer(
    _In_ PDEVICE_EXTENSION DeviceExtension
)
{
    // Asignar memoria para el buffer de audio
    DeviceExtension->AudioBuffer = ExAllocatePoolWithTag(NonPagedPool, 
                                                         DeviceExtension->BufferSize, 
                                                         POOL_TAG);
    if (DeviceExtension->AudioBuffer == NULL) {
        return STATUS_INSUFFICIENT_RESOURCES;
    }
    
    RtlZeroMemory(DeviceExtension->AudioBuffer, DeviceExtension->BufferSize);
    DeviceExtension->WritePosition = 0;
    DeviceExtension->ReadPosition = 0;
    
    return STATUS_SUCCESS;
}

VOID FreeAudioBuffer(
    _In_ PDEVICE_EXTENSION DeviceExtension
)
{
    if (DeviceExtension->AudioBuffer != NULL) {
        ExFreePoolWithTag(DeviceExtension->AudioBuffer, POOL_TAG);
        DeviceExtension->AudioBuffer = NULL;
    }
}
