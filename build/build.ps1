param(
    [string]$Configuration = "Release",
    [string]$Generator = "",
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

# Auto-detect Visual Studio generator if not specified
if ([string]::IsNullOrEmpty($Generator)) {
    Write-Host "Detecting available Visual Studio generators..." -ForegroundColor Yellow
    
    # Get list of available generators
    $generatorList = & cmake --help 2>$null | Select-String "Visual Studio"
    
    # Priority order: VS 2022, VS 2019, VS 2017, VS 2015
    $preferredGenerators = @(
        "Visual Studio 17 2022",
        "Visual Studio 16 2019",
        "Visual Studio 15 2017",
        "Visual Studio 14 2015"
    )
    
    $Generator = $null
    foreach ($preferred in $preferredGenerators) {
        if ($generatorList -match [regex]::Escape($preferred)) {
            $Generator = $preferred
            break
        }
    }
    
    if ([string]::IsNullOrEmpty($Generator)) {
        Write-Host "ERROR: No compatible Visual Studio generator found!" -ForegroundColor Red
        Write-Host "Available generators:" -ForegroundColor Yellow
        & cmake --help | Select-String "Visual Studio"
        Read-Host "Press Enter to exit"
        exit 1
    }
    
    Write-Host "Selected generator: $Generator" -ForegroundColor Green
}

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
    # Handle different Visual Studio versions and their architecture syntax
    if ($Generator -match "Visual Studio (16|17)") {
        # VS 2019 and VS 2022 use -A flag for architecture
        if ($Platform -eq "x64") {
            & cmake -G $Generator -A x64 ..
        } else {
            & cmake -G $Generator -A Win32 ..
        }
    } elseif ($Generator -match "Visual Studio (14|15)") {
        # VS 2015 and VS 2017 use Win64 suffix for 64-bit
        if ($Platform -eq "x64") {
            & cmake -G "$Generator Win64" ..
        } else {
            & cmake -G $Generator ..
        }
    } else {
        # Fallback for other generators
        & cmake -G $Generator ..
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
