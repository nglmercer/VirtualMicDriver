#include "virtual_mic.h"
#include "driver_core.h"
#include "ioctl_handlers.h"
#include "common.h"

// Forward declarations
VOID DriverUnload(
    _In_ PDRIVER_OBJECT DriverObject
);

NTSTATUS DispatchCreate(
    _In_ PDEVICE_OBJECT DeviceObject,
    _In_ PIRP Irp
);

NTSTATUS DispatchClose(
    _In_ PDEVICE_OBJECT DeviceObject,
    _In_ PIRP Irp
);

NTSTATUS DispatchDeviceControl(
    _In_ PDEVICE_OBJECT DeviceObject,
    _In_ PIRP Irp
);

NTSTATUS DispatchRead(
    _In_ PDEVICE_OBJECT DeviceObject,
    _In_ PIRP Irp
);

// Entry point del driver
NTSTATUS 
DriverEntry(
    _In_ PDRIVER_OBJECT DriverObject,
    _In_ PUNICODE_STRING RegistryPath
)
{
    NTSTATUS status;
    PDEVICE_OBJECT deviceObject = NULL;
    
    DEBUG_PRINT("VirtualMicrophone Driver Entry - Modular Version");
    
    // Inicializar el dispositivo
    status = InitializeDevice(DriverObject, RegistryPath);
    if (!NT_SUCCESS(status)) {
        ERROR_PRINT("Failed to initialize device: 0x%X", status);
        return status;
    }
    
    // Obtener el objeto de dispositivo creado
    deviceObject = DriverObject->DeviceObject;
    
    // Configurar funciones del driver
    DriverObject->MajorFunction[IRP_MJ_CREATE] = DispatchCreate;
    DriverObject->MajorFunction[IRP_MJ_CLOSE] = DispatchClose;
    DriverObject->MajorFunction[IRP_MJ_DEVICE_CONTROL] = DispatchDeviceControl;
    DriverObject->MajorFunction[IRP_MJ_READ] = DispatchRead;
    DriverObject->DriverUnload = DriverUnload;
    
    DEBUG_PRINT("Driver entry completed successfully");
    return STATUS_SUCCESS;
}

VOID
DriverUnload(
    _In_ PDRIVER_OBJECT DriverObject
)
{
    DEBUG_PRINT("DriverUnload called");
    
    if (DriverObject->DeviceObject != NULL) {
        CleanupDevice(DriverObject->DeviceObject);
    }
    
    DEBUG_PRINT("Driver unloaded");
}

NTSTATUS
DispatchCreate(
    _In_ PDEVICE_OBJECT DeviceObject,
    _In_ PIRP Irp
)
{
    DEBUG_PRINT("Device opened");
    
    Irp->IoStatus.Status = STATUS_SUCCESS;
    Irp->IoStatus.Information = 0;
    
    IoCompleteRequest(Irp, IO_NO_INCREMENT);
    return STATUS_SUCCESS;
}

NTSTATUS
DispatchClose(
    _In_ PDEVICE_OBJECT DeviceObject,
    _In_ PIRP Irp
)
{
    DEBUG_PRINT("Device closed");
    
    Irp->IoStatus.Status = STATUS_SUCCESS;
    Irp->IoStatus.Information = 0;
    
    IoCompleteRequest(Irp, IO_NO_INCREMENT);
    return STATUS_SUCCESS;
}

NTSTATUS
DispatchDeviceControl(
    _In_ PDEVICE_OBJECT DeviceObject,
    _In_ PIRP Irp
)
{
    NTSTATUS status = STATUS_SUCCESS;
    PIO_STACK_LOCATION irpStack = IoGetCurrentIrpStackLocation(Irp);
    ULONG ioControlCode = irpStack->Parameters.DeviceIoControl.IoControlCode;
    
    DEBUG_PRINT("IOCTL 0x%X received", ioControlCode);
    
    switch (ioControlCode) {
        case IOCTL_VIRTUALMIC_SEND_AUDIO:
            status = HandleSendAudio(DeviceObject, Irp);
            break;
            
        case IOCTL_VIRTUALMIC_SET_FORMAT:
            status = HandleSetFormat(DeviceObject, Irp);
            break;
            
        case IOCTL_VIRTUALMIC_GET_STATS:
            status = HandleGetStats(DeviceObject, Irp);
            break;
            
        case IOCTL_VIRTUALMIC_MUTE:
            status = HandleMute(DeviceObject, Irp);
            break;
            
        default:
            ERROR_PRINT("Unknown IOCTL: 0x%X", ioControlCode);
            status = STATUS_INVALID_DEVICE_REQUEST;
            break;
    }
    
    Irp->IoStatus.Status = status;
    if (status != STATUS_SUCCESS) {
        Irp->IoStatus.Information = 0;
    }
    
    IoCompleteRequest(Irp, IO_NO_INCREMENT);
    return status;
}

NTSTATUS
DispatchRead(
    _In_ PDEVICE_OBJECT DeviceObject,
    _In_ PIRP Irp
)
{
    DEBUG_PRINT("Read request received");
    
    // TODO: Implementar lectura de audio desde el buffer
    // Esto sería usado por WaveRT para obtener datos de audio
    
    Irp->IoStatus.Status = STATUS_SUCCESS;
    Irp->IoStatus.Information = 0;
    
    IoCompleteRequest(Irp, IO_NO_INCREMENT);
    return STATUS_SUCCESS;
}