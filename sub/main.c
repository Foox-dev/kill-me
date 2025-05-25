#include "raylib.h"
#include <stdio.h>
#include <stdbool.h>
#include <math.h>
#include <stdlib.h>

// Helper macros for min/max
#define MAX(a, b) ((a) > (b) ? (a) : (b))
#define MIN(a, b) ((a) < (b) ? (a) : (b))

// Add these color definitions after the includes
#define CYAN (Color){0, 255, 255, 255}
#define LIGHTBLUE (Color){173, 216, 230, 255}

#define SCREEN_WIDTH 1280
#define SCREEN_HEIGHT 720
#define MAX_DEPTH 25000.0f   // 25km depth
#define CRITICAL_TEMP 200.0f // Warning threshold (lowered for realism)
#define OPTIMAL_TEMP 80.0f   // Target temp for cooling
#define MELTDOWN_TEMP 350.0f // Reactor failure temp

// Enhanced system rates for realism (MUCH slower reactor heating)
#define BALLAST_FILL_RATE 2.0f
#define BALLAST_EMPTY_RATE 3.0f
#define REACTOR_HEAT_RATE 0.3f   // Faster heating
#define REACTOR_COOL_RATE 0.4f   // Faster cooling too
#define REACTOR_WARMUP_RATE 5.0f // Much faster warmup
#define BATTERY_CHARGE_RATE 0.8f
#define COOLING_POWER 15.0f // More effective cooling

// Power consumption (more realistic)
#define BATTERY_DRAIN_RATE 0.3f
#define LIGHT_POWER_DRAIN 0.8f
#define SONAR_POWER_DRAIN 1.2f
#define OXYGEN_SYSTEM_DRAIN 0.5f
#define COOLING_SYSTEM_DRAIN 2.0f
#define PROPULSION_POWER_DRAIN 1.5f

// Enhanced physics
#define BUOYANCY_FACTOR 0.8f
#define REALISTIC_CRUSH_DEPTH 2400.0f
#define PRESSURE_DAMAGE_START 500.0f
#define MAX_SPEED 15.0f
#define ACCELERATION 2.0f
#define WATER_RESISTANCE 0.3f
#define LIGHT_PENETRATION_DEPTH 200.0f
#define THERMAL_LAYER_DEPTH 300.0f

// New realistic constants
#define NITROGEN_NARCOSIS_DEPTH 1000.0f
#define EMERGENCY_SURFACE_SPEED 5.0f
#define BALLAST_RESPONSE_TIME 10.0f
#define TRIM_EFFECTIVENESS 0.5f

// Sonar constants
#define SONAR_RANGE 2000.0f      // Maximum sonar range in meters
#define SONAR_PING_INTERVAL 2.0f // Time between pings in seconds

typedef struct
{
  float depth;
  float speed;
  float vertical_speed; // New: separate vertical movement
  float oxygen;
  float reactor_temp;
  float hull_integrity;
  float battery_level;
  float nitrogen_level;       // New: nitrogen narcosis simulation
  float pressure_hull_stress; // New: structural stress
  bool reactor_active;
  bool sonar_active;
  bool ballast_tanks_filled;
  bool lights_active;
  bool oxygen_system_active;
  bool emergency_surface; // New: emergency surface mode
  float power_consumption;
  float ballast_level;
  float thrust;
  float trim_angle;
  float reactor_power;
  bool cooling_active;
  float target_depth;      // New: autopilot target
  bool autopilot_active;   // New: depth autopilot
  float hull_temperature;  // New: hull heating from reactor
  float water_temperature; // New: external water temp
  bool reactor_destroyed;  // NEW: reactor explosion state
  float sonar_ping_timer;  // NEW: timer for sonar pings
} SubmarineState;

typedef struct
{
  Rectangle bounds;
  const char *text;
  bool pressed;
} Button;

// Sound variables
static Sound reactorHum;
static Sound sonarPing; // NEW: sonar ping sound
static bool audioInitialized = false;

