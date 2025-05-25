#include "raylib.h"
#include "raymath.h"
#include <enet/enet.h>
#include <stdio.h>
#include <stdint.h>
#include <string.h> // Add this include for memset

#define SCREEN_WIDTH 800
#define SCREEN_HEIGHT 600

// Physics constants
#define GRAVITY 9.81f
#define JUMP_FORCE 5.0f
#define GROUND_LEVEL 1.0f

#define MAX_BLOCKS 1000
#define EDITOR_MODE_KEY KEY_TAB
#define DEBUG_MODE_KEY KEY_F3
#define FREECAM_KEY KEY_Z
#define HOST_KEY KEY_H
#define JOIN_KEY KEY_J

// Network defines
#define MAX_PLAYERS 8
#define PORT 1234
#define TICK_RATE 60
#define NETWORK_UPDATE_TIME (1.0f / TICK_RATE)

typedef struct
{
  Vector3 position;
  Vector3 size;
  Color color;
} Block;

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

typedef struct
{
  Vector3 position;
  Vector3 rotation;
  bool active;
  uint32_t clientId;
} NetworkPlayer;

// Add these global variables
static Block blocks[MAX_BLOCKS] = {0};
static int blockCount = 0;
static bool editorMode = false;
static bool debugMode = false;
static bool freeCamMode = false;
static Vector3 editorPosition = {0};
static Color currentBlockColor = RED;
static Camera3D editorCamera = {0};
static Vector3 lastPlayerPos = {0};

// Network globals
static NetworkPlayer networkPlayers[MAX_PLAYERS] = {0};
static ENetHost *host = NULL;
static ENetPeer *peer = NULL;
static bool isHost = false;
static bool isConnected = false;
static uint32_t clientId = 0;
static int playerCount = 0;

// Add this with other global variables
static bool gamePaused = false;

// Add after other globals
static Color playerColors[MAX_PLAYERS] = {
    BLUE,   // Host color
    RED,    // Client 1
    GREEN,  // Client 2
    YELLOW, // Client 3
    PURPLE, // Client 4
    ORANGE, // Client 5
    PINK,   // Client 6
    BROWN   // Client 7
};

// Add these globals after other globals
static char ipInput[256] = {0};
static bool showIpInput = false;
static int ipInputIndex = 0;

void DrawBlock(Block block)
{
  DrawCube(block.position, block.size.x, block.size.y, block.size.z, block.color);
  DrawCubeWires(block.position, block.size.x, block.size.y, block.size.z, BLACK);
}

