# CMakeLists.txt para pruebas del Virtual Microphone Driver

cmake_minimum_required(VERSION 3.16)

# Proyecto de pruebas
project(VirtualMicTests C)

# Configuración de pruebas
set(TEST_SOURCES
    test_audio_processing.c
    test_ioctl_handlers.c
)

# Configuración del compilador para pruebas
if(MSVC)
    add_compile_options(/W4)        # Nivel de advertencias
    add_compile_options(/Od)        # Sin optimización para debugging
    add_compile_options(/MDd)       # Runtime library debug
    add_compile_options(/Zi)        # Información de debugging
endif()

# Crear ejecutables de prueba
foreach(test_source ${TEST_SOURCES})
    get_filename_component(test_name ${test_source} NAME_WE)
    
    add_executable(${test_name} ${test_source})
    
    # Configurar propiedades del ejecutable
    set_target_properties(${test_name} PROPERTIES
        RUNTIME_OUTPUT_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}"
    )
    
    # Agregar prueba a CTest
    add_test(NAME ${test_name} COMMAND ${test_name})
    
    message(STATUS "Agregada prueba: ${test_name}")
endforeach()

# Mensaje informativo
message(STATUS "==========================================")
message(STATUS "CONFIGURACIÓN DE PRUEBAS")
message(STATUS "==========================================")
message(STATUS "Pruebas configuradas: ${TEST_SOURCES}")
message(STATUS "Para ejecutar pruebas:")
message(STATUS "  cmake --build . --target RUN_TESTS")
message(STATUS "  ctest")
message(STATUS "==========================================")