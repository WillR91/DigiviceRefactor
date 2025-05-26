@echo off
echo Running DigiviceSim with debug output capture...
.\build\Debug\DigiviceSim.exe > debug_output.txt 2>&1
echo Application finished. Check debug_output.txt for output.
pause
