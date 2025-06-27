#!/bin/bash

# === CONFIG ===
PORT=5000
LOG_DIR="logs"
FLASK_LOG="$LOG_DIR/flask.log"
SERVER_LOG="$LOG_DIR/server.log"
APP_PATH="client_rest/app.py"

# === FUNCȚIE: oprește procesul existent pe port ===
function kill_process_on_port() {
  local pid=$(lsof -ti tcp:$1)
  if [ -n "$pid" ]; then
    echo "[AUTOFIX] Portul $1 este ocupat de PID $pid. Îl opresc..."
    kill -9 "$pid"
    echo "[AUTOFIX] Procesul de pe portul $1 a fost oprit."
  fi
}

# === FUNCȚIE: pornește serverul Flask în background ===
function start_flask_server() {
  echo "[FLASK] Pornim Flask pe portul $PORT..."
  nohup python3 "$APP_PATH" > "$FLASK_LOG" 2>&1 &
  echo "[FLASK] Flask rulează în background. Log: $FLASK_LOG"
}

# === FUNCȚIE: pornește serverul C ===
function start_c_server() {
  bash "$(dirname "$0")/start_server.sh"
}

# === VERIFICĂ EXISTENȚA ENDPOINTURILOR CRITICE ===
function verify_required_endpoints() {
  local missing=0
  for endpoint in "/cut" "/extract_audio"; do
    if ! grep -q "@app.route(\"$endpoint\"" "$APP_PATH"; then
      echo "❌ EROARE: Lipsă endpoint $endpoint în $APP_PATH"
      missing=1
    fi
  done

  if [ "$missing" -eq 1 ]; then
    echo "⛔ Flask NU va fi pornit. Repară fișierul și rulează din nou."
    return 1
  fi

  return 0
}

# === EXECUȚIE ===
mkdir -p "$LOG_DIR"
kill_process_on_port $PORT

start_c_server

if verify_required_endpoints; then
  start_flask_server
  echo "✅ Ambele servere rulează în background."
else
  echo "⚠️ Serverul C rulează, dar Flask NU a fost pornit."
fi

echo "   ▶ server.log: $SERVER_LOG"
echo "   ▶ flask.log : $FLASK_LOG"
# === COMPILEAZĂ ȘI PORNEȘTE CLIENT USER ===
if [ ! -f "./bin/client_user" ]; then
  echo "[CLIENT_USER] Nu există binarul client_user. Îl compilez..."
  gcc -o bin/client_user client_user/main.c client_user/user_interface.c client_user/command_sender.c client_user/file_transfer.c -Iinclude
  if [ $? -eq 0 ]; then
    echo "[CLIENT_USER] Compilare cu succes."
  else
    echo "[CLIENT_USER] EROARE la compilare! Verifică sursele."
    exit 1
  fi
fi
nohup ./bin/client_user > logs/client_user.log 2>&1 &
echo "[CLIENT_USER] Clientul user rulează în background. Log: logs/client_user.log"