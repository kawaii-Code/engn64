#define NOB_IMPLEMENTATION
#include "nob.h"

#define ARRAY_LENGTH(a) (sizeof(a) / sizeof(*a))

#ifdef _WIN32
const char *compiler_flags[] = {
    // TC specifies that all source files are C
    // https://learn.microsoft.com/en-us/cpp/build/reference/compiler-options-listed-by-category
    "/nologo", "/TC", "/std:clatest",
};

const char *linker_flags[] = {
    "/link", "/NODEFAULTLIB:libcmt", "/SUBSYSTEM:WINDOWS",
};

const char *libraries[] = {
    "gdi32.lib", "msvcrt.lib", "raylib.lib",
    "winmm.lib", "user32.lib", "shell32.lib",
};
#else
#error Only windows is supported
#endif // _WIN32


void append_compiler_flags(Nob_Cmd *cmd) {
    for (int i = 0; i < NOB_ARRAY_LEN(compiler_flags); i++) {
        nob_cmd_append(cmd, compiler_flags[i]);
    }
}

void append_linker_flags(Nob_Cmd *cmd) {
    for (int i = 0; i < NOB_ARRAY_LEN(linker_flags); i++) {
        nob_cmd_append(cmd, linker_flags[i]);
    }
}

void append_libraries(Nob_Cmd *cmd) {
    for (int i = 0; i < NOB_ARRAY_LEN(libraries); i++) {
        nob_cmd_append(cmd, libraries[i]);
    }
}

void usage(const char *program_name) {
    fprintf(stderr, "Usage: %s\n", program_name);
}

int compile_sprite(const char *name, const char *sprite_path, const char *output_path) {
    Nob_Cmd compile = {0};
    nob_cmd_append(&compile, "png2c.exe", "-n", name, "-o", output_path, sprite_path);
    if (!nob_cmd_run_sync(compile)) {
        return 1;
    }
}

int main(int argc, char **argv) {
    NOB_GO_REBUILD_URSELF(argc, argv);

    const char *program_name = nob_shift_args(&argc, &argv);
    if (argc > 0) {
        usage(program_name);
        return 1;
    }

    Nob_Cmd build_png2c = {0};
    nob_cmd_append(&build_png2c, "cl.exe", "png2c.c", "/Fe:png2c.exe");
    append_compiler_flags(&build_png2c);
    if (!nob_cmd_run_sync(build_png2c)) {
        return 1;
    }

    compile_sprite("bullet", "art/bullet.png", "bullet.h");
    compile_sprite("ghost", "art/ghost.png", "ghost.h");
    compile_sprite("gunman", "art/gunman.png", "gunman.h");

    Nob_Cmd cmd = {0};
    nob_cmd_append(&cmd, "cl.exe", "shooter.c", "/Fe:shooter.exe");
    append_compiler_flags(&cmd);
    append_linker_flags(&cmd);
    append_libraries(&cmd);
    if (!nob_cmd_run_sync(cmd)) {
        return 1;
    }
}
