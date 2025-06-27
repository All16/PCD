@echo off
REM Script pentru pornirea serverului principal (C)
cd /d %~dp0..
cd bin
start cmd /k server.exe
cd ..

echo Serverul principal a fost pornit intr-o fereastra noua.
