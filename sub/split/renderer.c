#include "constants.h"
#include <string.h>

void drawButton(Button btn, Color color)
{
  DrawRectangleRec(btn.bounds, color);
  DrawRectangleLines(btn.bounds.x, btn.bounds.y, btn.bounds.width, btn.bounds.height, BLACK);
  DrawText(btn.text, btn.bounds.x + 5, btn.bounds.y + 5, 18, BLACK);
}

void drawProgressBar(int x, int y, int width, int height, float value, Color barColor, const char *label)
{
  // Background
  DrawRectangle(x, y, width, height, DARKGRAY);
  DrawRectangleLines(x, y, width, height, WHITE);

  // Fill bar
  int fillWidth = (int)(width * (value / 100.0f));
  DrawRectangle(x + 2, y + 2, fillWidth - 4, height - 4, barColor);

  // Label and value
  DrawText(label, x, y - 20, 16, WHITE);
  DrawText(TextFormat("%.1f%%", value), x + width - 60, y + (height / 2) - 8, 16, WHITE);
}

void drawSystemButton(int x, int y, const char *label, bool active, bool enabled)
{
  Color buttonColor = enabled ? (active ? GREEN : DARKGRAY) : RED;
  Color textColor = enabled ? WHITE : GRAY;

  DrawRectangle(x, y, 120, 25, buttonColor);
  DrawRectangleLines(x, y, 120, 25, WHITE);

  // Status indicator
  DrawCircle(x + 8, y + 12, 5, active ? GREEN : (enabled ? GRAY : RED));

  DrawText(label, x + 18, y + 6, 14, textColor);
}

