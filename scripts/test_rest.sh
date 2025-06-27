#!/bin/bash

# filepath: /home/vboxuser/PCD/Proiect/scripts/test_rest.sh

echo "Testare server REST..."

# Testare operație "cut"
echo "Testare operație 'cut'..."
curl -X POST -H "Content-Type: application/json" \
    -d '{"input": "test.mp4", "start": "00:00:10", "end": "00:00:20", "output": "cut_test.mp4"}' \
    http://localhost:5000/cut

# Testare operație "extract_audio"
echo "Testare operație 'extract_audio'..."
curl -X POST -H "Content-Type: application/json" \
    -d '{"input": "test.mp4", "output": "audio_test.mp3"}' \
    http://localhost:5000/extract_audio

# Testare operație "convert"
echo "Testare operație 'convert'..."
curl -X POST -H "Content-Type: application/json" \
    -d '{"input": "test.mp4", "format": "avi", "output_base": "converted_test"}' \
    http://localhost:5000/convert

echo "Testele pentru serverul REST au fost finalizate."