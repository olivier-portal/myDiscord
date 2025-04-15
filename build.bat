@echo off
set BUILD_DIR=build

if not exist %BUILD_DIR% (
    mkdir %BUILD_DIR%
)

cd %BUILD_DIR%

cmake ..
if errorlevel 1 (
    echo Erreur lors de la configuration CMake.
    pause
    exit /b
)

cmake --build .
if exist myDiscord.exe (
    echo Build réussi. Lancement de l'application...
    myDiscord.exe
) else (
    echo Échec de la compilation.
)

pause