void drawSubSystemPanel(int x, int y, const char *title, SubmarineState sub)
{
  // Check if backup power provides power for this panel type
  bool backup_power_available = sub.backup_power_active;
  bool main_power_available = sub.battery_level > 5.0f;
  bool any_power_available = main_power_available || backup_power_available;

  if (strcmp(title, "REACTOR SYSTEMS") == 0)
  {
    DrawRectangle(x, y, 280, 180, Fade(BLACK, 0.8f));
    DrawRectangleLines(x, y, 280, 180, WHITE);
    DrawText(title, x + 10, y + 5, 16, YELLOW);

    // Control Rods - Always available (manual)
    drawSystemButton(x + 10, y + 30, "Control Rods", !sub.reactor_control_rods_inserted, true);

    // Coolant Pumps - Needs power (battery OR backup)
    drawSystemButton(x + 140, y + 30, "Coolant Pumps", sub.reactor_coolant_pumps_active, any_power_available);

    // Steam Generator - Needs power (battery OR backup)
    drawSystemButton(x + 10, y + 60, "Power Gen", sub.reactor_steam_generator_active, any_power_available);

    // Power Turbine - Needs power (battery OR backup)
    drawSystemButton(x + 140, y + 60, "Power Turbine", sub.reactor_power_turbine_active, any_power_available);

    // Containment - Needs power (battery OR backup)
    drawSystemButton(x + 10, y + 90, "Containment", sub.reactor_containment_active, any_power_available);

    // Main Reactor - Always available (manual start/stop)
    drawSystemButton(x + 75, y + 120, "MAIN REACTOR", sub.reactor_active, true);

    // Power status indicator
    if (backup_power_available && !main_power_available)
    {
      DrawText("BACKUP POWER", x + 10, y + 150, 12, ORANGE);
    }
    else if (main_power_available)
    {
      DrawText("MAIN POWER", x + 10, y + 150, 12, GREEN);
    }
    else
    {
      DrawText("NO POWER", x + 10, y + 150, 12, RED);
    }
  }
  else if (strcmp(title, "LIFE SUPPORT") == 0)
  {
    DrawRectangle(x, y, 280, 180, Fade(BLACK, 0.8f));
    DrawRectangleLines(x, y, 280, 180, WHITE);
    DrawText(title, x + 10, y + 5, 16, CYAN);

    // All life support systems need power (battery OR backup)
    drawSystemButton(x + 10, y + 30, "Air Circulation", sub.air_circulation_active, any_power_available);
    drawSystemButton(x + 140, y + 30, "CO2 Scrubbers", sub.oxygen_scrubbers_active, any_power_available);
    drawSystemButton(x + 10, y + 60, "O2 Generator", sub.oxygen_generator_active, any_power_available);
    drawSystemButton(x + 140, y + 60, "Hull Monitor", sub.hull_monitoring_active, any_power_available);
    drawSystemButton(x + 10, y + 90, "MAIN O2 SYS", sub.oxygen_system_active, any_power_available);

    // Life support status
    int active_subsystems = (sub.air_circulation_active ? 1 : 0) +
                            (sub.oxygen_scrubbers_active ? 1 : 0) +
                            (sub.oxygen_generator_active ? 1 : 0);

    Color statusColor = (active_subsystems >= 2 && any_power_available) ? GREEN : (active_subsystems >= 1 ? ORANGE : RED);
    DrawText(TextFormat("SUB-SYSTEMS: %d/3", active_subsystems), x + 10, y + 120, 14, statusColor);

    if (sub.oxygen_system_active && active_subsystems >= 2 && any_power_available)
    {
      DrawText("LIFE SUPPORT: OPERATIONAL", x + 10, y + 140, 14, GREEN);
    }
    else
    {
      DrawText("LIFE SUPPORT: DEGRADED", x + 10, y + 140, 14, RED);
    }

    DrawText(TextFormat("O2: %.1f%% (%.1f/min)", sub.oxygen,
                        (sub.oxygen_system_active && any_power_available) ? 1.5f : -2.5f),
             x + 10, y + 160, 12, WHITE);

    // Power status indicator
    if (backup_power_available && !main_power_available)
    {
      DrawText("BACKUP POWER", x + 150, y + 90, 12, ORANGE);
    }
    else if (main_power_available)
    {
      DrawText("MAIN POWER", x + 150, y + 90, 12, GREEN);
    }
    else
    {
      DrawText("NO POWER", x + 150, y + 90, 12, RED);
    }
  }
  else if (strcmp(title, "NAVIGATION") == 0)
  {
    bool nav_power_available = (sub.battery_level > 10.0f) || backup_power_available;

    DrawRectangle(x, y, 280, 180, Fade(BLACK, 0.8f));
    DrawRectangleLines(x, y, 280, 180, WHITE);
    DrawText(title, x + 10, y + 5, 16, LIME);

    // Navigation systems need more power
    drawSystemButton(x + 10, y + 30, "Gyroscope", sub.gyroscope_active, nav_power_available);
    drawSystemButton(x + 140, y + 30, "Nav Computer", sub.navigation_computer_active, nav_power_available);
    drawSystemButton(x + 10, y + 60, "Depth Control", sub.depth_control_active, nav_power_available);

    // Ballast and Communications need power (battery > 0 OR backup)
    bool ballast_power_available = (sub.battery_level > 0.0f) || backup_power_available;

    drawSystemButton(x + 140, y + 60, "Ballast Ctrl", sub.ballast_control_active, ballast_power_available);
    drawSystemButton(x + 10, y + 90, "Communications", sub.communications_active, ballast_power_available);

    // Power status indicator
    if (backup_power_available && !nav_power_available)
    {
      DrawText("BACKUP POWER", x + 10, y + 120, 12, ORANGE);
    }
    else if (nav_power_available)
    {
      DrawText("FULL POWER", x + 10, y + 120, 12, GREEN);
    }
    else
    {
      DrawText("INSUFFICIENT POWER", x + 10, y + 120, 12, RED);
    }
  }
  else if (strcmp(title, "EMERGENCY SYSTEMS") == 0)
  {
    DrawRectangle(x, y, EMERGENCY_PANEL_WIDTH, EMERGENCY_PANEL_HEIGHT, Fade(BLACK, 0.8f));
    DrawRectangleLines(x, y, EMERGENCY_PANEL_WIDTH, EMERGENCY_PANEL_HEIGHT, WHITE);
    DrawText(title, x + 10, y + 5, 16, RED); // Red for emergency

    // Row 1 - Power & Lighting
    drawSystemButton(x + 10, y + 30, "BACKUP POWER", sub.backup_power_active, true); // Always available
    drawSystemButton(x + 140, y + 30, "Emerg Lights", sub.emergency_lighting_active, true);

    // Row 2 - Cooling & Air
    drawSystemButton(x + 10, y + 60, "EMERG COOLING", sub.emergency_cooling_active, true); // MOVED HERE
    drawSystemButton(x + 140, y + 60, "Emerg Air", sub.emergency_air_supply_active, true);

    // Row 3 - Pumps & Fire
    drawSystemButton(x + 10, y + 90, "Manual Bilge", sub.manual_bilge_pumps_active, true);
    drawSystemButton(x + 140, y + 90, "Fire Suppress", sub.fire_suppression_active, true);

    // Row 4 - Surface & Beacon
    drawSystemButton(x + 10, y + 120, "BALLAST BLOW", sub.manual_ballast_blow_active, true);
    drawSystemButton(x + 140, y + 120, "Distress Beacon", sub.distress_beacon_active, true);

    // Emergency status display
    Color statusColor = GREEN;
    const char *statusText = "READY";

    if (sub.hull_integrity < 50.0f)
    {
      statusColor = RED;
      statusText = "HULL BREACH";
    }
    else if (sub.reactor_temp > 400.0f)
    {
      statusColor = RED;
      statusText = "REACTOR CRITICAL";
    }
    else if (sub.oxygen < 30.0f)
    {
      statusColor = ORANGE;
      statusText = "LOW OXYGEN";
    }
    else if (sub.battery_level < 10.0f && !sub.backup_power_active)
    {
      statusColor = ORANGE;
      statusText = "LOW POWER";
    }

    DrawText("EMERGENCY STATUS:", x + 10, y + 150, 14, WHITE);
    DrawText(statusText, x + 10, y + 170, 14, statusColor);

    // Power source display
    if (sub.backup_power_active)
    {
      DrawText("BACKUP POWER: ACTIVE", x + 10, y + 190, 12, GREEN);
    }
    else if (sub.battery_level > 0)
    {
      DrawText("MAIN POWER: ACTIVE", x + 10, y + 190, 12, GREEN);
    }
    else
    {
      DrawText("NO POWER AVAILABLE", x + 10, y + 190, 12, RED);
    }

    // Emergency procedures
    DrawText("EMERGENCY PROCEDURES:", x + 10, y + 220, 14, YELLOW);
    DrawText("1. Start backup power", x + 10, y + 240, 10, LIGHTGRAY);
    DrawText("2. Activate emergency air", x + 10, y + 255, 10, LIGHTGRAY);
    DrawText("3. Manual ballast blow", x + 10, y + 270, 10, LIGHTGRAY);
    DrawText("4. Emergency surface", x + 10, y + 285, 10, LIGHTGRAY);
    DrawText("5. Activate distress beacon", x + 10, y + 300, 10, LIGHTGRAY);

    // Active emergency systems count
    int active_emergency = (sub.backup_power_active ? 1 : 0) +
                           (sub.emergency_lighting_active ? 1 : 0) +
                           (sub.manual_bilge_pumps_active ? 1 : 0) +
                           (sub.emergency_air_supply_active ? 1 : 0) +
                           (sub.manual_ballast_blow_active ? 1 : 0) +
                           (sub.fire_suppression_active ? 1 : 0) +
                           (sub.distress_beacon_active ? 1 : 0);

    DrawText(TextFormat("ACTIVE SYSTEMS: %d/7", active_emergency), x + 10, y + 320, 12,
             active_emergency > 0 ? GREEN : GRAY);
  }
  else
  {
    DrawRectangle(x, y, 280, 180, Fade(BLACK, 0.8f)); // Increased height for more systems
    DrawRectangleLines(x, y, 280, 180, WHITE);
    DrawText(title, x + 10, y + 5, 16, YELLOW);

    if (strcmp(title, "REACTOR SYSTEMS") == 0)
    {
      drawSystemButton(x + 10, y + 30, "Control Rods", !sub.reactor_control_rods_inserted, true);
      drawSystemButton(x + 140, y + 30, "Coolant Pumps", sub.reactor_coolant_pumps_active, sub.battery_level > 5);
      drawSystemButton(x + 10, y + 60, "Steam Gen", sub.reactor_steam_generator_active, sub.battery_level > 5);
      drawSystemButton(x + 140, y + 60, "Power Turbine", sub.reactor_power_turbine_active, sub.battery_level > 5);
      drawSystemButton(x + 10, y + 90, "Containment", sub.reactor_containment_active, sub.battery_level > 5);
      drawSystemButton(x + 75, y + 120, "MAIN REACTOR", sub.reactor_active, sub.battery_level > 10);

      // Reactor status text - UPDATED for faster startup feedback
      Color statusColor = GREEN;
      const char *statusText = "READY";

      if (sub.reactor_destroyed)
      {
        statusColor = RED;
        statusText = "DESTROYED";
      }
      else if (sub.reactor_active && sub.reactor_temp > 50.0f) // LOWERED threshold
      {
        if (sub.reactor_power > 10.0f) // LOWERED from 20%
        {
          statusColor = GREEN;
          statusText = "ONLINE";
        }
        else
        {
          statusColor = YELLOW;
          statusText = "WARMING UP";
        }
      }
      else if (!sub.reactor_control_rods_inserted && sub.reactor_coolant_pumps_active &&
               sub.reactor_steam_generator_active && sub.reactor_power_turbine_active &&
               sub.reactor_containment_active)
      {
        if (sub.reactor_active)
        {
          statusColor = ORANGE;
          statusText = "STARTING UP";
        }
        else
        {
          statusColor = YELLOW;
          statusText = "READY TO START";
        }
      }
      else
      {
        statusColor = ORANGE;
        statusText = "STARTUP REQUIRED";
      }

      DrawText(statusText, x + 10, y + 150, 14, statusColor);
    }
    else if (strcmp(title, "LIFE SUPPORT") == 0)
    {
      drawSystemButton(x + 10, y + 30, "Air Circulation", sub.air_circulation_active, sub.battery_level > 5);
      drawSystemButton(x + 140, y + 30, "CO2 Scrubbers", sub.oxygen_scrubbers_active, sub.battery_level > 5);
      drawSystemButton(x + 10, y + 60, "O2 Generator", sub.oxygen_generator_active, sub.battery_level > 5);
      drawSystemButton(x + 140, y + 60, "Hull Monitor", sub.hull_monitoring_active, sub.battery_level > 5);
      drawSystemButton(x + 10, y + 90, "MAIN O2 SYS", sub.oxygen_system_active, sub.battery_level > 5);

      // Life support status
      int active_subsystems = (sub.air_circulation_active ? 1 : 0) +
                              (sub.oxygen_scrubbers_active ? 1 : 0) +
                              (sub.oxygen_generator_active ? 1 : 0);

      Color statusColor = (active_subsystems >= 2 && sub.battery_level > 5) ? GREEN : (active_subsystems >= 1 ? ORANGE : RED);
      DrawText(TextFormat("SUB-SYSTEMS: %d/3", active_subsystems), x + 10, y + 120, 14, statusColor);

      if (sub.oxygen_system_active && active_subsystems >= 2 && sub.battery_level > 5)
      {
        DrawText("LIFE SUPPORT: OPERATIONAL", x + 10, y + 140, 14, GREEN);
      }
      else
      {
        DrawText("LIFE SUPPORT: DEGRADED", x + 10, y + 140, 14, RED);
      }

      DrawText(TextFormat("O2: %.1f%% (%.1f/min)", sub.oxygen,
                          (sub.oxygen_system_active && sub.battery_level > 5) ? 1.5f : -2.5f),
               x + 10, y + 160, 12, WHITE);
    }
    else if (strcmp(title, "NAVIGATION") == 0)
    {
      bool nav_power_available = (sub.battery_level > 10.0f) || backup_power_available;

      DrawRectangle(x, y, 280, 180, Fade(BLACK, 0.8f));
      DrawRectangleLines(x, y, 280, 180, WHITE);
      DrawText(title, x + 10, y + 5, 16, LIME);

      // Navigation systems need more power
      drawSystemButton(x + 10, y + 30, "Gyroscope", sub.gyroscope_active, nav_power_available);
      drawSystemButton(x + 140, y + 30, "Nav Computer", sub.navigation_computer_active, nav_power_available);
      drawSystemButton(x + 10, y + 60, "Depth Control", sub.depth_control_active, nav_power_available);

      // Ballast and Communications need power (battery > 0 OR backup)
      bool ballast_power_available = (sub.battery_level > 0.0f) || backup_power_available;

      drawSystemButton(x + 140, y + 60, "Ballast Ctrl", sub.ballast_control_active, ballast_power_available);
      drawSystemButton(x + 10, y + 90, "Communications", sub.communications_active, ballast_power_available);

      // Power status indicator
      if (backup_power_available && !nav_power_available)
      {
        DrawText("BACKUP POWER", x + 10, y + 120, 12, ORANGE);
      }
      else if (nav_power_available)
      {
        DrawText("FULL POWER", x + 10, y + 120, 12, GREEN);
      }
      else
      {
        DrawText("INSUFFICIENT POWER", x + 10, y + 120, 12, RED);
      }
    }
  }
}

