#include "debug.h"
#include <stdio.h>

void LogDebug(const char *message) {
    printf("DEBUG: %s\n", message);
}

void DrawDebugInfo(float playerPosX, float playerPosY, int blockCount) {
    DrawText(TextFormat("Player Position: (%.2f, %.2f)", playerPosX, playerPosY), 10, 10, 20, DARKGRAY);
    DrawText(TextFormat("Block Count: %d", blockCount), 10, 30, 20, DARKGRAY);
}