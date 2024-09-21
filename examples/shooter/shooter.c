#include "engn64.h"
#include "sprites.h"
#include "engn64_lib.c"



const float32 player_movement_speed = 32.0f; // pixels/second
const float32 player_shot_cooldown  = 1.0f;

typedef struct {
    float32 remaining; // seconds
} Timer;

typedef struct {
    uint32 *pixels; // pixelformat: ABGR
    int8    width;
    int8    height;
} Sprite;

typedef struct {
    float32x2 position;
    float32x2 velocity;
} Bullet;

typedef struct {
    int8x2 position;
} Monster;

typedef struct {
    Bullet  bullets[64]; // 64 is a reasonable upper bound
    uint8   bullets_len;

    Monster monsters[64];
    uint8   monsters_len;

    float32x2 player_position;
    Timer     player_reload;

    LCG_Random random;

    Sprite  ghost;
    Sprite  gunman;
    Sprite  bullet;

    Sound   shot;
    Sound   kill;
    Sound   step;
} Game;

void timer_tick(Timer *timer, float32 dt) {
    timer->remaining = max_float32(0.0f, timer->remaining - dt);
}

bool8 inside_screen(int8 x, int8 y) {
    return 0 <= x && x < 64 &&
           0 <= y && y < 64;
}

void draw_sprite(Screen *screen, int8 x, int8 y, Sprite sprite) {
    for (int8 sx = 0; sx < sprite.width; sx++) {
        for (int8 sy = 0; sy < sprite.height; sy++) {
            if (!inside_screen(sx + x, sy + y)) {
                continue;
            }
        
            uint32 sprite_pixel = sprite.pixels[sy * sprite.width + sx];
            // TODO: Use actual alpha
            if (color_alpha(sprite_pixel) == 0) {
                continue;
            }
            screen->pixels[(sy + y) * 64 + (sx + x)] = sprite_pixel;
        }
    }
}

void draw_background(Screen *screen, uint32 color) {
    for (int i = 0; i < 64 * 64; i++) {
        screen->pixels[i] = color;
    }
}

void init(Game *game) {
    // TODO: Get a seed from somewhere
    game->random = (LCG_Random){ .seed = 69 };

    game->player_position = (float32x2){32, 32};

    game->monsters[game->monsters_len++] = (Monster){
        .position = { 16, 16 },
    };

    game->ghost = (Sprite){
        .pixels = ghost_pixels,
        .width  = ghost_width,
        .height = ghost_height,
    };
    game->gunman = (Sprite){
        .pixels = gunman_pixels,
        .width  = gunman_width,
        .height = gunman_height,
    };
    game->bullet = (Sprite){
        .pixels = bullet_pixels,
        .width  = bullet_width,
        .height = bullet_height,
    };

    // TODO: Also precompile sounds in C code
    game->shot = LoadSound("art/shot.mp3");
    game->kill = LoadSound("art/kill.mp3");
    game->step = LoadSound("art/step4.wav");
}

void update_and_draw(Game *game, Screen *screen, Input *input, float32 dt) {
    game->player_position.x += input->x * player_movement_speed * dt;
    game->player_position.y += input->y * player_movement_speed * dt;
    
    if (abs_float32(input->x) > 0.1f || abs_float32(input->y) > 0.1f) {
        if (!IsSoundPlaying(game->step)) {
            SetSoundPitch(game->step, 0.95f + 0.1f * rand_float32(&game->random));
            PlaySound(game->step);
        }
    } else {
        StopSound(game->step);
    }

    int8 xx = (int8)game->player_position.x;
    int8 yy = (int8)game->player_position.y;
    if (input->btn1.just_pressed) {
        if (game->player_reload.remaining == 0.0f) {
            PlaySound(game->shot);
            game->bullets[game->bullets_len++] = (Bullet){
                .position = {(float32)xx + 7, (float32)yy + 1},
                .velocity = {10.0f, 0.0f},
            };
            game->player_reload.remaining = player_shot_cooldown;
        }
    }

    timer_tick(&game->player_reload, dt);

    for (uint32 i = 0; i < game->bullets_len; i++) {
        Bullet *b = &game->bullets[i];
        b->position.x += b->velocity.x * dt;
        b->position.y += b->velocity.y * dt;        
        if (b->position.x > 64.0f) {
            swap_remove(game->bullets, &game->bullets_len, i);
        }
    }

    for (uint32 bi = 0; bi < game->bullets_len; bi++) {
        for (uint32 mi = 0; mi < game->monsters_len; mi++) {
            Rect_Float32 monster_rect = {game->monsters[mi].position.x,
                                         game->monsters[mi].position.y,
                                         game->ghost.width,
                                         game->ghost.height};
            Rect_Float32 bullet_rect = {game->bullets[bi].position.x,
                                        game->bullets[bi].position.y,
                                        game->bullet.width,
                                        game->bullet.height};
                                    
            if (rect_intersect_rect(monster_rect, bullet_rect)) {
                PlaySound(game->kill);
                swap_remove(game->monsters, &game->monsters_len, mi);
                swap_remove(game->bullets, &game->bullets_len, bi);
            }
        }
    }

    // Draw everything
    draw_background(screen, ColorToInt(WHITE));
    draw_sprite(screen, xx, yy, game->gunman);
    for (uint32 i = 0; i < game->monsters_len; i++) {
        Monster *m = &game->monsters[i];
        draw_sprite(screen, m->position.x, m->position.y, game->ghost);
    }
    for (uint32 i = 0; i < game->bullets_len; i++) {
        Bullet *b = &game->bullets[i];
        draw_sprite(screen, (int8)b->position.x, (int8)b->position.y, game->bullet);
    }

    // Check that we didn't accidentally overflow
    assert(game->monsters_len < 64);
    assert(game->bullets_len < 64);
}



#include "engn64.c"
