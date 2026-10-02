@echo off
rem Launch Ignition (1997) Original Executable (MAINDOS.EXE) in DOSBox
echo Launching DOSBox with MAINDOS.EXE...
cd /d "%~dp0Ignition\Ignition\DOSBOX"
start "" "DOSBox.exe" -conf "..\dosbox_original.conf"
