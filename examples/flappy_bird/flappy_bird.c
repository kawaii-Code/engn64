#include "engn64.h"
#include "engn64_lib.c"

typedef struct {
    float32 x;
    float32 y;
} Pipe;

typedef struct {
    Pipe  *items;
    uint32 length;
    uint32 capacity;
} Dynamic_Array_Of_Pipes;

typedef struct {
    float32 x;
    float32 y;
    float32 velocity_y;
    float32 time_before_next_pipe_spawns;
    LCG_Random random;
    Dynamic_Array_Of_Pipes pipes;
} Game;

const float32 bird_jump_strength = 90.0;
const float32 bird_max_fall_speed = 120.0;
const float32 bird_gravity = 280.0;
const int32 bird_width = 6;
const int32 bird_height = 6;

const float32 pipe_spawn_cooldown = 3.0;
const float32 pipe_horizontal_speed = 26.0;
const float32 pipe_offset_from_center = 12.0;
const int32 pipe_width = 8.0;
const int32 pipe_height = 40.0;

void restart(Game *game) {
    dynamic_array_delete(&game->pipes);
    game->x = 64 * 0.25;
    game->y = 32;
    game->velocity_y = 0;
}

void init(Game *game) {
    game->random.seed = 420;
    restart(game);
}

void update_and_draw(Game *game, Screen *screen, Input *input, float32 dt) {
    Olivec_Canvas target = olivec_canvas(screen->pixels, 64, 64, 64);

    game->velocity_y -= bird_gravity * dt;
    if (input->btn1.just_pressed) {
        game->velocity_y = bird_jump_strength;
    }
    clamp_float32(&game->velocity_y, -1 * bird_max_fall_speed, bird_jump_strength);

    if (game->time_before_next_pipe_spawns == 0.0) {
        dynamic_array_append(&game->pipes, ((Pipe){
            .x = 64.0,
            .y = 32.0 + random_range_float32(&game->random, -8.0, 8.0),
        }));
        game->time_before_next_pipe_spawns = pipe_spawn_cooldown;
    }

    for (uint32 i = 0; i < game->pipes.length; i++) {
        Pipe *pipe = &game->pipes.items[i];
        pipe->x -= pipe_horizontal_speed * dt;

        float32 top_y = pipe->y - pipe_offset_from_center - pipe_height - 1;
        float32 bottom_y = pipe->y + pipe_offset_from_center;

        Rect_Float32 bird_rect = {
            .x = game->x,
            .y = game->y,
            .width = bird_width,
            .height = bird_height,
        };
        Rect_Float32 pipe1_rect = {
            .x = pipe->x,
            .y = top_y,
            .width = pipe_width,
            .height = pipe_height,
        };
        Rect_Float32 pipe2_rect = {
            .x = pipe->x,
            .y = bottom_y,
            .width = pipe_width,
            .height = pipe_height,
        };

        if (rect_intersect_rect_float32(bird_rect, pipe1_rect) ||
            rect_intersect_rect_float32(bird_rect, pipe2_rect)
        ) {
            restart(game);
        }
    }

    game->y -= game->velocity_y * dt;

    tick(&game->time_before_next_pipe_spawns, dt);

    olivec_fill(target, 0xFFEBCE87);
    olivec_rect(target, game->x, game->y, bird_width, bird_height, 0xFF00B4EB);
    for (uint32 i = 0; i < game->pipes.length; i++) {
        Pipe *pipe = &game->pipes.items[i];
        float32 top_y = pipe->y - pipe_offset_from_center - pipe_height - 1;
        float32 bottom_y = pipe->y + pipe_offset_from_center;

        olivec_rect(target, pipe->x, top_y, pipe_width, pipe_height, 0xFF00FF00);
        olivec_rect(target, pipe->x, bottom_y, pipe_width, pipe_height, 0xFF00FF00);
    }
    olivec_text(target, "flappy_bird", 0, 0, olivec_default_font, 1, 0xFF0000FF);
}

#include "engn64.c"
