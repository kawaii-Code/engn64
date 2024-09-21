#include "engn64.h"
#include "utils-webgl.c"

#include "gmtk-sprites.c"
#include "gmtk-sounds.c"
#include "gmtk-types-webgl.c"
#include "gmtk-levels.c"

// Yet Another Puzzle About Scaling
// YAPAS


////////////////////////
// Garbage
//
void timer_tick(Timer *timer, float32 dt) {
    timer->remaining = max_float32(0.0f, timer->remaining - dt);
}

bool8 inside_screen(int8 x, int8 y) {
    return 0 <= y && y < 64 &&
           0 <= x && x < 64;
}





////////////////////////
// Casts
int8 floor_float32_to_int8(float32 x) {
    assert(-128 <= x && x <= 127);
    return (int8)(x);
}

int8 ceil_float32_to_int8(float32 x) {
    return floor_float32_to_int8(x + 0.99f);
}

int8 round_float32_to_int8(float32 x) {
    assert(-128 <= x && x <= 127);
    return floor_float32_to_int8(x + 0.5f);
}

Rect_Int8 round_rect_float32_to_rect_int8(Rect_Float32 rect) {    
    return (Rect_Int8) {
        .x = round_float32_to_int8(rect.x),
        .y = round_float32_to_int8(rect.y),
        .width  = floor_float32_to_int8(rect.width),
        .height = floor_float32_to_int8(rect.height),
    };
}

Rect_Int8 floor_rect_float32_to_rect_int8(Rect_Float32 rect) {    
    return (Rect_Int8) {
        .x = floor_float32_to_int8(rect.x),
        .y = floor_float32_to_int8(rect.y),
        .width  = floor_float32_to_int8(rect.width),
        .height = floor_float32_to_int8(rect.height),
    };
}

Rect_Int8 rect_float32_to_rect_int8_rounded(Rect_Float32 rect) {    
    return (Rect_Int8) {
        .x = round_float32_to_int8(rect.x),
        .y = round_float32_to_int8(rect.y),
        .width  = floor_float32_to_int8(rect.width),
        .height = floor_float32_to_int8(rect.height),
    };
}

Rect_Float32 rect_int8_to_rect_float32(Rect_Int8 rect) {    
    return (Rect_Float32) {
        .x = rect.x,
        .y = rect.y,
        .width  = rect.width,
        .height = rect.height,
    };
}

Rect_Int8 tilemap_tile_rect(int8 x, int8 y) {
    return (Rect_Int8) {
        .x = x * TILEMAP_SINGLE_TILE_SIZE,
        .y = y * TILEMAP_SINGLE_TILE_SIZE,
        .width  = TILEMAP_SINGLE_TILE_SIZE,
        .height = TILEMAP_SINGLE_TILE_SIZE,
    };
}
bool8 advance_iterator(Tilemap_Solid_Blocks_Iterator *it) {
    it->x++;
    if (it->x == TILEMAP_WIDTH) {
        it->x = 0;
        it->y++;
        if (it->y == TILEMAP_HEIGHT) {
            return false;
        }
    }
    return true;
}

#include "gmtk-physics.c"



////////////////////////
// Drawing
void draw_background(Screen *screen, uint32 color) {
    for (uint32 i = 0; i < 64 * 64; i++) {
        screen->pixels[i] = color;
    }
}

void draw_sprite_extended(Screen *screen, int8 x, int8 y, Sprite sprite, bool8 flip_x) {
    for (int8 sx = 0; sx < sprite.width; sx++) {
        for (int8 sy = 0; sy < sprite.height; sy++) {
            int8 pixel_x;
            if (flip_x) {
                pixel_x = sprite.width - sx - 1 + x;
            } else {
                pixel_x = sx + x;
            }
            int8 pixel_y = sy + y;

            if (!inside_screen(pixel_x, pixel_y)) {
                continue;
            }

            uint32 sprite_pixel = sprite.pixels[sy * sprite.width + sx];
            // TODO: Use actual alpha
            if (color_alpha(sprite_pixel) == 0) {
                continue;
            }
            screen->pixels[pixel_y * 64 + pixel_x] = sprite_pixel;
        }
    }
}

void draw_sprite(Screen *screen, int8 x, int8 y, Sprite sprite) {
    draw_sprite_extended(screen, x, y, sprite, false);
}

