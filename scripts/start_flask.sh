#!/bin/bash
# Script pentru pornirea serverului Flask (REST API) pe Ubuntu
DIR="$(dirname "$0")/.."
cd "$DIR/client_rest" || exit 1
nohup python3 app.py > ../logs/flask.log 2>&1 &
echo "Serverul Flask rulează în background. Log: logs/flask.log"
