#include "audio_processing.h"
#include "common.h"

NTSTATUS WriteAudioToBuffer(
    _In_ PDEVICE_EXTENSION DeviceExtension,
    _In_ PVOID AudioData,
    _In_ ULONG DataLength,
    _Out_ PULONG BytesWritten
)
{
    KIRQL oldIrql;
    ULONG freeSpace;
    ULONG bytesToCopy;
    ULONG firstChunk;
    ULONG secondChunk;
    
    if (AudioData == NULL || DataLength == 0 || BytesWritten == NULL) {
        return STATUS_INVALID_PARAMETER;
    }
    
    if (!DeviceExtension->IsInitialized || DeviceExtension->AudioBuffer == NULL) {
        return STATUS_DEVICE_NOT_READY;
    }
    
    KeAcquireSpinLock(&DeviceExtension->BufferLock, &oldIrql);
    
    // Calcular espacio libre
    freeSpace = GetBufferFreeSpace(DeviceExtension);
    bytesToCopy = min(DataLength, freeSpace);
    
    if (bytesToCopy == 0) {
        KeReleaseSpinLock(&DeviceExtension->BufferLock, oldIrql);
        *BytesWritten = 0;
        return STATUS_BUFFER_TOO_SMALL;
    }
    
    // Calcular chunks para escritura circular
    firstChunk = min(bytesToCopy, 
                     DeviceExtension->BufferSize - DeviceExtension->WritePosition);
    secondChunk = bytesToCopy - firstChunk;
    
    // Copiar primer chunk
    if (firstChunk > 0) {
        RtlCopyMemory((PUCHAR)DeviceExtension->AudioBuffer + DeviceExtension->WritePosition,
                      AudioData, firstChunk);
    }
    
    // Copiar segundo chunk (si hay wrap-around)
    if (secondChunk > 0) {
        RtlCopyMemory(DeviceExtension->AudioBuffer,
                      (PUCHAR)AudioData + firstChunk, secondChunk);
        DeviceExtension->WritePosition = secondChunk;
    } else {
        DeviceExtension->WritePosition = (DeviceExtension->WritePosition + firstChunk) % 
                                         DeviceExtension->BufferSize;
    }
    
    KeReleaseSpinLock(&DeviceExtension->BufferLock, oldIrql);
    
    *BytesWritten = bytesToCopy;
    DEBUG_PRINT("Written %lu bytes to audio buffer", bytesToCopy);
    
    return STATUS_SUCCESS;
}

NTSTATUS ReadAudioFromBuffer(
    _In_ PDEVICE_EXTENSION DeviceExtension,
    _Out_ PVOID AudioData,
    _In_ ULONG MaxLength,
    _Out_ PULONG BytesRead
)
{
    KIRQL oldIrql;
    ULONG usedSpace;
    ULONG bytesToCopy;
    ULONG firstChunk;
    ULONG secondChunk;
    
    if (AudioData == NULL || MaxLength == 0 || BytesRead == NULL) {
        return STATUS_INVALID_PARAMETER;
    }
    
    if (!DeviceExtension->IsInitialized || DeviceExtension->AudioBuffer == NULL) {
        return STATUS_DEVICE_NOT_READY;
    }
    
    KeAcquireSpinLock(&DeviceExtension->BufferLock, &oldIrql);
    
    // Calcular espacio usado
    usedSpace = GetBufferUsedSpace(DeviceExtension);
    bytesToCopy = min(MaxLength, usedSpace);
    
    if (bytesToCopy == 0) {
        KeReleaseSpinLock(&DeviceExtension->BufferLock, oldIrql);
        *BytesRead = 0;
        return STATUS_SUCCESS;
    }
    
    // Calcular chunks para lectura circular
    firstChunk = min(bytesToCopy, 
                     DeviceExtension->BufferSize - DeviceExtension->ReadPosition);
    secondChunk = bytesToCopy - firstChunk;
    
    // Copiar primer chunk
    if (firstChunk > 0) {
        RtlCopyMemory(AudioData,
                      (PUCHAR)DeviceExtension->AudioBuffer + DeviceExtension->ReadPosition,
                      firstChunk);
    }
    
    // Copiar segundo chunk (si hay wrap-around)
    if (secondChunk > 0) {
        RtlCopyMemory((PUCHAR)AudioData + firstChunk,
                      DeviceExtension->AudioBuffer, secondChunk);
        DeviceExtension->ReadPosition = secondChunk;
    } else {
        DeviceExtension->ReadPosition = (DeviceExtension->ReadPosition + firstChunk) % 
                                        DeviceExtension->BufferSize;
    }
    
    KeReleaseSpinLock(&DeviceExtension->BufferLock, oldIrql);
    
    *BytesRead = bytesToCopy;
    DEBUG_PRINT("Read %lu bytes from audio buffer", bytesToCopy);
    
    return STATUS_SUCCESS;
}

