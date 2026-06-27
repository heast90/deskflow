<#
.SYNOPSIS
    Deskflow build script for Windows
.DESCRIPTION
    Build deskflow-core.exe using Ninja + VS 2026.
    Generates a temp .cmd file to load VS env and run cmake --build.
    Compatible with winremote-mcp Shell tool.
.PARAMETER Rebuild
    Full rebuild (clean then build)
.PARAMETER Config
    Build config: Release (default) | RelWithDebInfo | Debug
.EXAMPLE
    .\build-core.ps1
    .\build-core.ps1 -Rebuild
    .\build-core.ps1 -Config Debug
.NOTES
    Deploy via replace-deskflow.ps1 separately.
#>

param(
    [switch]$Rebuild,
    [ValidateSet('Release', 'RelWithDebInfo', 'Debug')]
    [string]$Config = 'Release'
)

$ErrorActionPreference = 'Stop'

# ===== Paths =====
$VS_PATH     = 'C:\Program Files\Microsoft Visual Studio\18\Enterprise'
$VCVARS      = "$VS_PATH\VC\Auxiliary\Build\vcvars64.bat"
$SOURCE_DIR  = 'D:\git\deskflow'
$BUILD_DIR   = "$SOURCE_DIR\build"
$BINARY      = "$BUILD_DIR\bin\deskflow-core.exe"
$LOG_DIR     = "$SOURCE_DIR\script\download"
$TIMESTAMP   = Get-Date -Format 'yyyyMMdd-HHmmss'
$BUILD_LOG   = "$LOG_DIR\build-core-$Config-$TIMESTAMP.log"
$TEMP_CMD    = "$LOG_DIR\_build-runner-$TIMESTAMP.cmd"

# ===== Main =====
try {
    # ----- Step 0: Check env -----
    Write-Host '>>> [1/3] Checking environment...'

    if (-not (Test-Path $VCVARS)) {
        throw "VCVARS not found: $VCVARS"
    }
    Write-Host "[OK] VS 2026: $VCVARS"

    if (-not (Test-Path "$BUILD_DIR\build.ninja")) {
        throw "build.ninja not found - run cmake-configure.bat first"
    }
    Write-Host "[OK] Build dir: $BUILD_DIR (Ninja)"

    # Ensure log dir exists
    New-Object -TypeName System.IO.DirectoryInfo -ArgumentList $LOG_DIR | ForEach-Object { if (-not $_.Exists) { $_.Create() } }

    # ----- Step 1: Generate temp batch -----
    Write-Host ">>> [2/3] Generating temp build script..."

    $cmdContent = @"
@echo off
setlocal enabledelayedexpansion

echo [VS] Loading VS 2026 environment...
call "$VCVARS" >nul 2>&1
if errorlevel 1 (
    echo [ERR] vcvars64.bat failed
    exit /b 1
)
echo [OK] VS environment loaded

cd /d "$BUILD_DIR"
if errorlevel 1 (
    echo [ERR] Cannot cd to build dir
    exit /b 1
)
echo [OK] Build dir: %CD%

"@

    if ($Rebuild) {
        $cmdContent += @"

echo [CLEAN] Cleaning...
cmake --build . --target clean >nul 2>&1
echo [OK] Clean done

"@
    }

    $cmdContent += @"

echo [BUILD] Building deskflow-core (config: $Config)...
cmake --build . --target deskflow-core --config $Config > "$BUILD_LOG" 2>&1
set BUILD_EXIT=!ERRORLEVEL!

echo [BUILD] Exit code: !BUILD_EXIT!
if "!BUILD_EXIT!"=="0" (
    echo [OK] Build successful!
    dir "$BINARY" 2>nul | findstr deskflow-core
) else (
    echo [ERR] Build FAILED with exit code !BUILD_EXIT!
    echo [ERR] Last 20 lines of log:
    type "$BUILD_LOG"
)

exit /b !BUILD_EXIT!
"@

    Set-Content -Path $TEMP_CMD -Value $cmdContent -Encoding ASCII
    Write-Host "[OK] Temp script: $TEMP_CMD"

    # ----- Step 2: Run build -----
    Write-Host ">>> [3/3] Running build..."
    $result = cmd.exe /c "`"$TEMP_CMD`""
    $exitCode = $LASTEXITCODE

    # Show output
    if ($result) {
        $result | ForEach-Object { Write-Host "  $_" }
    }

    if ($exitCode -ne 0) {
        Write-Host "[ERR] Build FAILED (exit code: $exitCode)"
        Write-Host "  Log: $BUILD_LOG"
        exit $exitCode
    }

    # Verify binary
    if (-not (Test-Path $BINARY)) {
        throw "Binary not found after build: $BINARY"
    }

    $fileInfo = Get-Item $BINARY
    Write-Host "[OK] Build successful!"
    Write-Host "  Output: $BINARY"
    Write-Host "  Size: $($fileInfo.Length / 1KB -as [int]) KB"
    Write-Host "  Modified: $($fileInfo.LastWriteTime)"

    # Clean up temp
    Remove-Item $TEMP_CMD -Force -ErrorAction SilentlyContinue

    Write-Host '>>> Done!'
    exit 0

} catch {
    Write-Host "[ERR] Script failed: $_"
    exit 1
}
