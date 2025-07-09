Write-Host "==========================================" -ForegroundColor Cyan
Write-Host "wxTimelineCtrl Build Setup Verification" -ForegroundColor Cyan
Write-Host "==========================================" -ForegroundColor Cyan

$issues = @()

# Check CMake
Write-Host "Checking CMake..." -ForegroundColor Yellow
try {
    $cmakeVersion = & cmake --version 2>&1 | Select-String "cmake version" | Select-Object -First 1
    if ($cmakeVersion) {
        Write-Host "✓ CMake found: $cmakeVersion" -ForegroundColor Green
    } else {
        $issues += "CMake not found or not in PATH"
    }
} catch {
    $issues += "CMake not found or not in PATH"
}

# Check WXWIN environment variable
Write-Host "Checking WXWIN environment variable..." -ForegroundColor Yellow
if ($env:WXWIN) {
    Write-Host "✓ WXWIN set to: $env:WXWIN" -ForegroundColor Green
    
    # Check if path exists
    if (Test-Path $env:WXWIN) {
        Write-Host "✓ WXWIN path exists" -ForegroundColor Green
        
        # Check for key wxWidgets files
        $wxConfigFiles = @(
            "lib\vc_x64_lib\wxmsw*.lib",
            "include\wx\wx.h",
            "lib\vc_lib\wxmsw*.lib"
        )
        
        $foundLib = $false
        foreach ($pattern in $wxConfigFiles) {
            $fullPath = Join-Path $env:WXWIN $pattern
            if (Test-Path $fullPath) {
                Write-Host "✓ Found wxWidgets libraries: $pattern" -ForegroundColor Green
                $foundLib = $true
                break
            }
        }
        
        if (-not $foundLib) {
            $issues += "wxWidgets libraries not found in expected locations"
        }
        
        # Check for wx.h
        $wxHeader = Join-Path $env:WXWIN "include\wx\wx.h"
        if (Test-Path $wxHeader) {
            Write-Host "✓ Found wx.h header file" -ForegroundColor Green
        } else {
            $issues += "wx.h header file not found"
        }
    } else {
        $issues += "WXWIN path does not exist: $env:WXWIN"
    }
} else {
    $issues += "WXWIN environment variable not set"
}

# Check Visual Studio
Write-Host "Checking Visual Studio..." -ForegroundColor Yellow
$vsWhere = "${env:ProgramFiles(x86)}\Microsoft Visual Studio\Installer\vswhere.exe"
if (Test-Path $vsWhere) {
    try {
        $vsInstances = & $vsWhere -latest -products * -requires Microsoft.VisualStudio.Component.VC.Tools.x86.x64 -property displayName
        if ($vsInstances) {
            Write-Host "✓ Visual Studio found: $vsInstances" -ForegroundColor Green
        } else {
            $issues += "Visual Studio with C++ tools not found"
        }
    } catch {
        $issues += "Could not determine Visual Studio installation"
    }
} else {
    $issues += "Visual Studio installer not found"
}

# Check source files
Write-Host "Checking source files..." -ForegroundColor Yellow
$requiredLibFiles = @(
    "..\lib\wxTimelineCtrl.h",
    "..\lib\TimelineItemData.h",
    "..\lib\TimelineArtProvider.h",
    "..\lib\FloatingItemPopupWindow.h"
)

$requiredAppFiles = @(
    "..\app\wxTimelineCtrlApp.cpp",
    "..\app\wxTimelineCtrlTestMainFrame.cpp",
    "..\app\SampleData.h"
)

foreach ($file in $requiredLibFiles) {
    if (Test-Path $file) {
        Write-Host "✓ Found library file: $file" -ForegroundColor Green
    } else {
        $issues += "Missing library file: $file"
    }
}

foreach ($file in $requiredAppFiles) {
    if (Test-Path $file) {
        Write-Host "✓ Found application file: $file" -ForegroundColor Green
    } else {
        $issues += "Missing application file: $file"
    }
}

# Check build directory structure
Write-Host "Checking build directory structure..." -ForegroundColor Yellow
$buildDirs = @("lib", "app")
foreach ($dir in $buildDirs) {
    if (Test-Path $dir) {
        Write-Host "✓ Found build directory: $dir" -ForegroundColor Green
    } else {
        $issues += "Missing build directory: $dir"
    }
}

# Summary
Write-Host ""
Write-Host "==========================================" -ForegroundColor Cyan
Write-Host "Verification Summary" -ForegroundColor Cyan
Write-Host "==========================================" -ForegroundColor Cyan

if ($issues.Count -eq 0) {
    Write-Host "✓ All checks passed! Ready to build." -ForegroundColor Green
    Write-Host ""
    Write-Host "To build the project, run:" -ForegroundColor Yellow
    Write-Host "  .\build.ps1" -ForegroundColor White
    Write-Host "  # or" -ForegroundColor Gray
    Write-Host "  .\build.bat" -ForegroundColor White
} else {
    Write-Host "✗ Issues found:" -ForegroundColor Red
    foreach ($issue in $issues) {
        Write-Host "  - $issue" -ForegroundColor Red
    }
    Write-Host ""
    Write-Host "Please resolve these issues before building." -ForegroundColor Yellow
}

Write-Host ""
Read-Host "Press Enter to exit"