void drawGauge(int centerX, int centerY, int radius, float value, float maxValue, Color color, const char *label)
{
  // Outer ring
  DrawCircleLines(centerX, centerY, radius, WHITE);
  DrawCircleLines(centerX, centerY, radius - 2, GRAY);

  // Value arc
  float angle = (value / maxValue) * 180.0f; // Half circle
  float startAngle = 180.0f;

  for (int i = 0; i < angle; i++)
  {
    float rad = (startAngle - i) * DEG2RAD;
    int x1 = centerX + (radius - 10) * cosf(rad);
    int y1 = centerY + (radius - 10) * sinf(rad);
    int x2 = centerX + (radius - 5) * cosf(rad);
    int y2 = centerY + (radius - 5) * sinf(rad);
    DrawLine(x1, y1, x2, y2, color);
  }

  // Center text
  DrawText(TextFormat("%.0f", value), centerX - 20, centerY - 8, 20, WHITE);
  DrawText(label, centerX - 30, centerY + 15, 16, WHITE);
}

void initAudio(void)
{
  InitAudioDevice();

  // Create a better reactor hum with multiple harmonics
  int sampleRate = 22050;
  int sampleCount = sampleRate; // 1 second of audio for smoother looping
  float frequency = 45.0f;      // Lower, more ominous frequency

  float *samples = (float *)malloc(sampleCount * sizeof(float));

  for (int i = 0; i < sampleCount; i++)
  {
    float t = (float)i / sampleRate;
    // Multiple harmonics for richer sound
    float sample = 0.0f;
    sample += 0.6f * sinf(2.0f * PI * frequency * t);        // Base frequency
    sample += 0.3f * sinf(2.0f * PI * frequency * 2.0f * t); // Second harmonic
    sample += 0.2f * sinf(2.0f * PI * frequency * 0.5f * t); // Sub harmonic
    sample += 0.1f * sinf(2.0f * PI * frequency * 3.0f * t); // Third harmonic

    // Add some subtle modulation for organic feel
    sample *= (0.8f + 0.2f * sinf(2.0f * PI * 2.0f * t));

    samples[i] = sample * 0.3f; // Reduce overall volume
  }

  Wave wave = {
      .frameCount = sampleCount,
      .sampleRate = sampleRate,
      .sampleSize = 32,
      .channels = 1,
      .data = samples};

  reactorHum = LoadSoundFromWave(wave);
  SetSoundVolume(reactorHum, 0.4f);

  free(samples);

  // Create sonar ping sound
  int pingSampleRate = 22050;
  int pingSampleCount = pingSampleRate * 0.3f; // 0.3 second ping
  float *pingSamples = (float *)malloc(pingSampleCount * sizeof(float));

  for (int i = 0; i < pingSampleCount; i++)
  {
    float t = (float)i / pingSampleRate;
    // Exponential decay envelope
    float envelope = expf(-t * 8.0f);

    // Frequency sweep from 1200Hz to 800Hz
    float freq = 1200.0f - (400.0f * t / 0.3f);
    float sample = envelope * sinf(2.0f * PI * freq * t);

    pingSamples[i] = sample * 0.8f;
  }

  Wave pingWave = {
      .frameCount = pingSampleCount,
      .sampleRate = pingSampleRate,
      .sampleSize = 32,
      .channels = 1,
      .data = pingSamples};

  sonarPing = LoadSoundFromWave(pingWave);
  SetSoundVolume(sonarPing, 0.6f);

  free(pingSamples);

  audioInitialized = true;
}

