# 🎤 Virtual Microphone Kernel Driver

## ⚠️ ADVERTENCIA IMPORTANTE
Este es un **driver de kernel** para Windows. Los drivers de kernel pueden causar:
- 💀 Pantallas azules (BSOD)
- 🔒 Inestabilidad del sistema
- 💥 Pérdida de datos
- 🛡️ Problemas de seguridad

**ÚSELO BAJO SU PROPIA RESPONSABILIDAD Y SOLO EN ENTORNOS DE DESARROLLO/PRUEBA**

## 📋 Descripción
Driver de kernel de Windows que crea un dispositivo de micrófono virtual capaz de:
- Recibir audio desde aplicaciones user-space vía IOCTL
- Exponer el dispositivo al subsistema WASAPI de Windows
- Procesar audio en tiempo real con baja latencia

## 🏗️ Estructura del Proyecto (Refactorizada)

```
VirtualMicDriver/
├── .github/workflows/          # CI/CD con GitHub Actions
│   └── ci.yml
├── docs/                       # Documentación adicional
├── include/                    # Archivos de encabezado
│   ├── virtual_mic.h          # Definiciones principales del driver
│   ├── driver_core.h          # Núcleo del driver
│   ├── audio_processing.h     # Procesamiento de audio
│   ├── ioctl_handlers.h       # Manejadores IOCTL
│   └── common.h               # Utilidades comunes
├── scripts/                    # Scripts de automatización
│   ├── build.ps1              # Script de construcción
│   └── test.ps1               # Script de pruebas
├── src/                        # Código fuente modular
│   ├── main.c                 # Punto de entrada del driver
│   ├── driver/                # Módulo del núcleo del driver
│   │   └── driver_core.c
│   ├── audio/                 # Módulo de procesamiento de audio
│   │   └── audio_processing.c
│   ├── ioctl/                 # Módulo de manejadores IOCTL
│   │   └── ioctl_handlers.c
│   └── common/                # Módulo de utilidades comunes
│       └── common.c
├── tests/                      # Pruebas automatizadas
│   ├── test_audio_processing.c
│   └── test_ioctl_handlers.c
├── CMakeLists.txt             # Configuración de compilación
├── virtual_mic.inf            # Archivo INF para instalación
└── README.md                  # Este archivo
```

## 🔧 Requisitos de Desarrollo

### Software Necesario
1. **Windows 10/11 x64** con modo de prueba activado
2. **Visual Studio 2019/2022** con workload "Desktop development with C++"
3. **Windows Driver Kit (WDK)** versión 10.0.19041 o superior (OBLIGATORIO)
   - Descargar desde: https://docs.microsoft.com/en-us/windows-hardware/drivers/download-the-wdk
   - Instalar el WDK completo (incluye Windows SDK)
4. **CMake** 3.16 o superior
5. **PowerShell** 5.1 o superior
6. **Certificado de firma de código** (para producción)

### Instalación del WDK
```powershell
# Descargar el instalador del WDK desde:
# https://learn.microsoft.com/en-us/windows-hardware/drivers/download-the-wdk

# Ejecutar el instalador y seleccionar:
# - Windows Driver Kit (WDK)
# - Windows SDK
# - Visual Studio extension para drivers

# El WDK se instalará en:
# C:\Program Files (x86)\Windows Kits\10\

# El proyecto detectará automáticamente la versión instalada
```

### Activar Modo de Prueba
```cmd
# Como administrador
bcdedit /set testsigning on

# Verificar el estado
bcdedit

# Reiniciar el sistema para que los cambios surtan efecto
```

### Verificar Instalación
```powershell
# Verificar WDK instalado
Get-ChildItem "C:\Program Files (x86)\Windows Kits\10\Include"

# Verificar Visual Studio
Get-ChildItem "C:\Program Files\Microsoft Visual Studio"

# Verificar CMake
cmake --version
```

## 🚀 Compilación Automatizada

### Opción 1: Usando PowerShell Script (Recomendado)
```powershell
# Compilar en modo Debug
.\scripts\build.ps1

# Compilar en modo Release
.\scripts\build.ps1 -Configuration Release

# Limpiar y compilar con pruebas
.\scripts\build.ps1 -Clean -Test

# Ver ayuda
.\scripts\build.ps1 -Help
```

