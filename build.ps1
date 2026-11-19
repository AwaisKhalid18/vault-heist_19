# Vault Heist one-command build script.
#   ./build.ps1          configure + build (Release)
#   ./build.ps1 run      build then start the full game
#   ./build.ps1 dev      build then start Partner A's dev launcher
#   ./build.ps1 test     build then run the unit tests (output saved to docs/test_output_A.txt)
#   ./build.ps1 bench    build then run the benchmarks (CSV written to bench/results/)
#   ./build.ps1 clean    delete the build folder
param([string]$Task = "build")

# "Continue", not "Stop": VsDevCmd and SFML's own CMake files write harmless
# warnings to stderr, and with "Stop" PowerShell treats those as fatal errors.
# Every step below checks $LASTEXITCODE explicitly instead.
$ErrorActionPreference = "Continue"
Push-Location $PSScriptRoot
try {
    if ($Task -eq "clean") {
        if (Test-Path build) { Remove-Item -Recurse -Force build }
        Write-Host "Cleaned."
        return
    }

    # Find Visual Studio and load its developer environment (compiler, cmake, ninja).
    $vswhere = "${env:ProgramFiles(x86)}\Microsoft Visual Studio\Installer\vswhere.exe"
    if (-not (Test-Path $vswhere)) { throw "Visual Studio Build Tools not found (vswhere missing)." }
    $vs = & $vswhere -latest -products * -requires Microsoft.VisualStudio.Component.VC.Tools.x86.x64 -property installationPath
    if (-not $vs) { throw "No Visual Studio installation with C++ tools found." }

    $devCmd = Join-Path $vs "Common7\Tools\VsDevCmd.bat"
    cmd /c "`"$devCmd`" -arch=x64 -no_logo && set" | ForEach-Object {
        if ($_ -match "^(.*?)=(.*)$") { Set-Item -Path ("env:" + $Matches[1]) -Value $Matches[2] }
    }

    cmake -S . -B build -G Ninja -DCMAKE_BUILD_TYPE=Release
    if ($LASTEXITCODE -ne 0) { throw "CMake configure failed." }
    cmake --build build
    if ($LASTEXITCODE -ne 0) { throw "Build failed." }

    switch ($Task) {
        "run"   { & .\build\vault_heist.exe }
        "dev"   { & .\build\dev_a.exe }
        # run_tests writes docs\test_output_A.txt itself, so piping into the
        # same file here would just fight it for the file handle.
        "test"  { & .\build\run_tests.exe; exit $LASTEXITCODE }
        "bench" { & .\build\run_bench.exe }
        default { Write-Host "Build OK. Executables are in .\build\" }
    }
}
finally {
    Pop-Location
}
