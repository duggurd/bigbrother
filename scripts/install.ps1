# BigBrother Installation Script
# Adds shortcuts to Windows Start Menu

Write-Host ""
Write-Host "=====================================================" -ForegroundColor Cyan
Write-Host " BigBrother - Start Menu Installation" -ForegroundColor Cyan
Write-Host "=====================================================" -ForegroundColor Cyan
Write-Host ""

# Get the directory where this script is located
$installDir = Split-Path -Parent $MyInvocation.MyCommand.Path

# Check if executables exist
if (-not (Test-Path "$installDir\bigbro.exe")) {
    Write-Host "ERROR: bigbro.exe not found in current directory!" -ForegroundColor Red
    Write-Host "Please run this script from the folder containing the executables." -ForegroundColor Red
    pause
    exit 1
}

if (-not (Test-Path "$installDir\bigbrother_monitor.exe")) {
    Write-Host "ERROR: bigbrother_monitor.exe not found in current directory!" -ForegroundColor Red
    pause
    exit 1
}

# Create Start Menu folder
$startMenuPath = "$env:APPDATA\Microsoft\Windows\Start Menu\Programs\BigBrother"
Write-Host "Creating Start Menu folder..." -ForegroundColor Yellow

try {
    New-Item -ItemType Directory -Force -Path $startMenuPath | Out-Null
    
    # Create WScript Shell object for shortcuts
    $WScriptShell = New-Object -ComObject WScript.Shell
    
    # Create BigBro Viewer shortcut
    Write-Host "Creating BigBro Viewer shortcut..." -ForegroundColor Yellow
    $shortcut = $WScriptShell.CreateShortcut("$startMenuPath\BigBro Viewer.lnk")
    $shortcut.TargetPath = "$installDir\bigbro.exe"
    $shortcut.WorkingDirectory = $installDir
    $shortcut.Description = "BigBrother Session Viewer"
    $shortcut.Save()
    
    # Create Monitor shortcut
    Write-Host "Creating BigBrother Monitor shortcut..." -ForegroundColor Yellow
    $shortcut = $WScriptShell.CreateShortcut("$startMenuPath\BigBrother Monitor.lnk")
    $shortcut.TargetPath = "$installDir\bigbrother_monitor.exe"
    $shortcut.WorkingDirectory = $installDir
    $shortcut.Description = "BigBrother CLI Monitor"
    $shortcut.Save()
    
    Write-Host ""
    Write-Host "=====================================================" -ForegroundColor Green
    Write-Host " Installation Complete!" -ForegroundColor Green
    Write-Host "=====================================================" -ForegroundColor Green
    Write-Host ""
    Write-Host "BigBro has been added to your Start Menu!" -ForegroundColor White
    Write-Host ""
    Write-Host "You can now:" -ForegroundColor Cyan
    Write-Host "  - Search 'BigBro' in Windows Start Menu" -ForegroundColor White
    Write-Host "  - Run 'BigBro Viewer' to view your sessions" -ForegroundColor White
    Write-Host "  - Run 'BigBrother Monitor' for CLI monitoring" -ForegroundColor White
    Write-Host ""
    Write-Host "To uninstall, run: uninstall.ps1" -ForegroundColor Gray
    Write-Host ""
    
} catch {
    Write-Host ""
    Write-Host "ERROR: Installation failed!" -ForegroundColor Red
    Write-Host $_.Exception.Message -ForegroundColor Red
    pause
    exit 1
}

pause
