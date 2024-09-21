#define TILEMAP_SINGLE_TILE_SIZE 4
#define TILEMAP_WIDTH      (64 / TILEMAP_SINGLE_TILE_SIZE)
#define TILEMAP_HEIGHT     (64 / TILEMAP_SINGLE_TILE_SIZE)
#define TILEMAP_TILE_COUNT (TILEMAP_WIDTH * TILEMAP_HEIGHT)

#define LEVELS_COUNT 16

const Rect_Int8 SCREEN_RECT = {0, 0, 64, 64};


// ABGR
const uint32 COLOR_PINK   = 0xFF6E00FF;
const uint32 COLOR_RED    = 0xFF0000FF;
const uint32 COLOR_BROWN  = 0xBB4444FF;
const uint32 COLOR_BLUE   = 0xFFFF0000;
const uint32 COLOR_CYAN   = 0xFFFFFF00;
const uint32 COLOR_BLACK  = 0xFF000000;
const uint32 COLOR_WHITE  = 0xFFFFFFFF;

const uint32 PLAYER_COLOR_MAIN  = 0xff826030;
const uint32 PLAYER_COLOR_FLASH = 0xffC2A070;
const uint32 PLAYER_COLOR_EYE   = 0xff000000;

const float32 PLAYER_FLASH_TIME = 0.2f;


const float32 PLAYER_HORIZONTAL_MOVEMENT_SPEED = 38.0f;
const float32 PLAYER_HORIZONTAL_TIME_PER_PX = 1 / PLAYER_HORIZONTAL_MOVEMENT_SPEED;


const float32 PLAYER_JUMP_BUFFER_TIME   = 0.25f;
const float32 PLAYER_COYOTE_TIME        = 0.18f;
const int32   PLAYER_JUMP_HEIGHT        = 9;
const float32 PLAYER_JUMP_TIME_TO_APEX  = 0.2f;
const float32 GRAVITY                   = 2 * PLAYER_JUMP_HEIGHT / (PLAYER_JUMP_TIME_TO_APEX * PLAYER_JUMP_TIME_TO_APEX);
const float32 PLAYER_INITIAL_JUMP_SPEED = GRAVITY * PLAYER_JUMP_TIME_TO_APEX;


const int32   PLAYER_CANDY_SCALE_INCREASE    = 8;
const float   PLAYER_COOLDOWN_BEFORE_SCALING = 1.2f;
const float32 PLAYER_SCALING_PER_SECOND      = 12.0f;
const float32 PLAYER_SCALE_TIME_PER_PX       = 1 / PLAYER_SCALING_PER_SECOND;



typedef struct {
    int8 x;
    int8 y;
} Tilemap_Solid_Blocks_Iterator;

typedef struct {
    float32 remaining; // seconds
} Timer;

typedef struct {
    uint32 *pixels; // pixelformat: ABGR
    int8    width;
    int8    height;
} Sprite;

typedef struct {
    Timer time_before_start;
    int32 remaining;
} Scaling;

typedef struct {
    float32 elapsed;
} Stopwatch;

typedef struct {
    Rect_Int8    rect;

    Stopwatch left;
    Stopwatch right;

    Stopwatch y_time;
    float32   velocity_y;

    Scaling      scaling;
    Stopwatch    scale_horizontally;
    Stopwatch    scale_vertically;

    Timer        flash_timer;
    bool8        flash_color;
    bool8        flashing;

    Timer        coyote_time;
    Timer        jump_buffer;
    bool8        was_falling_last_frame;
    bool8        looking_left;
    bool8        on_ground;
    bool8        has_key;
    bool8        next_scale_to_right;
} Player;

typedef struct {
    Rect_Int8 rect;
    Stopwatch y_time;
    float32   velocity_y;
} Box;

typedef struct {
    Box   *items;
    uint32 length;
    uint32 capacity;
} Dynamic_Array_Of_Boxes;

typedef enum {
    TILEMAP_TILE_TYPE_NONE = 0,
    TILEMAP_TILE_TYPE_BLOCK,
    TILEMAP_TILE_TYPE_COUNT,
} Tilemap_Tile_Type;

typedef enum {
    ORIENTATION_NONE = 0,
    ORIENTATION_N,
    ORIENTATION_NE,
    ORIENTATION_NW,
    ORIENTATION_COUNT,
} Tilemap_Tile_Orientation;

typedef struct {
    Tilemap_Tile_Type        tiles[TILEMAP_TILE_COUNT];
    Tilemap_Tile_Orientation orientations[TILEMAP_TILE_COUNT];
} Tilemap; 

typedef struct {
    Rect_Int8 rect;
} Candy;

typedef struct {
    Candy *items;
    uint32 length;
    uint32 capacity;
} Dynamic_Array_Of_Candies;

typedef enum {
    ENTITY_NONE   = 0,
    ENTITY_PLAYER = 1,
    ENTITY_BOX    = 2,
    ENTITY_TILE   = 3,
    ENTITY_COUNT  = 4,
    SCREEN        = 5,
} Entity;

typedef struct {
    Rect_Int8 rect;
} Door;

typedef struct {
    Door  *items;
    uint32 length;
    uint32 capacity;
} Dynamic_Array_Of_Doors;

typedef struct {
    Rect_Int8 *items;
    uint32     length;
    uint32     capacity;
} Dynamic_Array_Of_Rects;

typedef enum {
    NONE   = 0,
    LEFT   = 1,
    RIGHT  = 2,
    TOP    = 3,
    BOTTOM = 4,
} Collision_Side;

typedef struct {
    Entity e1;
    Entity e2;
    Collision_Side side; // from e1 to e2
    int32  param; // crutch
} Collision;

typedef struct {
    char                     title[64];
    Player                   player;
    Tilemap                  tilemap;
    Box   boxes[64];
    int32 boxes_length;
    Candy candies[64];
    int32 candies_length;
    Door door;
    Rect_Int8  key;
} Level;

typedef struct {
    Sprite player;
    Sprite box;
    Sprite candy;
    Sprite door;
    Sprite door_lit_up;
    Sprite dirt;
    Sprite dirt_n;
    Sprite dirt_nw;
    Sprite dirt_ne;
    Sprite key;
} Game_Sprites;

typedef struct {
    Sound jump;
    Sound scaling;
    Sound pickup;
    Sound next_level;
    Sound flash;
    Sound retry;
} Game_Sounds;


typedef struct {
    Level        current_level;
    Level        levels[LEVELS_COUNT];
    int32        current_level_index;
    float32      time_passed;
    Game_Sprites sprites;
    Game_Sounds  sounds;
} Game;

Game global_game;
Screen global_screen;
