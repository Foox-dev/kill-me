// This file declares the functions and structures related to the editor, including functions for updating the editor state and handling user input.

#ifndef EDITOR_H
#define EDITOR_H

#include "raylib.h"
#include "block.h"

void UpdateEditor(Camera3D *camera);
void DrawEditorBlocks(void);
void ToggleEditorMode(void);
void PlaceBlock(Vector3 position, Color color);
void DeleteLastBlock(void);

#endif // EDITOR_H