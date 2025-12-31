@echo off
echo ==========================================
echo INSTALANDO DRIVER DE MICROFONO VIRTUAL
echo ==========================================
echo.

REM Verificar privilegios de administrador
net session >nul 2>&1
if %errorlevel% neq 0 (
    echo ❌ ERROR: Este script debe ejecutarse como administrador
    echo    Click derecho -^> "Ejecutar como administrador"
    pause
    exit /b 1
)

echo [1/6] Verificando archivos...
if not exist "virtual_mic.sys" (
    echo ❌ ERROR: virtual_mic.sys no encontrado
    echo    Ejecuta build.bat primero para compilar el driver
    pause
    exit /b 1
)

if not exist "virtual_mic.inf" (
    echo ❌ ERROR: virtual_mic.inf no encontrado
    pause
    exit /b 1
)
echo ✅ Archivos encontrados
echo.

echo [2/6] Verificando modo de prueba...
bcdedit | findstr -i testsigning >nul
if %errorlevel% neq 0 (
    echo ⚠️  ADVERTENCIA: Modo de prueba no detectado
    echo    Para instalar drivers sin firmar necesitas:
    echo    bcdedit /set testsigning on
    echo    Luego reinicia tu computadora
    echo.
    set /p modoTest="¿Deseas continuar de todos modos? (S/N): "
    if /i "%modoTest%" neq "S" (
        echo Instalación cancelada
        pause
        exit /b 1
    )
) else (
    echo ✅ Modo de prueba detectado
)
echo.

echo [3/6] Copiando driver al sistema...
copy /Y "virtual_mic.sys" "C:\Windows\System32\drivers\"
if %errorlevel% neq 0 (
    echo ❌ Error al copiar el driver
    pause
    exit /b 1
)
echo ✅ Driver copiado exitosamente
echo.

echo [4/6] Creando servicio del driver...
sc query VirtualMic >nul 2>&1
if %errorlevel% equ 0 (
    echo ⚠️  El servicio ya existe, eliminando...
    sc stop VirtualMic >nul 2>&1
    sc delete VirtualMic
    timeout /t 2 >nul
)

sc create VirtualMic type= kernel binPath= C:\Windows\System32\drivers\virtual_mic.sys start= demand
if %errorlevel% neq 0 (
    echo ❌ Error al crear el servicio
    pause
    exit /b 1
)
echo ✅ Servicio creado exitosamente
echo.

echo [5/6] Iniciando driver...
sc start VirtualMic
if %errorlevel% neq 0 (
    echo ❌ Error al iniciar el driver
    echo    Revisando logs...
    sc query VirtualMic
    pause
    exit /b 1
)
echo ✅ Driver iniciado exitosamente
echo.

echo [6/6] Verificando instalación...
sc query VirtualMic
echo.
echo ==========================================
echo ✅ DRIVER INSTALADO EXITOSAMENTE
echo ==========================================
echo.
echo 📋 Información del driver:
sc qc VirtualMic
echo.
echo 🔍 Para ver logs del driver:
echo    - Abrir Visor de Eventos (eventvwr.msc)
echo    - Windows Logs -^> System
echo    - Buscar "VirtualMicrophone"
echo.
echo ⚠️  IMPORTANTE:
echo    - Este driver es para desarrollo/pruebas
echo    - NO es un micrófono virtual funcional completo
echo    - Requiere implementación WaveRT para ser detectado por aplicaciones
echo    - Usa VB-Cable o VoiceMeeter para resultados inmediatos
echo.
echo 🎯 Próximos pasos:
echo    1. Verificar que el driver esté corriendo
echo    2. Desarrollar aplicación user-space para enviar audio
echo    3. Implementar WaveRT para detección por aplicaciones
echo.
pause