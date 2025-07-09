#Requires -RunAsAdministrator

param(
    [string]$Version = "3.28.1",
    [switch]$Force
)

Write-Host "===========================================" -ForegroundColor Cyan
Write-Host "CMake Update Script" -ForegroundColor Cyan
Write-Host "===========================================" -ForegroundColor Cyan

# Check current CMake version
$currentVersion = $null
try {
    $cmakeOutput = & cmake --version 2>$null | Select-Object -First 1
    if ($cmakeOutput -match "cmake version ([0-9\.]+)") {
        $currentVersion = $matches[1]
        Write-Host "Current CMake version: $currentVersion" -ForegroundColor Yellow
    }
} catch {
    Write-Host "CMake not found or not in PATH" -ForegroundColor Yellow
}

if ($currentVersion -and -not $Force) {
    # Compare versions
    $current = [Version]$currentVersion
    $target = [Version]$Version
    
    if ($current -ge $target) {
        Write-Host "CMake $currentVersion is already up to date (target: $Version)" -ForegroundColor Green
        Read-Host "Press Enter to exit"
        exit 0
    }
}

Write-Host "Updating CMake to version $Version..." -ForegroundColor Yellow

# Check if winget is available (Windows 10 1709+ / Windows 11)
$useWinget = $false
try {
    $wingetVersion = & winget --version 2>$null
    if ($wingetVersion) {
        $useWinget = $true
        Write-Host "Using winget for installation..." -ForegroundColor Green
    }
} catch {
    Write-Host "winget not available, will download manually..." -ForegroundColor Yellow
}

if ($useWinget) {
    # Install using winget
    try {
        Write-Host "Installing CMake via winget..." -ForegroundColor Yellow
        & winget install --id Kitware.CMake --version $Version --silent --accept-package-agreements --accept-source-agreements
        
        if ($LASTEXITCODE -eq 0) {
            Write-Host "CMake installed successfully via winget!" -ForegroundColor Green
        } else {
            throw "winget installation failed with exit code $LASTEXITCODE"
        }
    } catch {
        Write-Host "winget installation failed: $($_.Exception.Message)" -ForegroundColor Red
        Write-Host "Falling back to manual download..." -ForegroundColor Yellow
        $useWinget = $false
    }
}

if (-not $useWinget) {
    # Manual download and installation
    $architecture = if ([Environment]::Is64BitOperatingSystem) { "x86_64" } else { "i386" }
    $downloadUrl = "https://github.com/Kitware/CMake/releases/download/v$Version/cmake-$Version-windows-$architecture.msi"
    $installerPath = "$env:TEMP\cmake-$Version-installer.msi"
    
    try {
        Write-Host "Downloading CMake $Version..." -ForegroundColor Yellow
        Write-Host "URL: $downloadUrl" -ForegroundColor Cyan
        
        Invoke-WebRequest -Uri $downloadUrl -OutFile $installerPath -TimeoutSec 300
        
        Write-Host "Installing CMake..." -ForegroundColor Yellow
        $installArgs = @(
            "/i", $installerPath,
            "/quiet",
            "ADD_CMAKE_TO_PATH=System"
        )
        
        $process = Start-Process -FilePath "msiexec.exe" -ArgumentList $installArgs -Wait -PassThru
        
        if ($process.ExitCode -eq 0) {
            Write-Host "CMake installed successfully!" -ForegroundColor Green
        } else {
            throw "Installation failed with exit code $($process.ExitCode)"
        }
        
        # Clean up installer
        Remove-Item -Path $installerPath -Force -ErrorAction SilentlyContinue
        
    } catch {
        Write-Host "ERROR: Failed to install CMake: $($_.Exception.Message)" -ForegroundColor Red
        if (Test-Path $installerPath) {
            Remove-Item -Path $installerPath -Force -ErrorAction SilentlyContinue
        }
        Read-Host "Press Enter to exit"
        exit 1
    }
}

# Refresh environment variables
Write-Host "Refreshing environment variables..." -ForegroundColor Yellow
$env:Path = [System.Environment]::GetEnvironmentVariable("Path", "Machine") + ";" + [System.Environment]::GetEnvironmentVariable("Path", "User")

# Verify installation
Write-Host "Verifying installation..." -ForegroundColor Yellow
try {
    Start-Sleep -Seconds 2  # Give system time to update PATH
    $newVersion = & cmake --version 2>$null | Select-Object -First 1
    if ($newVersion -match "cmake version ([0-9\.]+)") {
        $installedVersion = $matches[1]
        Write-Host "Successfully installed CMake version: $installedVersion" -ForegroundColor Green
        
        if ($installedVersion -eq $Version) {
            Write-Host "Installation verified!" -ForegroundColor Green
        } else {
            Write-Host "Warning: Installed version ($installedVersion) differs from requested version ($Version)" -ForegroundColor Yellow
        }
    } else {
        Write-Host "Warning: Could not verify CMake installation" -ForegroundColor Yellow
    }
} catch {
    Write-Host "Warning: Could not verify CMake installation: $($_.Exception.Message)" -ForegroundColor Yellow
    Write-Host "You may need to restart your terminal or computer." -ForegroundColor Yellow
}

Write-Host ""
Write-Host "===========================================" -ForegroundColor Green
Write-Host "CMake update completed!" -ForegroundColor Green
Write-Host "===========================================" -ForegroundColor Green
Write-Host ""
Write-Host "Note: You may need to restart your terminal or IDE to use the new CMake version." -ForegroundColor Yellow
Write-Host ""

Read-Host "Press Enter to exit"
