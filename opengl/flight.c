#define GLEW_STATIC
#include <GL/glew.h>
#include "flight.h"
#include <math.h>
#include <stdlib.h>
#include <stdio.h>

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

#define TERRAIN_SIZE 200
#define TERRAIN_RESOLUTION 5
#define MAX_HEIGHT 20.0f

#define MAX_TREES 1000
#define TREE_HEIGHT 5.0f

typedef struct
{
  float x, z;   // Position
  float height; // Tree height
} Tree;

Aircraft *createAircraft(void)
{
  Aircraft *aircraft = malloc(sizeof(Aircraft));
  aircraft->x = 0.0f;
  aircraft->y = 10.0f; // Start higher to see terrain
  aircraft->z = 0.0f;  // Start at origin
  aircraft->pitch = 0.0f;
  aircraft->yaw = 0.0f;
  aircraft->roll = 0.0f;
  aircraft->velocity = 0.0f;
  aircraft->lift = 0.0f;
  return aircraft;
}

void handleFlightControls(Aircraft *aircraft, GLFWwindow *window)
{
  // Pitch (W/S keys)
  if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
    aircraft->pitch += 1.0f;
  if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
    aircraft->pitch -= 1.0f;

  // Roll (A/D keys)
  if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
    aircraft->roll -= 1.0f;
  if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
    aircraft->roll += 1.0f;

  // Yaw (Q/E keys)
  if (glfwGetKey(window, GLFW_KEY_Q) == GLFW_PRESS)
    aircraft->yaw -= 1.0f;
  if (glfwGetKey(window, GLFW_KEY_E) == GLFW_PRESS)
    aircraft->yaw += 1.0f;

  // Speed (Space/Left Shift)
  if (glfwGetKey(window, GLFW_KEY_SPACE) == GLFW_PRESS)
  {
    aircraft->velocity += 0.5f; // Increased acceleration
  }
  if (glfwGetKey(window, GLFW_KEY_LEFT_SHIFT) == GLFW_PRESS)
  {
    aircraft->velocity -= 0.5f;
  }

  // Add big map toggle
  static int mLastPressed = GLFW_RELEASE;
  int mCurrent = glfwGetKey(window, GLFW_KEY_M);
  if (mCurrent == GLFW_PRESS && mLastPressed == GLFW_RELEASE)
  {
    aircraft->showBigMap = !aircraft->showBigMap;
  }
  mLastPressed = mCurrent;
}

void updateAircraft(Aircraft *aircraft, float deltaTime)
{
  // Basic physics simulation
  float gravity = -9.81f;

  // Update position based on orientation and velocity
  float pitch_rad = aircraft->pitch * M_PI / 180.0f;
  float yaw_rad = aircraft->yaw * M_PI / 180.0f;

  // Forward movement considering pitch and yaw
  aircraft->x += sinf(yaw_rad) * cosf(pitch_rad) * aircraft->velocity * deltaTime;
  aircraft->z += cosf(yaw_rad) * cosf(pitch_rad) * aircraft->velocity * deltaTime;
  aircraft->y += sinf(pitch_rad) * aircraft->velocity * deltaTime;

  // Apply gravity and lift
  aircraft->lift = aircraft->velocity * 0.1f;
  float vertical_force = aircraft->lift + gravity;
  aircraft->y += vertical_force * deltaTime;

  // Clamp values
  if (aircraft->y < 0)
    aircraft->y = 0;
  if (aircraft->velocity < 0)
    aircraft->velocity = 0;
  if (aircraft->velocity > 100)
    aircraft->velocity = 100;
}

