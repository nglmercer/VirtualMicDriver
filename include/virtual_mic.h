#ifndef VIRTUAL_MIC_H
#define VIRTUAL_MIC_H

#include <ntddk.h>
#include <ntstrsafe.h>

// IOCTL Codes
#define IOCTL_VIRTUALMIC_SEND_AUDIO     CTL_CODE(FILE_DEVICE_UNKNOWN, 0x800, METHOD_BUFFERED, FILE_ANY_ACCESS)
#define IOCTL_VIRTUALMIC_SET_FORMAT     CTL_CODE(FILE_DEVICE_UNKNOWN, 0x801, METHOD_BUFFERED, FILE_ANY_ACCESS)
#define IOCTL_VIRTUALMIC_GET_STATS      CTL_CODE(FILE_DEVICE_UNKNOWN, 0x802, METHOD_BUFFERED, FILE_READ_ACCESS)
#define IOCTL_VIRTUALMIC_MUTE           CTL_CODE(FILE_DEVICE_UNKNOWN, 0x803, METHOD_BUFFERED, FILE_WRITE_ACCESS)

// Estructuras de datos
typedef struct _AUDIO_FORMAT {
    ULONG SampleRate;
    USHORT Channels;
    USHORT BitsPerSample;
    USHORT BlockAlign;
    ULONG BytesPerSecond;
    USHORT FormatTag;
} AUDIO_FORMAT, *PAUDIO_FORMAT;

typedef struct _AUDIO_BUFFER_PACKET {
    ULONG64 Timestamp;
    ULONG DataLength;
    UCHAR Data[1]; // Flexible array member
} AUDIO_BUFFER_PACKET, *PAUDIO_BUFFER_PACKET;

typedef struct _SET_FORMAT_REQUEST {
    ULONG SampleRate;
    USHORT Channels;
    USHORT BitsPerSample;
} SET_FORMAT_REQUEST, *PSET_FORMAT_REQUEST;

typedef struct _DRIVER_STATS {
    BOOLEAN IsActive;
    ULONG64 SamplesProcessed;
    ULONG BufferUsage; // Percentage as integer (0-100)
    ULONG Underruns;
    ULONG Overruns;
    AUDIO_FORMAT CurrentFormat;
    ULONG64 UptimeMs;
} DRIVER_STATS, *PDRIVER_STATS;

// Configuración por defecto
#define DEFAULT_BUFFER_SIZE     8192
#define DEFAULT_SAMPLE_RATE     48000
#define DEFAULT_CHANNELS        2
#define DEFAULT_BITS_PER_SAMPLE 16
#define POOL_TAG                'VMic'

// Declaraciones de funciones del driver
DRIVER_INITIALIZE DriverEntry;
DRIVER_UNLOAD DriverUnload;

// Funciones de dispatch
DRIVER_DISPATCH DispatchCreate;
DRIVER_DISPATCH DispatchClose;
DRIVER_DISPATCH DispatchDeviceControl;
DRIVER_DISPATCH DispatchRead;

#endif // VIRTUAL_MIC_H
