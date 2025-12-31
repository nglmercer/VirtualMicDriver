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

# Verificar si estamos en Windows
if(NOT WIN32)
    message(FATAL_ERROR "Este proyecto solo puede compilarse en Windows")
endif()

# Detectar automáticamente la versión del WDK instalada
set(WDK_BASE_PATH "C:/Program Files (x86)/Windows Kits/10")
set(WDK_INCLUDE_BASE_PATH "${WDK_BASE_PATH}/Include")
set(WDK_LIB_BASE_PATH "${WDK_BASE_PATH}/Lib")

if(NOT EXISTS "${WDK_INCLUDE_BASE_PATH}")
    message(FATAL_ERROR "Windows Driver Kit (WDK) no encontrado en ${WDK_INCLUDE_BASE_PATH}")
    message(FATAL_ERROR "Por favor, instale el Windows Driver Kit para compilar este driver.")
    message(FATAL_ERROR "Descargar desde: https://docs.microsoft.com/en-us/windows-hardware/drivers/download-the-wdk")
endif()

# Buscar la versión más reciente del WDK instalada
file(GLOB WDK_VERSIONS LIST_DIRECTORIES true "${WDK_INCLUDE_BASE_PATH}/*")
list(SORT WDK_VERSIONS COMPARE NATURAL ORDER DESCENDING)
list(GET WDK_VERSIONS 0 WDK_LATEST_VERSION)

# Extraer solo el número de versión
get_filename_component(WDK_VERSION ${WDK_LATEST_VERSION} NAME)

message(STATUS "WDK version detected: ${WDK_VERSION}")

# Configurar rutas del WDK
set(WDK_INCLUDE_PATH "${WDK_INCLUDE_BASE_PATH}/${WDK_VERSION}/km")
set(WDK_SHARED_INCLUDE_PATH "${WDK_INCLUDE_BASE_PATH}/${WDK_VERSION}/shared")
set(WDK_LIB_PATH "${WDK_LIB_BASE_PATH}/${WDK_VERSION}/km/x64")

if(NOT EXISTS "${WDK_INCLUDE_PATH}")
    message(FATAL_ERROR "WDK headers not found in ${WDK_INCLUDE_PATH}")
endif()

if(NOT EXISTS "${WDK_LIB_PATH}")
    message(FATAL_ERROR "WDK libraries not found in ${WDK_LIB_PATH}")
endif()

# Configuración del compilador
if(MSVC)
    # Opciones del compilador para drivers de kernel
    add_compile_options(/kernel)
    add_compile_options(/GS-)
    add_compile_options(/Gz)
    add_compile_options(/Oi)
    add_compile_options(/Oy-)
    
    # Definiciones del preprocesador
    add_compile_definitions(DBG=1)
endif()

# Crear target del driver
add_library(${DRIVER_NAME} MODULE ${DRIVER_SOURCES})

# Configurar propiedades del driver - Construir como driver real
set_target_properties(${DRIVER_NAME} PROPERTIES
    SUFFIX ".sys"
    LINK_FLAGS "/DRIVER /SUBSYSTEM:NATIVE /ENTRY:DriverEntry"
)

# Incluir directorios de encabezados
target_include_directories(${DRIVER_NAME} PRIVATE ${INCLUDE_DIRS})

# Incluir directorios de WDK
target_include_directories(${DRIVER_NAME} PRIVATE
    ${WDK_INCLUDE_PATH}
    ${WDK_SHARED_INCLUDE_PATH}
)

target_link_directories(${DRIVER_NAME} PRIVATE
    ${WDK_LIB_PATH}
)

# Librerías necesarias
target_link_libraries(${DRIVER_NAME}
    ntoskrnl.lib
    hal.lib
    wmilib.lib
)

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
message(STATUS "REQUISITOS PARA COMPILAR:")
message(STATUS "✓ Windows Driver Kit (WDK) instalado")
message(STATUS "✓ Visual Studio con soporte para drivers")
message(STATUS "⚠ Certificado de firma de código (para producción)")
message(STATUS "⚠ Modo de prueba de Windows activado (para desarrollo)")
message(STATUS "==========================================")
message(STATUS "Para habilitar modo de prueba de Windows:")
message(STATUS "  1. Ejecutar: bcdedit /set testsigning on")
message(STATUS "  2. Reiniciar el sistema")
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