void drawTopAlarmBanners(SubmarineState sub)
{
  int banner_y = 0;
  static float alarm_flash_timer = 0;
  alarm_flash_timer += GetFrameTime();

  bool flash = (fmodf(alarm_flash_timer, 0.5f) < 0.25f); // Flash every 0.5 seconds

  // REACTOR CRITICAL ALARMS
  if (sub.reactor_destroyed)
  {
    Color banner_color = flash ? RED : MAROON;
    DrawRectangle(0, banner_y, SCREEN_WIDTH, 40, banner_color);
    DrawRectangleLines(0, banner_y, SCREEN_WIDTH, 40, WHITE);
    DrawText("*** REACTOR CORE MELTDOWN *** ABANDON SHIP *** REACTOR CORE MELTDOWN ***",
             SCREEN_WIDTH / 2 - 400, banner_y + 10, 20, WHITE);
    banner_y += 45;
  }
  else if (sub.reactor_temp > REACTOR_CRITICAL_TEMP)
  {
    Color banner_color = flash ? RED : DARKRED;
    DrawRectangle(0, banner_y, SCREEN_WIDTH, 40, banner_color);
    DrawRectangleLines(0, banner_y, SCREEN_WIDTH, 40, WHITE);
    DrawText(TextFormat("*** REACTOR CRITICAL: %.0f°C *** EMERGENCY COOLING REQUIRED ***", sub.reactor_temp),
             SCREEN_WIDTH / 2 - 350, banner_y + 10, 18, WHITE);
    banner_y += 45;
  }
  else if (sub.reactor_temp > REACTOR_WARNING_TEMP)
  {
    Color banner_color = flash ? ORANGE : BROWN;
    DrawRectangle(0, banner_y, SCREEN_WIDTH, 40, banner_color);
    DrawRectangleLines(0, banner_y, SCREEN_WIDTH, 40, WHITE);
    DrawText(TextFormat("*** REACTOR OVERHEATING: %.0f°C *** REDUCE POWER ***", sub.reactor_temp),
             SCREEN_WIDTH / 2 - 280, banner_y + 10, 16, WHITE);
    banner_y += 45;
  }

  // HULL INTEGRITY ALARMS
  if (sub.hull_integrity < 25.0f)
  {
    Color banner_color = flash ? RED : MAROON;
    DrawRectangle(0, banner_y, SCREEN_WIDTH, 40, banner_color);
    DrawRectangleLines(0, banner_y, SCREEN_WIDTH, 40, WHITE);
    DrawText("*** HULL BREACH IMMINENT *** EMERGENCY SURFACE *** HULL BREACH IMMINENT ***",
             SCREEN_WIDTH / 2 - 380, banner_y + 10, 18, WHITE);
    banner_y += 45;
  }
  else if (sub.hull_integrity < 50.0f)
  {
    Color banner_color = flash ? ORANGE : BROWN;
    DrawRectangle(0, banner_y, SCREEN_WIDTH, 40, banner_color);
    DrawRectangleLines(0, banner_y, SCREEN_WIDTH, 40, WHITE);
    DrawText(TextFormat("*** HULL DAMAGE: %.1f%% *** REDUCE DEPTH ***", sub.hull_integrity),
             SCREEN_WIDTH / 2 - 220, banner_y + 10, 16, WHITE);
    banner_y += 45;
  }

  // LIFE SUPPORT ALARMS
  if (sub.oxygen < 15.0f)
  {
    Color banner_color = flash ? RED : MAROON;
    DrawRectangle(0, banner_y, SCREEN_WIDTH, 40, banner_color);
    DrawRectangleLines(0, banner_y, SCREEN_WIDTH, 40, WHITE);
    DrawText("*** OXYGEN CRITICAL *** EMERGENCY SURFACE *** OXYGEN CRITICAL ***",
             SCREEN_WIDTH / 2 - 320, banner_y + 10, 18, WHITE);
    banner_y += 45;
  }
  else if (sub.oxygen < 30.0f)
  {
    Color banner_color = flash ? ORANGE : BROWN;
    DrawRectangle(0, banner_y, SCREEN_WIDTH, 40, banner_color);
    DrawRectangleLines(0, banner_y, SCREEN_WIDTH, 40, WHITE);
    DrawText(TextFormat("*** LOW OXYGEN: %.1f%% *** ACTIVATE LIFE SUPPORT ***", sub.oxygen),
             SCREEN_WIDTH / 2 - 250, banner_y + 10, 16, WHITE);
    banner_y += 45;
  }

  // POWER ALARMS
  if (sub.battery_level < 5.0f && !sub.backup_power_active)
  {
    Color banner_color = flash ? RED : MAROON;
    DrawRectangle(0, banner_y, SCREEN_WIDTH, 40, banner_color);
    DrawRectangleLines(0, banner_y, SCREEN_WIDTH, 40, WHITE);
    DrawText("*** TOTAL POWER FAILURE *** ACTIVATE BACKUP POWER ***",
             SCREEN_WIDTH / 2 - 280, banner_y + 10, 18, WHITE);
    banner_y += 45;
  }
  else if (sub.battery_level < 15.0f && !sub.backup_power_active)
  {
    Color banner_color = flash ? ORANGE : BROWN;
    DrawRectangle(0, banner_y, SCREEN_WIDTH, 40, banner_color);
    DrawRectangleLines(0, banner_y, SCREEN_WIDTH, 40, WHITE);
    DrawText(TextFormat("*** LOW POWER: %.1f%% *** START REACTOR ***", sub.battery_level),
             SCREEN_WIDTH / 2 - 200, banner_y + 10, 16, WHITE);
    banner_y += 45;
  }

  // DEPTH ALARMS
  if (sub.depth > REALISTIC_CRUSH_DEPTH)
  {
    Color banner_color = flash ? RED : MAROON;
    DrawRectangle(0, banner_y, SCREEN_WIDTH, 40, banner_color);
    DrawRectangleLines(0, banner_y, SCREEN_WIDTH, 40, WHITE);
    DrawText("*** CRUSH DEPTH EXCEEDED *** EMERGENCY BALLAST BLOW ***",
             SCREEN_WIDTH / 2 - 300, banner_y + 10, 18, WHITE);
    banner_y += 45;
  }
  else if (sub.depth > REALISTIC_CRUSH_DEPTH * 0.8f)
  {
    Color banner_color = flash ? ORANGE : BROWN;
    DrawRectangle(0, banner_y, SCREEN_WIDTH, 40, banner_color);
    DrawRectangleLines(0, banner_y, SCREEN_WIDTH, 40, WHITE);
    DrawText(TextFormat("*** APPROACHING CRUSH DEPTH: %.0fm ***", sub.depth),
             SCREEN_WIDTH / 2 - 200, banner_y + 10, 16, WHITE);
    banner_y += 45;
  }
}

