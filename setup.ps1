# setup.ps1 - Auto-download raylib if not present
# Downloads raylib 5.5 from GitHub releases to lib/raylib/

param(
    [string]$RaylibVersion = "5.5",
    [string]$RepoUrl = "https://github.com/raysan5/raylib/releases/download/5.5/raylib-5.5_win64_mingw-w64.zip",
    [string]$ZipFile = "raylib.zip"
)

function Write-Step($message) {
    Write-Host "[SETUP] $message" -ForegroundColor Cyan
}

function Write-Debug($message) {
    Write-Host "  [DEBUG] $message" -ForegroundColor Gray
}

function Write-Err($message) {
    Write-Host "[ERROR] $message" -ForegroundColor Red
}

# Early exit if all libs already exist (no output)
$cwd = $PWD.Path
$raylibDir = Join-Path $cwd "lib\raylib"
$tilesonDir = Join-Path $cwd "lib\tileson"
$raylibReady = (Test-Path (Join-Path $raylibDir "include\raylib.h")) -and (Test-Path (Join-Path $raylibDir "lib\libraylib.a"))
$tilesonReady = Test-Path (Join-Path $tilesonDir "tileson.hpp")

if ($raylibReady -and $tilesonReady) {
    exit 0
}

function Install-Raylib() {
    $cwd = $PWD.Path
    $installDir = Join-Path $cwd "lib\raylib"
    $zipPath = Join-Path $cwd $ZipFile
    $tempExtract = Join-Path $cwd "raylib-$($RaylibVersion)_win64_mingw-w64"
    
    Write-Step "Checking for raylib..."
    Write-Debug "Install directory: $installDir"
    
    if ((Test-Path (Join-Path $installDir "include\raylib.h")) -and 
        (Test-Path (Join-Path $installDir "lib\libraylib.a"))) {
        Write-Step "raylib already installed at $installDir"
        return
    }
    
    Write-Step "raylib not found. Downloading raylib $RaylibVersion from GitHub..."
    Write-Debug "URL: $RepoUrl"
    Write-Debug "Download path: $zipPath"
    
    Write-Debug "Downloading..."
    try {
        Invoke-WebRequest -Uri $RepoUrl -OutFile $zipPath -UserAgent "PowerShell"
    } catch {
        Write-Err "Download failed: $_"
        exit 1
    }
    
    if (-not (Test-Path $zipPath)) {
        Write-Err "Download failed - file not created"
        exit 1
    }
    
    Write-Step "Download complete. Extracting..."
    Write-Debug "Extracting to: $cwd"
    
    if (Test-Path $installDir) {
        Write-Debug "Removing existing $installDir"
        Remove-Item -Path $installDir -Recurse -Force -ErrorAction SilentlyContinue
    }
    if (Test-Path $tempExtract) {
        Write-Debug "Removing existing $tempExtract"
        Remove-Item -Path $tempExtract -Recurse -Force -ErrorAction SilentlyContinue
    }
    
    try {
        Expand-Archive -Path $zipPath -DestinationPath $cwd -Force
        Write-Debug "Archive extracted successfully"
    } catch {
        Write-Err "Extraction failed: $_"
        exit 1
    }
    
    if (Test-Path $tempExtract) {
        Write-Debug "Moving $tempExtract to $installDir"
        $libDir = Split-Path $installDir
        if (-not (Test-Path $libDir)) {
            New-Item -ItemType Directory -Path $libDir -Force | Out-Null
        }
        Move-Item -Path $tempExtract -Destination $installDir -Force
    } else {
        Write-Err "Extracted folder not found: $tempExtract"
        Write-Debug "Contents of ${cwd}:"
        Get-ChildItem $cwd | ForEach-Object { Write-Debug "  - $($_.Name)" }
        exit 1
    }
    
    Write-Debug "Cleaning up zip file"
    Remove-Item -Path $zipPath -Force -ErrorAction SilentlyContinue
    
    $headerPath = Join-Path $installDir "include\raylib.h"
    $libPath = Join-Path $installDir "lib\libraylib.a"
    
    if ((Test-Path $headerPath) -and (Test-Path $libPath)) {
        Write-Step "raylib installed successfully to $installDir" -ForegroundColor Green
    } else {
        Write-Err "Installation verification failed!"
        Write-Debug "Header exists: $(Test-Path $headerPath)"
        Write-Debug "Library exists: $(Test-Path $libPath)"
        exit 1
    }
}

function Install-Tileson() {
    $cwd = $PWD.Path
    $tilesonDir = Join-Path $cwd "lib\tileson"
    $tilesonFile = Join-Path $tilesonDir "tileson.hpp"
    
    Write-Step "Checking for tileson..."
    Write-Debug "Tileson path: $tilesonFile"
    
    if (Test-Path $tilesonFile) {
        Write-Step "tileson.hpp already exists at $tilesonDir"
        return
    }
    
    Write-Step "tileson.hpp not found!"
    Write-Host "  Please download tileson from: https://github.com/SSBMTonberry/tileson" -ForegroundColor Yellow
    Write-Host "  Copy 'tileson.hpp' to: lib/tileson/" -ForegroundColor Yellow
}

function Remove-OldRaylib() {
    $cwd = $PWD.Path
    $oldRaylib = Join-Path $cwd "raylib"
    
    if (Test-Path $oldRaylib) {
        Write-Step "Removing old bundled raylib folder..."
        Write-Debug "Removing $oldRaylib"
        Remove-Item -Path $oldRaylib -Recurse -Force -ErrorAction SilentlyContinue
        Write-Step "Old raylib folder removed" -ForegroundColor Green
    }
}

Remove-OldRaylib
Install-Raylib
Install-Tileson
