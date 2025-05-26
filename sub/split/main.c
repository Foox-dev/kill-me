#include "constants.h"

// Global variables
Sound reactorHum;
Sound sonarPing;
bool audioInitialized = false;
bool isPaused = false; // Add this line

void syncButtonStates(Button buttons[], SubmarineState submarine)
{
  // Sync button states with actual submarine state to prevent conflicts
  buttons[0].pressed = submarine.ballast_tanks_filled;
  buttons[1].pressed = submarine.lights_active;
  buttons[2].pressed = submarine.sonar_active;
  buttons[3].pressed = submarine.emergency_surface;
  buttons[4].pressed = submarine.autopilot_active;
  buttons[5].pressed = submarine.cooling_active;
}

int main(void)
{
  InitWindow(SCREEN_WIDTH, SCREEN_HEIGHT, "Advanced Nuclear Submarine Simulator");
  SetTargetFPS(60);

  initAudio();

  SubmarineState sub = initSubmarine();

  // Updated main control buttons (bottom right) - now includes autopilot
  Button buttons[] = {
      {(Rectangle){SCREEN_WIDTH - 240, SCREEN_HEIGHT - 160, 100, 30}, "Ballast", false},
      {(Rectangle){SCREEN_WIDTH - 135, SCREEN_HEIGHT - 160, 100, 30}, "Lights", false},
      {(Rectangle){SCREEN_WIDTH - 240, SCREEN_HEIGHT - 125, 100, 30}, "Sonar", false},
      {(Rectangle){SCREEN_WIDTH - 135, SCREEN_HEIGHT - 125, 100, 30}, "Emergency", false},
      {(Rectangle){SCREEN_WIDTH - 240, SCREEN_HEIGHT - 90, 100, 30}, "Autopilot", false},
      {(Rectangle){SCREEN_WIDTH - 135, SCREEN_HEIGHT - 90, 100, 30}, "Cooling", false},
  };

  while (!WindowShouldClose())
  {
    float deltaTime = GetFrameTime();

    // NEW SUBSYSTEM INPUT HANDLING
    handleSubSystemInput(&sub);

    // Enhanced keyboard controls

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

    // Handle pause with Escape key
    if (IsKeyPressed(KEY_ESCAPE))
    {
      isPaused = !isPaused;
    }

    // Only process game input if not paused
    if (!isPaused)
    {
      // Handle keyboard input
      if (IsKeyDown(KEY_UP))
      {
        sub.thrust = MIN(100.0f, sub.thrust + 50.0f * deltaTime);
      }
      else if (IsKeyDown(KEY_DOWN))
      {
        sub.thrust = MAX(-100.0f, sub.thrust - 50.0f * deltaTime);
      }
      else
      {
        // Gradually reduce thrust when no input
        if (sub.thrust > 0)
        {
          sub.thrust = MAX(0, sub.thrust - 25.0f * deltaTime);
        }
        else if (sub.thrust < 0)
        {
          sub.thrust = MIN(0, sub.thrust + 25.0f * deltaTime);
        }
      }

      // Mouse input handling
      Vector2 mousePoint = GetMousePosition();
      bool mousePressed = IsMouseButtonPressed(MOUSE_BUTTON_LEFT);

      if (mousePressed)
      {
        // Main control buttons
        for (int i = 0; i < 6; i++)
        {
          int x = SCREEN_WIDTH - 240 + (i % 2) * 105;
          int y = SCREEN_HEIGHT - 160 + (i / 2) * 35;
          Rectangle buttonRect = {x, y, 100, 30};

          if (CheckCollisionPointRec(mousePoint, buttonRect))
          {
            // Handle each button click
            switch (i)
            {
            case 0: // Ballast - now requires power for normal operation
              // Check if ballast control system has power
              if (sub.battery_level > 0.0f || sub.backup_power_active)
              {
                sub.ballast_tanks_filled = !sub.ballast_tanks_filled;
                buttons[i].pressed = sub.ballast_tanks_filled;
                if (sub.autopilot_active)
                {
                  sub.autopilot_active = false;
                  buttons[4].pressed = false;
                }
              }
              // If no power, ballast button does nothing (except emergency blow still works)
              break;
            case 1: // Lights
              if (sub.battery_level > 5.0f)
              {
                sub.lights_active = !sub.lights_active; // CHANGED: use 'sub' not 'submarine'
                buttons[i].pressed = sub.lights_active;
              }
              break;
            case 2: // Sonar
              if (sub.battery_level > 5.0f)
              {
                sub.sonar_active = !sub.sonar_active; // CHANGED: use 'sub' not 'submarine'
                buttons[i].pressed = sub.sonar_active;
              }
              break;
            case 3:                                           // Emergency
              sub.emergency_surface = !sub.emergency_surface; // CHANGED: use 'sub' not 'submarine'
              buttons[i].pressed = sub.emergency_surface;
              break;
            case 4: // Autopilot
            {
              bool nav_operational = sub.navigation_computer_active &&
                                     sub.gyroscope_active &&
                                     sub.depth_control_active &&
                                     sub.reactor_active;
              if (nav_operational)
              {
                sub.autopilot_active = !sub.autopilot_active; // CHANGED: use 'sub' not 'submarine'
                buttons[i].pressed = sub.autopilot_active;
                if (sub.autopilot_active)
                {
                  sub.target_depth = sub.depth;
                  buttons[0].pressed = sub.ballast_tanks_filled;
                }
              }
            }
            break;
            case 5: // Cooling
            {
              bool cooling_available = sub.reactor_coolant_pumps_active ||
                                       sub.emergency_cooling_active;
              if (cooling_available)
              {
                sub.cooling_active = !sub.cooling_active; // CHANGED: use 'sub' not 'submarine'
                buttons[i].pressed = sub.cooling_active;
              }
            }
            break;
            }
            break; // Exit loop after handling click
          }
        }
      }

      // Update submarine only if not paused
      updateSubmarineState(&sub, deltaTime); // CHANGED: use correct function name

      // Sync button states to prevent flickering
      syncButtonStates(buttons, sub);
    }

    // Audio management - Reactor hum based on TEMPERATURE, not just active state
    if (sub.reactor_temp > 50.0f) // Hum when hot, even if shut down
    {
      if (!IsSoundPlaying(reactorHum))
      {
        PlaySound(reactorHum);
      }

      // Adjust volume based on temperature
      float volume = MIN(1.0f, (sub.reactor_temp - 50.0f) / 300.0f);
      SetSoundVolume(reactorHum, volume * 0.6f);
    }
    else
    {
      if (IsSoundPlaying(reactorHum))
      {
        StopSound(reactorHum);
      }
    }

    // Sonar ping
    if (sub.sonar_active && sub.battery_level > 5.0f)
    {
      sub.sonar_ping_timer -= deltaTime;
      if (sub.sonar_ping_timer <= 0)
      {
        PlaySound(sonarPing);
        sub.sonar_ping_timer = SONAR_PING_INTERVAL;
      }
    }

    // Render everything (including pause overlay)
    BeginDrawing();
    ClearBackground(BLACK);

    renderSubmarine(sub, deltaTime, buttons);

    // Draw pause overlay if paused
    if (isPaused)
    {
      // Semi-transparent overlay
      DrawRectangle(0, 0, SCREEN_WIDTH, SCREEN_HEIGHT, Fade(BLACK, 0.5f));

      // Pause menu
      DrawRectangle(SCREEN_WIDTH / 2 - 200, SCREEN_HEIGHT / 2 - 100, 400, 200, Fade(BLACK, 0.9f));
      DrawRectangleLines(SCREEN_WIDTH / 2 - 200, SCREEN_HEIGHT / 2 - 100, 400, 200, WHITE);

      DrawText("GAME PAUSED", SCREEN_WIDTH / 2 - 100, SCREEN_HEIGHT / 2 - 80, 24, WHITE);
      DrawText("Press ESCAPE to resume", SCREEN_WIDTH / 2 - 120, SCREEN_HEIGHT / 2 - 40, 16, LIGHTGRAY);
      DrawText("Press UP/DOWN to control dive", SCREEN_WIDTH / 2 - 140, SCREEN_HEIGHT / 2 - 10, 14, LIGHTGRAY);
      DrawText("Click panels to control systems", SCREEN_WIDTH / 2 - 140, SCREEN_HEIGHT / 2 + 10, 14, LIGHTGRAY);
      DrawText("Press H to set hold depth", SCREEN_WIDTH / 2 - 120, SCREEN_HEIGHT / 2 + 30, 14, LIGHTGRAY);
      DrawText("Main controls at bottom right", SCREEN_WIDTH / 2 - 130, SCREEN_HEIGHT / 2 + 50, 14, LIGHTGRAY);
    }

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