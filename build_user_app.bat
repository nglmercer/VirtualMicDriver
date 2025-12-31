@echo off
echo ==========================================
echo CONSTRUYENDO APLICACION DE USUARIO
echo ==========================================
echo.

echo [1/2] Compilando aplicacion de usuario...
cl.exe user_app.c /Fe:VirtualMicUserApp.exe
if %errorlevel% neq 0 (
    echo ❌ Error al compilar la aplicacion de usuario
    pause
    exit /b 1
)

echo ✅ Aplicacion compilada exitosamente
echo.

echo [2/2] Verificando ejecutable...
if exist "VirtualMicUserApp.exe" (
    echo ✅ Ejecutable creado: VirtualMicUserApp.exe
    echo.
    echo ==========================================
    echo ✅ APLICACION CONSTRUIDA EXITOSAMENTE
    echo ==========================================
    echo.
    echo 📋 Para usar:
    echo    1. Asegurate de que el driver esté instalado
    echo    2. Ejecuta VirtualMicUserApp.exe
    echo    3. Sigue las instrucciones en pantalla
    echo.
    echo 💡 La aplicacion se conecta al driver via IOCTL
    echo    y permite enviar audio al kernel
    echo.
) else (
    echo ❌ Error: No se pudo crear el ejecutable
)

pause