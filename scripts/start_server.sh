#!/bin/bash
# Script pentru pornirea serverului principal (C) pe Ubuntu
DIR="$(dirname "$0")/.."
cd "$DIR" || exit 1
./bin/server > logs/server.log 2>&1 &
echo "Serverul principal rulează în background. Log: logs/server.log"
