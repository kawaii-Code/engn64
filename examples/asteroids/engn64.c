/*

engn64 -- game engine for 64x64 games

*/

typedef enum {
    DEBUG_SHOW_PIXEL_GRID = 1 << 0,
    DEBUG_PAUSE           = 1 << 1,
    DEBUG_RECORDING       = 1 << 2,
    DEBUG_PLAYBACK        = 1 << 3,
} Debug_Flags;

typedef struct {
    Game   game_at_start_of_recording;
    Input *inputs;
    int32  duration_frames;
} Debug_Recording;

typedef struct {
    Debug_Flags     flags;
    Debug_Recording recording;
    float32         game_speed;
    int32           playback_frame;
} Debug;

void debug_start_recording(Debug *debug, Game *game) {
    debug->recording.game_at_start_of_recording = *game;
    debug->recording.duration_frames = 0;
    debug->flags |= DEBUG_RECORDING;
}

void debug_record_frame_input(Debug *debug, Input *input) {
    assert(debug->recording.duration_frames < 10000);
    debug->recording.inputs[debug->recording.duration_frames] = *input;
    debug->recording.duration_frames++;
}

void debug_end_recording(Debug *debug) {
    debug->flags ^= DEBUG_RECORDING;
}

void debug_start_playback(Debug *debug, Game *game) {
    debug->playback_frame = 0;
    debug->flags |= DEBUG_PLAYBACK;
    *game = debug->recording.game_at_start_of_recording;
}

void debug_end_playback(Debug *debug) {
    debug->flags ^= DEBUG_PLAYBACK;
}

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

