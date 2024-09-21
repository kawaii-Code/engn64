#include "engn64.h"
#include "utils.c"



const float32 player_movement_speed = 32.0f; // pixels/second
const int8    player_width          = 8;
const int8    player_height         = 8;
const uint32  player_color          = 0xFF668800;
const uint32  player_eye_color      = 0xFFFFFF00;
const float32 player_radius = 4.0f;

const float32 player_shot_cooldown  = 1.0f;
const uint32  player_bullet_color   = 0xFF00FFFF;
const float32 player_bullet_speed   = 32.0f;
const int8    player_bullet_radius  = 1;

const float32 asteroid_spawn_cooldown = 2.0f; // seconds
const uint32  asteroid_color          = 0xFF444444;

const float32 lose_animation_duration = 2.0f;



typedef struct {
    float32 remaining; // seconds
} Timer;

typedef struct {
    uint32 *pixels; // pixelformat: ABGR
    int8    width;
    int8    height;
} Sprite;

typedef struct {
    Sound shoot_sound;
    Sound lose_sound;
    Sound hit_sound;
    Music music;
} Game_Assets;

typedef struct {
    float32x2 position;
    float32x2 velocity;
    float32   radius;
} Asteroid;

typedef struct {
    float32x2 position;
    float32x2 velocity;
} Bullet;

typedef struct {
    Bullet *items;
    uint64  length;
    uint64  capacity;
} Dynamic_Array_Of_Bullets;

typedef struct {
    Asteroid *items;
    uint64    length;
    uint64    capacity;
} Dynamic_Array_Of_Asteroids;

typedef struct {
    Dynamic_Array_Of_Bullets player_bullets;

    float32   player_angle;
    float32x2 player_position;
    float32x2 player_look;

    Dynamic_Array_Of_Asteroids asteroids;
    Timer                      asteroid_spawn_timer;
    
    Timer   lose_animation_timer;
    float32 time_survived;

    LCG_Random random;

    Game_Assets assets;
} Game;



bool8 inside_screen(int8 x, int8 y) {
    return 0 <= y && y < 64 &&
           0 <= x && x < 64;
}

float32 distance_float32x2(float32x2 p1, float32x2 p2) {
    return distance_float32(p1.x, p1.y, p2.x, p2.y);
}

void draw_background(Screen *screen, uint32 color) {
    for (uint32 i = 0; i < 64 * 64; i++) {
        screen->pixels[i] = color;
    }
}

void draw_circle(Screen *screen, int8 x, int8 y, int8 radius, uint32 color) {
    for (int8 xx = x - radius; xx <= x + radius; xx++) {
        for (int8 yy = y - radius; yy <= y + radius; yy++) {
            if (inside_screen(xx, yy)) {
                if (distance_int8(xx, yy, x, y) <= (float32)radius) {
                    screen->pixels[yy * 64 + xx] = color;
                }
            }
        }
    }
}

void timer_tick(Timer *timer, float32 dt) {
    timer->remaining = max_float32(0.0f, timer->remaining - dt);
}

void restart(Game *game) {
    game->player_position = (float32x2){32, 32};
}

void init(Game *game) {
    game->random = (LCG_Random) { .seed = 5 };
    
    game->assets.music = LoadMusicStream("music.mp3");
    game->assets.music.looping = true;
    PlayMusicStream(game->assets.music);
    SetMusicVolume(game->assets.music, 0.5f);

    game->assets.shoot_sound = LoadSound("shoot.wav");
    game->assets.lose_sound  = LoadSound("lose.wav");
    game->assets.hit_sound   = LoadSound("boom.wav");
    restart(game);
}

