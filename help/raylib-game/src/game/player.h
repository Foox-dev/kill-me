#ifndef PLAYER_H
#define PLAYER_H

#include "raylib.h"

typedef struct {
    Vector3 position;
    Vector3 rotation;
    Vector3 velocity;
    float moveSpeed;
    float turnSpeed;
    bool isJumping;
    float jumpVelocity;
} Player;

void InitPlayer(Player *player);
void UpdatePlayer(Player *player);
void DrawPlayer(Player player);

#endif // PLAYER_H