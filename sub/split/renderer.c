#include "constants.h"

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

void renderSubmarine(SubmarineState sub, float deltaTime, Button* buttons, float hold_depth)
{
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

  // Draw buttons
  for (int i = 0; i < 9; i++)
  {
    drawButton(buttons[i], buttons[i].pressed ? GREEN : GRAY);
  }

  // ENHANCED HUD with power generation display
  // Main status panel background
  DrawRectangle(SCREEN_WIDTH - 320, 10, 300, 450, Fade(BLACK, 0.7f));
  DrawRectangleLines(SCREEN_WIDTH - 320, 10, 300, 450, WHITE);

  DrawText("SUBMARINE CONTROL", SCREEN_WIDTH - 310, 20, 20, WHITE);

  // Depth display
  DrawText(TextFormat("DEPTH: %.1fm", sub.depth), SCREEN_WIDTH - 310, 50, 18, WHITE);
  DrawText(TextFormat("V.SPEED: %.1fm/s", sub.vertical_speed), SCREEN_WIDTH - 310, 70, 16, GRAY);

  // Reactor status with power generation
  DrawText("REACTOR STATUS:", SCREEN_WIDTH - 310, 100, 16, WHITE);

  // Power generation progress bar
  Color power_color = sub.reactor_power < 25 ? RED : (sub.reactor_power < 50 ? ORANGE : GREEN);
  drawProgressBar(SCREEN_WIDTH - 310, 120, 150, 18, sub.reactor_power, power_color, "POWER GEN");

  // Reactor temperature gauge
  Color reactor_color = sub.reactor_temp > CRITICAL_TEMP ? RED : (sub.reactor_temp > 150 ? ORANGE : GREEN);
  drawGauge(SCREEN_WIDTH - 250, 180, 35, sub.reactor_temp, 400.0f, reactor_color, "TEMP");

  // Power output indicator with efficiency
  float power_efficiency = 1.0f;
  if (sub.reactor_temp > 100.0f && sub.reactor_temp < 280.0f)
  {
    power_efficiency = 1.0f + ((sub.reactor_temp - 100.0f) / 180.0f) * 0.5f;
  }
  else if (sub.reactor_temp >= 280.0f)
  {
    float temp_excess = sub.reactor_temp - 280.0f;
    power_efficiency = MAX(0.0f, 1.5f - (temp_excess / 120.0f) * 1.5f);
  }

  float base_power = sub.reactor_active ? (sub.reactor_power * 10.0f) : 0.0f;
  float actual_power = base_power * power_efficiency;

  Color power_output_color = GREEN;
  if (power_efficiency < 0.5f)
    power_output_color = RED;
  else if (power_efficiency < 0.8f)
    power_output_color = ORANGE;

  DrawText(TextFormat("OUTPUT: %.1f MW (%.0f%%)", actual_power, power_efficiency * 100),
           SCREEN_WIDTH - 310, 220, 14, power_output_color);

  // System status bars
  Color battery_color = sub.battery_level < 20 ? RED : (sub.battery_level < 40 ? ORANGE : GREEN);
  drawProgressBar(SCREEN_WIDTH - 310, 250, 150, 20, sub.battery_level, battery_color, "BATTERY");

  Color hull_color = sub.hull_integrity < 25 ? RED : (sub.hull_integrity < 50 ? ORANGE : GREEN);
  drawProgressBar(SCREEN_WIDTH - 310, 290, 150, 20, sub.hull_integrity, hull_color, "HULL");

  Color oxygen_color = sub.oxygen < 20 ? RED : (sub.oxygen < 40 ? ORANGE : GREEN);
  drawProgressBar(SCREEN_WIDTH - 310, 330, 150, 20, sub.oxygen, oxygen_color, "OXYGEN");

  drawProgressBar(SCREEN_WIDTH - 310, 370, 150, 20, sub.ballast_level, BLUE, "BALLAST");

  // Additional status info
  DrawText(TextFormat("SPEED: %.1f kts", sub.speed * 1.94f), SCREEN_WIDTH - 310, 400, 16, WHITE);
  DrawText(TextFormat("LOAD: %.1fkW", sub.power_consumption), SCREEN_WIDTH - 310, 420, 16, WHITE);

  // Reactor startup progress indicator
  if (sub.reactor_active && sub.reactor_power < 100.0f)
  {
    DrawText(TextFormat("STARTUP: %.0f%%", sub.reactor_power), SCREEN_WIDTH - 150, 420, 14, YELLOW);
  }

  // Enhanced critical warnings with progressive temperature stages
  if (sub.reactor_destroyed)
  {
    DrawRectangle(0, 0, SCREEN_WIDTH, SCREEN_HEIGHT, Fade(WHITE, 0.8f));
    static float explosionFlash = 0;
    explosionFlash += deltaTime * 30;
    Color explosionColor = Fade(RED, 0.9f + 0.1f * sinf(explosionFlash));
    DrawText("*** REACTOR EXPLOSION ***", SCREEN_WIDTH / 2 - 200, 60, 40, explosionColor);
    DrawText("SUBMARINE DESTROYED", SCREEN_WIDTH / 2 - 150, 110, 28, BLACK);
    DrawText("CORE BREACH - RADIATION LEAK", SCREEN_WIDTH / 2 - 180, 140, 24, RED);
    DrawText(TextFormat("TEMP: %.0f°C", sub.reactor_temp), SCREEN_WIDTH / 2 - 80, 170, 24, BLACK);
  }
  else if (sub.reactor_temp > 15000.0f)
  {
    DrawRectangle(0, 0, SCREEN_WIDTH, SCREEN_HEIGHT, Fade(WHITE, 0.6f));
    static float preExplosionFlash = 0;
    preExplosionFlash += deltaTime * 25;
    Color preExplosionColor = Fade(RED, 0.9f + 0.1f * sinf(preExplosionFlash));
    DrawText("!!! CORE BREACH IMMINENT !!!", SCREEN_WIDTH / 2 - 200, 80, 32, preExplosionColor);
    DrawText("REACTOR CRITICAL FAILURE", SCREEN_WIDTH / 2 - 160, 120, 24, BLACK);
    DrawText(TextFormat("TEMP: %.0f°C", sub.reactor_temp), SCREEN_WIDTH / 2 - 80, 150, 24, RED);
  }
  else if (sub.reactor_temp > 500.0f)
  {
    DrawRectangle(0, 0, SCREEN_WIDTH, SCREEN_HEIGHT, Fade(RED, 0.5f));
    static float catastrophicFlash = 0;
    catastrophicFlash += deltaTime * 20;
    Color catastrophicColor = Fade(RED, 0.9f + 0.1f * sinf(catastrophicFlash));
    DrawText("*** CATASTROPHIC MELTDOWN ***", SCREEN_WIDTH / 2 - 200, 80, 32, catastrophicColor);
    DrawText("CORE TEMPERATURE CRITICAL", SCREEN_WIDTH / 2 - 160, 120, 24, WHITE);
    DrawText(TextFormat("TEMP: %.0f°C", sub.reactor_temp), SCREEN_WIDTH / 2 - 80, 150, 24, YELLOW);
  }
  else if (sub.reactor_temp > MELTDOWN_TEMP)
  {
    DrawRectangle(0, 0, SCREEN_WIDTH, SCREEN_HEIGHT, Fade(RED, 0.4f));
    static float meltdownFlash = 0;
    meltdownFlash += deltaTime * 15;
    Color meltdownColor = Fade(RED, 0.8f + 0.2f * sinf(meltdownFlash));
    DrawText("*** REACTOR MELTDOWN ***", SCREEN_WIDTH / 2 - 180, 100, 36, meltdownColor);
    DrawText("HULL INTEGRITY FAILING", SCREEN_WIDTH / 2 - 140, 140, 20, ORANGE);
  }
  else if (sub.reactor_temp > 300.0f)
  {
    DrawRectangle(0, 0, SCREEN_WIDTH, SCREEN_HEIGHT, Fade(RED, 0.3f));
    DrawText("!!! MELTDOWN IMMINENT !!!", SCREEN_WIDTH / 2 - 160, 130, 28, RED);
    DrawText("COOLING SYSTEM STRESSED", SCREEN_WIDTH / 2 - 130, 160, 20, ORANGE);
  }
  else if (sub.reactor_temp > CRITICAL_TEMP)
  {
    static float flash = 0;
    flash += deltaTime * 10;
    Color flashColor = Fade(ORANGE, 0.5f + 0.5f * sinf(flash));
    DrawRectangle(SCREEN_WIDTH - 320, 10, 300, 100, flashColor);
    DrawText("REACTOR CRITICAL!", SCREEN_WIDTH / 2 - 120, 160, 24, ORANGE);
  }
  else if (sub.reactor_temp > 150.0f)
  {
    DrawText("HIGH REACTOR TEMPERATURE", SCREEN_WIDTH / 2 - 140, 190, 20, YELLOW);
    if (sub.cooling_active)
    {
      DrawText("COOLING SYSTEM ACTIVE", SCREEN_WIDTH / 2 - 120, 210, 16, GREEN);
    }
  }

  if (sub.hull_integrity <= 0)
  {
    DrawText("*** HULL BREACH - SUBMARINE LOST ***", SCREEN_WIDTH / 2 - 280, 160, 32, RED);
  }

  if (sub.oxygen <= 0)
  {
    DrawText("*** NO OXYGEN - CREW LOST ***", SCREEN_WIDTH / 2 - 220, 220, 28, RED);
  }

  if (sub.emergency_surface)
  {
    static float emergencyFlash = 0;
    emergencyFlash += deltaTime * 5;
    Color flashColor = Fade(YELLOW, 0.5f + 0.5f * sinf(emergencyFlash));
    DrawRectangle(0, SCREEN_HEIGHT - 50, SCREEN_WIDTH, 50, flashColor);
    DrawText("!!! EMERGENCY SURFACE !!!", SCREEN_WIDTH / 2 - 140, SCREEN_HEIGHT - 35, 24, BLACK);
  }

  // Controls help
  DrawText("CONTROLS:", 200, 370, 16, LIGHTGRAY);
  DrawText("UP/DOWN: Dive Control", 200, 390, 14, LIGHTGRAY);
  DrawText("H: Set Hold Depth", 200, 405, 14, LIGHTGRAY);
  DrawText("Autopilot: Auto depth control", 200, 420, 14, LIGHTGRAY);
}