void UpdateEditor(Camera3D *camera)
{
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
  if (IsKeyPressed(KEY_ENTER) && blockCount < MAX_BLOCKS)
  {
    blocks[blockCount] = (Block){
        .position = editorPosition,
        .size = (Vector3){1.0f, 1.0f, 1.0f},
        .color = currentBlockColor};
    blockCount++;
  }

  // Delete last block
  if (IsKeyPressed(KEY_BACKSPACE) && blockCount > 0)
  {
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

bool CheckBlockCollision(Vector3 position, Vector3 size, Block block)
{
  return (position.x - size.x / 2 <= block.position.x + block.size.x / 2 &&
          position.x + size.x / 2 >= block.position.x - block.size.x / 2 &&
          position.y - size.y / 2 <= block.position.y + block.size.y / 2 &&
          position.y + size.y / 2 >= block.position.y - block.size.y / 2 &&
          position.z - size.z / 2 <= block.position.z + block.size.z / 2 &&
          position.z + size.z / 2 >= block.position.z - block.size.z / 2);
}

void InitNetwork(void)
{
  if (enet_initialize() != 0)
  {
    printf("Failed to initialize ENet.\n");
    return;
  }
}

// Add this cleanup function after InitNetwork
void CleanupNetwork(void)
{
  if (peer)
  {
    enet_peer_disconnect(peer, 0);
    peer = NULL;
  }

  if (host)
  {
    enet_host_destroy(host);
    host = NULL;
  }

  // Reset network state
  isHost = false;
  isConnected = false;
  clientId = 0;
  playerCount = 0;

  // Clear all network players
  for (int i = 0; i < MAX_PLAYERS; i++)
  {
    networkPlayers[i].active = false;
  }
}

void HostGame(void)
{
  CleanupNetwork(); // Cleanup any existing connections

  ENetAddress address;
  address.host = ENET_HOST_ANY;
  address.port = PORT;

  host = enet_host_create(&address, MAX_PLAYERS, 2, 0, 0);
  if (host == NULL)
  {
    printf("Failed to create host.\n");
    return;
  }

  isHost = true;
  clientId = 0;

  // Initialize host's player in network array
  networkPlayers[clientId] = (NetworkPlayer){
      .position = {0},
      .rotation = {0},
      .active = true,
      .clientId = clientId};
  playerCount = 1;
}

void JoinGame(const char *hostAddress)
{
  CleanupNetwork(); // Cleanup any existing connections

  host = enet_host_create(NULL, 1, 2, 0, 0);
  if (host == NULL)
  {
    printf("Failed to create client.\n");
    return;
  }

  ENetAddress address;
  enet_address_set_host(&address, hostAddress);
  address.port = PORT;

  peer = enet_host_connect(host, &address, 2, 0);
  if (peer == NULL)
  {
    printf("Failed to connect to host.\n");
    enet_host_destroy(host);
    host = NULL;
    return;
  }
}

void UpdateNetwork(Player *player)
{
  if (host == NULL)
    return;

  // Always update our own player in the network array
  networkPlayers[clientId] = (NetworkPlayer){
      .position = player->position,
      .rotation = player->rotation,
      .active = true,
      .clientId = clientId};

  ENetEvent event;
  while (enet_host_service(host, &event, 0) > 0)
  {
    switch (event.type)
    {
    case ENET_EVENT_TYPE_CONNECT:
      printf("New client connected: %u\n", event.peer->connectID);
      if (!isHost)
      {
        clientId = 1; // Client is always 1
        isConnected = true;
        networkPlayers[clientId].active = true;
      }
      else
      {
        // Host assigns ID 1 to the first client
        event.peer->data = (void *)(uintptr_t)1;
      }
      playerCount++;
      break;

    case ENET_EVENT_TYPE_RECEIVE:
    {
      NetworkPlayer *netPlayer = (NetworkPlayer *)event.packet->data;
      if (netPlayer && netPlayer->clientId < MAX_PLAYERS)
      {
        // Safely copy the network player data
        networkPlayers[netPlayer->clientId] = *netPlayer;
        networkPlayers[netPlayer->clientId].active = true;
      }
      enet_packet_destroy(event.packet);
      break;
    }

    case ENET_EVENT_TYPE_DISCONNECT:
      printf("Client disconnected: %u\n", event.peer->connectID);
      if (event.peer->data)
      {
        uint32_t id = (uint32_t)(uintptr_t)event.peer->data;
        if (id < MAX_PLAYERS)
        {
          networkPlayers[id].active = false;
          playerCount--;
        }
      }
      if (!isHost)
      {
        // Client disconnected from host
        isConnected = false;
        if (host)
          enet_host_destroy(host);
        host = NULL;
        peer = NULL;
      }
      break;
    }
  }

  // Send our player data
  if (isConnected || isHost)
  {
    NetworkPlayer netPlayer = networkPlayers[clientId];
    ENetPacket *packet = enet_packet_create(&netPlayer,
                                            sizeof(NetworkPlayer),
                                            ENET_PACKET_FLAG_RELIABLE);

    if (packet)
    {
      if (isHost)
        enet_host_broadcast(host, 0, packet);
      else if (peer)
        enet_peer_send(peer, 0, packet);
    }
  }
}

// Add this function before main
void DrawPlayerModel(Vector3 position, Vector3 rotation, Color color)
{
  // Create transformation matrix
  Matrix transform = MatrixIdentity();
  transform = MatrixMultiply(transform, MatrixRotateX(rotation.x));
  transform = MatrixMultiply(transform, MatrixRotateY(rotation.y));
  transform = MatrixMultiply(transform,
                             MatrixTranslate(position.x, position.y + 0.5f, position.z));

  // Draw player cube at ground level with rotation
  DrawCube(Vector3Zero(), 1.0f, 1.0f, 1.0f, color);
  DrawCubeWires(Vector3Zero(), 1.0f, 1.0f, 1.0f, BLACK);

  // Draw direction indicator (front of player)
  Vector3 front = Vector3Transform((Vector3){0.0f, 0.0f, 1.0f}, transform);
  DrawLine3D(position, front, BLACK);
}

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

  // Initialize editor camera
  editorCamera = (Camera3D){
      .position = (Vector3){0.0f, 5.0f, 5.0f},
      .target = (Vector3){0.0f, 0.0f, 0.0f},
      .up = (Vector3){0.0f, 1.0f, 0.0f},
      .fovy = 60.0f,
      .projection = CAMERA_PERSPECTIVE};

  InitNetwork();

  // Add this before main game loop in main()
  bool shouldClose = false;
  static float networkTimer = 0.0f;

  while (!shouldClose)
  {
    // Toggle debug mode
    if (IsKeyPressed(DEBUG_MODE_KEY))
      debugMode = !debugMode;

    // Toggle free cam outside editor mode
    if (IsKeyPressed(FREECAM_KEY) && !editorMode)
    {
      freeCamMode = !freeCamMode;
      if (freeCamMode)
      {
        editorCamera.position = camera.position;
        editorCamera.target = camera.target;
      }
    }

    // Toggle editor mode
    if (IsKeyPressed(EDITOR_MODE_KEY))
    {
      editorMode = !editorMode;
      if (editorMode)
      {
        EnableCursor();
        editorCamera.position = camera.position;
        editorCamera.target = camera.target;
      }
      else
      {
        DisableCursor();
        camera.position = player.position;
        freeCamMode = false;
      }
    }

    // Handle pause menu first
    if (IsKeyPressed(KEY_ESCAPE))
    {
      gamePaused = !gamePaused;
      if (gamePaused)
      {
        EnableCursor();
      }
      else if (!editorMode)
      {
        DisableCursor();
      }
    }

    // Early continue if paused, but still allow quitting
    if (gamePaused)
    {
      BeginDrawing();
      ClearBackground(RAYWHITE);

      // Draw pause menu
      DrawText("PAUSED",
               SCREEN_WIDTH / 2 - MeasureText("PAUSED", 40) / 2,
               SCREEN_HEIGHT / 3, 40, RED);

      DrawText("ESC - Resume",
               SCREEN_WIDTH / 2 - MeasureText("ESC - Resume", 20) / 2,
               SCREEN_HEIGHT / 2 - 40, 20, DARKGRAY);

      DrawText("H - Host Game",
               SCREEN_WIDTH / 2 - MeasureText("H - Host Game", 20) / 2,
               SCREEN_HEIGHT / 2, 20, DARKGRAY);

      DrawText("J - Join Game",
               SCREEN_WIDTH / 2 - MeasureText("J - Join Game", 20) / 2,
               SCREEN_HEIGHT / 2 + 30, 20, DARKGRAY);

      DrawText("Q - Quit",
               SCREEN_WIDTH / 2 - MeasureText("Q - Quit", 20) / 2,
               SCREEN_HEIGHT / 2 + 60, 20, DARKGRAY);

      // Show IP input when joining
      if (showIpInput)
      {
        DrawRectangle(SCREEN_WIDTH / 4, SCREEN_HEIGHT / 2 - 80,
                      SCREEN_WIDTH / 2, 40, LIGHTGRAY);
        DrawText("Enter IP:",
                 SCREEN_WIDTH / 4 + 10,
                 SCREEN_HEIGHT / 2 - 70, 20, BLACK);
        DrawText(ipInput,
                 SCREEN_WIDTH / 4 + 100,
                 SCREEN_HEIGHT / 2 - 70, 20, BLACK);
      }

      // Handle network controls in pause menu
      if (!showIpInput) // Only handle H/J when not inputting IP
      {
        if (IsKeyPressed(KEY_H) && !isConnected)
        {
          HostGame();
          gamePaused = false;
          DisableCursor();
          printf("Hosting game on port %d\n", PORT);
        }

        if (IsKeyPressed(KEY_J) && !isHost)
        {
          showIpInput = true;
          memset(ipInput, 0, sizeof(ipInput));
          ipInputIndex = 0;
        }
      }

      // Handle IP input with improved logic
      if (showIpInput)
      {
        // Don't process regular key presses when in IP input mode
        int key = GetCharPressed();
        while (key > 0)
        {
          // Only allow numbers, dots, and valid IP characters
          if (((key >= '0' && key <= '9') ||
               key == '.' ||
               (key >= 'a' && key <= 'z') ||
               (key >= 'A' && key <= 'Z')) &&
              (ipInputIndex < 255))
          {
            ipInput[ipInputIndex] = (char)key;
            ipInputIndex++;
          }
          key = GetCharPressed();
        }

        if (IsKeyPressed(KEY_BACKSPACE) && ipInputIndex > 0)
        {
          ipInputIndex--;
          ipInput[ipInputIndex] = '\0';
        }

        if (IsKeyPressed(KEY_ENTER) && ipInputIndex > 0)
        {
          JoinGame(ipInput);
          showIpInput = false;
          gamePaused = false;
          DisableCursor();
          printf("Attempting to connect to %s:%d\n", ipInput, PORT);
        }

        if (IsKeyPressed(KEY_ESCAPE))
        {
          showIpInput = false; // Cancel IP input
          memset(ipInput, 0, sizeof(ipInput));
          ipInputIndex = 0;
        }
      }

      if (IsKeyPressed(KEY_Q))
      {
        shouldClose = true;
      }

      EndDrawing();
      continue;
    }

    if (!editorMode && !freeCamMode)
    {
      // Store previous position for collision
      Vector3 prevPos = player.position;

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

      // Check collisions with all blocks
      for (int i = 0; i < blockCount; i++)
      {
        if (CheckBlockCollision(player.position, (Vector3){1.0f, 2.0f, 1.0f}, blocks[i]))
        {
          player.position = prevPos;
          break;
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
    }
    else if (editorMode || freeCamMode)
    {
      UpdateEditor(editorMode ? &editorCamera : &camera);
    }

    // Network updates
    networkTimer += GetFrameTime();
    if (networkTimer >= NETWORK_UPDATE_TIME)
    {
      UpdateNetwork(&player);
      networkTimer = 0.0f;
    }

    // Drawing
    BeginDrawing();
    ClearBackground(RAYWHITE);

    BeginMode3D(editorMode || freeCamMode ? editorCamera : camera);

    // Draw grid
    DrawGrid(20, 1.0f);

    // Draw all blocks
    for (int i = 0; i < blockCount; i++)
    {
      DrawBlock(blocks[i]);
    }

    // Draw local player
    if (editorMode || freeCamMode)
    {
      DrawPlayerModel(player.position, player.rotation, playerColors[clientId]);
    }

    // Draw network players
    for (int i = 0; i < MAX_PLAYERS; i++)
    {
      if (networkPlayers[i].active)
      {
        if (i == clientId && !editorMode && !freeCamMode)
          continue;

        DrawPlayerModel(networkPlayers[i].position,
                        networkPlayers[i].rotation,
                        playerColors[i]);
      }
    }

    EndMode3D();

    // Draw UI
    DrawFPS(10, 10);

    if (debugMode)
    {
      DrawText(TextFormat("Player Pos: %.2f, %.2f, %.2f",
                          player.position.x, player.position.y, player.position.z),
               10, 30, 20, RED);
      DrawText(TextFormat("Blocks: %d", blockCount), 10, 50, 20, RED);
      DrawText(TextFormat("Mode: %s",
                          editorMode ? "Editor" : (freeCamMode ? "FreeCam" : "Player")),
               10, 70, 20, RED);
      DrawText(TextFormat("Network: %s",
                          isHost ? "Host" : (isConnected ? "Client" : "Not Connected")),
               10, 90, 20, RED);
      DrawText(TextFormat("Players Online: %d", playerCount),
               10, 110, 20, RED);
      DrawText(TextFormat("Game State: %s",
                          gamePaused ? "PAUSED" : "RUNNING"),
               10, 130, 20, RED);
      DrawText(TextFormat("Client ID: %d", clientId), 10, 150, 20, RED);
      DrawText("Active Players:", 10, 170, 20, RED);
      for (int i = 0; i < MAX_PLAYERS; i++)
      {
        if (networkPlayers[i].active)
        {
          DrawText(TextFormat("Player %d: %.1f, %.1f, %.1f",
                              i,
                              networkPlayers[i].position.x,
                              networkPlayers[i].position.y,
                              networkPlayers[i].position.z),
                   20, 190 + (i * 20), 20, playerColors[i]);
        }
      }
    }

    if (editorMode)
    {
      DrawText("EDITOR MODE", 10, 30, 20, RED);
      DrawText("Controls:", 10, 50, 20, BLACK);
      DrawText("Arrow Keys: Move cursor", 10, 70, 20, DARKGRAY);
      DrawText("PgUp/PgDown: Move Up/Down", 10, 90, 20, DARKGRAY);
      DrawText("1-3: Change color", 10, 110, 20, DARKGRAY);
      DrawText("Enter: Place block", 10, 130, 20, DARKGRAY);
      DrawText("Backspace: Delete last block", 10, 150, 20, DARKGRAY);
      DrawText("TAB: Toggle editor", 10, 170, 20, DARKGRAY);
    }

    EndDrawing();
  }

  // Add cleanup
  if (peer)
  {
    enet_peer_disconnect(peer, 0);
    peer = NULL;
  }
  if (host)
  {
    enet_host_destroy(host);
    host = NULL;
  }
  enet_deinitialize();
  CloseWindow();
  return 0;
}