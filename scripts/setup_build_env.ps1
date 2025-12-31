# Setup Build Environment Script for Virtual Microphone Driver
# This script helps verify and set up the required build environment

param(
    [switch]$Verbose
)

function Write-ColorOutput {
    param(
        [string]$Message,
        [string]$Color = "White"
    )
    Write-Host $Message -ForegroundColor $Color
}

Write-ColorOutput "========================================" "Cyan"
Write-ColorOutput "Virtual Microphone Driver - Build Setup" "Cyan"
Write-ColorOutput "========================================" "Cyan"
Write-Host ""

# Check if running as administrator
$isAdmin = ([Security.Principal.WindowsPrincipal] [Security.Principal.WindowsIdentity]::GetCurrent()).IsInRole([Security.Principal.WindowsBuiltInRole]::Administrator)
if (-not $isAdmin) {
    Write-ColorOutput "⚠️  Warning: Not running as administrator" "Yellow"
    Write-ColorOutput "   Some operations may require elevated privileges" "Yellow"
    Write-Host ""
}

# Check Visual Studio
Write-ColorOutput "Checking Visual Studio installation..." "Green"
$vsPath = "${env:ProgramFiles(x86)}\Microsoft Visual Studio\2022"
if (Test-Path $vsPath) {
    Write-ColorOutput "✅ Visual Studio 2022 found at: $vsPath" "Green"
} else {
    Write-ColorOutput "❌ Visual Studio 2022 not found" "Red"
    Write-ColorOutput "   Please install Visual Studio 2022 with Desktop development with C++" "Red"
    Write-Host ""
    exit 1
}
Write-Host ""

# Check Windows Driver Kit
Write-ColorOutput "Checking Windows Driver Kit (WDK) installation..." "Green"
$wdkPath = "${env:ProgramFiles(x86)}\Windows Kits\10"
if (Test-Path $wdkPath) {
    Write-ColorOutput "✅ Windows Kits found at: $wdkPath" "Green"
    
    # List available versions
    Write-ColorOutput "   Available WDK/SDK versions:" "Cyan"
    Get-ChildItem -Path "$wdkPath\Include" | ForEach-Object {
        $version = $_.Name
        Write-ColorOutput "     - $version" "White"
        
        # Check for kernel-mode (km) directory
        $kmPath = "$wdkPath\Include\$version\km"
        $umPath = "$wdkPath\Include\$version\um"
        $sharedPath = "$wdkPath\Include\$version\shared"
        
        $hasKm = Test-Path $kmPath
        $hasUm = Test-Path $umPath
        $hasShared = Test-Path $sharedPath
        
        if ($Verbose) {
            Write-ColorOutput "       km (kernel-mode): $(if ($hasKm) { '✅' } else { '❌' })" "White"
            Write-ColorOutput "       um (user-mode): $(if ($hasUm) { '✅' } else { '❌' })" "White"
            Write-ColorOutput "       shared: $(if ($hasShared) { '✅' } else { '❌' })" "White"
        }
        
        if ($hasKm) {
            Write-ColorOutput "       ✅ Complete WDK (includes kernel-mode headers)" "Green"
        } else {
            Write-ColorOutput "       ⚠️  Windows SDK only (missing kernel-mode headers)" "Yellow"
        }
    }
    
    # Find latest version with km directory
    $latestWdkVersion = Get-ChildItem -Path "$wdkPath\Include" | 
                        Where-Object { Test-Path "$($_.FullName)\km" } |
                        Sort-Object Name -Descending |
                        Select-Object -First 1 -ExpandProperty Name
    
    if ($latestWdkVersion) {
        Write-Host ""
        Write-ColorOutput "   Latest complete WDK version: $latestWdkVersion" "Cyan"
    } else {
        Write-Host ""
        Write-ColorOutput "❌ ERROR: No complete WDK installation found!" "Red"
        Write-ColorOutput "   The kernel-mode (km) headers are missing." "Red"
        Write-ColorOutput "   Windows SDK is installed but WDK is not." "Red"
        Write-Host ""
        Write-ColorOutput "   SOLUTION:" "Yellow"
        Write-ColorOutput "   1. Download Windows Driver Kit (WDK):" "Yellow"
        Write-ColorOutput "      https://learn.microsoft.com/en-us/windows-hardware/drivers/download-the-wdk" "Cyan"
        Write-ColorOutput "   2. Run the WDK installer" "Yellow"
        Write-ColorOutput "   3. Make sure to select:" "Yellow"
        Write-ColorOutput "      - Windows Driver Kit (WDK)" "White"
        Write-ColorOutput "      - Windows Driver Kit - Windows 10, 11, and Server 2022" "White"
        Write-ColorOutput "      - Visual Studio extension for drivers" "White"
        Write-Host ""
        Write-ColorOutput "   NOTE: Windows SDK is NOT the same as WDK!" "Yellow"
        Write-ColorOutput "   WDK includes additional components for kernel driver development." "Yellow"
        Write-Host ""
    }
} else {
    Write-ColorOutput "❌ Windows Driver Kit not found" "Red"
    Write-Host ""
    Write-ColorOutput "   Please install Windows Driver Kit:" "Yellow"
    Write-ColorOutput "   https://learn.microsoft.com/en-us/windows-hardware/drivers/download-the-wdk" "Cyan"
    Write-Host ""
    exit 1
}
Write-Host ""

# Check CMake
Write-ColorOutput "Checking CMake installation..." "Green"
$cmakeVersion = cmake --version 2>&1 | Select-Object -First 1
if ($cmakeVersion) {
    Write-ColorOutput "✅ $cmakeVersion" "Green"
} else {
    Write-ColorOutput "❌ CMake not found" "Red"
    Write-ColorOutput "   Please install CMake from: https://cmake.org/download/" "Red"
    Write-Host ""
}
Write-Host ""

# Check test signing mode
Write-ColorOutput "Checking test signing mode..." "Green"
$testSigning = bcdedit | Select-String "testsigning"
if ($testSigning -match "Yes") {
    Write-ColorOutput "✅ Test signing mode is enabled" "Green"
} else {
    Write-ColorOutput "⚠️  Test signing mode is not enabled" "Yellow"
    Write-ColorOutput "   Enable it by running (as administrator):" "Yellow"
    Write-ColorOutput "   bcdedit /set testsigning on" "Cyan"
    Write-ColorOutput "   Then restart your computer" "Yellow"
}
Write-Host ""

# Summary
Write-ColorOutput "========================================" "Cyan"
Write-ColorOutput "Build Environment Check Complete" "Cyan"
Write-ColorOutput "========================================" "Cyan"
Write-Host ""
Write-ColorOutput "Next steps:" "Green"
Write-ColorOutput "1. If all checks pass, run:" "White"
Write-ColorOutput "   cd build" "Cyan"
Write-ColorOutput "   cmake .. -G `"Visual Studio 17 2022`" -A x64" "Cyan"
Write-ColorOutput "   cmake --build . --config Debug" "Cyan"
Write-Host ""
Write-ColorOutput "2. Or use the build script:" "White"
Write-ColorOutput "   .\scripts\build.ps1" "Cyan"
Write-Host ""
Write-ColorOutput "3. For more information, see README.md" "White"
Write-Host ""