void draw_rectangle(Screen *screen, Rect_Int8 rect, uint32 color) {
    for (int8 xx = rect.x; xx < rect.x + rect.width; xx++) {
        for (int8 yy = rect.y; yy < rect.y + rect.height; yy++) {
            if (inside_screen(xx, yy)) {
                screen->pixels[yy * 64 + xx] = color;
            }
        }
    }
}

void draw_pixel(Screen *screen, int8 x, int8 y, uint32 color) {
    if (inside_screen(x, y)) {
        screen->pixels[y * 64 + x] = color;
    }
}



void world_to_tilemap_position(float32 x, float32 y, int8 *out_x, int8 *out_y) {
    *out_x = x / TILEMAP_SINGLE_TILE_SIZE;
    *out_y = y / TILEMAP_SINGLE_TILE_SIZE;
    assert(0 <= *out_x && *out_x < TILEMAP_WIDTH);
    assert(0 <= *out_y && *out_y < TILEMAP_HEIGHT);
}

void clamp_rect_inside_rect_float32(Rect_Float32 *rect, Rect_Float32 bounds) {
    clamp_float32(&rect->x, bounds.x, bounds.x + bounds.width);
    clamp_float32(&rect->y, bounds.y, bounds.y + bounds.height);
    if (rect->x + rect->width >= bounds.x + bounds.width) {
        rect->x = bounds.x + bounds.width - rect->width;
    }
    if (rect->y + rect->height >= bounds.y + bounds.height) {
        rect->y = bounds.y + bounds.height - rect->height - 1;
    }
}

bool8 point_intersect_tilemap_float32(float32 x, float32 y, Tilemap *tilemap) {
    for (int8 yy = 0; yy < TILEMAP_HEIGHT; yy++) {
        for (int8 xx = 0; xx < TILEMAP_WIDTH; xx++) { 
            if (tilemap->tiles[yy * TILEMAP_WIDTH + xx] == TILEMAP_TILE_TYPE_BLOCK) {
                Rect_Float32 tile_rect = (Rect_Float32) {
                    .x = xx * TILEMAP_SINGLE_TILE_SIZE,
                    .y = yy * TILEMAP_SINGLE_TILE_SIZE,
                    .width  = TILEMAP_SINGLE_TILE_SIZE,
                    .height = TILEMAP_SINGLE_TILE_SIZE,
                };
                if (point_intersect_rect_float32(x, y, tile_rect)) {
                    return true;
                }
            }
        }
    }
    return false;
}

bool8 rect_intersect_tilemap_float32(Rect_Float32 rect, Tilemap *tilemap) {
    for (int8 y = 0; y < TILEMAP_HEIGHT; y++) {
        for (int8 x = 0; x < TILEMAP_WIDTH; x++) { 
            if (tilemap->tiles[y * TILEMAP_WIDTH + x] == TILEMAP_TILE_TYPE_BLOCK) {
                Rect_Float32 tile_rect = (Rect_Float32) {
                    .x = x * TILEMAP_SINGLE_TILE_SIZE,
                    .y = y * TILEMAP_SINGLE_TILE_SIZE,
                    .width  = TILEMAP_SINGLE_TILE_SIZE,
                    .height = TILEMAP_SINGLE_TILE_SIZE,
                };
                if (rect_intersect_rect_float32(rect, tile_rect)) {
                    return true;
                }
            }
        }
    }
    return false;
}

bool8 rect_intersect_tilemap_int8(Rect_Int8 rect, Tilemap *tilemap) {
    for (int8 y = 0; y < TILEMAP_HEIGHT; y++) {
        for (int8 x = 0; x < TILEMAP_WIDTH; x++) { 
            if (tilemap->tiles[y * TILEMAP_WIDTH + x] == TILEMAP_TILE_TYPE_BLOCK) {
                Rect_Int8 tile_rect = (Rect_Int8) {
                    .x = x * TILEMAP_SINGLE_TILE_SIZE,
                    .y = y * TILEMAP_SINGLE_TILE_SIZE,
                    .width  = TILEMAP_SINGLE_TILE_SIZE,
                    .height = TILEMAP_SINGLE_TILE_SIZE,
                };
                if (rect_intersect_rect_int8(rect, tile_rect)) {
                    return true;
                }
            }
        }
    }
    return false;
}

