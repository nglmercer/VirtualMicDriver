# build.ps1 - Script de construcción para Virtual Microphone Driver
# Este script automatiza el proceso de compilación del driver

param(
    [string]$Configuration = "Debug",
    [string]$Platform = "x64",
    [switch]$Clean = $false,
    [switch]$Test = $false,
    [switch]$Help = $false
)

# Driver name configuration
$DriverName = "virtual_mic"

function Show-Help {
    Write-Host "Uso: .\build.ps1 [opciones]"
    Write-Host ""
    Write-Host "Opciones:"
    Write-Host "  -Configuration <Debug|Release>  Configuración de compilación (por defecto: Debug)"
    Write-Host "  -Platform <x64|x86>             Plataforma objetivo (por defecto: x64)"
    Write-Host "  -Clean                          Limpiar antes de compilar"
    Write-Host "  -Test                           Ejecutar pruebas después de compilar"
    Write-Host "  -Help                           Mostrar esta ayuda"
    Write-Host ""
    Write-Host "Ejemplos:"
    Write-Host "  .\build.ps1                     Compilar en modo Debug x64"
    Write-Host "  .\build.ps1 -Configuration Release -Platform x64"
    Write-Host "  .\build.ps1 -Clean -Test        Limpiar, compilar y ejecutar pruebas"
}

function Test-Prerequisites {
    Write-Host "Verificando prerrequisitos..."
    
    # Verificar CMake
    $cmakePath = Get-Command cmake -ErrorAction SilentlyContinue
    if (-not $cmakePath) {
        Write-Error "CMake no está instalado o no está en el PATH"
        return $false
    }
    Write-Host "✅ CMake encontrado: $($cmakePath.Source)"
    
    # Verificar Visual Studio
    $msbuildPath = Get-Command msbuild -ErrorAction SilentlyContinue
    if (-not $msbuildPath) {
        Write-Error "MSBuild no está instalado o no está en el PATH"
        return $false
    }
    Write-Host "✅ MSBuild encontrado: $($msbuildPath.Source)"
    
    # Verificar WDK completo (con directorio km/)
    $wdkPath = "${env:ProgramFiles(x86)}\Windows Kits\10"
    if (-not (Test-Path $wdkPath)) {
        Write-Error "Windows Driver Kit (WDK) no encontrado en: $wdkPath"
        Write-Error "Por favor, ejecute: .\scripts\setup_build_env.ps1 -Verbose"
        Write-Error "E instale el WDK completo desde: https://learn.microsoft.com/en-us/windows-hardware/drivers/download-the-wdk"
        return $false
    }
    
    # Verificar que existe al menos una versión con directorio km/
    $hasWdk = $false
    Get-ChildItem -Path "$wdkPath\Include" -ErrorAction SilentlyContinue | ForEach-Object {
        $kmPath = "$($_.FullName)\km"
        if (Test-Path $kmPath) {
            $hasWdk = $true
            Write-Host "✅ WDK completo encontrado: versión $($_.Name)"
        }
    }
    
    if (-not $hasWdk) {
        Write-Error "❌ ERROR: No se encontró el WDK completo (falta directorio km/)"
        Write-Error "   Solo tiene Windows SDK instalado, no el Windows Driver Kit completo"
        Write-Error ""
        Write-Error "   SOLUCIÓN:"
        Write-Error "   1. Descargue el WDK: https://learn.microsoft.com/en-us/windows-hardware/drivers/download-the-wdk"
        Write-Error "   2. Ejecute el instalador y seleccione 'Windows Driver Kit (WDK)'"
        Write-Error "   3. Verifique la instalación ejecutando: .\scripts\setup_build_env.ps1 -Verbose"
        return $false
    }
    
    Write-Host "✅ Prerrequisitos verificados"
    return $true
}

function Invoke-Clean {
    Write-Host "Limpiando directorios de compilación..."
    
    if (Test-Path "build") {
        Remove-Item -Recurse -Force "build"
        Write-Host "✅ Directorio build eliminado"
    }
    
    if (Test-Path "CMakeFiles") {
        Remove-Item -Recurse -Force "CMakeFiles"
        Write-Host "✅ Directorio CMakeFiles eliminado"
    }
    
    if (Test-Path "CMakeCache.txt") {
        Remove-Item "CMakeCache.txt"
        Write-Host "✅ Archivo CMakeCache.txt eliminado"
    }
}

function Invoke-Configure {
    Write-Host "Configurando proyecto con CMake..."
    
    $cmakeArgs = @(
        "-S", ".",
        "-B", "build",
        "-G", "Visual Studio 17 2022",
        "-A", $Platform,
        "-DCMAKE_BUILD_TYPE=$Configuration"
    )
    
    Write-Host "Ejecutando: cmake $($cmakeArgs -join ' ')"
    
    try {
        & cmake $cmakeArgs
        if ($LASTEXITCODE -ne 0) {
            throw "CMake configuración falló con código $LASTEXITCODE"
        }
        Write-Host "✅ Configuración completada"
        return $true
    }
    catch {
        Write-Error "Error durante la configuración: $_"
        return $false
    }
}

