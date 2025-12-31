@echo off
echo ==========================================
echo CONSTRUYENDO DRIVER DE KERNEL VIRTUAL MIC
echo ==========================================
echo.

REM Verificar si estamos en Visual Studio Developer Command Prompt
where cl >nul 2>nul
if %errorlevel% neq 0 (
    echo ERROR: No se encuentra el compilador de Visual Studio
    echo Por favor, ejecuta este script desde "Developer Command Prompt for VS"
    echo o asegurate de que Visual Studio esté instalado con C++
    pause
    exit /b 1
)

REM Verificar WDK
if not exist "C:\Program Files (x86)\Windows Kits\10\Include\10.0.22000.0\km" (
    echo ERROR: Windows Driver Kit (WDK) no encontrado
    echo Por favor, instala WDK 10.0.22000 o superior
    pause
    exit /b 1
)

echo [1/4] Verificando requisitos...
echo ✅ Visual Studio encontrado
echo ✅ WDK encontrado
echo.

echo [2/4] Configurando variables...
set WDK_INCLUDE=C:\Program Files (x86)\Windows Kits\10\Include\10.0.22000.0
set WDK_LIB=C:\Program Files (x86)\Windows Kits\10\Lib\10.0.22000.0
set COMPILER_FLAGS=/kernel /GS- /Gz /Oi /Oy- /D_KERNEL_MODE /DDEBUG=1
set INCLUDE_PATHS=/I"%WDK_INCLUDE%\km" /I"%WDK_INCLUDE%\shared"
set LINK_FLAGS=/DRIVER /SUBSYSTEM:NATIVE /ENTRY:DriverEntry
set LIBRARIES=ntoskrnl.lib hal.lib wmilib.lib
echo.

echo [3/4] Compilando driver...
cl.exe %COMPILER_FLAGS% %INCLUDE_PATHS% /c main.c /Fomain.obj
if %errorlevel% neq 0 (
    echo ❌ Error al compilar main.c
    pause
    exit /b 1
)

echo [4/4] Enlazando driver...
link.exe %LINK_FLAGS% /LIBPATH:"%WDK_LIB%\km\x64" main.obj %LIBRARIES% /OUT:virtual_mic.sys
if %errorlevel% neq 0 (
    echo ❌ Error al enlazar el driver
    pause
    exit /b 1
)

echo.
echo ==========================================
echo ✅ DRIVER COMPILADO EXITOSAMENTE
echo ==========================================
echo.
echo Archivos generados:
dir virtual_mic.sys
echo.
echo ⚠️  IMPORTANTE:
echo    - Este driver necesita modo de prueba activado
echo    - Ejecuta: bcdedit /set testsigning on
echo    - Reinicia tu computadora
echo    - Luego ejecuta install.bat como administrador
echo.
echo 📋 Para instalar:
echo    1. Copiar virtual_mic.sys a C:\Windows\System32\drivers\
echo    2. Ejecutar install.bat como administrador
echo.
pause