bool8 is_rect_on_ground(Rect_Int8 rect, Level *level) {
    Rect_Int8 rect_1_pixel_below = rect;
    rect_1_pixel_below.height = 1;
    rect_1_pixel_below.y += rect.height;

    for (uint32 i = 0; i < level->boxes_length; i++) {
        Box *box = &level->boxes[i];
        if (rects_equal(box->rect, rect)) {
            continue;
        }
        if (rect_intersect_rect_int8(rect_1_pixel_below, box->rect)) {
            return true;
        }
    }

    if (!rects_equal(level->player.rect, rect) &&
        rect_intersect_rect_int8(level->player.rect, rect_1_pixel_below))
    {
        return true;
    }

    return rect_intersect_tilemap_int8(rect_1_pixel_below, &level->tilemap) ||
           rect.y + rect.height >= 64;
}



////////////////////////
// Main game functions
Level level_from_string(const char *string) {
    Level level = {0};

    int32 title_size = 0;
    for (title_size = 0; string[title_size] != '\n'; title_size++) {
        level.title[title_size] = string[title_size];
    }
    title_size++;

    Tilemap_Tile_Orientation orientation_table[256] = {ORIENTATION_NONE};
    orientation_table['|'] = ORIENTATION_NE;
    orientation_table['/'] = ORIENTATION_NW;
    orientation_table['='] = ORIENTATION_N;

    for (int8 y = 0; y < TILEMAP_HEIGHT; y++) {
        for (int8 x = 0; x < TILEMAP_WIDTH; x++) {            
            Rect_Int8 tile_rect = tilemap_tile_rect(x, y);

            uint8 c = string[title_size + y * TILEMAP_WIDTH + x]; 
            switch (c) {
                case '/':
                case '|':
                case '=':
                case '#': {
                    level.tilemap.tiles[y * TILEMAP_WIDTH + x] = TILEMAP_TILE_TYPE_BLOCK;
                    level.tilemap.orientations[y * TILEMAP_WIDTH + x] = orientation_table[c];
                } break;

                case 'd': {
                    Rect_Int8 door_rect = tile_rect;
                    door_rect.y -= 4;
                    door_rect.height += 4;
                    level.door = (Door) {
                        .rect = door_rect,
                    };
                } break;

                case 'b': {
                    level.boxes[level.boxes_length++] =(Box) {
                        .rect = tile_rect,
                    };
                } break;

                case 'p': {
                    level.player.rect = tile_rect;
                } break;

                case 'c': {
                    Rect_Int8 candy_rect = tile_rect;
                    level.candies[level.candies_length++] = (Candy) {
                        .rect = candy_rect,
                    };
                } break;

                case 'k': {
                    Rect_Int8 key_rect = tile_rect;
                    key_rect.y += 1;
                    level.key = key_rect;
                } break;
            }
        }
    }

    return level;
}

float32 lerp_float32(float32 a, float32 b, float32 t) {
    return a + (b - a) * t;
}

float32 min_float32(float32 a, float32 b) {
    return a < b ? a : b;
}

typedef struct {
    bool8          is_intersecting;
    Collision_Side side;
} Intersection;

Intersection rect_intersect_tilemap(Rect_Int8 rect, Tilemap *tilemap) {
    Intersection result = {0};
    Tilemap_Solid_Blocks_Iterator it = {0};
    do {
        if (tilemap->tiles[it.y * TILEMAP_WIDTH + it.x] == TILEMAP_TILE_TYPE_BLOCK) {
            Rect_Int8 tile_rect = tilemap_tile_rect(it.x, it.y);
            if (rect_intersect_rect_int8(rect, tile_rect)) {
                result.is_intersecting = true;
                if (rect.y < tile_rect.y) {
                    result.side = BOTTOM;
                }
                if (rect.y > tile_rect.y) {
                    result.side = TOP;
                }
                return result;
            }
        }
    } while (advance_iterator(&it));

    return result;
}

Rect_Float32 combine_two_rects(Rect_Float32 r1, Rect_Float32 r2) {
    return (Rect_Float32) {
        .x = min_float32(r1.x, r2.x),
        .y = min_float32(r1.y, r2.y),
        .width = r1.width + r2.width,
        .height = r1.height + r2.height,
    };
}

void stopwatch_tick(Stopwatch *stopwatch, float32 dt) {
    stopwatch->elapsed += dt;
}

