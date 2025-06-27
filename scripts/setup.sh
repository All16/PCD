#!/bin/bash

echo "Configurare mediu pentru V-Edit..."

# Navigăm în rădăcina proiectului
cd "$(dirname "$0")/.."

# Creare directoare corecte
echo "Creare directoare..."
mkdir -p bin logs tmp
mkdir -p videos/incoming videos/outgoing videos/processing

# Compilare cod
echo "Compilare cod sursă..."
make clean
make all

echo "Configurarea a fost finalizată."