void renderSubmarine(SubmarineState sub, float deltaTime, Button buttons[])
{
  // Add this line at the beginning to declare hold_depth
  static float hold_depth = -1.0f;

  // Draw alarm banners first (on top)
  drawTopAlarmBanners(sub); // ADD THIS LINE

  // Enhanced lighting
  float light_level = expf(-sub.depth / LIGHT_PENETRATION_DEPTH);
  light_level = MAX(0.001f, MIN(1.0f, light_level));

  // More realistic water colors
  Color water_color;
  if (sub.depth < 50)
  {
    water_color = (Color){30, 144, 255, 255};
  }
  else if (sub.depth < 200)
  {
    water_color = (Color){0, 100, 200, 255};
  }
  else if (sub.depth < 1000)
  {
    water_color = (Color){0, 50, 150, 255};
  }
  else if (sub.depth < 2000)
  {
    water_color = (Color){0, 20, 80, 255};
  }
  else
  {
    water_color = (Color){0, 10, 40, 255};
  }

  water_color.r *= light_level;
  water_color.g *= light_level;
  water_color.b *= light_level;

  DrawRectangle(0, 0, SCREEN_WIDTH, SCREEN_HEIGHT, water_color);

  // Enhanced depth markers with smooth scrolling
  int current_depth = (int)sub.depth;
  int marker_interval = 50;

  for (int i = -6; i <= 6; i++)
  {
    int marker_depth = ((current_depth / marker_interval) * marker_interval) + (i * marker_interval);
    if (marker_depth >= 0 && marker_depth <= MAX_DEPTH)
    {
      float pixel_offset = (sub.depth - current_depth) * 1.6f;
      float y = SCREEN_HEIGHT / 2 + (i * 80) + pixel_offset;

      if (y > -50 && y < SCREEN_HEIGHT + 50)
      {
        DrawLine(0, y, SCREEN_WIDTH, y, Fade(WHITE, 0.3f));
        DrawText(TextFormat("%dm", marker_depth), 10, y - 10, 20, WHITE);

        if (abs(marker_depth - current_depth) < marker_interval / 2)
        {
          DrawRectangle(0, y - 2, 60, 4, YELLOW);
        }
      }
    }
  }

  // Enhanced submarine with rotation
  float subY = SCREEN_HEIGHT / 2;
  float subX = SCREEN_WIDTH / 2;
  float rotation = sub.trim_angle;

  // Submarine body with rotation effect
  Rectangle sub_hull = {subX - 60, subY - 12, 120, 24};
  DrawRectanglePro(sub_hull, (Vector2){60, 12}, rotation, DARKGRAY);

  // Conning tower
  DrawRectangle(subX - 10, subY - 25, 20, 20, GRAY);

  // Propeller with thrust indicator
  Color prop_color = fabsf(sub.thrust) > 10 ? ORANGE : LIGHTGRAY;
  DrawRectangle(subX - 75, subY - 6, 15, 12, prop_color);

  DrawText("SUB", subX - 15, subY - 5, 16, WHITE);

  // BALLAST BUBBLE EFFECTS
  static float bubble_timer = 0;
  bubble_timer += deltaTime;

  // Bubbles when draining ballast (normal operation)
  if (!sub.ballast_tanks_filled && sub.ballast_level > 0.0f)
  {
    // Generate bubbles from ballast vents
    for (int i = 0; i < 8; i++)
    {
      float bubble_offset = fmodf(bubble_timer * 60.0f + i * 45.0f, 360.0f);
      float bubble_y = subY + 15 - (bubble_offset * 0.8f);
      float bubble_x = subX - 40 + (i % 4) * 20 + sinf(bubble_timer * 3.0f + i) * 5;

      if (bubble_y > subY - 200 && bubble_y < subY + 30)
      {
        float bubble_size = 3.0f + sinf(bubble_timer * 4.0f + i) * 1.0f;
        float alpha = MAX(0, 1.0f - (subY + 15 - bubble_y) / 200.0f);
        DrawCircle(bubble_x, bubble_y, bubble_size, Fade(LIGHTBLUE, alpha * 0.6f));
      }
    }
  }

  // EMERGENCY SURFACE BUBBLES (more intense)
  if (sub.manual_ballast_blow_active || sub.emergency_surface)
  {
    // Massive bubble stream during emergency blow
    for (int i = 0; i < 20; i++)
    {
      float emergency_offset = fmodf(bubble_timer * 120.0f + i * 18.0f, 360.0f);
      float bubble_y = subY + 20 - (emergency_offset * 1.2f);
      float bubble_x = subX - 50 + (i % 8) * 12.5f + sinf(bubble_timer * 5.0f + i) * 8;

      if (bubble_y > subY - 300 && bubble_y < subY + 30)
      {
        float bubble_size = 4.0f + sinf(bubble_timer * 6.0f + i) * 2.0f;
        float alpha = MAX(0, 1.0f - (subY + 20 - bubble_y) / 300.0f);
        DrawCircle(bubble_x, bubble_y, bubble_size, Fade(WHITE, alpha * 0.8f));
      }
    }

    // Emergency surface indicator with more dramatic bubbles
    DrawText("EMERGENCY BALLAST BLOW", subX - 80, subY - 40, 12, RED);
  }

  // Enhanced lighting effects
  if (sub.lights_active && sub.battery_level > 5.0f)
  {
    float intensity = 0.6f * (1.0f - light_level) * (sub.battery_level / 100.0f);
    DrawCircle(subX + 70, subY, 150, Fade(YELLOW, intensity * 0.3f));
    DrawCircle(subX + 70, subY, 100, Fade(WHITE, intensity * 0.2f));
  }

  // Enhanced sonar with ping indication
  if (sub.sonar_active && sub.battery_level > 5.0f)
  {
    static float sonar_pulse = 0;
    sonar_pulse += deltaTime * 2.0f;
    float pulse = sinf(sonar_pulse);

    // Normal sonar rings
    DrawCircleLines(subX, subY, 80 + pulse * 20, Fade(GREEN, 0.7f));
    DrawCircleLines(subX, subY, 130 + pulse * 30, Fade(GREEN, 0.5f));
    DrawCircleLines(subX, subY, 180 + pulse * 40, Fade(GREEN, 0.3f));

    // Ping indicator - bright flash when ping occurs
    float ping_progress = sub.sonar_ping_timer / SONAR_PING_INTERVAL;
    if (ping_progress < 0.1f) // Flash for first 10% of ping cycle
    {
      float ping_intensity = (0.1f - ping_progress) / 0.1f; // Fade from 1 to 0
      DrawCircle(subX, subY, 200, Fade(GREEN, ping_intensity * 0.6f));
      DrawCircle(subX, subY, 250, Fade(GREEN, ping_intensity * 0.4f));

      // Sonar range indicator during ping
      DrawCircleLines(subX, subY, SONAR_RANGE * 0.1f, Fade(YELLOW, ping_intensity));
    }

    // Sonar display text
    DrawText(TextFormat("SONAR: %.0fm", SONAR_RANGE), subX - 50, subY + 40, 12, GREEN);
  }

  // Hold/target depth markers
  if (hold_depth >= 0)
  {
    float hold_offset = (hold_depth - sub.depth) * 1.6f;
    float hold_y = SCREEN_HEIGHT / 2 + hold_offset;

    if (hold_y > 0 && hold_y < SCREEN_HEIGHT)
    {
      DrawLine(0, hold_y, SCREEN_WIDTH, hold_y, Fade(YELLOW, 0.8f));
      DrawText(TextFormat("HOLD: %.0fm", hold_depth),
               SCREEN_WIDTH - 180, hold_y - 15, 16, YELLOW);
    }
  }

  if (sub.autopilot_active)
  {
    float target_offset = (sub.target_depth - sub.depth) * 1.6f;
    float target_y = SCREEN_HEIGHT / 2 + target_offset;

    if (target_y > 0 && target_y < SCREEN_HEIGHT)
    {
      DrawLine(0, target_y, SCREEN_WIDTH, target_y, Fade(CYAN, 0.8f));
      DrawText(TextFormat("TARGET: %.0fm", sub.target_depth),
               SCREEN_WIDTH - 180, target_y + 5, 16, CYAN);
    }
  }

  // SUBMARINE SYSTEMS CONTROL PANELS - LEFT SIDE
  drawSubSystemPanel(10, 10, "REACTOR SYSTEMS", sub);
  drawSubSystemPanel(10, 200, "LIFE SUPPORT", sub);
  drawSubSystemPanel(10, 390, "NAVIGATION", sub);

  // EMERGENCY SYSTEMS PANEL - RIGHT SIDE TOP
  drawSubSystemPanel(EMERGENCY_PANEL_X, EMERGENCY_PANEL_Y, "EMERGENCY SYSTEMS", sub);

  // MAIN CONTROLS - BOTTOM RIGHT (RENDER ONLY - NO MOUSE INTERACTION)
  DrawRectangle(SCREEN_WIDTH - 250, SCREEN_HEIGHT - 200, 230, 180, Fade(BLACK, 0.8f));
  DrawRectangleLines(SCREEN_WIDTH - 250, SCREEN_HEIGHT - 200, 230, 180, WHITE);
  DrawText("MAIN CONTROLS", SCREEN_WIDTH - 240, SCREEN_HEIGHT - 190, 14, WHITE);

  for (int i = 0; i < 6; i++)
  {
    int x = SCREEN_WIDTH - 240 + (i % 2) * 105;
    int y = SCREEN_HEIGHT - 160 + (i / 2) * 35;

    Color btnColor = GRAY;

    // ONLY render based on button state - NO MOUSE INTERACTION HERE
    switch (i)
    {
    case 0: // Ballast - check if ballast control has power
      if (sub.battery_level <= 0.0f && !sub.backup_power_active)
        btnColor = DARKGRAY; // No power - ballast control disabled
      else
        btnColor = buttons[i].pressed ? GREEN : GRAY;
      break;
    case 1: // Lights
      btnColor = buttons[i].pressed ? GREEN : GRAY;
      break;
    case 2: // Sonar - check if can be activated
      if (sub.battery_level <= 5.0f)
        btnColor = DARKGRAY; // Can't activate
      else
        btnColor = buttons[i].pressed ? GREEN : GRAY;
      break;
    case 3: // Emergency
      btnColor = buttons[i].pressed ? RED : GRAY;
      break;
    case 4: // Autopilot - check if nav systems are operational
    {
      bool nav_operational = sub.navigation_computer_active && sub.gyroscope_active &&
                             sub.depth_control_active && sub.reactor_active;
      if (!nav_operational)
        btnColor = DARKGRAY; // Can't activate
      else
        btnColor = buttons[i].pressed ? GREEN : GRAY;
    }
    break;
    case 5: // Cooling - check if cooling systems are available
    {
      bool cooling_available = sub.reactor_coolant_pumps_active || sub.emergency_cooling_active;
      if (!cooling_available)
        btnColor = DARKGRAY; // Can't activate
      else
        btnColor = buttons[i].pressed ? GREEN : GRAY;
    }
    break;
    }

    // JUST DRAW THE BUTTON - NO INTERACTION
    DrawRectangle(x, y, 100, 30, btnColor);
    DrawRectangleLines(x, y, 100, 30, WHITE);
    DrawText(buttons[i].text, x + 5, y + 8, 12, WHITE);
  }

  // ENHANCED STATUS PANEL WITH BIGGER TEXT
  int panel_height = SCREEN_HEIGHT - 240;
  DrawRectangle(SCREEN_WIDTH - 420, 10, 400, panel_height, Fade(BLACK, 0.7f));
  DrawRectangleLines(SCREEN_WIDTH - 420, 10, 400, panel_height, WHITE);

  DrawText("SUBMARINE STATUS", SCREEN_WIDTH - 410, 20, 20, WHITE); // Bigger title

  // NAVIGATION & POSITION INFO
  int y_pos = 50;
  DrawText("NAVIGATION:", SCREEN_WIDTH - 410, y_pos, 16, YELLOW); // Bigger section headers
  y_pos += 25;
  DrawText(TextFormat("DEPTH: %.1fm", sub.depth), SCREEN_WIDTH - 410, y_pos, 14, WHITE); // Bigger text
  y_pos += 18;
  DrawText(TextFormat("V.SPEED: %.2fm/s", sub.vertical_speed), SCREEN_WIDTH - 410, y_pos, 12,
           sub.vertical_speed > 0 ? RED : (sub.vertical_speed < -0.5f ? ORANGE : GREEN));
  y_pos += 18;
  DrawText(TextFormat("SPEED: %.1f kts", sub.speed * 1.94f), SCREEN_WIDTH - 410, y_pos, 12, WHITE);
  y_pos += 18;
  DrawText(TextFormat("TRIM: %.2f°", sub.trim_angle), SCREEN_WIDTH - 410, y_pos, 12,
           fabsf(sub.trim_angle) > 15 ? RED : (fabsf(sub.trim_angle) > 5 ? ORANGE : GREEN));

  // REACTOR STATUS
  y_pos += 30;
  DrawText("REACTOR:", SCREEN_WIDTH - 410, y_pos, 16, YELLOW);
  y_pos += 25;

  // Power output progress bar
  Color power_color = sub.reactor_power < 25 ? RED : (sub.reactor_power < 50 ? ORANGE : GREEN);
  drawProgressBar(SCREEN_WIDTH - 410, y_pos, 140, 15, sub.reactor_power, power_color, ""); // Bigger progress bar
  DrawText("POWER", SCREEN_WIDTH - 410, y_pos - 18, 12, WHITE);
  y_pos += 25;

  // Reactor temperature with color coding
  Color reactor_color;
  if (sub.reactor_temp > 20000)
    reactor_color = MAGENTA;
  else if (sub.reactor_temp > 10000)
    reactor_color = RED;
  else if (sub.reactor_temp > 5000)
    reactor_color = ORANGE;
  else if (sub.reactor_temp > 1000)
    reactor_color = YELLOW;
  else if (sub.reactor_temp > 200)
    reactor_color = GREEN;
  else
    reactor_color = GREEN;

  DrawText(TextFormat("TEMP: %.0f°C", sub.reactor_temp), SCREEN_WIDTH - 410, y_pos, 14, reactor_color);
  y_pos += 18;
  DrawText(TextFormat("STATUS: %s", sub.reactor_destroyed ? "DESTROYED" : (sub.reactor_active ? "ONLINE" : "OFFLINE")),
           SCREEN_WIDTH - 410, y_pos, 12, sub.reactor_destroyed ? MAGENTA : (sub.reactor_active ? GREEN : RED));

  // POWER SYSTEMS
  y_pos += 30;
  DrawText("POWER:", SCREEN_WIDTH - 410, y_pos, 16, YELLOW);
  y_pos += 25;

  Color battery_color = sub.battery_level < 20 ? RED : (sub.battery_level < 40 ? ORANGE : GREEN);
  drawProgressBar(SCREEN_WIDTH - 410, y_pos, 140, 15, sub.battery_level, battery_color, "");
  DrawText("BATTERY", SCREEN_WIDTH - 410, y_pos - 18, 12, WHITE);
  y_pos += 25;

  DrawText(TextFormat("LOAD: %.1fkW", sub.power_consumption), SCREEN_WIDTH - 410, y_pos, 12,
           sub.power_consumption > 80 ? RED : (sub.power_consumption > 60 ? ORANGE : GREEN));
  y_pos += 18;
  DrawText(TextFormat("BACKUP: %s", sub.backup_power_active ? "ACTIVE" : "STANDBY"),
           SCREEN_WIDTH - 410, y_pos, 12, sub.backup_power_active ? GREEN : GRAY);

  // HULL STATUS
  y_pos += 30;
  DrawText("HULL:", SCREEN_WIDTH - 410, y_pos, 16, YELLOW);
  y_pos += 25;

  Color hull_color = sub.hull_integrity < 25 ? RED : (sub.hull_integrity < 50 ? ORANGE : GREEN);
  drawProgressBar(SCREEN_WIDTH - 410, y_pos, 140, 15, sub.hull_integrity, hull_color, "");
  DrawText("INTEGRITY", SCREEN_WIDTH - 410, y_pos - 18, 12, WHITE);
  y_pos += 25;

  DrawText(TextFormat("PRESSURE: %.1f bar", sub.depth * 0.1f), SCREEN_WIDTH - 410, y_pos, 12,
           sub.depth > 1500 ? RED : (sub.depth > 1000 ? ORANGE : GREEN));

  // LIFE SUPPORT
  y_pos += 30;
  DrawText("LIFE SUPPORT:", SCREEN_WIDTH - 410, y_pos, 16, YELLOW);
  y_pos += 25;

  Color oxygen_color = sub.oxygen < 20 ? RED : (sub.oxygen < 40 ? ORANGE : GREEN);
  drawProgressBar(SCREEN_WIDTH - 410, y_pos, 140, 15, sub.oxygen, oxygen_color, "");
  DrawText("OXYGEN", SCREEN_WIDTH - 410, y_pos - 18, 12, WHITE);
  y_pos += 25;

  Color hull_temp_color = sub.hull_temperature > 40 ? RED : (sub.hull_temperature > 30 ? ORANGE : (sub.hull_temperature < 5 ? BLUE : GREEN));
  DrawText(TextFormat("INTERNAL: %.1f°C", sub.hull_temperature), SCREEN_WIDTH - 410, y_pos, 12, hull_temp_color);
  y_pos += 18;
  DrawText(TextFormat("EXTERNAL: %.1f°C", sub.water_temperature), SCREEN_WIDTH - 410, y_pos, 12, CYAN);

  // BALLAST SYSTEM
  y_pos += 30;
  DrawText("BALLAST:", SCREEN_WIDTH - 410, y_pos, 16, YELLOW);
  y_pos += 25;

  drawProgressBar(SCREEN_WIDTH - 410, y_pos, 140, 15, sub.ballast_level, BLUE, "");
  DrawText("TANKS", SCREEN_WIDTH - 410, y_pos - 18, 12, WHITE);
  y_pos += 25;

  if (sub.manual_ballast_blow_active)
    DrawText("EMERGENCY BLOW", SCREEN_WIDTH - 410, y_pos, 12, RED);
  else if (!sub.ballast_tanks_filled && sub.ballast_level > 0.0f)
    DrawText("DRAINING", SCREEN_WIDTH - 410, y_pos, 12, ORANGE);
  else if (sub.ballast_tanks_filled && sub.ballast_level < 100.0f)
    DrawText("FILLING", SCREEN_WIDTH - 410, y_pos, 12, YELLOW);
  else if (sub.ballast_tanks_filled)
    DrawText("FULL (DIVING)", SCREEN_WIDTH - 410, y_pos, 12, BLUE);
  else
    DrawText("EMPTY (SURFACE)", SCREEN_WIDTH - 410, y_pos, 12, GREEN);

  // NAVIGATION SYSTEMS
  y_pos += 30;
  DrawText("NAVIGATION:", SCREEN_WIDTH - 410, y_pos, 16, YELLOW);
  y_pos += 25;

  bool nav_operational = sub.navigation_computer_active && sub.gyroscope_active && sub.depth_control_active;
  DrawText(TextFormat("STATUS: %s", nav_operational ? "OPERATIONAL" : "OFFLINE"),
           SCREEN_WIDTH - 410, y_pos, 12, nav_operational ? GREEN : RED);
  y_pos += 18;

  DrawText(TextFormat("AUTOPILOT: %s", sub.autopilot_active ? "ENGAGED" : "MANUAL"),
           SCREEN_WIDTH - 410, y_pos, 12, sub.autopilot_active ? CYAN : GRAY);
  y_pos += 18;

  if (sub.autopilot_active)
  {
    DrawText(TextFormat("TARGET: %.0fm", sub.target_depth), SCREEN_WIDTH - 410, y_pos, 12, CYAN);
    y_pos += 18;
    float depth_error = fabsf(sub.target_depth - sub.depth);
    DrawText(TextFormat("ERROR: %.1fm", depth_error), SCREEN_WIDTH - 410, y_pos, 12,
             depth_error < 5 ? GREEN : (depth_error < 20 ? ORANGE : RED));
  }

  // OTHER SYSTEMS
  y_pos += 30;
  DrawText("SYSTEMS:", SCREEN_WIDTH - 410, y_pos, 16, YELLOW);
  y_pos += 25;

  // SONAR
  if (sub.sonar_active)
    DrawText("SONAR: ACTIVE", SCREEN_WIDTH - 410, y_pos, 12, GREEN);
  else
    DrawText("SONAR: OFFLINE", SCREEN_WIDTH - 410, y_pos, 12, RED);
  y_pos += 18;

  // COOLING
  DrawText(TextFormat("COOLING: %s", sub.cooling_active ? "ACTIVE" : "INACTIVE"),
           SCREEN_WIDTH - 410, y_pos, 12, sub.cooling_active ? GREEN : RED);
  y_pos += 18;

  // LIGHTS
  DrawText(TextFormat("LIGHTS: %s", sub.lights_active ? "ON" : "OFF"),
           SCREEN_WIDTH - 410, y_pos, 12, sub.lights_active ? YELLOW : GRAY);

  // CRITICAL ALERTS (if space allows)
  y_pos += 30;
  if (y_pos < panel_height - 100) // Only show if there's space
  {
    DrawText("ALERTS:", SCREEN_WIDTH - 410, y_pos, 14, RED);
    y_pos += 20;

    if (sub.reactor_destroyed)
    {
      DrawText(">>> REACTOR DESTROYED <<<", SCREEN_WIDTH - 410, y_pos, 12, MAGENTA);
      y_pos += 18;
    }
    else if (sub.reactor_temp > 20000)
    {
      DrawText(">>> REACTOR CRITICAL <<<", SCREEN_WIDTH - 410, y_pos, 12, MAGENTA);
      y_pos += 18;
    }
    else if (sub.reactor_temp > 1000)
    {
      DrawText(">>> REACTOR OVERHEATING <<<", SCREEN_WIDTH - 410, y_pos, 12, ORANGE);
      y_pos += 18;
    }

    if (sub.hull_integrity < 25)
    {
      DrawText(">>> HULL BREACH IMMINENT <<<", SCREEN_WIDTH - 410, y_pos, 12, RED);
      y_pos += 18;
    }

    if (sub.oxygen < 20)
    {
      DrawText(">>> OXYGEN CRITICAL <<<", SCREEN_WIDTH - 410, y_pos, 12, RED);
      y_pos += 18;
    }

    if (sub.battery_level < 10 && !sub.backup_power_active)
    {
      DrawText(">>> POWER CRITICAL <<<", SCREEN_WIDTH - 410, y_pos, 12, ORANGE);
      y_pos += 18;
    }
  }
}