int main(void) {
    int32 default_window_size = 640;
    int32 window_width = default_window_size;
    int32 window_height = default_window_size;

    SetTraceLogLevel(LOG_NONE);
    SetConfigFlags(FLAG_WINDOW_RESIZABLE);
    InitWindow(window_width, window_height, "YAPAS");
    InitAudioDevice();
    SetTargetFPS(60);
    SetExitKey(0);

    Rect_Int32 game_window = {0, 0, window_width, window_height};
    Image image = GenImageColor(64, 64, WHITE);
    ImageFormat(&image, PIXELFORMAT_UNCOMPRESSED_R8G8B8A8);
    Texture2D target = LoadTextureFromImage(image);

    Debug debug = {0};
    debug.game_speed = 1.0f;
    // 10000 frames is the recording limit
    debug.recording.inputs = malloc(10000 * sizeof *debug.recording.inputs);

    Game   game   = {0};
    Input  input  = {0};
    Screen screen = {0};
    init(&game);
    while (!WindowShouldClose()) {
        if (IsKeyPressed(KEY_F11)) {
            if (!IsWindowFullscreen()) {
                window_width  = GetMonitorWidth(0);
                window_height = GetMonitorHeight(0);
                resize_game_window(&game_window, window_width, window_height);
            } else {
                window_width  = GetScreenWidth();
                window_height = GetScreenHeight();
                resize_game_window(&game_window, window_width, window_height);
            }
            ToggleFullscreen();
        }
        if (IsWindowResized()) {
            // `window_width` and `window_height` is the size of the actual window.
            // `game_window_width` and `game_window_height` account for the game size
            // being 64x64. If `window_width != window_height`, then the image would
            // stretch and the pixels would be ruined. To counteract that, I keep
            // game_window, where `width == height` and append black borders to the
            // leftover part of the window.
            window_width  = GetScreenWidth(); // TODO: Is GetScreenWidth() the right function?
            window_height = GetScreenHeight();
            resize_game_window(&game_window, window_width, window_height);
        }
        

        // Debug Input
        {
            if (IsKeyPressed(KEY_LEFT_BRACKET)) {
                debug.game_speed -= 0.25f;
            }
            if (IsKeyPressed(KEY_RIGHT_BRACKET)) {
                debug.game_speed += 0.25f;
            }
            if (IsKeyPressed(KEY_L)) {
                if (debug.flags & DEBUG_PLAYBACK) {
                    debug_end_playback(&debug);
                } else if (debug.flags & DEBUG_RECORDING) {
                    debug_end_recording(&debug);
                    debug_start_playback(&debug, &game);
                } else {
                    debug_start_recording(&debug, &game);
                }
            }
            if (IsKeyPressed(KEY_G)) {
                debug.flags ^= DEBUG_SHOW_PIXEL_GRID;
            }
        }

        // Game input
        input = (Input){0};
        bool8 move_up    = IsKeyDown(KEY_W) || IsKeyDown(KEY_UP);
        bool8 move_left  = IsKeyDown(KEY_A) || IsKeyDown(KEY_LEFT);
        bool8 move_down  = IsKeyDown(KEY_S) || IsKeyDown(KEY_DOWN);
        bool8 move_right = IsKeyDown(KEY_D) || IsKeyDown(KEY_RIGHT);
        bool8 btn1       = IsKeyDown(KEY_Z) || IsKeyDown(KEY_SPACE) || IsKeyDown(KEY_UP) || IsKeyDown(KEY_W);
        bool8 btn2       = IsKeyDown(KEY_X) || IsKeyDown(KEY_R);
        bool8 btn3       = IsKeyDown(KEY_P) || IsKeyDown(KEY_COMMA);
        bool8 btn4       = IsKeyDown(KEY_N) || IsKeyDown(KEY_PERIOD);
        bool8 using_keyboard = move_right || move_left || move_up || move_down || btn1 || btn2 || btn3 || btn4;
        // Keyboard wins over a gamepad
        if (using_keyboard) {
            if (move_right) {
                input.x += 1.0f;
            }
            if (move_left) {
                input.x -= 1.0f;
            }
            if (move_up) {
                input.y -= 1.0f;
            }
            if (move_down) {
                input.y += 1.0f;
            }
            input.btn1.pressed      = btn1;
            input.btn2.pressed      = btn2;
            input.btn3.pressed      = btn3;
            input.btn4.pressed      = btn4;
            input.btn1.just_pressed = IsKeyPressed(KEY_Z) || IsKeyPressed(KEY_SPACE) || IsKeyPressed(KEY_UP) || IsKeyPressed(KEY_W);
            input.btn2.just_pressed = IsKeyPressed(KEY_X) || IsKeyPressed(KEY_R);
            input.btn3.just_pressed = IsKeyPressed(KEY_P) || IsKeyPressed(KEY_COMMA);
            input.btn4.just_pressed = IsKeyPressed(KEY_N) || IsKeyPressed(KEY_PERIOD);
        } else if (IsGamepadAvailable(0)) {
            float32 move_x = GetGamepadAxisMovement(0, GAMEPAD_AXIS_LEFT_X);
            float32 move_y = GetGamepadAxisMovement(0, GAMEPAD_AXIS_LEFT_Y);

            // TODO: Research this problem more
            // Prevent slight controller faults (mine had always given 0.1 in y when idle)
            float32 epsilon = 0.15f;
            if (abs_float32(move_x) < epsilon) move_x = 0.0f;
            if (abs_float32(move_y) < epsilon) move_y = 0.0f;
            
            input.x = move_x;
            input.y = move_y;
            
            input.btn1.pressed      = IsGamepadButtonDown(0, GAMEPAD_BUTTON_RIGHT_FACE_UP);
            input.btn2.pressed      = IsGamepadButtonDown(0, GAMEPAD_BUTTON_RIGHT_FACE_RIGHT);
            input.btn3.pressed      = IsGamepadButtonDown(0, GAMEPAD_BUTTON_RIGHT_FACE_DOWN);
            input.btn4.pressed      = IsGamepadButtonDown(0, GAMEPAD_BUTTON_RIGHT_FACE_LEFT);
            input.btn1.just_pressed = IsGamepadButtonPressed(0, GAMEPAD_BUTTON_RIGHT_FACE_UP);
            input.btn2.just_pressed = IsGamepadButtonPressed(0, GAMEPAD_BUTTON_RIGHT_FACE_RIGHT);
            input.btn3.just_pressed = IsGamepadButtonPressed(0, GAMEPAD_BUTTON_RIGHT_FACE_DOWN);
            input.btn4.just_pressed = IsGamepadButtonPressed(0, GAMEPAD_BUTTON_RIGHT_FACE_LEFT);
        }

        float32 dt = GetFrameTime();

        if (debug.flags & DEBUG_PAUSE) {
            dt = 0.0f;
        }
        assert((debug.flags & (DEBUG_RECORDING | DEBUG_PLAYBACK)) != (DEBUG_RECORDING | DEBUG_PLAYBACK));
        if (debug.flags & DEBUG_RECORDING) {
            debug_record_frame_input(&debug, &input);
        }
        if (debug.flags & DEBUG_PLAYBACK) {
            input = debug.recording.inputs[debug.playback_frame];
            debug.playback_frame += 1;
            if (debug.playback_frame == debug.recording.duration_frames) {
                debug_start_playback(&debug, &game);
            }
        }
        dt *= debug.game_speed;
        
        update_and_draw(&game, &screen, &input, dt);
        
        BeginDrawing();
        {
            ClearBackground(BLACK);

            // This draws a 64x64 texture (upscaled) containing the pixels
            // of the game.
            UpdateTexture(target, screen.pixels);
            DrawTexturePro(target, (Rectangle){0, 0, 64, 64}, to_rect_float32(game_window), (Vector2){0, 0}, 0.0f, WHITE);
            
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
        }
        
        EndDrawing();
    }

    CloseAudioDevice();
    CloseWindow();
}

#ifdef _WIN32

#define WINDOWS_LEAN_AND_MEAN
#define ShowCursor Win32ShowCursor
#define CloseWindow Win32CloseWindow
#define Rectangle Win32Rectangle
#include "windows.h"
#undef ShowCursor
#undef CloseWindow
#undef Rectangle

int WinMain(
  HINSTANCE hInstance,
  HINSTANCE hPrevInstance,
  LPSTR     lpCmdLine,
  int       nShowCmd
) {
	(void)hInstance;
	(void)hPrevInstance;
	(void)lpCmdLine;
	(void)nShowCmd;
	main();
}
#endif
