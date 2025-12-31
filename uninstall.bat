@echo off
echo ==========================================
echo DESINSTALANDO DRIVER DE MICROFONO VIRTUAL
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

echo [1/4] Verificando servicio...
sc query VirtualMic >nul 2>&1
if %errorlevel% neq 0 (
    echo ⚠️  El servicio VirtualMic no existe
    echo    Puede que el driver ya esté desinstalado
    goto :eliminar_archivo
)
echo ✅ Servicio encontrado
echo.

echo [2/4] Deteniendo driver...
sc stop VirtualMic
if %errorlevel% neq 0 (
    echo ⚠️  Advertencia: No se pudo detener el driver
    echo    Puede que ya esté detenido
)
timeout /t 2 >nul
echo ✅ Driver detenido
echo.

echo [3/4] Eliminando servicio...
sc delete VirtualMic
if %errorlevel% neq 0 (
    echo ❌ Error al eliminar el servicio
    pause
    exit /b 1
)
echo ✅ Servicio eliminado
echo.

:eliminar_archivo
echo [4/4] Eliminando archivo del driver...
if exist "C:\Windows\System32\drivers\virtual_mic.sys" (
    del "C:\Windows\System32\drivers\virtual_mic.sys"
    if %errorlevel% equ 0 (
        echo ✅ Archivo del driver eliminado
    ) else (
        echo ⚠️  No se pudo eliminar el archivo
        echo    Puede estar en uso. Reinicia y vuelve a intentar.
    )
) else (
    echo ℹ️  El archivo del driver no existe
)
echo.

echo ==========================================
echo ✅ DRIVER DESINSTALADO EXITOSAMENTE
echo ==========================================
echo.
echo 🧹 Limpieza completada:
echo    - Servicio eliminado
echo    - Driver detenido
echo    - Archivo eliminado (si era posible)
echo.
echo 💡 Notas:
echo    - Es recomendable reiniciar el sistema
echo    - Verifica en el Visor de Eventos que no haya errores
echo    - Si tienes problemas, reinicia en Modo Seguro
echo.
pause