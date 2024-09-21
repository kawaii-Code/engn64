@echo off

set "CFLAGS="
set "WARNINGS=-Wall -Wextra -Wpedantic -Wno-missing-braces -Werror -Wno-implicit-function-declaration"
set "LIBRARIES=-lgdi32 -lkernel32 -lmsvcrt -lopengl32 -lraylib -lshell32 -luser32 -lwinmm -ladvapi32"

clang %CFLAGS% %WARNINGS% -Os -nostdlib gmtk.c -o gmtk2024.exe %LIBRARIES% -Xlinker /NODEFAULTLIB:libcmt -Xlinker /SUBSYSTEM:windows || popd && exit /b 69
