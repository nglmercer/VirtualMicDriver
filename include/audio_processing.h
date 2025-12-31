#ifndef AUDIO_PROCESSING_H
#define AUDIO_PROCESSING_H

#include "virtual_mic.h"
#include "driver_core.h"

// Funciones de procesamiento de audio
NTSTATUS WriteAudioToBuffer(
    _In_ PDEVICE_EXTENSION DeviceExtension,
    _In_ PVOID AudioData,
    _In_ ULONG DataLength,
    _Out_ PULONG BytesWritten
);

NTSTATUS ReadAudioFromBuffer(
    _In_ PDEVICE_EXTENSION DeviceExtension,
    _Out_ PVOID AudioData,
    _In_ ULONG MaxLength,
    _Out_ PULONG BytesRead
);

NTSTATUS SetAudioFormat(
    _In_ PDEVICE_EXTENSION DeviceExtension,
    _In_ ULONG SampleRate,
    _In_ USHORT Channels,
    _In_ USHORT BitsPerSample
);

VOID GetCurrentAudioFormat(
    _In_ PDEVICE_EXTENSION DeviceExtension,
    _Out_ PAUDIO_FORMAT Format
);

// Funciones de utilidad para el buffer circular
ULONG GetBufferFreeSpace(
    _In_ PDEVICE_EXTENSION DeviceExtension
);

ULONG GetBufferUsedSpace(
    _In_ PDEVICE_EXTENSION DeviceExtension
);

BOOLEAN IsBufferEmpty(
    _In_ PDEVICE_EXTENSION DeviceExtension
);

BOOLEAN IsBufferFull(
    _In_ PDEVICE_EXTENSION DeviceExtension
);

#endif // AUDIO_PROCESSING_H