NTSTATUS SetAudioFormat(
    _In_ PDEVICE_EXTENSION DeviceExtension,
    _In_ ULONG SampleRate,
    _In_ USHORT Channels,
    _In_ USHORT BitsPerSample
)
{
    if (!IS_VALID_SAMPLE_RATE(SampleRate)) {
        return STATUS_INVALID_PARAMETER;
    }
    
    if (!IS_VALID_CHANNELS(Channels)) {
        return STATUS_INVALID_PARAMETER;
    }
    
    if (!IS_VALID_BITS_PER_SAMPLE(BitsPerSample)) {
        return STATUS_INVALID_PARAMETER;
    }
    
    // TODO: Implementar cambio de formato dinámico
    // Por ahora solo validamos los parámetros
    
    DEBUG_PRINT("Audio format set - SampleRate: %lu, Channels: %u, Bits: %u",
                SampleRate, Channels, BitsPerSample);
    
    return STATUS_SUCCESS;
}

VOID GetCurrentAudioFormat(
    _In_ PDEVICE_EXTENSION DeviceExtension,
    _Out_ PAUDIO_FORMAT Format
)
{
    if (Format == NULL) {
        return;
    }
    
    // Formato por defecto
    Format->SampleRate = DEFAULT_SAMPLE_RATE;
    Format->Channels = DEFAULT_CHANNELS;
    Format->BitsPerSample = DEFAULT_BITS_PER_SAMPLE;
    Format->BlockAlign = (DEFAULT_CHANNELS * DEFAULT_BITS_PER_SAMPLE) / 8;
    Format->BytesPerSecond = DEFAULT_SAMPLE_RATE * Format->BlockAlign;
    Format->FormatTag = 1; // WAVE_FORMAT_PCM
}

ULONG GetBufferFreeSpace(
    _In_ PDEVICE_EXTENSION DeviceExtension
)
{
    ULONG usedSpace;
    ULONG freeSpace;
    
    if (DeviceExtension->WritePosition >= DeviceExtension->ReadPosition) {
        usedSpace = DeviceExtension->WritePosition - DeviceExtension->ReadPosition;
    } else {
        usedSpace = DeviceExtension->BufferSize - DeviceExtension->ReadPosition + 
                    DeviceExtension->WritePosition;
    }
    
    freeSpace = DeviceExtension->BufferSize - usedSpace - 1; // Dejar 1 byte de espacio
    return freeSpace;
}

ULONG GetBufferUsedSpace(
    _In_ PDEVICE_EXTENSION DeviceExtension
)
{
    ULONG usedSpace;
    
    if (DeviceExtension->WritePosition >= DeviceExtension->ReadPosition) {
        usedSpace = DeviceExtension->WritePosition - DeviceExtension->ReadPosition;
    } else {
        usedSpace = DeviceExtension->BufferSize - DeviceExtension->ReadPosition + 
                    DeviceExtension->WritePosition;
    }
    
    return usedSpace;
}

BOOLEAN IsBufferEmpty(
    _In_ PDEVICE_EXTENSION DeviceExtension
)
{
    return DeviceExtension->WritePosition == DeviceExtension->ReadPosition;
}

BOOLEAN IsBufferFull(
    _In_ PDEVICE_EXTENSION DeviceExtension
)
{
    return GetBufferFreeSpace(DeviceExtension) == 0;
}