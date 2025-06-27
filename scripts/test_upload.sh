#!/bin/bash

# filepath: /home/vboxuser/PCD/Proiect/scripts/test_upload.sh

echo "Testare încărcare fișiere pe server..."

# Testare încărcare fișier video
echo "Încărcare fișier video..."
curl -X POST -F "file=@test.mp4" http://localhost:5000/upload

# Testare descărcare fișier procesat
echo "Descărcare fișier procesat..."
curl -O http://localhost:5000/download/cut_test.mp4

echo "Testele pentru încărcarea fișierelor au fost finalizate."