SubmarineState initSubmarine(void)
{
  return (SubmarineState){
      .depth = 0,
      .speed = 0,
      .vertical_speed = 0,
      .oxygen = 100,
      .reactor_temp = 25,
      .hull_integrity = 100,
      .battery_level = 80,
      .nitrogen_level = 0,
      .pressure_hull_stress = 0,
      .reactor_active = false,
      .sonar_active = false,
      .ballast_tanks_filled = false,
      .lights_active = false,
      .oxygen_system_active = true,
      .emergency_surface = false,
      .power_consumption = 0,
      .ballast_level = 20,
      .thrust = 0,
      .trim_angle = 0,
      .reactor_power = 0,
      .cooling_active = false,
      .target_depth = 0,
      .autopilot_active = false,
      .hull_temperature = 25,
      .water_temperature = 20,
      .reactor_destroyed = false,
      .sonar_ping_timer = 0}; // NEW
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

void updateSubmarineState(SubmarineState *sub, float deltaTime)
{
  // Enhanced power consumption calculation
  sub->power_consumption = 0.1f; // Base consumption
  if (sub->lights_active)
    sub->power_consumption += LIGHT_POWER_DRAIN;
  if (sub->sonar_active)
    sub->power_consumption += SONAR_POWER_DRAIN;
  if (sub->oxygen_system_active)
    sub->power_consumption += OXYGEN_SYSTEM_DRAIN;
  if (sub->cooling_active)
    sub->power_consumption += COOLING_SYSTEM_DRAIN;
  if (fabsf(sub->thrust) > 0)
    sub->power_consumption += PROPULSION_POWER_DRAIN * (fabsf(sub->thrust) / 100.0f);

  // Realistic water temperature based on depth
  if (sub->depth < THERMAL_LAYER_DEPTH)
  {
    sub->water_temperature = 20.0f - (sub->depth / THERMAL_LAYER_DEPTH) * 15.0f;
  }
  else
  {
    sub->water_temperature = 5.0f - (sub->depth - THERMAL_LAYER_DEPTH) / 1000.0f;
  }
  sub->water_temperature = MAX(2.0f, sub->water_temperature);

  // Enhanced battery management
  bool systems_powered = sub->battery_level > 5.0f;
  if (!sub->reactor_active || sub->reactor_power < 30.0f)
  {
    sub->battery_level = MAX(0, sub->battery_level -
                                    (sub->power_consumption * BATTERY_DRAIN_RATE * deltaTime));
  }

  // Realistic ballast system with physics
  float ballast_target = sub->ballast_tanks_filled ? 100.0f : 0.0f;
  float ballast_rate = sub->ballast_tanks_filled ? BALLAST_FILL_RATE : BALLAST_EMPTY_RATE;
  sub->ballast_level += (ballast_target - sub->ballast_level) * ballast_rate * deltaTime;
  sub->ballast_level = MAX(0, MIN(100, sub->ballast_level));

  // Enhanced buoyancy and depth physics
  float buoyancy_force = (50.0f - sub->ballast_level) * BUOYANCY_FACTOR;
  float trim_effect = sinf(sub->trim_angle * DEG2RAD) * sub->speed * 0.1f;
  float thrust_vertical = sub->thrust * sinf(sub->trim_angle * DEG2RAD) * 0.05f;

  // Emergency surface override
  if (sub->emergency_surface)
  {
    buoyancy_force += EMERGENCY_SURFACE_SPEED;
    sub->ballast_level = MAX(0, sub->ballast_level - BALLAST_EMPTY_RATE * 2.0f * deltaTime);
  }

  // Smooth vertical movement
  float target_vertical = buoyancy_force + trim_effect + thrust_vertical;
  sub->vertical_speed += (target_vertical - sub->vertical_speed) * 2.0f * deltaTime;
  sub->depth = MAX(0, MIN(MAX_DEPTH, sub->depth - sub->vertical_speed * deltaTime));

  // Autopilot system
  if (sub->autopilot_active && sub->reactor_active)
  {
    float depth_error = sub->target_depth - sub->depth;
    if (fabsf(depth_error) > 5.0f)
    {
      if (depth_error > 0)
      {
        // Need to go deeper
        sub->ballast_tanks_filled = true;
        sub->trim_angle = MIN(15.0f, depth_error * 0.1f);
      }
      else
      {
        // Need to surface
        sub->ballast_tanks_filled = false;
        sub->trim_angle = MAX(-15.0f, depth_error * 0.1f);
      }
    }
  }

  // Enhanced speed physics
  float target_speed = 0;
  if (systems_powered && sub->reactor_active && sub->reactor_power > 20.0f)
  {
    target_speed = sub->thrust * MAX_SPEED / 100.0f;
  }

  sub->speed += (target_speed - sub->speed) * ACCELERATION * deltaTime;
  sub->speed *= (1.0f - WATER_RESISTANCE * deltaTime);

  // Realistic pressure effects
  float pressure = sub->depth / 10.0f;                    // Rough atmospheric pressure
  sub->pressure_hull_stress = pressure / 240.0f * 100.0f; // % of crush depth

  if (sub->depth > PRESSURE_DAMAGE_START)
  {
    float damage_factor = powf((sub->depth - PRESSURE_DAMAGE_START) /
                                   (REALISTIC_CRUSH_DEPTH - PRESSURE_DAMAGE_START),
                               2.0f);
    sub->hull_integrity -= damage_factor * 2.0f * deltaTime;
  }

  // Nitrogen narcosis simulation
  if (sub->depth > NITROGEN_NARCOSIS_DEPTH)
  {
    sub->nitrogen_level += (sub->depth - NITROGEN_NARCOSIS_DEPTH) * 0.0001f * deltaTime;
  }
  else
  {
    sub->nitrogen_level = MAX(0, sub->nitrogen_level - 0.1f * deltaTime);
  }

  // Enhanced reactor system with realistic nuclear physics
  if (sub->reactor_active && !sub->reactor_destroyed)
  {
    // Faster reactor warmup
    sub->reactor_power = MIN(100.0f, sub->reactor_power + REACTOR_WARMUP_RATE * deltaTime);

    // Base temperature from reactor operation - MUCH HIGHER without cooling
    float power_factor = sub->reactor_power / 100.0f;
    float base_temp = sub->water_temperature + (300.0f * power_factor);

    // REALISTIC HEATING: Reactor naturally wants to get very hot without cooling
    float target_temp = base_temp;

    // Runaway heating starts earlier and is more aggressive
    if (sub->reactor_temp > 180.0f)
    {
      float excess_temp = sub->reactor_temp - 180.0f;
      float runaway_multiplier = 1.0f + (excess_temp / 200.0f) * 0.8f;
      target_temp *= runaway_multiplier;
    }

    // Critical runaway above 280°C
    if (sub->reactor_temp > 280.0f)
    {
      float critical_excess = sub->reactor_temp - 280.0f;
      target_temp += critical_excess * 1.2f;
    }

    // CATASTROPHIC EXPLOSION at 20000°C
    if (sub->reactor_temp >= 20000.0f)
    {
      sub->reactor_destroyed = true;
      sub->reactor_active = false;
      sub->reactor_power = 0;
      sub->hull_integrity = 0;      // Instant destruction
      sub->reactor_temp = 25000.0f; // Peak explosion temp
    }

    // Heat up towards target temperature - NO ARTIFICIAL LIMITS
    if (!sub->reactor_destroyed)
    {
      float heat_rate = REACTOR_HEAT_RATE * 3.0f;
      sub->reactor_temp += (target_temp - sub->reactor_temp) * heat_rate * deltaTime;
    }

    // Heat transfer to hull
    float heat_transfer_rate = 0.05f + (sub->reactor_temp / 500.0f) * 0.15f;
    sub->hull_temperature += (sub->reactor_temp - sub->hull_temperature) * heat_transfer_rate * deltaTime;

    // Battery charging with power efficiency
    if (sub->reactor_power > 25.0f && !sub->reactor_destroyed)
    {
      float power_efficiency = 1.0f;
      if (sub->reactor_temp > 100.0f && sub->reactor_temp < 280.0f)
      {
        power_efficiency = 1.0f + ((sub->reactor_temp - 100.0f) / 180.0f) * 0.5f;
      }
      else if (sub->reactor_temp >= 280.0f)
      {
        float temp_excess = sub->reactor_temp - 280.0f;
        power_efficiency = MAX(0.0f, 1.5f - (temp_excess / 120.0f) * 1.5f);
      }

      float base_charge_rate = BATTERY_CHARGE_RATE * (sub->reactor_power / 100.0f);
      float actual_charge_rate = base_charge_rate * power_efficiency;
      sub->battery_level = MIN(100.0f, sub->battery_level + actual_charge_rate * deltaTime);
    }
  }
  else
  {
    // Reactor shutdown or destroyed
    if (!sub->reactor_destroyed)
    {
      sub->reactor_power = MAX(0.0f, sub->reactor_power - REACTOR_WARMUP_RATE * 1.5f * deltaTime);
    }
    else
    {
      sub->reactor_power = 0; // No power if destroyed
    }

    // Decay heat continues (even if destroyed)
    float decay_heat_temp = sub->water_temperature;

    if (sub->reactor_destroyed)
    {
      // Explosion aftermath - very hot for a while
      decay_heat_temp = 1000.0f * expf(-deltaTime * 0.1f); // Exponential cooling from explosion
    }
    else if (sub->reactor_temp > 80.0f)
    {
      float decay_factor = (sub->reactor_temp - 80.0f) / 400.0f;
      decay_heat_temp += 60.0f * decay_factor;
    }

    // Runaway can continue even when shut down if temp is extreme
    if (sub->reactor_temp > 350.0f && !sub->reactor_destroyed)
    {
      float runaway_heat = (sub->reactor_temp - 350.0f) * 0.3f;
      decay_heat_temp += runaway_heat;
    }

    // Cool down towards decay heat temperature
    if (sub->reactor_temp > decay_heat_temp)
    {
      float cool_rate = sub->reactor_destroyed ? REACTOR_COOL_RATE * 0.5f : REACTOR_COOL_RATE * 2.0f;
      sub->reactor_temp = MAX(decay_heat_temp, sub->reactor_temp - cool_rate * deltaTime);
    }
  }

  // FIXED COOLING SYSTEM: Applied AFTER reactor heating calculation
  if (sub->cooling_active && systems_powered && !sub->reactor_destroyed)
  {
    float cooling_target = sub->water_temperature + 90.0f; // Target ~110°C
    float cooling_effectiveness = 1.0f;

    // Reduced effectiveness at depth
    cooling_effectiveness *= 1.0f - (sub->depth / MAX_DEPTH) * 0.3f;

    // Cooling becomes less effective at very high temps but NEVER stops completely
    if (sub->reactor_temp > 250.0f)
    {
      float temp_excess = (sub->reactor_temp - 250.0f) / 500.0f;
      cooling_effectiveness *= MAX(0.4f, 1.0f - temp_excess * 0.5f); // Never below 40% effectiveness
    }

    // Above 450°C, cooling system becomes less efficient but still works
    if (sub->reactor_temp > 450.0f)
    {
      cooling_effectiveness *= 0.6f; // Still 60% effective at extreme temps
    }

    // ALWAYS apply cooling when active - this PREVENTS temperature rise
    float cooling_power = COOLING_POWER * 3.0f * cooling_effectiveness; // Stronger cooling

    // Cool towards target temperature OR prevent further heating
    if (sub->reactor_temp > cooling_target)
    {
      // Active cooling - brings temperature down
      sub->reactor_temp = MAX(cooling_target, sub->reactor_temp - cooling_power * deltaTime);
    }
    else
    {
      // Maintenance cooling - prevents temperature from rising above target
      sub->reactor_temp = MIN(sub->reactor_temp, cooling_target);
    }
  }

  // Sonar ping system
  if (sub->sonar_active && systems_powered)
  {
    sub->sonar_ping_timer += deltaTime;

    if (sub->sonar_ping_timer >= SONAR_PING_INTERVAL)
    {
      sub->sonar_ping_timer = 0.0f;

      // Play sonar ping sound
      if (audioInitialized && !IsSoundPlaying(sonarPing))
      {
        // Adjust volume based on depth (sound travels differently)
        float depth_factor = 1.0f - (sub->depth / SONAR_RANGE) * 0.3f;
        depth_factor = MAX(0.3f, depth_factor);
        SetSoundVolume(sonarPing, 0.6f * depth_factor);
        PlaySound(sonarPing);
      }
    }
  }
  else
  {
    sub->sonar_ping_timer = 0.0f; // Reset timer when sonar is off
  }

  // Clamp values
  sub->hull_integrity = MAX(0, MIN(100, sub->hull_integrity));
  sub->oxygen = MAX(0, MIN(100, sub->oxygen));
  sub->battery_level = MAX(0, MIN(100, sub->battery_level));
  sub->nitrogen_level = MAX(0, MIN(100, sub->nitrogen_level));
}

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

    EndDrawing();
  }

  // Cleanup
  if (audioInitialized)
  {
    UnloadSound(reactorHum);
    UnloadSound(sonarPing); // NEW: cleanup sonar sound
    CloseAudioDevice();
  }
  CloseWindow();
  return 0;
}