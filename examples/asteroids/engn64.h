#ifndef ENGN_64_H
#define ENGN_64_H



#include "raylib.h"



typedef char               int8;
typedef unsigned char      uint8;
typedef short              int16;
typedef unsigned short     uint16;
typedef unsigned int       uint32;
typedef int                int32;
typedef unsigned long long uint64;
typedef long long          int64;
typedef float              float32;
typedef double             float64;
typedef int8               bool8;
typedef int32              bool32;

typedef union {
    int8 e[2];
    struct { int8 x, y; };
} int8x2;
typedef union {
    int32 e[2];
    struct { int32 x, y; };
} int32x2;
typedef union {
    float32 e[2];
    struct { float32 x, y; };
} float32x2;

typedef Rectangle Rect_Float32;
typedef struct {
    int8 x, y, width, height;
} Rect_Int8;
typedef struct {
    int32 x, y, width, height;
} Rect_Int32;

typedef struct {
    bool8 pressed;
    bool8 just_pressed;
} Input_Button;

// @improvement Rename btn* to triangle, square, ...?
typedef struct {
    float32      x;    // left stick x-axis
    float32      y;    // left stick y-axis
    Input_Button btn1; // triangle (y)
    Input_Button btn2; // square   (x)
    Input_Button btn3; // cross    (a)
    Input_Button btn4; // circle   (b)
} Input;

typedef struct {
    uint32 pixels[64 * 64]; // pixel format: ABGR
} Screen;

#define assert(x) if (!(x)) { *(volatile char *)0 = 0; }



#endif // ENGN_64_H
