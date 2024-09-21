@echo off

set "CFLAGS=/nologo /TC /std:clatest /Os"
set "WARNINGS=/Wall /wd4820 /wd5045 /wd4201"
set "LIBRARIES=gdi32.lib kernel32.lib msvcrt.lib opengl32.lib raylib.lib shell32.lib user32.lib winmm.lib"

pushd bin
cl %CFLAGS% %WARNINGS% ../example-game.c /I .. /Fe:game.exe /link %LIBRARIES% /NODEFAULTLIB:libcmt || popd && exit /b 69
popd