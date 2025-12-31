# CMakeLists.txt para Virtual Microphone Driver - Versión Modular
# NOTA: Este es un archivo de configuración para compilar drivers Windows reales.
# Se necesita Windows Driver Kit (WDK) y Visual Studio con soporte para drivers.

cmake_minimum_required(VERSION 3.16)
project(VirtualMicDriver C)

# Configuración del driver
set(DRIVER_NAME "virtual_mic")

# Fuentes del driver organizadas por módulos
set(DRIVER_SOURCES 
    src/main.c
    src/driver/driver_core.c
    src/audio/audio_processing.c
    src/ioctl/ioctl_handlers.c
    src/common/common.c
)

# Directorios de encabezados
set(INCLUDE_DIRS
    ${CMAKE_CURRENT_SOURCE_DIR}/include
)

# Opción para construir versión mock (sin WDK)
option(BUILD_MOCK_DRIVER "Construir versión mock sin WDK" OFF)

# Verificar si estamos en Windows
if(NOT WIN32)
    message(FATAL_ERROR "Este proyecto solo puede compilarse en Windows")
endif()

# Configuración del compilador
if(MSVC)
    if(NOT BUILD_MOCK_DRIVER)
        # Opciones del compilador para drivers de kernel (solo con WDK)
        add_compile_options(/kernel)
        add_compile_options(/GS-)
        add_compile_options(/Gz)
        add_compile_options(/Oi)
        add_compile_options(/Oy-)
    endif()
    
    # Definiciones del preprocesador
    add_compile_definitions(DBG=1)
endif()

# Crear target del driver
add_library(${DRIVER_NAME} MODULE ${DRIVER_SOURCES})

# Configurar propiedades del driver
if(BUILD_MOCK_DRIVER)
    # Construir como aplicación de consola para pruebas
    set_target_properties(${DRIVER_NAME} PROPERTIES
        SUFFIX ".exe"
    )
    message(STATUS "==========================================")
    message(STATUS "MODO MOCK ACTIVADO - Sin WDK")
    message(STATUS "Construyendo aplicación de prueba en lugar de driver")
    message(STATUS "==========================================")
else()
    # Construir como driver real
    set_target_properties(${DRIVER_NAME} PROPERTIES
        SUFFIX ".sys"
        LINK_FLAGS "/DRIVER /SUBSYSTEM:NATIVE /ENTRY:DriverEntry"
    )
endif()

# Incluir directorios de encabezados
target_include_directories(${DRIVER_NAME} PRIVATE ${INCLUDE_DIRS})

# Incluir directorios de WDK (solo si están disponibles y no es mock)
if(NOT BUILD_MOCK_DRIVER AND EXISTS "C:/Program Files (x86)/Windows Kits/10/Include")
    target_include_directories(${DRIVER_NAME} PRIVATE
        "C:/Program Files (x86)/Windows Kits/10/Include/10.0.22000.0/km"
        "C:/Program Files (x86)/Windows Kits/10/Include/10.0.22000.0/shared"
    )
    
    target_link_directories(${DRIVER_NAME} PRIVATE
        "C:/Program Files (x86)/Windows Kits/10/Lib/10.0.22000.0/km/x64"
    )
elseif(BUILD_MOCK_DRIVER)
    # Para modo mock, usar headers mock predefinidos
    message(STATUS "Usando headers mock para modo sin WDK")
    
    target_include_directories(${DRIVER_NAME} PRIVATE
        "${CMAKE_CURRENT_SOURCE_DIR}/mock"
    )
endif()

# Librerías necesarias
if(NOT BUILD_MOCK_DRIVER)
    target_link_libraries(${DRIVER_NAME}
        ntoskrnl.lib
        hal.lib
        wmilib.lib
    )
else()
    # Para modo mock, usar librerías estándar
    target_link_libraries(${DRIVER_NAME}
        kernel32.lib
        user32.lib
    )
endif()

# Mensaje informativo
message(STATUS "==========================================")
message(STATUS "CONFIGURACIÓN DE DRIVER DE KERNEL MODULAR")
message(STATUS "==========================================")
message(STATUS "Estructura del proyecto:")
message(STATUS "  - src/main.c              : Punto de entrada del driver")
message(STATUS "  - src/driver/             : Núcleo del driver")
message(STATUS "  - src/audio/              : Procesamiento de audio")
message(STATUS "  - src/ioctl/              : Manejadores IOCTL")
message(STATUS "  - src/common/             : Utilidades comunes")
message(STATUS "  - include/                : Archivos de encabezado")
message(STATUS "  - tests/                  : Pruebas automatizadas")
message(STATUS "==========================================")
message(STATUS "IMPORTANTE: Para compilar este driver necesitas:")
message(STATUS "1. Windows Driver Kit (WDK) instalado")
message(STATUS "2. Visual Studio con soporte para drivers")
message(STATUS "3. Certificado de firma de código (para producción)")
message(STATUS "4. Modo de prueba de Windows activado (para desarrollo)")
message(STATUS "==========================================")
message(STATUS "Este driver es para fines educativos y requiere")
message(STATUS "conocimientos avanzados de desarrollo de drivers.")
message(STATUS "==========================================")

# Opciones adicionales para desarrollo
option(BUILD_TESTS "Construir pruebas unitarias" ON)
option(BUILD_DOCS "Construir documentación" ON)

# Agregar subdirectorio de pruebas si está habilitado
if(BUILD_TESTS)
    message(STATUS "Las pruebas unitarias están habilitadas")
    enable_testing()
    add_subdirectory(tests)
endif()

if(BUILD_DOCS)
    message(STATUS "La generación de documentación está habilitada")
endif()