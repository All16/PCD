#!/bin/bash

# filepath: /home/vboxuser/PCD/Proiect/scripts/run_all.sh

echo "Pornire aplicație V-Edit..."

# Pornește serverul REST
echo "Pornire server REST..."
./server/server_rest &

# Pornește serverul UNIX
echo "Pornire server UNIX..."
./server/server_unix &

# Pornește serverul INET
echo "Pornire server INET..."
./server/server_inet &

# Așteaptă câteva secunde pentru inițializare
sleep 2

echo "Aplicația a fost pornită. Serverele rulează."
echo "Pentru a opri aplicația, utilizați comanda 'killall server_rest server_unix server_inet'."