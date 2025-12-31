#include "ioctl_handlers.h"
#include "audio_processing.h"
#include "common.h"

NTSTATUS HandleSendAudio(
    _In_ PDEVICE_OBJECT DeviceObject,
    _In_ PIRP Irp
)
{
    NTSTATUS status;
    PIO_STACK_LOCATION irpStack = IoGetCurrentIrpStackLocation(Irp);
    PAUDIO_BUFFER_PACKET packet;
    ULONG inputBufferLength = irpStack->Parameters.DeviceIoControl.InputBufferLength;
    PDEVICE_EXTENSION deviceExtension = (PDEVICE_EXTENSION)DeviceObject->DeviceExtension;
    PVOID inputBuffer = Irp->AssociatedIrp.SystemBuffer;
    ULONG bytesWritten;
    
    DEBUG_PRINT("HandleSendAudio called");
    
    // Validar buffer de entrada
    if (!ValidateAudioPacket(inputBuffer, inputBufferLength)) {
        return STATUS_INVALID_PARAMETER;
    }
    
    packet = (PAUDIO_BUFFER_PACKET)inputBuffer;
    
    // Validar que el driver esté inicializado
    if (!deviceExtension->IsInitialized) {
        ERROR_PRINT("Device not initialized");
        return STATUS_DEVICE_NOT_READY;
    }
    
    // Escribir datos en el buffer de audio
    status = WriteAudioToBuffer(deviceExtension, 
                               packet->Data, 
                               packet->DataLength, 
                               &bytesWritten);
    
    if (NT_SUCCESS(status)) {
        Irp->IoStatus.Information = bytesWritten;
        DEBUG_PRINT("Successfully written %lu bytes to buffer", bytesWritten);
    } else {
        ERROR_PRINT("Failed to write audio to buffer: 0x%X", status);
    }
    
    return status;
}

NTSTATUS HandleSetFormat(
    _In_ PDEVICE_OBJECT DeviceObject,
    _In_ PIRP Irp
)
{
    PIO_STACK_LOCATION irpStack = IoGetCurrentIrpStackLocation(Irp);
    PSET_FORMAT_REQUEST formatRequest;
    ULONG inputBufferLength = irpStack->Parameters.DeviceIoControl.InputBufferLength;
    PDEVICE_EXTENSION deviceExtension = (PDEVICE_EXTENSION)DeviceObject->DeviceExtension;
    
    DEBUG_PRINT("HandleSetFormat called");
    
    // Validar buffer de entrada
    if (!ValidateFormatRequest(Irp->AssociatedIrp.SystemBuffer, inputBufferLength)) {
        ERROR_PRINT("Invalid format request");
        return STATUS_INVALID_PARAMETER;
    }
    
    formatRequest = (PSET_FORMAT_REQUEST)Irp->AssociatedIrp.SystemBuffer;
    
    // Establecer formato de audio
    return SetAudioFormat(deviceExtension,
                         formatRequest->SampleRate,
                         formatRequest->Channels,
                         formatRequest->BitsPerSample);
}

NTSTATUS HandleGetStats(
    _In_ PDEVICE_OBJECT DeviceObject,
    _In_ PIRP Irp
)
{
    PIO_STACK_LOCATION irpStack = IoGetCurrentIrpStackLocation(Irp);
    ULONG outputBufferLength = irpStack->Parameters.DeviceIoControl.OutputBufferLength;
    PDRIVER_STATS stats;
    PDEVICE_EXTENSION deviceExtension = (PDEVICE_EXTENSION)DeviceObject->DeviceExtension;
    AUDIO_FORMAT currentFormat;
    
    DEBUG_PRINT("HandleGetStats called");
    
    // Validar buffer de salida
    if (!ValidateStatsBuffer(Irp->AssociatedIrp.SystemBuffer, outputBufferLength)) {
        ERROR_PRINT("Invalid stats buffer");
        return STATUS_INVALID_PARAMETER;
    }
    
    stats = (PDRIVER_STATS)Irp->AssociatedIrp.SystemBuffer;
    RtlZeroMemory(stats, sizeof(DRIVER_STATS));
    
    // Llenar estadísticas básicas
    stats->IsActive = deviceExtension->IsInitialized;
    stats->SamplesProcessed = 0; // TODO: Implementar contador de muestras
    // Calculate buffer usage as percentage (0-100)
    stats->BufferUsage = (GetBufferUsedSpace(deviceExtension) * 100) / deviceExtension->BufferSize;
    stats->Underruns = 0; // TODO: Implementar contador de underruns
    stats->Overruns = 0;  // TODO: Implementar contador de overruns
    stats->UptimeMs = 0;  // TODO: Implementar contador de tiempo activo
    
    // Obtener formato actual
    GetCurrentAudioFormat(deviceExtension, &currentFormat);
    RtlCopyMemory(&stats->CurrentFormat, &currentFormat, sizeof(AUDIO_FORMAT));
    
    Irp->IoStatus.Information = sizeof(DRIVER_STATS);
    DEBUG_PRINT("Stats retrieved successfully");
    
    return STATUS_SUCCESS;
}

