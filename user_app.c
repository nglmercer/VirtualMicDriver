#include <windows.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// IOCTL Codes (mismos que en el driver)
#define IOCTL_VIRTUALMIC_SEND_AUDIO     CTL_CODE(FILE_DEVICE_UNKNOWN, 0x800, METHOD_BUFFERED, FILE_ANY_ACCESS)
#define IOCTL_VIRTUALMIC_SET_FORMAT     CTL_CODE(FILE_DEVICE_UNKNOWN, 0x801, METHOD_BUFFERED, FILE_ANY_ACCESS)
#define IOCTL_VIRTUALMIC_GET_STATS      CTL_CODE(FILE_DEVICE_UNKNOWN, 0x802, METHOD_BUFFERED, FILE_READ_ACCESS)
#define IOCTL_VIRTUALMIC_MUTE           CTL_CODE(FILE_DEVICE_UNKNOWN, 0x803, METHOD_BUFFERED, FILE_WRITE_ACCESS)

// Estructuras de datos (mismas que en el driver)
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
    FLOAT BufferUsage;
    ULONG Underruns;
    ULONG Overruns;
    AUDIO_FORMAT CurrentFormat;
    ULONG64 UptimeMs;
} DRIVER_STATS, *PDRIVER_STATS;

// Funciones de utilidad
HANDLE OpenVirtualMicDevice();
BOOL SendAudioToDriver(HANDLE hDevice, const BYTE* audioData, ULONG dataLength);
BOOL SetAudioFormat(HANDLE hDevice, ULONG sampleRate, USHORT channels, USHORT bitsPerSample);
BOOL GetDriverStats(HANDLE hDevice, PDRIVER_STATS stats);
BOOL SetMuteState(HANDLE hDevice, BOOL mute);
void PrintStats(const DRIVER_STATS* stats);
ULONG64 GetTimestamp();

int main() {
    HANDLE hDevice;
    DRIVER_STATS stats;
    BYTE testAudio[1024];
    
    printf("=== Aplicacion de Usuario para Virtual Microphone ===\n");
    printf("Conectando con el driver de kernel...\n\n");
    
    // Abrir dispositivo
    hDevice = OpenVirtualMicDevice();
    if (hDevice == INVALID_HANDLE_VALUE) {
        printf("❌ Error: No se pudo abrir el dispositivo virtual_mic\n");
        printf("   Asegurate de que el driver esté instalado y funcionando\n");
        printf("   Ejecuta install.bat como administrador primero\n");
        return 1;
    }
    
    printf("✅ Dispositivo abierto exitosamente\n");
    
    // Obtener estadísticas del driver
    if (GetDriverStats(hDevice, &stats)) {
        printf("📊 Estadísticas del driver:\n");
        PrintStats(&stats);
    }
    
    // Configurar formato de audio
    printf("\n🔧 Configurando formato de audio...\n");
    if (SetAudioFormat(hDevice, 48000, 2, 16)) {
        printf("✅ Formato configurado: 48kHz, 16-bit, Stereo\n");
    } else {
        printf("❌ Error al configurar formato\n");
    }
    
    // Generar audio de prueba (onda sinusoidal)
    printf("\n🎵 Generando audio de prueba...\n");
    for (int i = 0; i < sizeof(testAudio); i += 2) {
        // Generar onda sinusoidal a 440Hz
        double time = (double)i / (48000 * 2); // 48kHz, 16-bit
        double sample = sin(2 * M_PI * 440 * time) * 0.5; // 440Hz, amplitud 0.5
        short sample16 = (short)(sample * 32767);
        
        testAudio[i] = sample16 & 0xFF;
        testAudio[i + 1] = (sample16 >> 8) & 0xFF;
    }
    
    // Enviar audio al driver
    printf("📤 Enviando audio al driver...\n");
    if (SendAudioToDriver(hDevice, testAudio, sizeof(testAudio))) {
        printf("✅ Audio enviado exitosamente (%zu bytes)\n", sizeof(testAudio));
    } else {
        printf("❌ Error al enviar audio\n");
    }
    
    // Obtener estadísticas actualizadas
    printf("\n📊 Estadísticas actualizadas:\n");
    if (GetDriverStats(hDevice, &stats)) {
        PrintStats(&stats);
    }
    
    // Prueba de mute
    printf("\n🔇 Probando función de mute...\n");
    if (SetMuteState(hDevice, TRUE)) {
        printf("✅ Micrófono silenciado\n");
    }
    
    if (SetMuteState(hDevice, FALSE)) {
        printf("✅ Micrófono des-silenciado\n");
    }
    
    // Cerrar dispositivo
    CloseHandle(hDevice);
    printf("\n✅ Dispositivo cerrado\n");
    printf("👋 Aplicación finalizada\n");
    
    return 0;
}

