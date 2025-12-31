# test.ps1 - Script de pruebas para Virtual Microphone Driver
# Este script ejecuta todas las pruebas automatizadas del proyecto

param(
    [string]$TestFilter = "*",
    [switch]$Verbose = $false,
    [switch]$Coverage = $false,
    [switch]$Help = $false
)

function Show-Help {
    Write-Host "Uso: .\test.ps1 [opciones]"
    Write-Host ""
    Write-Host "Opciones:"
    Write-Host "  -TestFilter <pattern>   Filtro de pruebas (por defecto: *)"
    Write-Host "  -Verbose                Mostrar salida detallada"
    Write-Host "  -Coverage               Generar informe de cobertura"
    Write-Host "  -Help                   Mostrar esta ayuda"
    Write-Host ""
    Write-Host "Ejemplos:"
    Write-Host "  .\test.ps1                        Ejecutar todas las pruebas"
    Write-Host "  .\test.ps1 -TestFilter *audio*    Ejecutar solo pruebas de audio"
    Write-Host "  .\test.ps1 -Verbose               Ejecutar con salida detallada"
}

function Test-Prerequisites {
    Write-Host "Verificando prerrequisitos para pruebas..."
    
    # Verificar que existen pruebas
    $testFiles = Get-ChildItem -Path "tests" -Filter "test_*.c" -ErrorAction SilentlyContinue
    if ($testFiles.Count -eq 0) {
        Write-Warning "No se encontraron archivos de prueba en el directorio tests/"
        return $false
    }
    
    Write-Host "✅ Encontrados $($testFiles.Count) archivos de prueba"
    return $true
}

function Invoke-TestCompilation {
    Write-Host "Compilando pruebas..."
    
    $testFiles = Get-ChildItem -Path "tests" -Filter "test_*.c"
    $compiledTests = 0
    $totalTests = $testFiles.Count
    
    foreach ($testFile in $testFiles) {
        $outputName = $testFile.BaseName + ".exe"
        $outputPath = Join-Path "tests" $outputName
        
        Write-Host "Compilando: $($testFile.Name)"
        
        try {
            # Compilar con cl.exe (Visual Studio)
            $compileArgs = @(
                $testFile.FullName,
                "/Fe:$outputPath",
                "/Fo:tests\",
                "/W4",          # Nivel de advertencias
                "/Od",          # Sin optimización para debugging
                "/MDd",         # Runtime library debug
                "/Zi"           # Información de debugging
            )
            
            & cl.exe $compileArgs 2>$null
            if ($LASTEXITCODE -eq 0) {
                Write-Host "   ✅ Compilado exitosamente"
                $compiledTests++
            } else {
                Write-Host "   ❌ Error de compilación"
            }
        }
        catch {
            Write-Host "   ❌ Error: $_"
        }
    }
    
    Write-Host "`nCompilación de pruebas: $compiledTests/$totalTests exitosas"
    return ($compiledTests -eq $totalTests)
}

function Invoke-TestExecution {
    param(
        [string]$Filter
    )
    
    Write-Host "Ejecutando pruebas con filtro: $Filter"
    
    $testExecutables = Get-ChildItem -Path "tests" -Filter "test_*.exe" -ErrorAction SilentlyContinue
    $filteredTests = $testExecutables | Where-Object { $_.Name -like $Filter }
    
    if ($filteredTests.Count -eq 0) {
        Write-Warning "No se encontraron pruebas que coincidan con el filtro: $Filter"
        return $true
    }
    
    $passedTests = 0
    $totalTests = $filteredTests.Count
    $testResults = @()
    
    foreach ($test in $filteredTests) {
        Write-Host "`n--- Ejecutando: $($test.Name) ---"
        
        try {
            $startTime = Get-Date
            
            if ($Verbose) {
                & $test.FullName
            } else {
                & $test.FullName | Out-Null
            }
            
            $exitCode = $LASTEXITCODE
            $endTime = Get-Date
            $duration = $endTime - $startTime
            
            $result = @{
                Name = $test.Name
                Passed = ($exitCode -eq 0)
                ExitCode = $exitCode
                Duration = $duration
            }
            
            $testResults += $result
            
            if ($exitCode -eq 0) {
                Write-Host "✅ PASADA ($($duration.TotalSeconds.ToString('F2'))s)"
                $passedTests++
            } else {
                Write-Host "❌ FALLIDA (código: $exitCode, $($duration.TotalSeconds.ToString('F2'))s)"
            }
        }
        catch {
            Write-Host "❌ ERROR: $_"
            $testResults += @{
                Name = $test.Name
                Passed = $false
                ExitCode = -1
                Duration = [TimeSpan]::Zero
            }
        }
    }
    
    # Mostrar resumen
    Write-Host "`n=== RESUMEN DE PRUEBAS ==="
    Write-Host "Total: $totalTests"
    Write-Host "Pasadas: $passedTests"
    Write-Host "Fallidas: $($totalTests - $passedTests)"
    Write-Host "Porcentaje de éxito: $([math]::Round(($passedTests / $totalTests) * 100, 1))%"
    
    if ($Verbose) {
        Write-Host "`n=== DETALLES DE PRUEBAS ==="
        foreach ($result in $testResults) {
            $estado = if ($result.Passed) { 'PASADA' } else { 'FALLIDA' }
            Write-Host ("{0}: {1} ({2}s)" -f $result.Name, $estado, $result.Duration.TotalSeconds.ToString('F2'))
        }
    }
    
    return ($passedTests -eq $totalTests)
}

