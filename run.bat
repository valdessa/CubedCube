@echo off
cd "D:\GitHub\CubedCube"

rem https://github.com/dolphin-emu/dolphin/blob/master/Source/Core/Core/Config/GraphicsSettings.cpp
rem Ejecuta Dolphin con el archivo .dol y establece el tamaño de la ventana
start "" "D:\GameCube\Dolphin-x64\Dolphin.exe" "--exec=D:\GitHub\CubedCube\CubedCube.dol"^
 --config=Graphics.Settings.ShowFPS=False^
 --batch 

exit
