#include "raylib.h"
#include "raymath.h"

#define SCREEN_WIDTH 800
#define SCREEN_HEIGHT 600

// Physics constants
#define GRAVITY 9.81f
#define JUMP_FORCE 5.0f
#define GROUND_LEVEL 1.0f

typedef struct
{
  Vector3 position;
  Vector3 rotation;
  Vector3 velocity;
  float moveSpeed;
  float turnSpeed;
  bool isJumping;
  float jumpVelocity;
} Player;

int main(void)
{
  InitWindow(SCREEN_WIDTH, SCREEN_HEIGHT, "Simple 3D Game");
  SetTargetFPS(60);
  DisableCursor();

  // Initialize player
  Player player = {
      .position = (Vector3){0.0f, GROUND_LEVEL, 0.0f},
      .rotation = (Vector3){0.0f, 0.0f, 0.0f},
      .velocity = (Vector3){0.0f, 0.0f, 0.0f},
      .moveSpeed = 0.1f,
      .turnSpeed = 0.003f,
      .isJumping = false,
      .jumpVelocity = 0.0f};

  // Setup camera
  Camera3D camera = {
      .position = player.position,
      .target = (Vector3){0.0f, 1.0f, -1.0f},
      .up = (Vector3){0.0f, 1.0f, 0.0f},
      .fovy = 60.0f,
      .projection = CAMERA_PERSPECTIVE};

  while (!WindowShouldClose())
  {
    // Get mouse movement for camera rotation
    Vector2 mouseDelta = GetMouseDelta();

    // Handle movement (WASD)
    Vector3 forward = Vector3Subtract(camera.target, camera.position);
    forward.y = 0; // Keep movement on the horizontal plane
    forward = Vector3Normalize(forward);

    // Calculate right vector
    Vector3 right = Vector3CrossProduct(forward, camera.up);
    right = Vector3Normalize(right);

    // Reset movement
    Vector3 moveDir = {0};

    if (IsKeyDown(KEY_W))
      moveDir = Vector3Add(moveDir, forward);
    if (IsKeyDown(KEY_S))
      moveDir = Vector3Subtract(moveDir, forward);
    if (IsKeyDown(KEY_D))
      moveDir = Vector3Add(moveDir, right);
    if (IsKeyDown(KEY_A))
      moveDir = Vector3Subtract(moveDir, right);

    // Normalize movement vector to maintain consistent speed
    if (Vector3Length(moveDir) > 0)
    {
      moveDir = Vector3Normalize(moveDir);
      player.position = Vector3Add(player.position,
                                   Vector3Scale(moveDir, player.moveSpeed));
    }

    // Handle jumping
    if (IsKeyPressed(KEY_SPACE) && !player.isJumping)
    {
      player.isJumping = true;
      player.jumpVelocity = JUMP_FORCE;
    }

    // Apply gravity and update jump
    if (player.isJumping)
    {
      player.jumpVelocity -= GRAVITY * GetFrameTime();
      player.position.y += player.jumpVelocity * GetFrameTime();

      // Check for ground collision
      if (player.position.y <= GROUND_LEVEL)
      {
        player.position.y = GROUND_LEVEL;
        player.jumpVelocity = 0.0f;
        player.isJumping = false;
      }
    }

    // Update camera position to follow player
    camera.position = player.position;

    // Update camera rotation based on mouse movement
    player.rotation.x -= mouseDelta.y * player.turnSpeed;
    player.rotation.y -= mouseDelta.x * player.turnSpeed;

    // Clamp vertical rotation to avoid flipping
    player.rotation.x = Clamp(player.rotation.x, -1.5f, 1.5f);

    // Calculate camera target based on rotation
    camera.target.x = camera.position.x - sinf(player.rotation.y);
    camera.target.y = camera.position.y + sinf(player.rotation.x);
    camera.target.z = camera.position.z - cosf(player.rotation.y);

    // Drawing
    BeginDrawing();
    ClearBackground(RAYWHITE);

    BeginMode3D(camera);
    DrawGrid(20, 1.0f);
    EndMode3D();

    DrawFPS(10, 10);
    EndDrawing();
  }

  CloseWindow();
  return 0;
}