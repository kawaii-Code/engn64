#include "string.h"
#include "stdio.h"
#include "raylib.h"

int main(int argc, char **argv) {
    if (argc < 2) {
        fprintf(stderr, "Usage: %s <file>\n", argv[0]);
        return 1;
    }
    
    Wave wave = LoadWave(argv[1]);
    const char *sound_name = GetFileNameWithoutExt(argv[1]);
    
    int data_size = wave.frameCount * wave.channels * wave.sampleSize / 8;
    if (wave.sampleSize == 32) {
        printf("float %s_data[%d] = {\n", sound_name, data_size/4);
        for (int i = 0; i < data_size/4; i++) {
            printf("%.4ff, ", ((float *)wave.data)[i]);
        }
    } else {
        printf("unsigned char %s_data[%d] = {\n", sound_name, data_size);
        for (int i = 0; i < data_size; i++) {
            printf("0x%x, ", ((unsigned char *)wave.data)[i]);
        }
    }
    printf("\n};\n");
    printf("\n");
    printf("const Wave %s = (Wave){\n", sound_name);
    printf("    .frameCount = %d,\n", wave.frameCount);
    printf("    .sampleRate = %d,\n", wave.sampleRate);
    printf("    .sampleSize = %d,\n", wave.sampleSize);
    printf("    .channels   = %d,\n", wave.channels);
    printf("    .data       = (void*)%s_data,\n", sound_name);
    printf("};\n");
}