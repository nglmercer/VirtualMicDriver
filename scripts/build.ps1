# build.ps1 - Build script for Virtual Microphone Driver
# This script automates the driver compilation process

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
    Write-Host "Usage: .\build.ps1 [options]"
    Write-Host ""
    Write-Host "Options:"
    Write-Host "  -Configuration <Debug|Release>  Build configuration (default: Debug)"
    Write-Host "  -Platform <x64|x86>             Target platform (default: x64)"
    Write-Host "  -Clean                          Clean before building"
    Write-Host "  -Test                           Run tests after building"
    Write-Host "  -Help                           Show this help"
    Write-Host ""
    Write-Host "Examples:"
    Write-Host "  .\build.ps1                     Build in Debug x64"
    Write-Host "  .\build.ps1 -Configuration Release -Platform x64"
    Write-Host "  .\build.ps1 -Clean -Test        Clean, build and run tests"
}

function Test-Prerequisites {
    Write-Host "Checking prerequisites..."
    
    # Check CMake - search in common locations if not in PATH
    $cmakePath = Get-Command cmake -ErrorAction SilentlyContinue
    if (-not $cmakePath) {
        # Try common installation paths
        $cmakePaths = @(
            "C:\Program Files\CMake\bin\cmake.exe",
            "C:\Program Files (x86)\CMake\bin\cmake.exe",
            "$env:APPDATA\Python\Python*\Scripts\cmake.exe"
        )
        
        foreach ($path in $cmakePaths) {
            if ($path -like "*\*") {
                # Handle wildcards
                $parent = Split-Path $path
                $leaf = Split-Path $path -Leaf
                $foundFiles = Get-ChildItem -Path $parent -Filter $leaf -ErrorAction SilentlyContinue
                if ($foundFiles) {
                    $cmakePath = $foundFiles[0].FullName
                    Write-Host "[OK] CMake found: $cmakePath"
                    break
                }
            } else {
                # Regular path
                if (Test-Path $path) {
                    $cmakePath = $path
                    Write-Host "[OK] CMake found: $cmakePath"
                    break
                }
            }
        }
        
        if (-not $cmakePath) {
            Write-Error "CMake is not installed or not in PATH"
            return $false
        }
    } else {
        Write-Host "[OK] CMake found: $($cmakePath.Source)"
    }
    
    # Store cmake path globally
    $Global:CMakeExe = $cmakePath
    
    # Check Visual Studio / MSBuild
    $msbuildPath = Get-Command msbuild -ErrorAction SilentlyContinue
    if (-not $msbuildPath) {
        # Search for MSBuild in Visual Studio
        $vsPath = "${env:ProgramFiles}\Microsoft Visual Studio\2022"
        $vsPathx86 = "${env:ProgramFiles(x86)}\Microsoft Visual Studio\2022"
        
        if (Test-Path $vsPath) {
            $msbuildPath = "$vsPath\MSBuild\Current\Bin\MSBuild.exe"
        } elseif (Test-Path $vsPathx86) {
            $msbuildPath = "$vsPathx86\MSBuild\Current\Bin\MSBuild.exe"
        }
        
        if ($msbuildPath -and (Test-Path $msbuildPath)) {
            # Found MSBuild
        } elseif (Test-Path "${env:ProgramFiles(x86)}\Microsoft Visual Studio\2019") {
            # Try VS 2019
            $msbuildPath = "${env:ProgramFiles(x86)}\Microsoft Visual Studio\2019\BuildTools\MSBuild\Current\Bin\MSBuild.exe"
            if (-not (Test-Path $msbuildPath)) {
                Write-Error "MSBuild is not installed"
                return $false
            }
        } else {
            Write-Error "MSBuild is not installed"
            return $false
        }
    }
    Write-Host "[OK] MSBuild found: $msbuildPath"
    
    # Check WDK (complete, with km/ directory)
    $wdkPath = "${env:ProgramFiles(x86)}\Windows Kits\10"
    if (-not (Test-Path $wdkPath)) {
        Write-Error "Windows Driver Kit (WDK) not found at: $wdkPath"
        Write-Error "Please run: .\scripts\setup_build_env.ps1 -Verbose"
        Write-Error "And install WDK from: https://learn.microsoft.com/en-us/windows-hardware/drivers/download-the-wdk"
        return $false
    }
    
    # Check that at least one version has km/ directory
    $hasWdk = $false
    Get-ChildItem -Path "$wdkPath\Include" -ErrorAction SilentlyContinue | ForEach-Object {
        $kmPath = "$($_.FullName)\km"
        if (Test-Path $kmPath) {
            $hasWdk = $true
            Write-Host "[OK] Complete WDK found: version $($_.Name)"
        }
    }
    
    if (-not $hasWdk) {
        Write-Error "[ERROR] Complete WDK not found (missing km/ directory)"
        Write-Error "   Only Windows SDK is installed, not complete Windows Driver Kit"
        Write-Error ""
        Write-Error "   SOLUTION:"
        Write-Error "   1. Download WDK: https://learn.microsoft.com/en-us/windows-hardware/drivers/download-the-wdk"
        Write-Error "   2. Run installer and select 'Windows Driver Kit (WDK)'"
        Write-Error "   3. Verify installation with: .\scripts\setup_build_env.ps1 -Verbose"
        return $false
    }
    
    Write-Host "[OK] Prerequisites verified"
    return $true
}

