#!/bin/bash

# Dossier de build
BUILD_DIR="build"

# Crée le dossier s'il n'existe pas
mkdir -p $BUILD_DIR
cd $BUILD_DIR

# Lance CMake et compile
cmake ..
make

# Lancer l’exécutable s’il existe
if [ -f myDiscord.exe ]; then
    echo "✅ Build réussi. Lancement de l’application..."
    ./myDiscord.exe
else
    echo "❌ Échec de la compilation."
fi
