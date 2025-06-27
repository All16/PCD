@echo off
REM Script pentru pornirea ambelor servere (C si Flask)
call "%~dp0start_server.bat"
call "%~dp0start_flask.bat"
echo Ambele servere au fost pornite.