### Opción 2: Usando CMake directamente
```cmd
# Crear directorio de compilación
mkdir build
cd build

# Configurar
cmake .. -G "Visual Studio 17 2022" -A x64

# Compilar
cmake --build . --config Debug
```

### Opción 3: Usando Visual Studio con WDK
1. Abrir Visual Studio con WDK instalado
2. Crear nuevo proyecto "Kernel Mode Driver (KMDF)"
3. Copiar el código de los módulos `src/`
4. Configurar proyecto para x64
5. Compilar en modo Debug/Release

## 🧪 Pruebas Automatizadas

### Ejecutar todas las pruebas
```powershell
.\scripts\test.ps1
```

### Ejecutar pruebas específicas
```powershell
# Solo pruebas de audio
.\scripts\test.ps1 -TestFilter *audio*

# Con salida detallada
.\scripts\test.ps1 -Verbose

# Con análisis de cobertura
.\scripts\test.ps1 -Coverage
```

### Tipos de pruebas disponibles
- **test_audio_processing.c**: Pruebas del módulo de procesamiento de audio
- **test_ioctl_handlers.c**: Pruebas de los manejadores IOCTL

## 📦 Instalación

### Paso 1: Preparar archivos
1. Copiar `virtual_mic.sys` a `C:\Windows\System32\drivers\`
2. Tener `virtual_mic.inf` listo

### Paso 2: Instalar driver
```cmd
# Como administrador
cd VirtualMicDriver

# Crear servicio
sc create VirtualMic type= kernel binPath= C:\Windows\System32\drivers\virtual_mic.sys start= demand

# Iniciar driver
sc start VirtualMic

