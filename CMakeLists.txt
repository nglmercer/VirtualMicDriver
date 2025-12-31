# CMakeLists.txt for Virtual Microphone Driver - Modular Version
# NOTE: This configuration file builds real Windows drivers.
# Requires Windows Driver Kit (WDK) and Visual Studio with driver support.

cmake_minimum_required(VERSION 3.16)
project(VirtualMicDriver C)

# Driver configuration
set(DRIVER_NAME "virtual_mic")

# Driver sources organized by modules
set(DRIVER_SOURCES 
    src/main.c
    src/driver/driver_core.c
    src/audio/audio_processing.c
    src/ioctl/ioctl_handlers.c
    src/common/common.c
)

# Header directories
set(INCLUDE_DIRS
    ${CMAKE_CURRENT_SOURCE_DIR}/include
)

# Check if we are on Windows
if(NOT WIN32)
    message(FATAL_ERROR "This project can only be built on Windows")
endif()

# Auto-detect installed WDK version
set(WDK_BASE_PATH "C:/Program Files (x86)/Windows Kits/10")
set(WDK_INCLUDE_BASE_PATH "${WDK_BASE_PATH}/Include")
set(WDK_LIB_BASE_PATH "${WDK_BASE_PATH}/Lib")

if(NOT EXISTS "${WDK_INCLUDE_BASE_PATH}")
    message(FATAL_ERROR "Windows Driver Kit (WDK) not found at ${WDK_INCLUDE_BASE_PATH}")
    message(FATAL_ERROR "Please install Windows Driver Kit to build this driver.")
    message(FATAL_ERROR "Download from: https://docs.microsoft.com/en-us/windows-hardware/drivers/download-the-wdk")
endif()

# Find latest installed WDK version
file(GLOB WDK_VERSIONS LIST_DIRECTORIES true "${WDK_INCLUDE_BASE_PATH}/*")
list(SORT WDK_VERSIONS COMPARE NATURAL ORDER DESCENDING)

# Find a version that has kernel-mode components
set(WDK_VERSION_FOUND FALSE)
foreach(WDK_VERSION_CANDIDATE ${WDK_VERSIONS})
    get_filename_component(WDK_VERSION_NAME ${WDK_VERSION_CANDIDATE} NAME)
    set(WDK_KM_TEST_PATH "${WDK_INCLUDE_BASE_PATH}/${WDK_VERSION_NAME}/km")
    if(EXISTS "${WDK_KM_TEST_PATH}")
        set(WDK_VERSION ${WDK_VERSION_NAME})
        set(WDK_LATEST_VERSION ${WDK_VERSION_CANDIDATE})
        set(WDK_VERSION_FOUND TRUE)
        break()
    endif()
endforeach()

if(NOT WDK_VERSION_FOUND)
    message(FATAL_ERROR "No WDK version with kernel-mode components found")
endif()

message(STATUS "WDK version detected: ${WDK_VERSION}")

# Configure WDK paths
set(WDK_INCLUDE_PATH "${WDK_INCLUDE_BASE_PATH}/${WDK_VERSION}/km")
set(WDK_SHARED_INCLUDE_PATH "${WDK_INCLUDE_BASE_PATH}/${WDK_VERSION}/shared")
set(WDK_LIB_PATH "${WDK_LIB_BASE_PATH}/${WDK_VERSION}/km/x64")

# Verify that complete WDK is installed (not just Windows SDK)
if(NOT EXISTS "${WDK_INCLUDE_PATH}")
    message(FATAL_ERROR "==========================================")
    message(FATAL_ERROR "WINDOWS DRIVER KIT (WDK) NOT FOUND")
    message(FATAL_ERROR "==========================================")
    message(FATAL_ERROR "Windows SDK detected but complete WDK is missing")
    message(FATAL_ERROR "")
    message(FATAL_ERROR "The 'km' directory does not exist at:")
    message(FATAL_ERROR "  ${WDK_INCLUDE_PATH}")
    message(FATAL_ERROR "")
    message(FATAL_ERROR "SOLUTION:")
    message(FATAL_ERROR "1. Download complete Windows Driver Kit (WDK):")
    message(FATAL_ERROR "   https://learn.microsoft.com/en-us/windows-hardware/drivers/download-the-wdk")
    message(FATAL_ERROR "")
    message(FATAL_ERROR "2. Run installer and make sure to select:")
    message(FATAL_ERROR "   Windows Driver Kit (WDK)")
    message(FATAL_ERROR "   Windows Driver Kit - Windows 10, 11, and Server 2022")
    message(FATAL_ERROR "   Windows SDK (if offered)")
    message(FATAL_ERROR "   Visual Studio extension for drivers")
    message(FATAL_ERROR "")
    message(FATAL_ERROR "IMPORTANT NOTE:")
    message(FATAL_ERROR "Windows SDK is NOT the same as WDK.")
    message(FATAL_ERROR "WDK includes additional components for")
    message(FATAL_ERROR "kernel driver development (km/ directory)")
    message(FATAL_ERROR "==========================================")
