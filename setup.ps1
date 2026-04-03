# setup.ps1 - Auto-download raylib if not present
# Downloads raylib from GitHub master branch to lib/raylib/

param(
    [string]$RepoUrl = "https://github.com/raysan5/raylib/archive/refs/heads/master.zip",
    [string]$ZipFile = "raylib.zip",
    [string]$TempDir = "lib_temp",
    [string]$InstallDir = "lib/raylib"
)

function Write-Step($message) {
    Write-Host "[SETUP] $message" -ForegroundColor Cyan
}

function Install-Raylib() {
    Write-Step "Checking for raylib..."

    if ((Test-Path "$InstallDir/include/raylib.h") -and (Test-Path "$InstallDir/lib/libraylib.a")) {
        Write-Step "raylib already installed at $InstallDir"
        return
    }

    Write-Step "raylib not found. Downloading from GitHub (master branch)..."

    # Download
    Invoke-WebRequest -Uri $RepoUrl -OutFile $ZipFile
    Write-Step "Download complete. Extracting..."

    # Extract to temp
    Expand-Archive -Path $ZipFile -DestinationPath $TempDir -Force

    # Create lib/raylib/ structure
    New-Item -ItemType Directory -Path $InstallDir -Force | Out-Null
    New-Item -ItemType Directory -Path "$InstallDir/include" -Force | Out-Null
    New-Item -ItemType Directory -Path "$InstallDir/lib" -Force | Out-Null

    # Move contents
    $sourceDir = "$TempDir/raylib-master"
    Move-Item -Path "$sourceDir/*" -Destination "$InstallDir/" -Force

    # Cleanup
    Remove-Item -Path $ZipFile -Force -ErrorAction SilentlyContinue
    Remove-Item -Path $TempDir -Recurse -Force -ErrorAction SilentlyContinue

    Write-Step "raylib installed to $InstallDir" -ForegroundColor Green
}

Install-Raylib