void reload_current_level(Game *game) {
    Level original_level = game->levels[game->current_level_index];
    
    Level copy = {0};
    
    
    for (uint32 i = 0; i < 64; i++) {
        copy.title[i] = original_level.title[i];
    }
    

    for (uint32 i = 0; i < TILEMAP_TILE_COUNT; i++) {
        copy.tilemap.tiles[i] = original_level.tilemap.tiles[i];
        copy.tilemap.orientations[i] = original_level.tilemap.orientations[i];
    }
    

    for (uint32 i = 0; i < original_level.boxes_length; i++) {
        copy.boxes[i] = original_level.boxes[i];
    }
    for (uint32 i = 0; i < original_level.candies_length; i++) {
        copy.candies[i] = original_level.candies[i];
    }
    copy.player = original_level.player;
    copy.door = original_level.door;
    copy.key = original_level.key;
    copy.boxes_length = original_level.boxes_length;
    copy.candies_length = original_level.candies_length;
    
    game->current_level = copy;
}

void init(void) {
    Game *game = &global_game;
    *game = (Game){0};
    global_screen = (Screen){0};

    game->levels[0]  = level_from_string(LEVEL1);
    game->levels[1]  = level_from_string(LEVEL2);
    game->levels[2]  = level_from_string(LEVEL3);
    game->levels[3]  = level_from_string(LEVEL4);
    game->levels[4]  = level_from_string(LEVEL5);
    game->levels[5]  = level_from_string(LEVEL6);
    game->levels[6]  = level_from_string(LEVEL7);
    game->levels[7]  = level_from_string(LEVEL8);
    game->levels[8]  = level_from_string(LEVEL9);
    game->levels[9]  = level_from_string(LEVEL10);
    game->levels[10] = level_from_string(LEVEL11);
    game->levels[11] = level_from_string(LEVEL12);
    game->levels[12] = level_from_string(LEVEL13);
    game->levels[13] = level_from_string(LEVEL14);
    game->levels[14] = level_from_string(LEVEL15);
    game->levels[15] = level_from_string(LEVEL16);
    game->current_level_index = 0;

    game->sprites = (Game_Sprites){
        .player = (Sprite) {
            .width = 4,
            .height = 4,
            .pixels = player_pixels,
        },
        .box = (Sprite) {
            .width = 4,
            .height = 4,
            .pixels = box_pixels,
        },
        .candy = (Sprite) {
            .width = 4,
            .height = 4,
            .pixels = candy_pixels,
        },
        .dirt = (Sprite) {
            .width = 4,
            .height = 4,
            .pixels = dirt_pixels,
        },
        .dirt_n = (Sprite) {
            .width = 4,
            .height = 4,
            .pixels = dirt_n_pixels,
        },
        .dirt_nw = (Sprite) {
            .width = 4,
            .height = 4,
            .pixels = dirt_nw_pixels,
        },
        .dirt_ne = (Sprite) {
            .width = 4,
            .height = 4,
            .pixels = dirt_ne_pixels,
        },
        .door = (Sprite) {
            .width = 4,
            .height = 8,
            .pixels = door_pixels,
        },
        .door_lit_up = (Sprite) {
            .width = 4,
            .height = 8,
            .pixels = door_lit_up_pixels,
        },
        .key = (Sprite) {
            .width = 4,
            .height = 4,
            .pixels = key_pixels,
        }
    };

    //game->sounds = (Game_Sounds) {
    //    .jump = LoadSoundFromWave(jump),
    //    .scaling = LoadSoundFromWave(scaling),
    //    .pickup = LoadSoundFromWave(pickup),
    //    .next_level = LoadSoundFromWave(next_level),
    //    .flash = LoadSoundFromWave(flash),
    //    .retry = LoadSoundFromWave(retry),
    //};

    reload_current_level(game);
}

void draw_all_pixels_to_the_canvas_through_the_madness_of_wasm_and_js(uint32 *pixels);

