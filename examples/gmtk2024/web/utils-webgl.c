#define color_alpha(x) ((uint32)(x) >> 24)



////////////////////////////////////
// Math
float32 max_float32(float32 a, float32 b) {
    return a > b ? a : b;
}

float32 abs_float32(float32 a) {
    return a < 0 ? -a : a;
}

void clamp_float32(float32 *a, float32 min, float32 max) {
    assert(min < max);
    if (*a < min) {
        *a = min;
    } else if (*a > max) {
        *a = max;
    }
}



////////////////////////////////////
// Collision detection
bool8 rect_intersect_rect_float32(Rect_Float32 rect1, Rect_Float32 rect2) {
    return rect1.x < rect2.x + rect2.width  && rect1.x + rect1.width  > rect2.x &&
           rect1.y < rect2.y + rect2.height && rect1.y + rect1.height > rect2.y;
}

bool8 rect_intersect_rect_int8(Rect_Int8 rect1, Rect_Int8 rect2) {
    return rect1.x < rect2.x + rect2.width  && rect1.x + rect1.width  > rect2.x &&
           rect1.y < rect2.y + rect2.height && rect1.y + rect1.height > rect2.y;
}

bool8 point_intersect_rect_float32(float32 x, float32 y, Rect_Float32 rect) {
    return rect.x <= x && x < rect.x + rect.width &&
           rect.y <= y && y < rect.y + rect.height;
}

bool8 point_intersect_rect_int8(int8 x, int8 y, Rect_Int8 rect) {
    return rect.x <= x && x < rect.x + rect.width &&
           rect.y <= y && y < rect.y + rect.height;
}



////////////////////////////////////
// Random

// https://en.wikipedia.org/wiki/Linear_congruential_generator
typedef struct {
    uint64 seed;
} LCG_Random;

#define RANDOM_INT32_MAX (0x7fffffff)

// Stolen from musl libc
// https://elixir.bootlin.com/musl/v1.2.5/source/src/prng/rand.c#L11
int32 rand_int32(LCG_Random *random) {
    random->seed = 6364136223846793005ULL * random->seed + 1;
    return random->seed >> 33;
}

int32 rand_range_int32(LCG_Random *random, int32 min, int32 max) {
    return min + (rand_int32(random) % (max - min));
}

float32 rand_float32(LCG_Random *random) {
    return (float32)rand_int32(random) / (float32)RANDOM_INT32_MAX;
}

float32 rand_range_float32(LCG_Random *random, float32 min, float32 max) {
    assert(min < max);
    return min + (max - min) * rand_float32(random);
}




////////////////////////////////////
// Rectangles
Rect_Float32 to_rect_float32(Rect_Int32 r) {
    return (Rect_Float32) {
        .x      = (float32)r.x,
        .y      = (float32)r.y,
        .width  = (float32)r.width,
        .height = (float32)r.height,
    };
}

bool8 rect_intersect_rect(Rect_Float32 r1, Rect_Float32 r2) {
    return CheckCollisionRecs(r1, r2);
}

void my_memcpy(void *dest, void *src, uint32 n)
{
    char *csrc = (char *)src;
    char *cdest = (char *)dest;
    
    for (int i=0; i<n; i++) {
        cdest[i] = csrc[i];
    }
}

////////////////////////////////////
// Generic swap
void _swap_internal(void *a, void *b, void *t, uint32 size) {
    my_memcpy(t, a, size);
    my_memcpy(a, b, size);
    my_memcpy(b, t, size);
}
#define swap(a, b) _swap_internal(&(a), &(b), (char[(sizeof(a) == sizeof(b)) ? sizeof(a) : -1]){0}, sizeof(a))
#define swap_remove(arr, len, i) do { \
    swap(arr[i], arr[*(len) - 1]);    \
    (*len)--;                         \
} while (0)



////////////////////////////////////
// Dynamic array
//
// Example usage:
//
// typedef struct {
//     Something  *items;
//     uint64      length;
//     uint64      capacity;
// } Dynamic_Array_Of_Something;

// #define dynamic_array_append(da, item)                                                   \
//     do {                                                                                 \
//         if ((da)->length >= (da)->capacity) {                                            \
//             (da)->capacity = ((da)->capacity == 0) ? 64 : 2 * (da)->capacity;            \
//             (da)->items = realloc((da)->items, (da)->capacity * sizeof(*(da)->items)); \
//         }                                                                                \
//         (da)->items[(da)->length++] = (item);                                            \
//     } while (0)

// #define dynamic_array_delete(da) \
//     do {                         \
//         free((da)->items);     \
//         (da)->items    = 0;      \
//         (da)->length   = 0;      \
//         (da)->capacity = 0;      \
//     } while (0)

// #define dynamic_array_swap_remove(da, i) swap_remove((da)->items, &(da)->length, i)