function Invoke-CoverageAnalysis {
    Write-Host "Generando análisis de cobertura..."
    
    # Esto es un placeholder para análisis de cobertura real
    # En un entorno de producción, usaríamos herramientas como:
    # - OpenCppCoverage (Windows)
    # - gcov/lcov (Linux)
    # - Visual Studio Code Coverage
    
    Write-Host "✅ Análisis de cobertura generado (simulado)"
    Write-Host "   Nota: En producción, se usarían herramientas de cobertura reales"
}

function Invoke-StaticAnalysis {
    Write-Host "Ejecutando análisis estático..."
    
    # Buscar archivos fuente
    $sourceFiles = Get-ChildItem -Path "src" -Filter "*.c" -Recurse
    $headerFiles = Get-ChildItem -Path "include" -Filter "*.h" -Recurse
    
    Write-Host "Analizando $($sourceFiles.Count) archivos fuente y $($headerFiles.Count) encabezados"
    
    # Análisis básico de código
    $issues = 0
    
    foreach ($file in ($sourceFiles + $headerFiles)) {
        $content = Get-Content $file.FullName -Raw
        
        # Buscar patrones comunes de problemas
        if ($content -match 'strcpy\s*\(') {
            Write-Warning "$($file.Name): Uso de strcpy (inseguro)"
            $issues++
        }
        
        if ($content -match 'gets\s*\(') {
            Write-Warning "$($file.Name): Uso de gets (inseguro)"
            $issues++
        }
        
        if ($content -match 'malloc\s*\([^)]*\)\s*[^;]*;') {
            # Verificar si hay free correspondiente
            if (-not ($content -match 'free\s*\(')) {
                Write-Warning "$($file.Name): Posible pérdida de memoria (malloc sin free)"
                $issues++
            }
        }
    }
    
    if ($issues -eq 0) {
        Write-Host "✅ No se encontraron problemas en el análisis estático"
    } else {
        Write-Host "⚠️  Se encontraron $issues posibles problemas"
    }
    
    return ($issues -eq 0)
}

# Main execution
if ($Help) {
    Show-Help
    exit 0
}

Write-Host "=========================================="
Write-Host "Virtual Microphone Driver Test Script"
Write-Host "Filtro: $TestFilter"
Write-Host "=========================================="

# Verificar prerrequisitos
if (-not (Test-Prerequisites)) {
    exit 1
}

# Compilar pruebas
if (-not (Invoke-TestCompilation)) {
    Write-Error "Falló la compilación de pruebas"
    exit 1
}

# Ejecutar análisis estático
Write-Host "`n"
if (-not (Invoke-StaticAnalysis)) {
    Write-Warning "El análisis estático encontró problemas"
}

# Ejecutar pruebas
Write-Host "`n"
$testsPassed = Invoke-TestExecution -Filter $TestFilter

# Generar cobertura si se solicitó
if ($Coverage) {
    Write-Host "`n"
    Invoke-CoverageAnalysis
}

if ($testsPassed) {
    Write-Host "`n✅ Todas las pruebas pasaron exitosamente"
    exit 0
} else {
    Write-Host "`n❌ Algunas pruebas fallaron"
    exit 1
}