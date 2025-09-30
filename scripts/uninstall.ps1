# BigBrother Uninstallation Script
# Removes shortcuts from Windows Start Menu

Write-Host ""
Write-Host "=====================================================" -ForegroundColor Cyan
Write-Host " BigBrother - Uninstaller" -ForegroundColor Cyan
Write-Host "=====================================================" -ForegroundColor Cyan
Write-Host ""

# Start Menu folder path
$startMenuPath = "$env:APPDATA\Microsoft\Windows\Start Menu\Programs\BigBrother"

# Check if folder exists
if (Test-Path $startMenuPath) {
    Write-Host "Removing Start Menu shortcuts..." -ForegroundColor Yellow
    
    try {
        Remove-Item -Path $startMenuPath -Recurse -Force
        
        Write-Host ""
        Write-Host "=====================================================" -ForegroundColor Green
        Write-Host " Uninstallation Complete!" -ForegroundColor Green
        Write-Host "=====================================================" -ForegroundColor Green
        Write-Host ""
        Write-Host "BigBrother shortcuts have been removed from Start Menu." -ForegroundColor White
        Write-Host ""
        Write-Host "Note: Your session data is still stored at:" -ForegroundColor Cyan
        Write-Host "  $env:APPDATA\BigBrother\focus_log.json" -ForegroundColor White
        Write-Host ""
        Write-Host "Delete this folder manually if you want to remove all data." -ForegroundColor Gray
        Write-Host ""
        
    } catch {
        Write-Host ""
        Write-Host "ERROR: Uninstallation failed!" -ForegroundColor Red
        Write-Host $_.Exception.Message -ForegroundColor Red
        pause
        exit 1
    }
} else {
    Write-Host "BigBrother is not installed in Start Menu." -ForegroundColor Yellow
    Write-Host ""
}

pause
