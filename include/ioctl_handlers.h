#ifndef IOCTL_HANDLERS_H
#define IOCTL_HANDLERS_H

#include "virtual_mic.h"
#include "driver_core.h"

// Handlers para cada IOCTL
NTSTATUS HandleSendAudio(
    _In_ PDEVICE_OBJECT DeviceObject,
    _In_ PIRP Irp
);

NTSTATUS HandleSetFormat(
    _In_ PDEVICE_OBJECT DeviceObject,
    _In_ PIRP Irp
);

NTSTATUS HandleGetStats(
    _In_ PDEVICE_OBJECT DeviceObject,
    _In_ PIRP Irp
);

NTSTATUS HandleMute(
    _In_ PDEVICE_OBJECT DeviceObject,
    _In_ PIRP Irp
);

// Funciones auxiliares para validación
BOOLEAN ValidateAudioPacket(
    _In_ PVOID InputBuffer,
    _In_ ULONG InputBufferLength
);

BOOLEAN ValidateFormatRequest(
    _In_ PVOID InputBuffer,
    _In_ ULONG InputBufferLength
);

BOOLEAN ValidateStatsBuffer(
    _In_ PVOID OutputBuffer,
    _In_ ULONG OutputBufferLength
);

BOOLEAN ValidateMuteRequest(
    _In_ PVOID InputBuffer,
    _In_ ULONG InputBufferLength
);

#endif // IOCTL_HANDLERS_H