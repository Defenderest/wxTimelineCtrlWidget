param(
    [string]$Configuration = "Release",
    [string]$Generator = "Visual Studio 17 2022",
    [string]$Platform = "x64"
)

Write-Host "==========================================" -ForegroundColor Cyan
Write-Host "wxTimelineCtrl Build Script" -ForegroundColor Cyan
Write-Host "==========================================" -ForegroundColor Cyan

# Check if WXWIN environment variable is set
if (-not $env:WXWIN) {
    Write-Host "ERROR: WXWIN environment variable is not set!" -ForegroundColor Red
    Write-Host "Please set it to your wxWidgets installation directory." -ForegroundColor Yellow
    Write-Host "Example: `$env:WXWIN = 'C:\wxWidgets-3.2.4'" -ForegroundColor Yellow
    Read-Host "Press Enter to exit"
    exit 1
}

Write-Host "Using wxWidgets from: $env:WXWIN" -ForegroundColor Green

# Create build directory if it doesn't exist
$buildDir = "generated"
if (-not (Test-Path $buildDir)) {
    New-Item -ItemType Directory -Path $buildDir | Out-Null
}

Set-Location $buildDir

Write-Host "Building configuration: $Configuration" -ForegroundColor Yellow
Write-Host "Using generator: $Generator" -ForegroundColor Yellow
Write-Host "Target platform: $Platform" -ForegroundColor Yellow

# Generate build files
Write-Host "Generating CMake build files..." -ForegroundColor Yellow
try {
    if ($Platform -eq "x64") {
        & cmake -G $Generator -A x64 ..
    } else {
        & cmake -G $Generator -A Win32 ..
    }
    
    if ($LASTEXITCODE -ne 0) {
        throw "CMake generation failed with exit code $LASTEXITCODE"
    }
} catch {
    Write-Host "ERROR: CMake generation failed!" -ForegroundColor Red
    Write-Host $_.Exception.Message -ForegroundColor Red
    Read-Host "Press Enter to exit"
    exit $LASTEXITCODE
}

# Build the project
Write-Host "Building project..." -ForegroundColor Yellow
try {
    & cmake --build . --config $Configuration
    
    if ($LASTEXITCODE -ne 0) {
        throw "Build failed with exit code $LASTEXITCODE"
    }
} catch {
    Write-Host "ERROR: Build failed!" -ForegroundColor Red
    Write-Host $_.Exception.Message -ForegroundColor Red
    Read-Host "Press Enter to exit"
    exit $LASTEXITCODE
}

Write-Host ""
Write-Host "==========================================" -ForegroundColor Green
Write-Host "Build completed successfully!" -ForegroundColor Green
Write-Host "==========================================" -ForegroundColor Green
Write-Host ""

$exePath = Join-Path (Get-Location) "$Configuration\wxTimelineCtrlTest.exe"
Write-Host "Executable location: $exePath" -ForegroundColor Cyan
Write-Host ""

if (Test-Path $exePath) {
    Write-Host "To run the application:" -ForegroundColor Yellow
    Write-Host "  cd $Configuration" -ForegroundColor White
    Write-Host "  .\wxTimelineCtrlTest.exe" -ForegroundColor White
    Write-Host ""
    
    $runNow = Read-Host "Would you like to run the application now? (y/n)"
    if ($runNow -eq "y" -or $runNow -eq "Y") {
        Set-Location $Configuration
        & .\wxTimelineCtrlTest.exe
    }
} else {
    Write-Host "Warning: Executable not found at expected location" -ForegroundColor Yellow
}

Write-Host ""
Read-Host "Press Enter to exit"