function Invoke-Clean {
    Write-Host "Cleaning build directories..."
    
    if (Test-Path "build") {
        Remove-Item -Recurse -Force "build"
        Write-Host "[OK] build directory removed"
    }
    
    if (Test-Path "CMakeFiles") {
        Remove-Item -Recurse -Force "CMakeFiles"
        Write-Host "[OK] CMakeFiles directory removed"
    }
    
    if (Test-Path "CMakeCache.txt") {
        Remove-Item "CMakeCache.txt"
        Write-Host "[OK] CMakeCache.txt removed"
    }
}

function Invoke-Configure {
    Write-Host "Configuring project with CMake..."
    
    $cmakeArgs = @(
        "-S", ".",
        "-B", "build",
        "-G", "Visual Studio 17 2022",
        "-A", $Platform,
        "-DCMAKE_BUILD_TYPE=$Configuration"
    )
    
    Write-Host "Running: cmake $($cmakeArgs -join ' ')"
    
    try {
        & $Global:CMakeExe $cmakeArgs
        if ($LASTEXITCODE -ne 0) {
            throw "CMake configuration failed with code $LASTEXITCODE"
        }
        Write-Host "[OK] Configuration completed"
        return $true
    }
    catch {
        Write-Error "Error during configuration: $_"
        return $false
    }
}

function Invoke-Build {
    Write-Host "Building driver..."
    
    $buildArgs = @(
        "--build", "build",
        "--config", $Configuration
    )
    
    Write-Host "Running: cmake $($buildArgs -join ' ')"
    
    try {
        & $Global:CMakeExe $buildArgs
        if ($LASTEXITCODE -ne 0) {
            throw "Build failed with code $LASTEXITCODE"
        }
        Write-Host "[OK] Build completed"
        return $true
    }
    catch {
        Write-Error "Error during build: $_"
        return $false
    }
}

function Invoke-Tests {
    Write-Host "Running tests..."
    
    # Search for test executables
    $testFiles = Get-ChildItem -Path "tests" -Filter "test_*.exe" -Recurse -ErrorAction SilentlyContinue
    
    if ($testFiles.Count -eq 0) {
        Write-Warning "No test executables found"
        return $true
    }
    
    $passedTests = 0
    $totalTests = $testFiles.Count
    
    foreach ($testFile in $testFiles) {
        Write-Host "Running: $($testFile.Name)"
        
        try {
            & $testFile.FullName
            if ($LASTEXITCODE -eq 0) {
                Write-Host "   [PASS]"
                $passedTests++
            } else {
                Write-Host "   [FAIL] (code: $LASTEXITCODE)"
            }
        }
        catch {
            Write-Host "   [ERROR]: $_"
        }
    }
    
    Write-Host "`nTest results: $passedTests/$totalTests passed"
    return ($passedTests -eq $totalTests)
}

function Show-BuildResults {
    Write-Host "`n=== Build Results ==="
    
    # Search for .sys file in build directory
    $driverFiles = Get-ChildItem -Path "build" -Filter "$DriverName.sys" -Recurse -ErrorAction SilentlyContinue
    
    if ($driverFiles) {
        foreach ($driverFile in $driverFiles) {
            Write-Host "[OK] Driver built successfully:"
            Write-Host "   Path: $($driverFile.FullName)"
            
            $fileInfo = Get-Item $driverFile.FullName
            Write-Host "   Size: $([math]::Round($fileInfo.Length / 1KB, 2)) KB"
            Write-Host "   Date: $($fileInfo.LastWriteTime)"
        }
    } else {
        Write-Host "[ERROR] Driver not found in build/ directory"
        Write-Host "   Looking for: build\$Configuration\$DriverName.sys"
        $expectedPath = "build\$Configuration\$DriverName.sys"
        if (Test-Path $expectedPath) {
            Write-Host "   [OK] Found at: $expectedPath"
        } else {
            Write-Host "   [ERROR] Not found"
        }
    }
    
    # Search for PDB files
    $pdbFiles = Get-ChildItem -Path "build" -Filter "$DriverName.pdb" -Recurse -ErrorAction SilentlyContinue
    if ($pdbFiles) {
        foreach ($pdbFile in $pdbFiles) {
            Write-Host "[OK] PDB file generated: $($pdbFile.FullName)"
        }
    }
    
    # Check INF file
    $infPath = "virtual_mic.inf"
    if (Test-Path $infPath) {
        Write-Host "[OK] INF file found: $infPath"
    } else {
        Write-Host "[WARNING] INF file not found: $infPath"
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
Write-Host "Configuration: $Configuration"
Write-Host "Platform: $Platform"
Write-Host "=========================================="

# Check prerequisites
if (-not (Test-Prerequisites)) {
    exit 1
}

# Clean if requested
if ($Clean) {
    Invoke-Clean
}

# Configure
if (-not (Invoke-Configure)) {
    exit 1
}

# Build
if (-not (Invoke-Build)) {
    exit 1
}

# Show results
Show-BuildResults

# Run tests if requested
if ($Test) {
    Write-Host "`n"
    if (-not (Invoke-Tests)) {
        exit 1
    }
}

Write-Host "`n[OK] Build process completed successfully"
exit 0