void renderAircraft(const Aircraft *aircraft)
{
  glPushMatrix();

  // Apply aircraft transformations
  glTranslatef(aircraft->x, aircraft->y, aircraft->z);
  glRotatef(aircraft->yaw, 0.0f, 1.0f, 0.0f);
  glRotatef(aircraft->pitch, 1.0f, 0.0f, 0.0f);
  glRotatef(aircraft->roll, 0.0f, 0.0f, 1.0f);

  // Draw a simple aircraft shape
  glBegin(GL_TRIANGLES);
  // Fuselage
  glColor3f(0.8f, 0.8f, 0.8f);
  glVertex3f(-0.5f, 0.0f, 2.0f);
  glVertex3f(0.5f, 0.0f, 2.0f);
  glVertex3f(0.0f, 0.0f, -2.0f);

  // Wings
  glColor3f(0.7f, 0.7f, 0.7f);
  glVertex3f(-2.0f, 0.0f, 0.0f);
  glVertex3f(2.0f, 0.0f, 0.0f);
  glVertex3f(0.0f, 0.0f, -1.0f);
  glEnd();

  glPopMatrix();
}

float *generateTerrain()
{
  int vertices = (TERRAIN_SIZE / TERRAIN_RESOLUTION) * (TERRAIN_SIZE / TERRAIN_RESOLUTION);
  float *heightmap = malloc(vertices * sizeof(float));

  // Generate heightmap using simple noise
  for (int z = 0; z < TERRAIN_SIZE / TERRAIN_RESOLUTION; z++)
  {
    for (int x = 0; x < TERRAIN_SIZE / TERRAIN_RESOLUTION; x++)
    {
      float height = sinf(x * 0.1f) * cosf(z * 0.1f) * MAX_HEIGHT;
      height += sinf(x * 0.05f + z * 0.05f) * MAX_HEIGHT * 0.5f;
      heightmap[z * (TERRAIN_SIZE / TERRAIN_RESOLUTION) + x] = height;
    }
  }

  return heightmap;
}

void renderTerrain(float *heightmap)
{
  static float *cached_heightmap = NULL;
  if (!cached_heightmap)
  {
    cached_heightmap = generateTerrain();
  }

  glPushMatrix();
  glColor3f(0.3f, 0.5f, 0.3f);

  // Draw terrain mesh
  for (int z = 0; z < TERRAIN_SIZE / TERRAIN_RESOLUTION - 1; z++)
  {
    glBegin(GL_TRIANGLE_STRIP);
    for (int x = 0; x < TERRAIN_SIZE / TERRAIN_RESOLUTION; x++)
    {
      int index = z * (TERRAIN_SIZE / TERRAIN_RESOLUTION) + x;
      int next_index = (z + 1) * (TERRAIN_SIZE / TERRAIN_RESOLUTION) + x;

      // Calculate vertex colors based on height
      float h1 = heightmap[index] / MAX_HEIGHT;
      float h2 = heightmap[next_index] / MAX_HEIGHT;

      // Current row vertex
      glColor3f(0.2f + h1 * 0.3f, 0.4f + h1 * 0.3f, 0.2f);
      glVertex3f(
          x * TERRAIN_RESOLUTION - TERRAIN_SIZE / 2,
          heightmap[index],
          z * TERRAIN_RESOLUTION - TERRAIN_SIZE / 2);

      // Next row vertex
      glColor3f(0.2f + h2 * 0.3f, 0.4f + h2 * 0.3f, 0.2f);
      glVertex3f(
          x * TERRAIN_RESOLUTION - TERRAIN_SIZE / 2,
          heightmap[next_index],
          (z + 1) * TERRAIN_RESOLUTION - TERRAIN_SIZE / 2);
    }
    glEnd();
  }

  glPopMatrix();
}

