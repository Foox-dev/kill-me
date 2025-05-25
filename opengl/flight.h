#ifndef FLIGHT_H
#define FLIGHT_H

#include <GLFW/glfw3.h>

#define WINDOW_WIDTH 800
#define WINDOW_HEIGHT 600
#define MINIMAP_SIZE 150
#define TERRAIN_SIZE 200
#define TERRAIN_RESOLUTION 5

typedef struct
{
  float x, y;   // Position in terrain
  float height; // Height at this point
  int hasTree;  // Boolean for tree presence
} TerrainPoint;

typedef struct
{
  float x, y, z;  // Position
  float pitch;    // Up/down rotation
  float yaw;      // Left/right rotation
  float roll;     // Roll rotation
  float velocity; // Forward speed
  float lift;     // Vertical force
  int showBigMap; // Toggle for large map view
} Aircraft;

// Function declarations
Aircraft *createAircraft(void);
void updateAircraft(Aircraft *aircraft, float deltaTime);
void handleFlightControls(Aircraft *aircraft, GLFWwindow *window);
void renderAircraft(const Aircraft *aircraft);
float *generateTerrain(void);
void renderTerrain(float *heightmap);
void renderTrees(float *heightmap);
void renderMinimap(const Aircraft *aircraft, float *heightmap);
void renderHUD(const Aircraft *aircraft);
void renderControls(void);
void generateTrees(float *heightmap);

#endif