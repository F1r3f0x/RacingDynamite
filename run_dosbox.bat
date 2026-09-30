@echo off
rem Launch Ignition (1997) Rebuilt Pure C Engine in DOSBox
echo Launching DOSBox with MREBUILT.EXE...
cd /d "%~dp0Ignition\Ignition\DOSBOX"
start "" "DOSBox.exe" -conf "..\dosbox_rebuilt.conf"
