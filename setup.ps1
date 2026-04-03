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

    $cwd = $PWD.Path
    $installDir = Join-Path $cwd "lib/raylib"
    $zipFile = Join-Path $cwd "raylib.zip"
    
    if ((Test-Path (Join-Path $installDir "include/raylib.h")) -and (Test-Path (Join-Path $installDir "lib/libraylib.a"))) {
        Write-Step "raylib already installed at $installDir"
        return
    }

    Write-Step "raylib not found. Downloading raylib $RaylibVersion from GitHub..."

    # Download using curl.exe
    Write-Step "Downloading..."
    & curl.exe -L -o $zipFile $RepoUrl

    if (-not (Test-Path $zipFile)) {
        Write-Host "[SETUP] Download failed!" -ForegroundColor Red
        exit 1
    }

    Write-Step "Download complete. Extracting..."

    # Extract using bash (more reliable for this structure)
    Write-Step "Extracting..."
    $bashPath = "C:\Users\Epb\scoop\apps\git\current\usr\bin\bash.exe"
    $raylibFolder = "raylib-5.5_win64_mingw-w64"
    $extractCmd = "cd '$cwd' && rm -rf '$raylibFolder' && unzip -o -q 'raylib.zip' && rm -f 'raylib.zip'"
    & $bashPath -c $extractCmd
    
    # Move extracted folder to lib/raylib using PowerShell
    $extractedPath = Join-Path $cwd $raylibFolder
    if (Test-Path $extractedPath) {
        if (Test-Path $installDir) {
            Remove-Item -Path $installDir -Recurse -Force
        }
        Move-Item -Path $extractedPath -Destination $installDir
    }

    # Verify installation
    if ((Test-Path (Join-Path $installDir "include/raylib.h")) -and (Test-Path (Join-Path $installDir "lib/libraylib.a"))) {
        Write-Step "raylib installed successfully to $installDir" -ForegroundColor Green
    } else {
        Write-Host "[SETUP] Installation verification failed!" -ForegroundColor Red
        exit 1
    }
}

Install-Raylib
