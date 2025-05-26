#include "constants.h"

// Function declarations
SubmarineState initSubmarine(void);
void updateSubmarineState(SubmarineState *sub, float deltaTime);
void initAudio(void);
void renderSubmarine(SubmarineState sub, float deltaTime, Button* buttons, float hold_depth);

// Global variables
Sound reactorHum;
Sound sonarPing;
bool audioInitialized = false;

int main(void)
{
  InitWindow(SCREEN_WIDTH, SCREEN_HEIGHT, "Advanced Nuclear Submarine Simulator");
  SetTargetFPS(60);

  initAudio();

  SubmarineState sub = initSubmarine();

  Button buttons[] = {
      {(Rectangle){20, 20, 150, 35}, "Reactor Power", false},
      {(Rectangle){20, 60, 150, 35}, "Ballast Control", false},
      {(Rectangle){20, 100, 150, 35}, "Sonar", false},
      {(Rectangle){20, 140, 150, 35}, "Emergency Blow", false},
      {(Rectangle){20, 180, 150, 35}, "Lights", false},
      {(Rectangle){20, 220, 150, 35}, "O2 System", true},
      {(Rectangle){20, 260, 150, 35}, "Cooling System", false},
      {(Rectangle){20, 300, 150, 35}, "Autopilot", false},
      {(Rectangle){20, 340, 150, 35}, "Emergency Surface", false},
  };

  static float hold_depth = -1;

  while (!WindowShouldClose())
  {
    float deltaTime = GetFrameTime();

    // Input handling
    Vector2 mousePos = GetMousePosition();
    bool mousePressed = IsMouseButtonPressed(MOUSE_BUTTON_LEFT);

    for (int i = 0; i < 9; i++)
    {
      if (CheckCollisionPointRec(mousePos, buttons[i].bounds) && mousePressed)
      {
        buttons[i].pressed = !buttons[i].pressed;

        switch (i)
        {
        case 0:
          sub.reactor_active = buttons[i].pressed;
          break;
        case 1:
          sub.ballast_tanks_filled = buttons[i].pressed;
          break;
        case 2:
          sub.sonar_active = buttons[i].pressed;
          break;
        case 3: // Emergency blow
          sub.emergency_surface = buttons[i].pressed;
          if (buttons[i].pressed)
          {
            sub.ballast_level = 0;
            sub.autopilot_active = false;
            buttons[7].pressed = false;
          }
          break;
        case 4:
          sub.lights_active = buttons[i].pressed;
          break;
        case 5:
          sub.oxygen_system_active = buttons[i].pressed;
          break;
        case 6:
          sub.cooling_active = buttons[i].pressed;
          break;
        case 7: // Autopilot
          sub.autopilot_active = buttons[i].pressed;
          if (buttons[i].pressed)
          {
            sub.target_depth = sub.depth;
            sub.emergency_surface = false;
            buttons[3].pressed = false;
          }
          break;
        case 8: // Emergency surface
          sub.emergency_surface = buttons[i].pressed;
          buttons[3].pressed = buttons[i].pressed;
          break;
        }
      }
    }

    // Enhanced keyboard controls
    if (IsKeyPressed(KEY_H))
    {
      hold_depth = sub.depth;
    }

    // Manual depth control
    if (!sub.autopilot_active)
    {
      if (IsKeyDown(KEY_UP) && sub.reactor_active)
      {
        sub.trim_angle = MAX(-30.0f, sub.trim_angle - 30.0f * deltaTime);
        sub.thrust = MIN(100.0f, sub.thrust + 50.0f * deltaTime);
      }
      else if (IsKeyDown(KEY_DOWN))
      {
        sub.trim_angle = MIN(30.0f, sub.trim_angle + 30.0f * deltaTime);
        sub.thrust = MAX(-50.0f, sub.thrust - 30.0f * deltaTime);
      }
      else
      {
        sub.trim_angle *= 0.9f;
        sub.thrust *= 0.95f;
      }
    }

    // Autopilot depth adjustment
    if (sub.autopilot_active)
    {
      if (IsKeyDown(KEY_UP))
      {
        sub.target_depth = MAX(0, sub.target_depth - 50.0f * deltaTime);
      }
      if (IsKeyDown(KEY_DOWN))
      {
        sub.target_depth = MIN(MAX_DEPTH, sub.target_depth + 50.0f * deltaTime);
      }
    }

    updateSubmarineState(&sub, deltaTime);

    // START DRAWING
    BeginDrawing();
    ClearBackground(BLACK);

    renderSubmarine(sub, deltaTime, buttons, hold_depth);

    EndDrawing();
  }

  // Cleanup
  if (audioInitialized)
  {
    UnloadSound(reactorHum);
    UnloadSound(sonarPing);
    CloseAudioDevice();
  }
  CloseWindow();
  return 0;
}