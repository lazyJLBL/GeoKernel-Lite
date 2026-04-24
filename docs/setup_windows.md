# Windows Setup

This repository is validated first on Windows with MSYS2 `g++`.

## Required Tools

- CMake
- MSYS2 UCRT64 `g++`
- Python 3.11+

Install CMake with:

```powershell
winget install Kitware.CMake -e
```

Install Python dependencies:

```powershell
python -m pip install -r requirements.txt
```

Build and test:

```powershell
cmake -S . -B build
cmake --build build
ctest --test-dir build
pytest
```