void renderHUD(const Aircraft *aircraft)
{
  // Switch to orthographic projection for 2D rendering
  glMatrixMode(GL_PROJECTION);
  glPushMatrix();
  glLoadIdentity();
  glOrtho(0, WINDOW_WIDTH, WINDOW_HEIGHT, 0, -1, 1);

  glMatrixMode(GL_MODELVIEW);
  glPushMatrix();
  glLoadIdentity();

  // Disable depth test for HUD
  glDisable(GL_DEPTH_TEST);

  // Set HUD color and line width
  glColor3f(0.0f, 1.0f, 0.0f);
  glLineWidth(2.0f);

  glBegin(GL_LINES);

  // Crosshair
  glVertex2f(WINDOW_WIDTH / 2 - 20, WINDOW_HEIGHT / 2);
  glVertex2f(WINDOW_WIDTH / 2 + 20, WINDOW_HEIGHT / 2);
  glVertex2f(WINDOW_WIDTH / 2, WINDOW_HEIGHT / 2 - 20);
  glVertex2f(WINDOW_WIDTH / 2, WINDOW_HEIGHT / 2 + 20);

  // Speed indicator (left side)
  float speedHeight = WINDOW_HEIGHT / 2 - (aircraft->velocity * 2.0f);
  glVertex2f(30, WINDOW_HEIGHT / 2 + 50);
  glVertex2f(30, WINDOW_HEIGHT / 2 - 50);
  // Speed marker
  glVertex2f(25, speedHeight);
  glVertex2f(35, speedHeight);

  // Altitude indicator (right side)
  float altHeight = WINDOW_HEIGHT / 2 - (aircraft->y * 2.0f);
  glVertex2f(WINDOW_WIDTH - 30, WINDOW_HEIGHT / 2 + 50);
  glVertex2f(WINDOW_WIDTH - 30, WINDOW_HEIGHT / 2 - 50);
  // Altitude marker
  glVertex2f(WINDOW_WIDTH - 35, altHeight);
  glVertex2f(WINDOW_WIDTH - 25, altHeight);

  // Artificial horizon
  float pitchOffset = aircraft->pitch * 2.0f;
  glVertex2f(WINDOW_WIDTH / 2 - 50, WINDOW_HEIGHT / 2 + pitchOffset);
  glVertex2f(WINDOW_WIDTH / 2 + 50, WINDOW_HEIGHT / 2 + pitchOffset);

  // Roll indicator
  float rollRad = aircraft->roll * M_PI / 180.0f;
  float rx = cosf(rollRad) * 30.0f;
  float ry = sinf(rollRad) * 30.0f;
  glVertex2f(WINDOW_WIDTH / 2 - rx, WINDOW_HEIGHT / 2 - ry);
  glVertex2f(WINDOW_WIDTH / 2 + rx, WINDOW_HEIGHT / 2 + ry);

  glEnd();

  // Reset states
  glLineWidth(1.0f);
  glEnable(GL_DEPTH_TEST);

  // Restore matrices
  glMatrixMode(GL_PROJECTION);
  glPopMatrix();
  glMatrixMode(GL_MODELVIEW);
  glPopMatrix();
}

void renderMinimap(const Aircraft *aircraft, float *heightmap)
{
  glMatrixMode(GL_PROJECTION);
  glPushMatrix();
  glLoadIdentity();
  glOrtho(0, WINDOW_WIDTH, WINDOW_HEIGHT, 0, -1, 1);

  glMatrixMode(GL_MODELVIEW);
  glPushMatrix();
  glLoadIdentity();

  // Draw minimap background
  glColor4f(0.0f, 0.0f, 0.0f, 0.5f);
  glBegin(GL_QUADS);
  glVertex2f(WINDOW_WIDTH - MINIMAP_SIZE - 10, 10);
  glVertex2f(WINDOW_WIDTH - 10, 10);
  glVertex2f(WINDOW_WIDTH - 10, MINIMAP_SIZE + 10);
  glVertex2f(WINDOW_WIDTH - MINIMAP_SIZE - 10, MINIMAP_SIZE + 10);
  glEnd();

  // Draw terrain heightmap
  glPointSize(2.0f);
  glBegin(GL_POINTS);
  for (int z = 0; z < TERRAIN_SIZE / TERRAIN_RESOLUTION; z++)
  {
    for (int x = 0; x < TERRAIN_SIZE / TERRAIN_RESOLUTION; x++)
    {
      float h = heightmap[z * (TERRAIN_SIZE / TERRAIN_RESOLUTION) + x] / MAX_HEIGHT;
      glColor3f(h, h, h);
      float mapX = WINDOW_WIDTH - MINIMAP_SIZE - 5 + (x * MINIMAP_SIZE / (TERRAIN_SIZE / TERRAIN_RESOLUTION));
      float mapY = 15 + (z * MINIMAP_SIZE / (TERRAIN_SIZE / TERRAIN_RESOLUTION));
      glVertex2f(mapX, mapY);
    }
  }
  glEnd();

  // Draw aircraft position
  glColor3f(1.0f, 0.0f, 0.0f);
  glPointSize(4.0f);
  glBegin(GL_POINTS);
  float mapX = WINDOW_WIDTH - MINIMAP_SIZE - 5 + ((aircraft->x + TERRAIN_SIZE / 2) * MINIMAP_SIZE / TERRAIN_SIZE);
  float mapY = 15 + ((aircraft->z + TERRAIN_SIZE / 2) * MINIMAP_SIZE / TERRAIN_SIZE);
  glVertex2f(mapX, mapY);
  glEnd();

  glPopMatrix();
  glMatrixMode(GL_PROJECTION);
  glPopMatrix();
}

