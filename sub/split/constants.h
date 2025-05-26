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
#define DARKRED (Color){139, 0, 0, 255} // Add this line

#define SCREEN_WIDTH 1920  // Full HD width
#define SCREEN_HEIGHT 1080 // Full HD height
#define MAX_DEPTH 25000.0f // 25km depth

// Enhanced system rates for realism (REALISTIC reactor heating)
#define BALLAST_FILL_RATE 35.0f  // Slower fill - about 3 seconds to fill
#define BALLAST_EMPTY_RATE 50.0f // Slower empty - about 2 seconds to empty

// REALISTIC REACTOR HEATING RATES (based on real nuclear physics)
#define REACTOR_BASE_HEAT_RATE 5.0f       // Base heating when active (°C/sec)
#define REACTOR_EXPONENTIAL_FACTOR 0.001f // Exponential heating factor
#define REACTOR_RESIDUAL_HEAT_RATE 0.5f   // Heat even when shut down (decay heat)
#define REACTOR_COOL_RATE 1.2f            // Cooling rate with pumps
#define REACTOR_WARMUP_RATE 5.0f          // Startup heating
#define REACTOR_EMERGENCY_COOL_RATE 3.0f  // Emergency cooling effectiveness

// Critical temperatures (realistic for naval reactors)
#define REACTOR_NORMAL_TEMP 285.0f   // Normal operating temperature (°C)
#define REACTOR_WARNING_TEMP 320.0f  // Warning threshold
#define REACTOR_CRITICAL_TEMP 380.0f // Critical threshold
#define REACTOR_MELTDOWN_TEMP 450.0f // Meltdown/destruction
#define REACTOR_SCRAM_TEMP 350.0f    // Auto-scram temperature

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
#define BALLAST_EFFECT 5.0f // Effect of ballast on buoyancy

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

  // Main systems
  bool reactor_active;
  bool sonar_active;
  bool ballast_tanks_filled;
  bool lights_active;
  bool oxygen_system_active;
  bool emergency_surface; // New: emergency surface mode
  bool cooling_active;
  bool autopilot_active; // New: depth autopilot

  // Reactor sub-systems
  bool reactor_control_rods_inserted;  // Control rods position
  bool reactor_coolant_pumps_active;   // Coolant circulation
  bool reactor_steam_generator_active; // Steam generation
  bool reactor_power_turbine_active;   // Power turbine

  // Life support sub-systems
  bool oxygen_scrubbers_active; // CO2 scrubbers
  bool oxygen_generator_active; // O2 generator
  bool air_circulation_active;  // Air circulation fans

  // Navigation sub-systems
  bool navigation_computer_active; // Navigation computer
  bool gyroscope_active;           // Gyroscope system
  bool depth_control_active;       // Depth control system

  // Additional reactor sub-systems
  bool reactor_containment_active; // Reactor containment
  bool emergency_cooling_active;   // Emergency cooling

  // Additional life support sub-systems
  bool hull_monitoring_active; // Hull monitoring
  bool backup_power_active;    // Backup power

  // Additional navigation sub-systems
  bool ballast_control_active; // Ballast control
  bool communications_active;  // Communications

  // Emergency/General subsystems
  bool manual_bilge_pumps_active;   // Manual bilge pumps
  bool emergency_lighting_active;   // Emergency lighting
  bool emergency_air_supply_active; // Emergency air supply
  bool manual_ballast_blow_active;  // Manual ballast blow
  bool distress_beacon_active;      // Distress beacon
  bool fire_suppression_active;     // Fire suppression

  float power_consumption;
  float ballast_level;
  float thrust;
  float trim_angle;
  float reactor_power;
  float target_depth;      // New: autopilot target
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

// Function declarations
SubmarineState initSubmarine(void);
void updateSubmarineState(SubmarineState *sub, float deltaTime); // CORRECTED function name
void initAudio(void);
void renderSubmarine(SubmarineState sub, float deltaTime, Button *buttons);
void handleSubSystemInput(SubmarineState *sub);
void drawDial(int centerX, int centerY, int radius, float value, float maxValue, Color color, const char *label, const char *unit);
void drawSmallDial(int centerX, int centerY, int radius, float value, float maxValue, Color color, const char *label);

// Global variables
extern Sound reactorHum;
extern Sound sonarPing;
extern bool audioInitialized;

// Emergency panel coordinates
#define EMERGENCY_PANEL_X 350
#define EMERGENCY_PANEL_Y 10
#define EMERGENCY_PANEL_WIDTH 320
#define EMERGENCY_PANEL_HEIGHT 360

#endif