# Verificar estado
sc query VirtualMic
```

### Paso 3: Desinstalar (si es necesario)
```cmd
# Como administrador
sc stop VirtualMic
sc delete VirtualMic
del C:\Windows\System32\drivers\virtual_mic.sys
```

## 🔍 Funcionalidades Implementadas

### ✅ Completado
- [x] **Estructura modular del driver**
- [x] **Sistema de compilación con CMake**
- [x] **CI/CD con GitHub Actions**
- [x] **Pruebas automatizadas**
- [x] **Scripts de build y test en PowerShell**
- [x] **DriverEntry y DriverUnload modulares**
- [x] **Handlers IRP (Create, Close, DeviceControl, Read)**
- [x] **IOCTL básicos:**
  - `IOCTL_VIRTUALMIC_SEND_AUDIO` - Enviar audio al driver
  - `IOCTL_VIRTUALMIC_SET_FORMAT` - Configurar formato de audio
  - `IOCTL_VIRTUALMIC_GET_STATS` - Obtener estadísticas
  - `IOCTL_VIRTUALMIC_MUTE` - Silenciar/desenmudecer
- [x] **Buffer circular completo para audio**
- [x] **Sincronización con spinlocks**
- [x] **Validación de parámetros**
- [x] **Sistema de logging mejorado**

### 🔄 En Desarrollo
- [ ] Integración con WaveRT (WDM Audio)
- [ ] Implementación completa de IPortWaveRT
- [ ] IMiniportWaveRT y IMiniportWaveRTInputStream
- [ ] Registro con el subsistema WASAPI
- [ ] Control de volumen real
- [ ] Manejo de múltiples aplicaciones consumidoras

### ❌ Pendiente (Crítico)
- [ ] Validación de permisos y seguridad mejorada
- [ ] Manejo de errores robusto
- [ ] Logging detallado con ETW
- [ ] Testing exhaustivo en hardware real
- [ ] Certificación WHQL

## 🧪 Calidad del Código

### Análisis Estático
- ✅ Análisis con CodeQL en CI/CD
- ✅ Verificación de estilo de código
- ✅ Detección de vulnerabilidades comunes
- ✅ Análisis de dependencias

### Pruebas
- ✅ Pruebas unitarias para módulos individuales
- ✅ Pruebas de integración
- ✅ Pruebas de estrés para buffer circular
- ✅ Simulación de condiciones de error

### Documentación
- ✅ Documentación inline en código
- ✅ README con instrucciones completas
- ✅ Diagramas de arquitectura
- ✅ Guías de contribución

## ⚠️ Limitaciones Conocidas

### Técnicas
1. **NO implementa WaveRT completamente**: Este driver es una base educativa
2. **NO tiene interfaz WASAPI completa**: Las aplicaciones no pueden detectarlo como micrófono real
3. **Buffer mejorado**: Implementa buffer circular completo con sincronización
4. **Sin WaveRT Port**: No integra completamente con el subsistema de audio de Windows
5. **Sin certificación**: Requerirá firma digital y certificación WHQL para producción

### De Seguridad
1. **Validación básica de permisos**: Implementa validación de parámetros pero necesita mejora
2. **Protección de buffer mejorada**: Implementa validación de límites pero puede mejorar
3. **Sin ACL completo**: Falta control de acceso a procesos por completo
4. **Sin firma**: No funcionará en producción sin certificado EV

## 🎯 Para Hacerlo Funcional Realmente

### Requisitos Adicionales
1. **Implementar WaveRT completo**:
   ```c
   // Necesita implementar:
   IPortWaveRT* port;
   IMiniportWaveRT* miniport;
   IMiniportWaveRTInputStream* stream;
   ```

2. **Registrar con PnP Manager**:
   ```c
   // Registrar como dispositivo de audio
   IoRegisterDeviceInterface()
   ```

3. **Integrar con PortCls**:
   ```c
   // Usar PortCls para audio
   PortClsCreatePortDriver()
   ```

4. **Implementar descriptor WaveRT**:
   ```c
   // Descriptor de dispositivo WaveRT
   PPCFILTER_DESCRIPTOR FilterDescriptor;
   ```

## 📚 Recursos y Referencias

### Documentación Microsoft
- [Windows Driver Kit (WDK)](https://docs.microsoft.com/en-us/windows-hardware/drivers/)
- [WaveRT Port Driver](https://docs.microsoft.com/en-us/windows-hardware/drivers/audio/wavert-port-driver)
- [PortCls System Driver](https://docs.microsoft.com/en-us/windows-hardware/drivers/audio/portcls-system-driver)
- [Audio Drivers Overview](https://docs.microsoft.com/en-us/windows-hardware/drivers/audio/audio-drivers)

### Ejemplos de Referencia
- [SysVAD Sample Driver](https://github.com/microsoft/Windows-driver-samples/tree/main/audio/sysvad)
- [MSVAD Sample](https://github.com/microsoft/Windows-driver-samples/tree/main/audio/msvad)
- [Simple Audio Sample](https://github.com/microsoft/Windows-driver-samples/tree/main/audio/simpleaudiosample)

### Software de Referencia
- [VB-Cable Virtual Audio](https://vb-audio.com/Cable/)
- [VoiceMeeter](https://vb-audio.com/Voicemeeter/)
- [Virtual Audio Cable](https://vac.muzychenko.net/)

## 🤝 Contribuir

### Guía de Contribución
1. Fork el proyecto
2. Crear una rama para tu feature (`git checkout -b feature/AmazingFeature`)
3. Commit tus cambios (`git commit -m 'Add some AmazingFeature'`)
4. Push a la rama (`git push origin feature/AmazingFeature`)
5. Abrir un Pull Request

### Estándares de Código
- Seguir el estilo de código del proyecto
- Agregar pruebas para nuevas funcionalidades
- Actualizar documentación
- Ejecutar pruebas antes de hacer push

## 📝 Licencia

Este proyecto es para fines educativos. Consulta el archivo LICENSE para más detalles.

## 🚨 ADVERTENCIA FINAL

**Este driver es UNA BASE EDUCATIVA MODULAR** y **NO ES COMPLETAMENTE FUNCIONAL** como micrófono virtual real. 
Para crear un micrófono virtual funcional necesitas:

1. **Conocimientos avanzados** de drivers de kernel
2. **Experiencia con WDK y WaveRT**
3. **Certificado de firma de código EV**
4. **Testing en múltiples sistemas**
5. **Aprobación de Microsoft (HLK testing)**

**⚠️ NO INTENTES INSTALARLO EN PRODUCCIÓN ⚠️**

Para resultados inmediatos, considera:
- VB-Cable (comercial, $30)
- VoiceMeeter (gratis/comercial)
- Virtual Audio Cable (comercial)

---

**Este proyecto demuestra la complejidad de crear drivers de kernel reales y proporciona una base modular para aprendizaje y desarrollo.**
