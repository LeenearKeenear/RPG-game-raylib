# setup.ps1 - Auto-download raylib if not present
# Downloads raylib 5.5 from GitHub releases to lib/raylib/

param(
    [string]$RaylibVersion = "5.5",
    [string]$RepoUrl = "https://github.com/raysan5/raylib/releases/download/5.5/raylib-5.5_win64_mingw-w64.zip"
)

function Write-Step($message) {
    Write-Host "[SETUP] $message" -ForegroundColor Cyan
}

function Install-Raylib() {
    Write-Step "Checking for raylib..."

    $installDir = "lib/raylib"
    if ((Test-Path "$installDir/include/raylib.h") -and (Test-Path "$installDir/lib/libraylib.a")) {
        Write-Step "raylib already installed at $installDir"
        return
    }

    Write-Step "raylib not found. Downloading raylib $RaylibVersion from GitHub..."

    # Download using curl.exe
    Write-Step "Downloading..."
    & curl.exe -L -o raylib.zip $RepoUrl

    if (-not (Test-Path "raylib.zip")) {
        Write-Host "[SETUP] Download failed!" -ForegroundColor Red
        exit 1
    }

    Write-Step "Download complete. Extracting..."

    # Clean and extract using bash
    bash -c "rm -rf lib/raylib raylib-5.5_win64_mingw-w64 && unzip -o -q raylib.zip && mkdir -p lib/raylib && mv raylib-5.5_win64_mingw-w64/* lib/raylib/ && rm -rf raylib-5.5_win64_mingw-w64 raylib.zip"

    # Verify installation
    if ((Test-Path "$installDir/include/raylib.h") -and (Test-Path "$installDir/lib/libraylib.a")) {
        Write-Step "raylib installed successfully to $installDir" -ForegroundColor Green
    } else {
        Write-Host "[SETUP] Installation verification failed!" -ForegroundColor Red
        exit 1
    }
}

Install-Raylib
