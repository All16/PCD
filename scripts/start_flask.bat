@echo off
REM Script pentru pornirea rapidă a serverului Flask (REST API)
cd /d %~dp0..
cd client_rest
start cmd /k python app.py
cd ..

echo Serverul Flask a fost pornit intr-o fereastra noua.