void update_and_draw(Game *game, Screen *screen, Input *input, float32 dt) {
    UpdateMusicStream(game->assets.music);
    LCG_Random *random = &game->random;
    game->player_look = (float32x2){input->x, input->y};

    if (game->lose_animation_timer.remaining == 0) {        
        if (input->btn3.pressed) {
            float32x2 *p = &game->player_position;
            p->x += game->player_look.x * player_movement_speed * dt;
            p->y += game->player_look.y * player_movement_speed * dt;
            clamp_float32(&p->x, 0, 64);
            clamp_float32(&p->y, 0, 64);
        }

        if (input->btn2.just_pressed && !(game->player_look.x == 0 && game->player_look.y == 0)) {
            PlaySound(game->assets.shoot_sound);
            float32x2 start_position = game->player_position;
            start_position.x += 4 * game->player_look.x;
            start_position.y += 4 * game->player_look.y;
            dynamic_array_append(&game->player_bullets, ((Bullet) {
                .position = start_position,
                .velocity = game->player_look,
            }));
        }

        for (uint32 ai = 0; ai < game->asteroids.length; ai++) {
            Asteroid *a = &game->asteroids.items[ai];
            Vector2 center = {(float32)a->position.x, (float32)a->position.y};
            Rect_Float32 player_rect = {
                game->player_position.x - 4, game->player_position.y - 4,
                player_width, player_height};
            if (CheckCollisionCircleRec(center, a->radius - 1, player_rect)) {
                PlaySound(game->assets.lose_sound);
                game->lose_animation_timer.remaining = lose_animation_duration;
            }
        }
        game->time_survived += dt;
    } else {
        timer_tick(&game->lose_animation_timer, dt);
        if (game->lose_animation_timer.remaining == 0.0) {
            Game_Assets temp = game->assets;
            *game = (Game){0};
            game->assets = temp;
            restart(game);
            return;
        }
    }
    for (uint32 i = 0; i < game->player_bullets.length; i++) {
        Bullet *b = &game->player_bullets.items[i];
        b->position.x += player_bullet_speed * b->velocity.x * dt;
        b->position.y += player_bullet_speed * b->velocity.y * dt;
    }

    // Update asteroids
    Timer *asteroid_timer = &game->asteroid_spawn_timer;
    timer_tick(asteroid_timer, dt);
    if (asteroid_timer->remaining == 0) {
        asteroid_timer->remaining = asteroid_spawn_cooldown;

        float32 radius = rand_range_float32(random, 2.0f, 7.0f);
        // Select a position that won't instantly kill the player
        float32x2 position;
        do {
            position = (float32x2){rand_range_float32(random, 0, 64), rand_range_float32(random, 0, 64)};
        } while (distance_float32x2(position, game->player_position) < radius + player_radius + 1.0f);        
        float32 speed = rand_range_float32(random, 1.0f, 5.0f);
        float32x2 velocity = rand_point_on_circle(random);
        velocity.x *= speed;
        velocity.y *= speed;

        dynamic_array_append(&game->asteroids, ((Asteroid) {
            .position = position,
            .velocity = velocity,
            .radius = radius,
        }));
    }

    for (uint32 i = 0; i < game->asteroids.length; i++) {
        Asteroid *a = &game->asteroids.items[i];
        a->position.x += a->velocity.x * dt;
        a->position.y += a->velocity.y * dt;
    }

    // Check collisions between asteroids / bullets
    for (uint32 ai = 0; ai < game->asteroids.length; ai++) {
        for (uint32 bi = 0; bi < game->player_bullets.length; bi++) {
            Asteroid *a = &game->asteroids.items[ai];
            Bullet *b = &game->player_bullets.items[bi];
            if (CheckCollisionCircles((Vector2){a->position.x, a->position.y}, a->radius,
                                      (Vector2){b->position.x, b->position.y}, player_bullet_radius))
            {
                PlaySound(game->assets.hit_sound);
                dynamic_array_swap_remove(&game->player_bullets, bi);
                dynamic_array_swap_remove(&game->asteroids, ai);
            }
        }
    }

    // Remove things outside of the screen
    Rect_Float32 screen_rect = {0, 0, 64, 64};
    for (uint32 i = 0; i < game->asteroids.length; i++) {
        Asteroid *a = &game->asteroids.items[i];
        if (!CheckCollisionCircleRec((Vector2){a->position.x, a->position.y}, a->radius, screen_rect)) {
            dynamic_array_swap_remove(&game->asteroids, i);
        }
    }
    for (uint32 i = 0; i < game->player_bullets.length; i++) {
        Bullet *b = &game->player_bullets.items[i];
        if (!CheckCollisionCircleRec((Vector2){b->position.x, b->position.y}, player_bullet_radius, screen_rect)) {
            dynamic_array_swap_remove(&game->player_bullets, i);
        }
    }



    draw_background(screen, 0);

    // Draw player
    float32x2 p = game->player_position;
    for (int32 i = -4; i < 4; i++) {
        for (int32   j = -4; j < 4; j++) {
            int8 x = (int8)(p.x + i);
            int8 y = (int8)(p.y + j);
            if (inside_screen(x, y)) {
                screen->pixels[y * 64 + x] = player_color;
            }
        }
    }

    float32 mx = p.x + 2 * game->player_look.x;
    float32 my = p.y + 2 * game->player_look.y;
    for (int32 i = -1; i < 1; i++) {
        for (int32 j = -1; j < 1; j++) {
            int8 x = (int8)(mx + i);
            int8 y = (int8)(my + j);
            if (inside_screen(x, y)) {
                screen->pixels[y * 64 + x] = player_eye_color;
            }
        }
    }

    // Draw asteroids
    for (uint32 i = 0; i < game->asteroids.length; i++) {
        Asteroid *a = &game->asteroids.items[i];
        draw_circle(screen, (int8)a->position.x, (int8)a->position.y, (int8)a->radius, asteroid_color);
    }

    // Draw bullets
    for (uint32 i = 0; i < game->player_bullets.length; i++) {
        Bullet *b = &game->player_bullets.items[i];
        b->position.x += b->velocity.x * dt;
        b->position.y += b->velocity.y * dt;
        draw_circle(screen, (int8)b->position.x, (int8)b->position.y, 1, player_bullet_color);
    }

    float32 t = game->time_survived;
    DrawText(TextFormat("%02d:%02d", (int32)(t / 60.0f), (int32)t % 60), 0, 0, 19, WHITE);
}



#include "engn64.c"