NTSTATUS HandleMute(
    _In_ PDEVICE_OBJECT DeviceObject,
    _In_ PIRP Irp
)
{
    PIO_STACK_LOCATION irpStack = IoGetCurrentIrpStackLocation(Irp);
    ULONG inputBufferLength = irpStack->Parameters.DeviceIoControl.InputBufferLength;
    PBOOLEAN muteState;
    
    DEBUG_PRINT("HandleMute called");
    
    // Validar buffer de entrada
    if (!ValidateMuteRequest(Irp->AssociatedIrp.SystemBuffer, inputBufferLength)) {
        ERROR_PRINT("Invalid mute request");
        return STATUS_INVALID_PARAMETER;
    }
    
    muteState = (PBOOLEAN)Irp->AssociatedIrp.SystemBuffer;
    
    DEBUG_PRINT("Mute state: %s", *muteState ? "TRUE" : "FALSE");
    
    // TODO: Implementar lógica de mute real
    // Por ahora, solo registramos el estado
    
    return STATUS_SUCCESS;
}

BOOLEAN ValidateAudioPacket(
    _In_ PVOID InputBuffer,
    _In_ ULONG InputBufferLength
)
{
    PAUDIO_BUFFER_PACKET packet;
    
    if (InputBuffer == NULL || InputBufferLength == 0) {
        return FALSE;
    }
    
    if (InputBufferLength < sizeof(AUDIO_BUFFER_PACKET)) {
        return FALSE;
    }
    
    packet = (PAUDIO_BUFFER_PACKET)InputBuffer;
    
    // Validar tamaño de datos
    if (packet->DataLength > InputBufferLength - sizeof(AUDIO_BUFFER_PACKET)) {
        return FALSE;
    }
    
    return TRUE;
}

BOOLEAN ValidateFormatRequest(
    _In_ PVOID InputBuffer,
    _In_ ULONG InputBufferLength
)
{
    PSET_FORMAT_REQUEST formatRequest;
    
    if (InputBuffer == NULL || InputBufferLength < sizeof(SET_FORMAT_REQUEST)) {
        return FALSE;
    }
    
    formatRequest = (PSET_FORMAT_REQUEST)InputBuffer;
    
    // Validar formato
    if (!IS_VALID_SAMPLE_RATE(formatRequest->SampleRate)) {
        return FALSE;
    }
    
    if (!IS_VALID_CHANNELS(formatRequest->Channels)) {
        return FALSE;
    }
    
    if (!IS_VALID_BITS_PER_SAMPLE(formatRequest->BitsPerSample)) {
        return FALSE;
    }
    
    return TRUE;
}

BOOLEAN ValidateStatsBuffer(
    _In_ PVOID OutputBuffer,
    _In_ ULONG OutputBufferLength
)
{
    if (OutputBuffer == NULL || OutputBufferLength < sizeof(DRIVER_STATS)) {
        return FALSE;
    }
    
    return TRUE;
}

BOOLEAN ValidateMuteRequest(
    _In_ PVOID InputBuffer,
    _In_ ULONG InputBufferLength
)
{
    if (InputBuffer == NULL || InputBufferLength < sizeof(BOOLEAN)) {
        return FALSE;
    }
    
    return TRUE;
}
