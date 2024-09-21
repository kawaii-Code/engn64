/*

engn64 -- game engine for 64x64 games

TODO
- Raw sound loading

*/

typedef enum {
    DEBUG_SHOW_PIXEL_GRID = 1 << 0,
    DEBUG_PAUSE           = 1 << 1,
    DEBUG_RECORDING       = 1 << 2,
    DEBUG_PLAYBACK        = 1 << 3,
} Debug_Flags;

typedef struct {
    Debug_Flags     flags;
    float32         game_speed;
    int32           playback_frame;
} Debug;


void resize_game_window(Rect_Int32 *game_window, int32 width, int32 height) {
    if (width > height) {
        game_window->x = (width - height) / 2;
        game_window->y = 0;
        game_window->width = height;
        game_window->height = height;
    } else {
        game_window->x = 0;
        game_window->y = (height - width) / 2;
        game_window->width = width;
        game_window->height = width;
    }    
}

void raylib_js_set_entry(void (*entry)(void));

int main(void) {
    init();
    raylib_js_set_entry(update_and_draw);
}
#if 0
    int32 default_window_size = 640;
    int32 window_width = default_window_size;
    int32 window_height = default_window_size;

    // SetTraceLogLevel(LOG_NONE);
    // SetConfigFlags(FLAG_WINDOW_RESIZABLE);
    InitWindow(window_width, window_height, "engn64");
    // InitAudioDevice();
    SetTargetFPS(60);
    // SetExitKey(0);

    Rect_Int32 game_window = {0, 0, window_width, window_height};
    // Image image = GenImageColor(64, 64, WHITE);
    // ImageFormat(&image, PIXELFORMAT_UNCOMPRESSED_R8G8B8A8);
    // Texture2D target = LoadTextureFromImage(image);

    Debug debug = {0};
    debug.game_speed = 1.0f;
    // 10000 frames is the recording limit
    // debug.recording.inputs = malloc(10000 * sizeof *debug.recording.inputs);    

    Game   game   = {0};
    Screen screen = {0};
    while (!WindowShouldClose()) {
        // if (IsKeyPressed(KEY_F11)) {
        //     if (!IsWindowFullscreen()) {
        //         window_width  = GetMonitorWidth(0);
        //         window_height = GetMonitorHeight(0);
        //         resize_game_window(&game_window, window_width, window_height);
        //     } else {
        //         window_width  = GetScreenWidth();
        //         window_height = GetScreenHeight();
        //         resize_game_window(&game_window, window_width, window_height);
        //     }
        //     ToggleFullscreen();
        // }
        // if (IsWindowResized()) {
        //     // `window_width` and `window_height` is the size of the actual window.
        //     // `game_window_width` and `game_window_height` account for the game size
        //     // being 64x64. If `window_width != window_height`, then the image would
        //     // stretch and the pixels would be ruined. To counteract that, I keep
        //     // game_window, where `width == height` and append black borders to the
        //     // leftover part of the window.
        //     window_width  = GetScreenWidth(); // TODO: Is GetScreenWidth() the right function?
        //     window_height = GetScreenHeight();
        //     resize_game_window(&game_window, window_width, window_height);
        // }
        

        // Debug Input
        {
            if (IsKeyPressed(KEY_LEFT_BRACKET)) {
                debug.game_speed -= 0.25f;
            }
            if (IsKeyPressed(KEY_RIGHT_BRACKET)) {
                debug.game_speed += 0.25f;
            }
            if (IsKeyPressed(KEY_G)) {
                debug.flags ^= DEBUG_SHOW_PIXEL_GRID;
            }
            if (IsKeyPressed(KEY_P)) {
                debug.flags ^= DEBUG_PAUSE;
            }
        }

        float32 dt = GetFrameTime();

        if (debug.flags & DEBUG_PAUSE) {
            dt = 0.0f;
        }
        assert((debug.flags & (DEBUG_RECORDING | DEBUG_PLAYBACK)) != (DEBUG_RECORDING | DEBUG_PLAYBACK));
        dt *= debug.game_speed;
        
        update_and_draw(dt);
        
        BeginDrawing();
        {
            ClearBackground(BLACK);

            // This draws a 64x64 texture (upscaled) containing the pixels
            // of the game.
            // UpdateTexture(target, screen.pixels);
            // DrawTexturePro(target, (Rectangle){0, 0, 64, 64}, to_rect_float32(game_window), (Vector2){0, 0}, 0.0f, WHITE);
            
            // Debug drawing
            if (debug.flags & DEBUG_SHOW_PIXEL_GRID) {
                Rect_Float32 game_window_float32 = to_rect_float32(game_window);
                float32 x = game_window_float32.x;
                for (uint32 xi = 0; xi < 64; xi += 4) {
                    DrawLineV((Vector2){x, game_window_float32.y},
                              (Vector2){x, game_window_float32.y + game_window_float32.height},
                              GRAY);
                    x += 4 * game_window_float32.width / 64.0f;
                }
                float32 y = game_window_float32.y;
                for (uint32 yi = 0; yi < 64; yi += 4) {
                    DrawLineV((Vector2){game_window_float32.x, y},
                              (Vector2){game_window_float32.x + game_window_float32.width, y},
                              GRAY);
                    y += 4 * game_window_float32.height / 64.0f;
                }
            }
            if (debug.flags & DEBUG_PAUSE) {
                DrawText("PAUSED", 0, 0, 32, PINK);
            }
            float32 debug_circle_radius = 16;
            if (debug.flags & DEBUG_RECORDING) {
                DrawCircle(window_width - debug_circle_radius,
                           debug_circle_radius,
                           debug_circle_radius,
                           RED);
                DrawText("REC",
                         window_width - 4 * debug_circle_radius,
                         5,
                         debug_circle_radius,
                         RED);
            }
            if (debug.flags & DEBUG_PLAYBACK) {
                DrawCircle(window_width - debug_circle_radius,
                    debug_circle_radius,
                    debug_circle_radius,
                    GREEN);
                DrawText("PLAY",
                    window_width - 5 * debug_circle_radius,
                    5,
                    debug_circle_radius,
                    GREEN);
            }
            if (debug.game_speed != 1.0f) {
                DrawText(TextFormat("SPEED: %.2f", debug.game_speed), 0, 20, 32, PINK);
            }
            
            DrawText(game.current_level.title, 8, 8, 27, WHITE);
        }
        
        EndDrawing();
    }

    CloseAudioDevice();
    CloseWindow();
}

#endif