Sure, here's the proposed content for the file `/raylib-game/raylib-game/src/game/block.c`:

#include "block.h"
#include "raylib.h"

static Block blocks[MAX_BLOCKS];
static int blockCount = 0;

void InitBlocks() {
    blockCount = 0;
}

void AddBlock(Vector3 position, Vector3 size, Color color) {
    if (blockCount < MAX_BLOCKS) {
        blocks[blockCount].position = position;
        blocks[blockCount].size = size;
        blocks[blockCount].color = color;
        blockCount++;
    }
}

void DrawBlocks() {
    for (int i = 0; i < blockCount; i++) {
        DrawCube(blocks[i].position, blocks[i].size.x, blocks[i].size.y, blocks[i].size.z, blocks[i].color);
        DrawCubeWires(blocks[i].position, blocks[i].size.x, blocks[i].size.y, blocks[i].size.z, BLACK);
    }
}

bool CheckBlockCollision(Vector3 position, Vector3 size) {
    for (int i = 0; i < blockCount; i++) {
        if (CheckCollisionBoxes((BoundingBox){blocks[i].position, (Vector3){blocks[i].position.x + blocks[i].size.x, blocks[i].position.y + blocks[i].size.y, blocks[i].position.z + blocks[i].size.z}}, (BoundingBox){position, (Vector3){position.x + size.x, position.y + size.y, position.z + size.z}})) {
            return true;
        }
    }
    return false;
}