#ifndef BLOCK_H
#define BLOCK_H

#include "raylib.h"

typedef struct {
    Vector3 position;
    Vector3 size;
    Color color;
} Block;

void DrawBlock(Block block);
void CreateBlock(Vector3 position, Vector3 size, Color color, Block *block);

#endif // BLOCK_H