void renderControls(void)
{
  glMatrixMode(GL_PROJECTION);
  glPushMatrix();
  glLoadIdentity();
  glOrtho(0, WINDOW_WIDTH, WINDOW_HEIGHT, 0, -1, 1);

  glMatrixMode(GL_MODELVIEW);
  glPushMatrix();
  glLoadIdentity();

  glColor3f(0.0f, 1.0f, 0.0f);
  glLineWidth(2.0f);

  // Draw control indicators using lines
  float startX = 10;
  float startY = 20;
  float spacing = 15;

  // Draw boxes for each control
  for (int i = 0; i < 6; i++)
  {
    float y = startY + i * spacing;
    glBegin(GL_LINE_STRIP);
    glVertex2f(startX, y);
    glVertex2f(startX + 100, y);
    glVertex2f(startX + 100, y + 10);
    glVertex2f(startX, y + 10);
    glVertex2f(startX, y);
    glEnd();
  }

  glLineWidth(1.0f);
  glPopMatrix();
  glMatrixMode(GL_PROJECTION);
  glPopMatrix();
}

static Tree trees[MAX_TREES];
static int numTrees = 0;

void generateTrees(float *heightmap)
{
  srand(42); // Fixed seed for consistent terrain
  numTrees = 0;

  for (int i = 0; i < MAX_TREES; i++)
  {
    float x = (rand() % TERRAIN_SIZE) - TERRAIN_SIZE / 2;
    float z = (rand() % TERRAIN_SIZE) - TERRAIN_SIZE / 2;

    // Get height at this position
    int mapX = (x + TERRAIN_SIZE / 2) / TERRAIN_RESOLUTION;
    int mapZ = (z + TERRAIN_SIZE / 2) / TERRAIN_RESOLUTION;
    float height = heightmap[mapZ * (TERRAIN_SIZE / TERRAIN_RESOLUTION) + mapX];

    trees[numTrees].x = x;
    trees[numTrees].z = z;
    trees[numTrees].height = TREE_HEIGHT + (rand() % 3);
    numTrees++;
  }
}

void renderTrees(float *heightmap)
{
  // Draw trees using heightmap for y-position
  glColor3f(0.2f, 0.5f, 0.2f);
  for (int z = 0; z < TERRAIN_SIZE / TERRAIN_RESOLUTION; z += 5)
  {
    for (int x = 0; x < TERRAIN_SIZE / TERRAIN_RESOLUTION; x += 5)
    {
      if (rand() % 10 == 0) // 10% chance of tree
      {
        float worldX = (x * TERRAIN_RESOLUTION) - TERRAIN_SIZE / 2;
        float worldZ = (z * TERRAIN_RESOLUTION) - TERRAIN_SIZE / 2;
        float height = heightmap[z * (TERRAIN_SIZE / TERRAIN_RESOLUTION) + x];

        glPushMatrix();
        glTranslatef(worldX, height, worldZ);

        // Draw tree (simple triangle)
        glBegin(GL_TRIANGLES);
        glVertex3f(-2.0f, 0.0f, 0.0f);
        glVertex3f(2.0f, 0.0f, 0.0f);
        glVertex3f(0.0f, 4.0f, 0.0f);
        glEnd();

        glPopMatrix();
      }
    }
  }
}