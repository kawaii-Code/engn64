#include <stdio.h>
#include <stdint.h>
#include <assert.h>
#include "raylib.h"

int main(int argc, char **argv) {
    char *program_name = argv[0];
    if (argc < 2) {
        fprintf(stderr, "usage: %s <filename>\n", program_name);
        return 1;
    }
    
    SetTraceLogLevel(LOG_NONE);
    char *filename = argv[1];
    Image image = LoadImage(filename);
    ImageFormat(&image, PIXELFORMAT_UNCOMPRESSED_R8G8B8A8);
    assert(image.width < 256);
    assert(image.height < 256);
    for (int y = 0; y < image.height; y++) {
        for (int x = 0; x < image.width; x++) {
            uint32_t pixel = ((uint32_t *)image.data)[y * image.width + x];
            // Prints in little endian (ABGR), but that's fine.
            printf("0x%08x,", pixel);
        }
        printf("\n");
    }
}