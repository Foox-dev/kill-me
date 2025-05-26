#ifndef CONSTANTS_H
#define CONSTANTS_H

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

// Global variables
extern Sound reactorHum;
extern Sound sonarPing;
extern bool audioInitialized;

#endif