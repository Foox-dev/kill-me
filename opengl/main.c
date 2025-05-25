#define GLEW_STATIC
#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <GL/glu.h>
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include "flight.h"

#define WINDOW_WIDTH 800
#define WINDOW_HEIGHT 600

Aircraft *plane;
float *heightmap; // Add this global variable
float lastTime;

void setupCamera(const Aircraft *aircraft)
{
  glMatrixMode(GL_PROJECTION);
  glLoadIdentity();
  gluPerspective(45.0f, (float)WINDOW_WIDTH / WINDOW_HEIGHT, 0.1f, 1000.0f);

  glMatrixMode(GL_MODELVIEW);
  glLoadIdentity();

  // Position camera relative to aircraft
  glRotatef(-aircraft->pitch, 1.0f, 0.0f, 0.0f);
  glRotatef(-aircraft->yaw, 0.0f, 1.0f, 0.0f);
  glRotatef(-aircraft->roll, 0.0f, 0.0f, 1.0f);
  glTranslatef(-aircraft->x, -aircraft->y, -aircraft->z);
}

int main(void)
{
  // Initialize GLFW
  if (!glfwInit())
  {
    fprintf(stderr, "Failed to initialize GLFW\n");
    return -1;
  }

  // Create window
  GLFWwindow *window = glfwCreateWindow(800, 600, "Flight Simulator", NULL, NULL);
  if (!window)
  {
    fprintf(stderr, "Failed to create window\n");
    glfwTerminate();
    return -1;
  }

  // Make context current
  glfwMakeContextCurrent(window);

  // Initialize GLEW after creating OpenGL context
  GLenum err = glewInit();
  if (err != GLEW_OK)
  {
    fprintf(stderr, "Failed to initialize GLEW: %s\n", glewGetErrorString(err));
    glfwTerminate();
    return -1;
  }

  // Set up initial OpenGL state
  glEnable(GL_DEPTH_TEST);
  glClearColor(0.5f, 0.7f, 1.0f, 1.0f); // Sky blue background

  // Create aircraft and initialize time
  plane = createAircraft();
  heightmap = generateTerrain();
  generateTrees(heightmap); // Generate trees after terrain
  lastTime = glfwGetTime();

  // Main loop
  while (!glfwWindowShouldClose(window))
  {
    float currentTime = glfwGetTime();
    float deltaTime = currentTime - lastTime;

    // Handle input and update physics
    handleFlightControls(plane, window);
    updateAircraft(plane, deltaTime);

    // Clear buffers
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    // Set up camera
    setupCamera(plane);

    // Render world
    renderTerrain(heightmap);
    renderAircraft(plane);
    renderTrees(heightmap);
    renderHUD(plane);
    renderMinimap(plane, heightmap);
    renderControls();

    // If big map is enabled
    if (plane->showBigMap)
    {
      glPushMatrix();
      glLoadIdentity();
      renderMinimap(plane, heightmap);
    }

    // Swap buffers and poll events
    glfwSwapBuffers(window);
    glfwPollEvents();

    lastTime = currentTime;
  }

  // Cleanup
  free(plane);
  free(heightmap);
  glfwTerminate();
  return 0;
}