void update_and_draw() {
    BeginDrawing();
    float32 dt = GetFrameTime();
    Game *game = &global_game;
    Screen local_screen= {0};
    Screen *screen = &local_screen;
    Level *current_level = &game->current_level;
    Player *player   = &current_level->player;
    Tilemap *tilemap = &current_level->tilemap;

    Candy *candies = current_level->candies;
    int32 *candies_length = &current_level->candies_length;
    
    Box *boxes = current_level->boxes;
    int32 *boxes_length = &current_level->boxes_length;


    if (IsKeyPressed(KEY_X) || IsKeyPressed(KEY_R)) {
        reload_current_level(game);
        // PlaySound(game->sounds.retry);
    }
    if (IsKeyPressed(KEY_P) || IsKeyPressed(KEY_COMMA)) {
        if (game->current_level_index > 0) {
            game->current_level_index -= 1;
            // PlaySound(game->sounds.next_level);
        }
        reload_current_level(game);
    }
    if (IsKeyPressed(KEY_N) || IsKeyPressed(KEY_PERIOD)) {
        if (game->current_level_index < LEVELS_COUNT) {
            game->current_level_index += 1;
            // PlaySound(game->sounds.next_level);
        }
        reload_current_level(game);
    }

    if (IsKeyDown(KEY_D) || IsKeyDown(KEY_RIGHT)) {
        stopwatch_tick(&player->right, dt);
        player->looking_left = false;
    } else {
        player->right.elapsed = 0.0f;
    }
    if (IsKeyDown(KEY_A) || IsKeyDown(KEY_LEFT)) {
        stopwatch_tick(&player->left, dt);
        player->looking_left = true;
    } else {
        player->left.elapsed = 0.0f;
    }

    // update player
    {
        // Trigger collisions
        for (uint32 i = 0; i < *candies_length; i++) {
            Candy *candy = &candies[i];
            if (rect_intersect_rect_int8(player->rect, candy->rect)) {
                swap_remove(candies, candies_length, i);
                if (player->scaling.remaining == 0) {
                    player->scaling.time_before_start.remaining = PLAYER_COOLDOWN_BEFORE_SCALING;
                    player->flashing = true;
                }
                player->scaling.remaining += PLAYER_CANDY_SCALE_INCREASE;
            }
        }



        while (player->right.elapsed > PLAYER_HORIZONTAL_TIME_PER_PX ||
               player->left.elapsed  > PLAYER_HORIZONTAL_TIME_PER_PX)
        {
            Physics_Step player_step_x = { .before = player->rect, .after = player->rect };

            int delta = 0;
            if (player->right.elapsed > PLAYER_HORIZONTAL_TIME_PER_PX) {
                delta = 1;
                player->right.elapsed -= PLAYER_HORIZONTAL_TIME_PER_PX;
            }
            if (player->left.elapsed > PLAYER_HORIZONTAL_TIME_PER_PX) {
                delta = -1;
                player->left.elapsed -= PLAYER_HORIZONTAL_TIME_PER_PX;
            }

            player_step_x.after.x += delta;

            Simulate_Step_Result result = simulate_step(current_level, player_step_x);
            if (!result.has_conflict) {
                player->rect = player_step_x.after;
            } else {
                Rect_Int8 *rects[64];
                int32 rects_count = 1;
                rects[0] = &player->rect;

                bool8 ok = true;
                for (int i = 0; i < result.conflicts_count; i++) {
                    Physics_Conflict conflict = result.conflicts[i];
                    if (conflict.other != ENTITY_BOX) {
                        ok = false;
                        break;
                    }
                    while (conflict.other == ENTITY_BOX) {
                        Box *box = &boxes[conflict.param];
                        rects[rects_count++] = &box->rect;

                        Physics_Step box_step = { .before = box->rect, .after = box->rect };
                        box_step.after.x += delta;
                        Simulate_Step_Result multi_step_result = simulate_step(current_level, box_step);
                        if (!multi_step_result.has_conflict) {
                            break;
                        } else {
                            conflict = multi_step_result.conflicts[0];
                            if (conflict.other != ENTITY_BOX) {
                                ok = false;
                                break;
                            }
                        }
                    }
                }

                if (ok) {
                    // Move all affected boxes and player
                    for (int i = 0; i < rects_count; i++) {
                        rects[i]->x += delta;
                    }
                }
            }
        }

        bool8 player_is_on_ground = is_rect_on_ground(player->rect, current_level);
        bool8 *player_was_falling_last_frame = &player->was_falling_last_frame;
        if (!player_is_on_ground) {
            player->velocity_y -= GRAVITY * dt;
            bool8 player_was_on_ground = !*player_was_falling_last_frame;
            if (player_was_on_ground && player->velocity_y < 0.0f) {
                player->coyote_time.remaining = PLAYER_COYOTE_TIME;
            }
        }
        if (IsKeyPressed(KEY_Z) || IsKeyPressed(KEY_SPACE) || IsKeyPressed(KEY_UP) || IsKeyPressed(KEY_W)) {
            player->jump_buffer.remaining = PLAYER_JUMP_BUFFER_TIME;
        }
        *player_was_falling_last_frame = !player_is_on_ground;
        if (player->jump_buffer.remaining > 0) {
            if (player_is_on_ground || player->coyote_time.remaining > 0.0f) {
                // PlaySound(game->sounds.jump);
                player->velocity_y = PLAYER_INITIAL_JUMP_SPEED;
                player->coyote_time.remaining = 0.0f;
                player->jump_buffer.remaining = 0.0f;
                player->y_time.elapsed = 0.0f;
                *player_was_falling_last_frame = false;
            }
        }

        if (player->velocity_y != 0) {
            float32 time_per_px;
            int delta = 0;
            if (player->velocity_y < 0) {
                time_per_px = -1 / player->velocity_y;
                delta = 1;
            } else {
                time_per_px = 1 / player->velocity_y;
                delta = -1;
            }

            stopwatch_tick(&player->y_time, dt);
            while (player->y_time.elapsed > time_per_px) {
                Physics_Step player_step_y = { .before = player->rect, .after = player->rect };
                player_step_y.after.y = player->rect.y + delta;
                Simulate_Step_Result result = simulate_step(current_level, player_step_y);
                if (!result.has_conflict) {
                    player->rect.y = player_step_y.after.y;
                    player->y_time.elapsed -= time_per_px;
                } else {
                    player->velocity_y = 0.0f;
                    player->y_time.elapsed = 0.0f;
                }
            }
        }

        if (player->scaling.remaining > 0) {
            timer_tick(&player->scaling.time_before_start, dt);
        }
        if (player->scaling.remaining > 0 && player->scaling.time_before_start.remaining == 0.0f) {
            player->flashing = false;
            stopwatch_tick(&player->scale_horizontally, dt);
            stopwatch_tick(&player->scale_vertically, dt);

            while (player->scale_horizontally.elapsed > PLAYER_SCALE_TIME_PER_PX) {
                Physics_Step player_step_width = { .before = player->rect, .after = player->rect };

                if (player->next_scale_to_right) {
                    player_step_width.after.width += 1;
                } else {
                    player_step_width.after.x     -= 1;
                    player_step_width.after.width += 1;
                }
                player->next_scale_to_right = !player->next_scale_to_right;

                Simulate_Step_Result result = simulate_step(current_level, player_step_width);
                if (!result.has_conflict) {
                    player->rect = player_step_width.after;
                    // PlaySound(game->sounds.scaling);
                    player->scaling.remaining -= 1;
                }

                player->scale_horizontally.elapsed -= PLAYER_SCALE_TIME_PER_PX;
            }

            while (player->scale_vertically.elapsed > PLAYER_SCALE_TIME_PER_PX) {
                Physics_Step player_step_height = { .before = player->rect, .after = player->rect };

                player_step_height.after.y      -= 1;
                player_step_height.after.height += 1;
                Simulate_Step_Result result = simulate_step(current_level, player_step_height);
                if (!result.has_conflict) {
                    player->rect = player_step_height.after;
                    // PlaySound(game->sounds.scaling);
                    player->scaling.remaining -= 1;
                }

                player->scale_vertically.elapsed -= PLAYER_SCALE_TIME_PER_PX;
            }
        } else {
            player->scale_horizontally.elapsed = 0;
            player->scale_vertically.elapsed = 0;
            player->next_scale_to_right = false;
        }




        if (rect_intersect_rect_int8(player->rect, current_level->key)) {
            // PlaySound(game->sounds.pickup);
            player->has_key = true;
        }
        
        if (rect_intersect_rect_int8(player->rect, current_level->door.rect)) {
            if (player->has_key) {
                if (game->current_level_index < LEVELS_COUNT) {
                    // PlaySound(game->sounds.next_level);
                    game->current_level_index += 1;
                }
                reload_current_level(game);
            }
        }
        
        timer_tick(&player->jump_buffer, dt);
        timer_tick(&player->coyote_time, dt);
    }

    // Update boxes
    {
        for (uint32 i = 0; i < *boxes_length; i++) {
            Box *box = &boxes[i];

            if (!is_rect_on_ground(box->rect, current_level)) {
                box->velocity_y -= GRAVITY * dt;
            } else {
                box->velocity_y = 0.0f;
            }

            // copypasta from player
            if (box->velocity_y != 0) {
                float32 time_per_px;
                int delta = 0;
                if (box->velocity_y < 0) {
                    time_per_px = -1 / box->velocity_y;
                    delta = 1;
                } else {
                    time_per_px = 1 / box->velocity_y;
                    delta = -1;
                }

                stopwatch_tick(&box->y_time, dt);
                while (box->y_time.elapsed > time_per_px) {
                    Physics_Step box_step_y = {
                        .before = box->rect,
                        .after = box->rect,
                };
                    box_step_y.after.y = box->rect.y + delta;
                    Simulate_Step_Result result = simulate_step(current_level, box_step_y);
                    if (!result.has_conflict) {
                        box->rect.y = box_step_y.after.y;
                    } else {
                        if (box->rect.y > result.conflicts[0].other_rect.y) {
                            box->velocity_y = 0.0f;
                        }
                    }

                    box->y_time.elapsed -= time_per_px;
                }
            }
        }
    }



    // Drawing
    {
        draw_background(screen, COLOR_BLACK);

        Sprite orientation_table[ORIENTATION_COUNT] = {
            game->sprites.dirt,
            game->sprites.dirt_n,
            game->sprites.dirt_ne,
            game->sprites.dirt_nw,
        };

        for (int8 y = 0; y < TILEMAP_HEIGHT; y++) {
            for (int8 x = 0; x < TILEMAP_WIDTH; x++) {
                switch (tilemap->tiles[y * TILEMAP_WIDTH + x]) {
                    case TILEMAP_TILE_TYPE_BLOCK: {
                        Rect_Int8 tile_rect = (Rect_Int8){
                            .x = x * TILEMAP_SINGLE_TILE_SIZE,
                            .y = y * TILEMAP_SINGLE_TILE_SIZE,
                            .width  = TILEMAP_SINGLE_TILE_SIZE,
                            .height = TILEMAP_SINGLE_TILE_SIZE,
                        };
                        draw_sprite(screen, tile_rect.x, tile_rect.y, orientation_table[tilemap->orientations[y * TILEMAP_WIDTH + x]]);
                    } break;
                    default: break;
                }
            }
        }

        Sprite sprite = player->has_key ? game->sprites.door_lit_up : game->sprites.door;
        Rect_Int8 door_rect = current_level->door.rect;
        draw_sprite(screen, door_rect.x, door_rect.y, sprite);
        
        for (uint32 i = 0; i < *boxes_length; i++) {
            Box *box = &boxes[i];
            Rect_Int8 rect = box->rect;
            draw_sprite(screen, rect.x, rect.y, game->sprites.box);
        }

        for (uint32 i = 0; i < *candies_length; i++) {
            Candy *candy = &candies[i];
            Rect_Int8 rect = candy->rect;
            draw_sprite(screen, rect.x, rect.y, game->sprites.candy);
        }

        if (!player->has_key) {
            Rect_Int8 rect = current_level->key;
            draw_sprite(screen, rect.x, rect.y, game->sprites.key);
        }

        timer_tick(&player->flash_timer, dt);
        if (player->flashing) {
            if (player->flash_timer.remaining == 0.0f) {
                player->flash_color = !player->flash_color;
                player->flash_timer.remaining = PLAYER_FLASH_TIME;
                if (player->flash_color) {
                    // PlaySound(game->sounds.flash);
                }
            }
        } else {
            player->flash_color = false;
        }
        uint32 player_color = player->flash_color ? PLAYER_COLOR_FLASH : PLAYER_COLOR_MAIN;

        Rect_Int8 player_rect = player->rect;
        draw_rectangle(screen, player_rect, player_color);
        if (player->looking_left) {
            float32 x = player_rect.x;
            draw_pixel(screen, x + 0, player_rect.y + 2, PLAYER_COLOR_EYE);
            draw_pixel(screen, x + 2, player_rect.y + 2, PLAYER_COLOR_EYE);
        } else {
            float32 x = player_rect.x + player_rect.width - 1;
            draw_pixel(screen, x - 0, player_rect.y + 2, PLAYER_COLOR_EYE);
            draw_pixel(screen, x - 2, player_rect.y + 2, PLAYER_COLOR_EYE);
        }

    }
    
    draw_all_pixels_to_the_canvas_through_the_madness_of_wasm_and_js(screen->pixels);

    EndDrawing();
}



#include "engn64.c"
