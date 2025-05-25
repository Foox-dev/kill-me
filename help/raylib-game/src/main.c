#include "raylib.h"
#include "game/player.h"
#include "game/editor.h"
#include "network/network.h"
#include "utils/config.h"

int main(void)
{
    // Initialize the game window
    InitWindow(SCREEN_WIDTH, SCREEN_HEIGHT, "Raylib Game");

    // Set target frames per second
    SetTargetFPS(60);

    // Initialize game components
    Player player = InitPlayer();
    Camera3D camera = InitCamera();
    InitEditor();

    // Main game loop
    while (!WindowShouldClose())
    {
        // Update game state
        UpdatePlayer(&player);
        UpdateEditor(&camera);

        // Begin drawing
        BeginDrawing();
        ClearBackground(RAYWHITE);

        // Draw game elements
        BeginMode3D(camera);
        DrawPlayer(player);
        EndMode3D();

        // Draw UI elements
        DrawText("Use WASD to move, ESC to exit", 10, 10, 20, DARKGRAY);

        EndDrawing();
    }

    // Cleanup resources
    CloseWindow();
    return 0;
}