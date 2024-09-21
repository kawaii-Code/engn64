CFLAGS="-g"

clang "$CFLAGS" -Wall -Wextra -Wpedantic -Wno-missing-braces gmtk.c bin/libraylib.a -o bin/game -lm -ldl -lpthread