HANDLE OpenVirtualMicDevice() {
    return CreateFile(
        L"\\\\.\\VirtualMicrophone",     // Nombre del dispositivo
        GENERIC_READ | GENERIC_WRITE,    // Acceso read/write
        0,                               // No compartir
        NULL,                            // Seguridad por defecto
        OPEN_EXISTING,                   // Abrir existente
        0,                               // Atributos por defecto
        NULL                             // No template file
    );
}

BOOL SendAudioToDriver(HANDLE hDevice, const BYTE* audioData, ULONG dataLength) {
    AUDIO_BUFFER_PACKET* packet;
    ULONG packetSize = sizeof(AUDIO_BUFFER_PACKET) + dataLength - 1;
    DWORD bytesReturned;
    BOOL result;
    
    // Asignar memoria para el paquete
    packet = (AUDIO_BUFFER_PACKET*)malloc(packetSize);
    if (packet == NULL) {
        printf("❌ Error: No se pudo asignar memoria para el paquete\n");
        return FALSE;
    }
    
    // Llenar el paquete
    packet->Timestamp = GetTimestamp();
    packet->DataLength = dataLength;
    memcpy(packet->Data, audioData, dataLength);
    
    // Enviar al driver
    result = DeviceIoControl(
        hDevice,
        IOCTL_VIRTUALMIC_SEND_AUDIO,
        packet,
        packetSize,
        NULL,
        0,
        &bytesReturned,
        NULL
    );
    
    free(packet);
    
    if (!result) {
        printf("❌ Error en DeviceIoControl: %lu\n", GetLastError());
    }
    
    return result;
}

BOOL SetAudioFormat(HANDLE hDevice, ULONG sampleRate, USHORT channels, USHORT bitsPerSample) {
    SET_FORMAT_REQUEST formatRequest;
    DWORD bytesReturned;
    
    formatRequest.SampleRate = sampleRate;
    formatRequest.Channels = channels;
    formatRequest.BitsPerSample = bitsPerSample;
    
    return DeviceIoControl(
        hDevice,
        IOCTL_VIRTUALMIC_SET_FORMAT,
        &formatRequest,
        sizeof(formatRequest),
        NULL,
        0,
        &bytesReturned,
        NULL
    );
}

BOOL GetDriverStats(HANDLE hDevice, PDRIVER_STATS stats) {
    DWORD bytesReturned;
    
    return DeviceIoControl(
        hDevice,
        IOCTL_VIRTUALMIC_GET_STATS,
        NULL,
        0,
        stats,
        sizeof(DRIVER_STATS),
        &bytesReturned,
        NULL
    );
}

BOOL SetMuteState(HANDLE hDevice, BOOL mute) {
    DWORD bytesReturned;
    
    return DeviceIoControl(
        hDevice,
        IOCTL_VIRTUALMIC_MUTE,
        &mute,
        sizeof(mute),
        NULL,
        0,
        &bytesReturned,
        NULL
    );
}

void PrintStats(const DRIVER_STATS* stats) {
    printf("   Estado: %s\n", stats->IsActive ? "Activo" : "Inactivo");
    printf("   Muestras procesadas: %llu\n", stats->SamplesProcessed);
    printf("   Uso del buffer: %.2f%%\n", stats->BufferUsage * 100);
    printf("   Underruns: %lu\n", stats->Underruns);
    printf("   Overruns: %lu\n", stats->Overruns);
    printf("   Formato actual: %lu Hz, %u canales, %u bits\n",
           stats->CurrentFormat.SampleRate,
           stats->CurrentFormat.Channels,
           stats->CurrentFormat.BitsPerSample);
    printf("   Tiempo activo: %llu ms\n", stats->UptimeMs);
}

ULONG64 GetTimestamp() {
    FILETIME ft;
    GetSystemTimeAsFileTime(&ft);
    return ((ULONG64)ft.dwHighDateTime << 32) | ft.dwLowDateTime;
}