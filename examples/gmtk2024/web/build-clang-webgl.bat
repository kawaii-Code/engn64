@echo off

set "CFLAGS=-g"
set "WARNINGS=-Wall -Wextra -Wpedantic -Wno-missing-braces -Werror -Wno-implicit-function-declaration"
set "LIBRARIES=-lgdi32 -lkernel32 -lmsvcrt -lopengl32 -lraylib -lshell32 -luser32 -lwinmm -ladvapi32"

"\bin\wasi\bin\clang.exe" %CFLAGS% --target=wasm32 -O3 -nostdlib gmtk-webgl.c -o yapas.wasm -fno-builtin -fno-lto -mbulk-memory -Wl,--no-entry,--export-table,--allow-undefined,--export=main
