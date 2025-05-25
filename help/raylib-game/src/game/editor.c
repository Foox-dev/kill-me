// This file implements the editor functionalities, allowing the user to place and manipulate blocks in the game world.

#include "editor.h"
#include "block.h"
#include "raylib.h"

static Vector3 editorPosition = {0};
static Color currentBlockColor = RED;
static Block blocks[MAX_BLOCKS] = {0};
static int blockCount = 0;

void UpdateEditor(Camera3D *camera) {
    // Editor controls
    if (IsKeyPressed(KEY_ONE))
        currentBlockColor = RED;
    if (IsKeyPressed(KEY_TWO))
        currentBlockColor = GREEN;
    if (IsKeyPressed(KEY_THREE))
        currentBlockColor = BLUE;

    // Move editor position
    if (IsKeyDown(KEY_RIGHT))
        editorPosition.x += 0.1f;
    if (IsKeyDown(KEY_LEFT))
        editorPosition.x -= 0.1f;
    if (IsKeyDown(KEY_UP))
        editorPosition.z -= 0.1f;
    if (IsKeyDown(KEY_DOWN))
        editorPosition.z += 0.1f;
    if (IsKeyDown(KEY_PAGE_UP))
        editorPosition.y += 0.1f;
    if (IsKeyDown(KEY_PAGE_DOWN))
        editorPosition.y -= 0.1f;

    // Place block
    if (IsKeyPressed(KEY_ENTER) && blockCount < MAX_BLOCKS) {
        blocks[blockCount] = (Block){
            .position = editorPosition,
            .size = (Vector3){1.0f, 1.0f, 1.0f},
            .color = currentBlockColor};
        blockCount++;
    }

    // Delete last block
    if (IsKeyPressed(KEY_BACKSPACE) && blockCount > 0) {
        blockCount--;
    }

    // Add free camera movement
    Vector3 forward = Vector3Normalize(Vector3Subtract(camera->target, camera->position));
    Vector3 right = Vector3Normalize(Vector3CrossProduct(forward, camera->up));

    if (IsKeyDown(KEY_W))
        camera->position = Vector3Add(camera->position, Vector3Scale(forward, 0.1f));
    if (IsKeyDown(KEY_S))
        camera->position = Vector3Subtract(camera->position, Vector3Scale(forward, 0.1f));
    if (IsKeyDown(KEY_D))
        camera->position = Vector3Add(camera->position, Vector3Scale(right, 0.1f));
    if (IsKeyDown(KEY_A))
        camera->position = Vector3Subtract(camera->position, Vector3Scale(right, 0.1f));

    Vector2 mouseDelta = GetMouseDelta();
    camera->target.x = camera->position.x - sinf(mouseDelta.x * 0.003f);
    camera->target.y = camera->position.y + sinf(mouseDelta.y * 0.003f);
    camera->target.z = camera->position.z - cosf(mouseDelta.x * 0.003f);
}

void DrawBlock(Block block) {
    DrawCube(block.position, block.size.x, block.size.y, block.size.z, block.color);
    DrawCubeWires(block.position, block.size.x, block.size.y, block.size.z, BLACK);
}