endif()

if(NOT EXISTS "${WDK_LIB_PATH}")
    message(FATAL_ERROR "WDK libraries not found in ${WDK_LIB_PATH}")
endif()

# Compiler configuration
if(MSVC)
    # Override Debug build flags to disable RTC (not available in kernel mode)
    set(CMAKE_C_FLAGS_DEBUG "/kernel /GS- /Gz /MTd /Zi /Od /D _DEBUG")
    
    # Compiler options for kernel drivers
    add_compile_options(/kernel)
    add_compile_options(/GS-)
    add_compile_options(/Gz)
    add_compile_options(/Oi)
    add_compile_options(/Oy-)
    
    # Preprocessor definitions
    add_compile_definitions(DBG=1)
    
    # Target architecture definitions (required for kernel drivers)
    if(CMAKE_SIZEOF_VOID_P EQUAL 8)
        add_compile_definitions(_WIN64)
        add_compile_definitions(_AMD64_)
    elseif(CMAKE_SIZEOF_VOID_P EQUAL 4)
        add_compile_definitions(_X86_)
    endif()
endif()

# Create driver target
add_library(${DRIVER_NAME} MODULE ${DRIVER_SOURCES})

# Configure driver properties - Build as real driver
set_target_properties(${DRIVER_NAME} PROPERTIES
    SUFFIX ".sys"
    LINK_FLAGS "/DRIVER /SUBSYSTEM:NATIVE /ENTRY:DriverEntry /MANIFEST:NO /NODEFAULTLIB:libcmtd /NODEFAULTLIB:libcmt /NODEFAULTLIB:libcd /NODEFAULTLIB:libcpmt /NODEFAULTLIB:kernel32 /NODEFAULTLIB:user32 /NODEFAULTLIB:gdi32 /NODEFAULTLIB:winspool /NODEFAULTLIB:shell32 /NODEFAULTLIB:ole32 /NODEFAULTLIB:oleaut32 /NODEFAULTLIB:uuid /NODEFAULTLIB:comdlg32 /NODEFAULTLIB:advapi32"
    LINKER_LANGUAGE C
)

# Include header directories
target_include_directories(${DRIVER_NAME} PRIVATE ${INCLUDE_DIRS})

# Include WDK directories
target_include_directories(${DRIVER_NAME} PRIVATE
    ${WDK_INCLUDE_PATH}
    ${WDK_SHARED_INCLUDE_PATH}
)

# Target-specific compile options to override defaults
target_compile_options(${DRIVER_NAME} PRIVATE
    /MTd
    /GS-
    /Gz
    /Oi
    /Oy-
    /RTC-
)

target_link_directories(${DRIVER_NAME} PRIVATE
    ${WDK_LIB_PATH}
)

# Required libraries
target_link_libraries(${DRIVER_NAME}
    ntoskrnl.lib
    hal.lib
    wmilib.lib
)

# Informational message
message(STATUS "==========================================")
message(STATUS "MODULAR KERNEL DRIVER CONFIGURATION")
message(STATUS "==========================================")
message(STATUS "Project structure:")
message(STATUS "  - src/main.c              : Driver entry point")
message(STATUS "  - src/driver/             : Driver core")
message(STATUS "  - src/audio/              : Audio processing")
message(STATUS "  - src/ioctl/              : IOCTL handlers")
message(STATUS "  - src/common/             : Common utilities")
message(STATUS "  - include/                : Header files")
message(STATUS "  - tests/                  : Automated tests")
message(STATUS "==========================================")
message(STATUS "REQUIREMENTS TO BUILD:")
message(STATUS "  Windows Driver Kit (WDK) installed")
message(STATUS "  Visual Studio with driver support")
message(STATUS "  Code signing certificate (for production)")
message(STATUS "  Windows test mode enabled (for development)")
message(STATUS "==========================================")
message(STATUS "To enable Windows test mode:")
message(STATUS "  1. Run: bcdedit /set testsigning on")
message(STATUS "  2. Restart system")
message(STATUS "==========================================")

# Additional options for development
option(BUILD_TESTS "Build unit tests" ON)
option(BUILD_DOCS "Build documentation" ON)

# Add tests subdirectory if enabled
if(BUILD_TESTS)
    message(STATUS "Unit tests are enabled")
    enable_testing()
    add_subdirectory(tests)
endif()

if(BUILD_DOCS)
    message(STATUS "Documentation generation is enabled")
endif()
