$ErrorActionPreference = "Stop"

function Test-Command($Name) {
    return $null -ne (Get-Command $Name -ErrorAction SilentlyContinue)
}

if (-not (Test-Command "cmake")) {
    Write-Host "CMake was not found. Installing Kitware CMake with winget..."
    winget install Kitware.CMake -e
}

if (-not (Test-Command "g++")) {
    Write-Host "g++ was not found in PATH. Install MSYS2 and add C:\msys64\ucrt64\bin to PATH."
}

if (-not (Test-Command "python")) {
    throw "Python was not found in PATH."
}

python -m pip install -r requirements.txt

Write-Host "Bootstrap complete. Run: cmake -S . -B build; cmake --build build; ctest --test-dir build; pytest"