function Invoke-Build {
    Write-Host "Compilando el driver..."
    
    $buildArgs = @(
        "--build", "build",
        "--config", $Configuration
    )
    
    Write-Host "Ejecutando: cmake $($buildArgs -join ' ')"
    
    try {
        & cmake $buildArgs
        if ($LASTEXITCODE -ne 0) {
            throw "Compilación falló con código $LASTEXITCODE"
        }
        Write-Host "✅ Compilación completada"
        return $true
    }
    catch {
        Write-Error "Error durante la compilación: $_"
        return $false
    }
}

function Invoke-Tests {
    Write-Host "Ejecutando pruebas..."
    
    # Buscar ejecutables de prueba
    $testFiles = Get-ChildItem -Path "tests" -Filter "test_*.exe" -Recurse -ErrorAction SilentlyContinue
    
    if ($testFiles.Count -eq 0) {
        Write-Warning "No se encontraron ejecutables de prueba"
        return $true
    }
    
    $passedTests = 0
    $totalTests = $testFiles.Count
    
    foreach ($testFile in $testFiles) {
        Write-Host "Ejecutando: $($testFile.Name)"
        
        try {
            & $testFile.FullName
            if ($LASTEXITCODE -eq 0) {
                Write-Host "   ✅ PASADA"
                $passedTests++
            } else {
                Write-Host "   ❌ FALLIDA (código: $LASTEXITCODE)"
            }
        }
        catch {
            Write-Host "   ❌ ERROR: $_"
        }
    }
    
    Write-Host "`nResultados de pruebas: $passedTests/$totalTests pasadas"
    return ($passedTests -eq $totalTests)
}

function Show-BuildResults {
    Write-Host "`n=== Resultados de la compilación ==="
    
    # Buscar el archivo .sys en el directorio build
    $driverFiles = Get-ChildItem -Path "build" -Filter "$DriverName.sys" -Recurse -ErrorAction SilentlyContinue
    
    if ($driverFiles) {
        foreach ($driverFile in $driverFiles) {
            Write-Host "✅ Driver compilado exitosamente:"
            Write-Host "   Ruta: $($driverFile.FullName)"
            
            $fileInfo = Get-Item $driverFile.FullName
            Write-Host "   Tamaño: $([math]::Round($fileInfo.Length / 1KB, 2)) KB"
            Write-Host "   Fecha: $($fileInfo.LastWriteTime)"
        }
    } else {
        Write-Host "❌ Driver no encontrado en el directorio build/"
        Write-Host "   Buscando en: build\$Configuration\$DriverName.sys"
        $expectedPath = "build\$Configuration\$DriverName.sys"
        if (Test-Path $expectedPath) {
            Write-Host "   ✅ Encontrado en: $expectedPath"
        } else {
            Write-Host "   ❌ No encontrado"
        }
    }
    
    # Buscar archivos PDB
    $pdbFiles = Get-ChildItem -Path "build" -Filter "$DriverName.pdb" -Recurse -ErrorAction SilentlyContinue
    if ($pdbFiles) {
        foreach ($pdbFile in $pdbFiles) {
            Write-Host "✅ Archivo PDB generado: $($pdbFile.FullName)"
        }
    }
    
    # Verificar archivo INF
    $infPath = "virtual_mic.inf"
    if (Test-Path $infPath) {
        Write-Host "✅ Archivo INF encontrado: $infPath"
    } else {
        Write-Host "⚠️  Archivo INF no encontrado: $infPath"
    }
}

# Main execution
if ($Help) {
    Show-Help
    exit 0
}

Write-Host "=========================================="
Write-Host "Virtual Microphone Driver Build Script"
Write-Host "Driver: $DriverName"
Write-Host "Configuración: $Configuration"
Write-Host "Plataforma: $Platform"
Write-Host "=========================================="

# Verificar prerrequisitos
if (-not (Test-Prerequisites)) {
    exit 1
}

# Limpiar si se solicitó
if ($Clean) {
    Invoke-Clean
}

# Configurar
if (-not (Invoke-Configure)) {
    exit 1
}

# Construir
if (-not (Invoke-Build)) {
    exit 1
}

# Mostrar resultados
Show-BuildResults

# Ejecutar pruebas si se solicitó
if ($Test) {
    Write-Host "`n"
    if (-not (Invoke-Tests)) {
        exit 1
    }
}

Write-Host "`n✅ Proceso de compilación completado exitosamente"
exit 0
