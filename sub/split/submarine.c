#include "constants.h"

SubmarineState initSubmarine(void)
{
  return (SubmarineState){
      .depth = 0,
      .speed = 0,
      .vertical_speed = 0,
      .oxygen = 100,
      .reactor_temp = 25,
      .hull_integrity = 100,
      .battery_level = 0, // Start at 0% battery
      .nitrogen_level = 0,
      .pressure_hull_stress = 0,

      // Main systems
      .reactor_active = false,
      .sonar_active = false,
      .ballast_tanks_filled = false,
      .lights_active = false,
      .oxygen_system_active = false,
      .emergency_surface = false,
      .cooling_active = false,
      .autopilot_active = false,

      // Reactor sub-systems
      .reactor_control_rods_inserted = true,   // Start with rods IN (safe)
      .reactor_coolant_pumps_active = false,   // Coolant pumps OFF
      .reactor_steam_generator_active = false, // Steam generator OFF
      .reactor_power_turbine_active = false,   // Power turbine OFF

      // Life support sub-systems
      .oxygen_scrubbers_active = false, // CO2 scrubbers OFF
      .oxygen_generator_active = false, // O2 generator OFF
      .air_circulation_active = false,  // Air circulation OFF

      // Navigation sub-systems
      .navigation_computer_active = false, // Nav computer OFF
      .gyroscope_active = false,           // Gyroscope OFF
      .depth_control_active = false,       // Depth control OFF

      // Additional subsystems
      .reactor_containment_active = false, // Reactor containment
      .emergency_cooling_active = false,   // Emergency cooling
      .backup_power_active = false,        // Backup power
      .hull_monitoring_active = false,     // Hull monitoring
      .ballast_control_active = false,     // Ballast control
      .communications_active = false,      // Communications

      // Emergency/General subsystems - ALL MANUAL (no power required)
      .manual_bilge_pumps_active = false,   // Manual bilge pumps
      .emergency_lighting_active = false,   // Emergency lighting
      .emergency_air_supply_active = false, // Emergency air supply
      .manual_ballast_blow_active = false,  // Manual ballast blow
      .distress_beacon_active = false,      // Distress beacon
      .fire_suppression_active = false,     // Fire suppression

      .power_consumption = 0,
      .ballast_level = 20,
      .thrust = 0,
      .trim_angle = 0,
      .reactor_power = 0,
      .target_depth = 0,
      .hull_temperature = 25,
      .water_temperature = 20,
      .reactor_destroyed = false,
      .sonar_ping_timer = 0};
}

// Helper function for button click detection
static bool checkSubSystemButtonClick(int x, int y, int mouseX, int mouseY)
{
  return (mouseX >= x && mouseX <= x + 120 && mouseY >= y && mouseY <= y + 25);
}

// Subsystem input handling function
void handleSubSystemInput(SubmarineState *sub)
{
  if (IsMouseButtonPressed(MOUSE_LEFT_BUTTON))
  {
    Vector2 mousePos = GetMousePosition();
    int mx = (int)mousePos.x;
    int my = (int)mousePos.y;

    // REACTOR SYSTEMS PANEL (10, 10, 280, 180)
    if (mx >= 10 && mx <= 290 && my >= 10 && my <= 190)
    {
      // Control Rods button (20, 40) - NO battery requirement (manual operation)
      if (checkSubSystemButtonClick(20, 40, mx, my))
      {
        sub->reactor_control_rods_inserted = !sub->reactor_control_rods_inserted;
      }
      // Coolant Pumps button (150, 40) - Requires battery OR backup power
      else if (checkSubSystemButtonClick(150, 40, mx, my) &&
               (sub->battery_level > 5 || sub->backup_power_active))
      {
        sub->reactor_coolant_pumps_active = !sub->reactor_coolant_pumps_active;
      }
      // Steam Generator button (20, 70) - Requires battery OR backup power
      else if (checkSubSystemButtonClick(20, 70, mx, my) &&
               (sub->battery_level > 5 || sub->backup_power_active))
      {
        sub->reactor_steam_generator_active = !sub->reactor_steam_generator_active;
      }
      // Power Turbine button (150, 70) - Requires battery OR backup power
      else if (checkSubSystemButtonClick(150, 70, mx, my) &&
               (sub->battery_level > 5 || sub->backup_power_active))
      {
        sub->reactor_power_turbine_active = !sub->reactor_power_turbine_active;
      }
      // Containment button (20, 100) - Requires battery OR backup power
      else if (checkSubSystemButtonClick(20, 100, mx, my) &&
               (sub->battery_level > 5 || sub->backup_power_active))
      {
        sub->reactor_containment_active = !sub->reactor_containment_active;
      }
      // Emergency Cooling button (150, 100) - Requires battery OR backup power
      else if (checkSubSystemButtonClick(150, 100, mx, my) &&
               (sub->battery_level > 10 || sub->backup_power_active))
      {
        sub->emergency_cooling_active = !sub->emergency_cooling_active;
      }
      // Main Reactor button (75, 130) - NO battery requirement for manual start/stop
      else if (checkSubSystemButtonClick(75, 130, mx, my))
      {
        bool reactor_can_operate = !sub->reactor_control_rods_inserted &&
                                   sub->reactor_coolant_pumps_active &&
                                   sub->reactor_steam_generator_active &&
                                   sub->reactor_power_turbine_active &&
                                   sub->reactor_containment_active;
        if (reactor_can_operate || sub->reactor_active) // Allow shutdown even without all systems
        {
          sub->reactor_active = !sub->reactor_active;
        }
      }
    }

    // LIFE SUPPORT PANEL (10, 200, 280, 180)
    else if (mx >= 10 && mx <= 290 && my >= 200 && my <= 380)
    {
      // Air Circulation button (20, 230)
      if (checkSubSystemButtonClick(20, 230, mx, my) &&
          (sub->battery_level > 5 || sub->backup_power_active))
      {
        sub->air_circulation_active = !sub->air_circulation_active;
      }
      // CO2 Scrubbers button (150, 230)
      else if (checkSubSystemButtonClick(150, 230, mx, my) &&
               (sub->battery_level > 5 || sub->backup_power_active))
      {
        sub->oxygen_scrubbers_active = !sub->oxygen_scrubbers_active;
      }
      // O2 Generator button (20, 260)
      else if (checkSubSystemButtonClick(20, 260, mx, my) &&
               (sub->battery_level > 5 || sub->backup_power_active))
      {
        sub->oxygen_generator_active = !sub->oxygen_generator_active;
      }
      // Hull Monitoring button (150, 260)
      else if (checkSubSystemButtonClick(150, 260, mx, my) &&
               (sub->battery_level > 5 || sub->backup_power_active))
      {
        sub->hull_monitoring_active = !sub->hull_monitoring_active;
      }
      // Main O2 System button (20, 290) - STRICT requirements
      else if (checkSubSystemButtonClick(20, 290, mx, my) &&
               (sub->battery_level > 5 || sub->backup_power_active))
      {
        int active_subsystems = (sub->air_circulation_active ? 1 : 0) +
                                (sub->oxygen_scrubbers_active ? 1 : 0) +
                                (sub->oxygen_generator_active ? 1 : 0);
        if (active_subsystems >= 2 || sub->oxygen_system_active)
        {
          sub->oxygen_system_active = !sub->oxygen_system_active;
        }
      }
      // Backup Power button (150, 290) - NO battery requirement (manual start)
      else if (checkSubSystemButtonClick(150, 290, mx, my))
      {
        sub->backup_power_active = !sub->backup_power_active;
      }
    }

    // NAVIGATION PANEL (10, 390, 280, 180)
    else if (mx >= 10 && mx <= 290 && my >= 390 && my <= 570)
    {
      // Gyroscope button (20, 420)
      if (checkSubSystemButtonClick(20, 420, mx, my) &&
          (sub->battery_level > 10 || sub->backup_power_active))
      {
        sub->gyroscope_active = !sub->gyroscope_active;
      }
      // Nav Computer button (150, 420)
      else if (checkSubSystemButtonClick(150, 420, mx, my) &&
               (sub->battery_level > 10 || sub->backup_power_active))
      {
        sub->navigation_computer_active = !sub->navigation_computer_active;
      }
      // Depth Control button (20, 450)
      else if (checkSubSystemButtonClick(20, 450, mx, my) &&
               (sub->battery_level > 10 || sub->backup_power_active))
      {
        sub->depth_control_active = !sub->depth_control_active;
      }
      // Ballast Control button (150, 450) - Requires power (battery > 0 OR backup)
      else if (checkSubSystemButtonClick(150, 450, mx, my) &&
               (sub->battery_level > 0.0f || sub->backup_power_active))
      {
        sub->ballast_control_active = !sub->ballast_control_active;
      }
      // Communications button (20, 480)
      else if (checkSubSystemButtonClick(20, 480, mx, my) &&
               (sub->battery_level > 5 || sub->backup_power_active))
      {
        sub->communications_active = !sub->communications_active;
      }
    }

    // EMERGENCY/GENERAL SYSTEMS PANEL (350, 10, 320, 360) - FIXED COORDINATES
    else if (mx >= 350 && mx <= 670 && my >= 10 && my <= 370)
    {
      // Backup Power button (360, 40) - FIXED
      if (checkSubSystemButtonClick(360, 40, mx, my))
      {
        sub->backup_power_active = !sub->backup_power_active;
      }
      // Emergency Lighting button (490, 40) - FIXED
      else if (checkSubSystemButtonClick(490, 40, mx, my))
      {
        sub->emergency_lighting_active = !sub->emergency_lighting_active;
      }
      // Emergency Cooling button (360, 70) - FIXED
      else if (checkSubSystemButtonClick(360, 70, mx, my))
      {
        sub->emergency_cooling_active = !sub->emergency_cooling_active;
      }
      // Emergency Air Supply button (490, 70) - FIXED
      else if (checkSubSystemButtonClick(490, 70, mx, my))
      {
        sub->emergency_air_supply_active = !sub->emergency_air_supply_active;
      }
      // Manual Bilge Pumps button (360, 100) - FIXED
      else if (checkSubSystemButtonClick(360, 100, mx, my))
      {
        sub->manual_bilge_pumps_active = !sub->manual_bilge_pumps_active;
      }
      // Fire Suppression button (490, 100) - FIXED
      else if (checkSubSystemButtonClick(490, 100, mx, my))
      {
        sub->fire_suppression_active = !sub->fire_suppression_active;
      }
      // Manual Ballast Blow button (360, 130) - FIXED
      else if (checkSubSystemButtonClick(360, 130, mx, my))
      {
        sub->manual_ballast_blow_active = !sub->manual_ballast_blow_active;
        if (sub->manual_ballast_blow_active)
        {
          sub->ballast_tanks_filled = false;
          sub->emergency_surface = true;
        }
      }
      // Distress Beacon button (490, 130) - FIXED
      else if (checkSubSystemButtonClick(490, 130, mx, my))
      {
        sub->distress_beacon_active = !sub->distress_beacon_active;
      }
    }
  }
}

static void updatePowerAndEnvironment(SubmarineState *sub, float deltaTime)
{
  // Power consumption calculation - sub-systems consume power individually
  sub->power_consumption = 0.1f; // Base consumption (emergency systems)

  // Check if we have any power source
  bool has_power = sub->battery_level > 0 || sub->backup_power_active;

  // Main systems
  if (sub->lights_active && has_power)
    sub->power_consumption += LIGHT_POWER_DRAIN;
  if (sub->sonar_active && has_power)
    sub->power_consumption += SONAR_POWER_DRAIN;

  // Reactor sub-systems
  if (sub->reactor_coolant_pumps_active && has_power)
    sub->power_consumption += 0.8f;
  if (sub->reactor_steam_generator_active && has_power)
    sub->power_consumption += 0.3f;
  if (sub->reactor_power_turbine_active && has_power)
    sub->power_consumption += 0.2f;
  if (sub->reactor_containment_active && has_power)
    sub->power_consumption += 0.4f;
  if (sub->emergency_cooling_active && has_power)
    sub->power_consumption += 1.5f; // High power consumption
  if (sub->cooling_active && has_power)
    sub->power_consumption += COOLING_SYSTEM_DRAIN;

  // Life support sub-systems
  if (sub->oxygen_scrubbers_active && has_power)
    sub->power_consumption += 0.5f;
  if (sub->oxygen_generator_active && has_power)
    sub->power_consumption += 1.2f;
  if (sub->air_circulation_active && has_power)
    sub->power_consumption += 0.4f;
  if (sub->hull_monitoring_active && has_power)
    sub->power_consumption += 0.3f;

  // Navigation sub-systems
  if (sub->navigation_computer_active && has_power)
    sub->power_consumption += 0.6f;
  if (sub->gyroscope_active && has_power)
    sub->power_consumption += 0.3f;
  if (sub->depth_control_active && has_power)
    sub->power_consumption += 0.7f;
  if (sub->ballast_control_active && has_power)
    sub->power_consumption += 0.5f;
  if (sub->communications_active && has_power)
    sub->power_consumption += 0.4f;

  if (fabsf(sub->thrust) > 0 && has_power)
    sub->power_consumption += PROPULSION_POWER_DRAIN * (fabsf(sub->thrust) / 100.0f);

  // Backup power has its own consumption but reduces overall load
  if (sub->backup_power_active)
  {
    sub->power_consumption += 0.5f; // Backup power consumes energy from battery
    // But provides alternative power source for critical systems
  }

  // Water temperature based on depth
  if (sub->depth < THERMAL_LAYER_DEPTH)
  {
    sub->water_temperature = 20.0f - (sub->depth / THERMAL_LAYER_DEPTH) * 15.0f;
  }
  else
  {
    sub->water_temperature = 5.0f - (sub->depth - THERMAL_LAYER_DEPTH) / 1000.0f;
  }
  sub->water_temperature = MAX(2.0f, sub->water_temperature);

  // Battery system
  bool battery_overheated = sub->reactor_temp > 600.0f; // MUCH higher threshold
  bool reactor_providing_power = sub->reactor_active && sub->reactor_power > 30.0f && !battery_overheated;

  // Only drain battery if not being charged by reactor
  if (!reactor_providing_power && sub->battery_level > 0)
  {
    float drain_multiplier = battery_overheated ? 2.0f : 1.0f;

    // Only drain if systems are actually consuming power
    if (has_power && sub->power_consumption > 0.1f)
    {
      sub->battery_level = MAX(0, sub->battery_level -
                                      (sub->power_consumption * BATTERY_DRAIN_RATE * drain_multiplier * deltaTime));
    }
  }

  if (battery_overheated && sub->battery_level > 0)
  {
    sub->battery_level = MAX(0, sub->battery_level - 3.0f * deltaTime);
  }

  // Turn off systems that require power when power is lost
  if (!has_power)
  {
    sub->lights_active = false;
    sub->sonar_active = false;
    // Reactor systems already handled in updateReactorSubsystems
    // Life support systems already handled in updateLifeSupportSubsystems
    // Navigation systems already handled in updateNavigationSubsystems
  }

  // SUBMARINE INTERNAL TEMPERATURE CALCULATION
  float base_internal_temp = sub->water_temperature + 10.0f; // Base: water temp + 10°C

  // Reactor heat contribution
  if (sub->reactor_active)
  {
    float reactor_heat_factor = (sub->reactor_temp - 50.0f) / 100.0f; // Heat above 50°C
    reactor_heat_factor = MAX(0.0f, MIN(3.0f, reactor_heat_factor));  // Cap at 3x
    base_internal_temp += reactor_heat_factor * 15.0f;                // Up to +45°C from reactor
  }

  // Life support systems cooling effect
  if (sub->air_circulation_active)
  {
    base_internal_temp -= 5.0f; // Air circulation cools
  }

  // Emergency cooling effect
  if (sub->emergency_cooling_active)
  {
    base_internal_temp -= 8.0f; // Emergency cooling helps internal temp too
  }

  // Depth pressure effect (deeper = colder hull)
  float depth_factor = sub->depth / 1000.0f; // Per 1000m
  base_internal_temp -= depth_factor * 2.0f;

  // Hull breach effect (water flooding)
  if (sub->hull_integrity < 50.0f)
  {
    float breach_factor = (50.0f - sub->hull_integrity) / 50.0f;
    base_internal_temp = sub->water_temperature + (base_internal_temp - sub->water_temperature) * (1.0f - breach_factor);
  }

  // Emergency heating
  if (sub->emergency_lighting_active)
  {
    base_internal_temp += 2.0f; // Emergency lights provide some heat
  }

  // Smooth temperature change
  float temp_change_rate = 0.5f; // How fast internal temp changes
  sub->hull_temperature += (base_internal_temp - sub->hull_temperature) * temp_change_rate * deltaTime;
  sub->hull_temperature = MAX(-10.0f, MIN(80.0f, sub->hull_temperature)); // Reasonable limits
}

static void updateReactorSubsystems(SubmarineState *sub, float deltaTime)
{
  bool systems_powered = sub->battery_level > 5.0f || sub->backup_power_active;
  bool battery_overheated = sub->reactor_temp > 600.0f;

  // Systems fail when overheated at RUNAWAY temps, but backup power can keep them running
  if (battery_overheated && !sub->backup_power_active)
  {
    sub->reactor_coolant_pumps_active = false;
    sub->reactor_steam_generator_active = false;
    sub->reactor_power_turbine_active = false;
    if (sub->reactor_active)
      sub->reactor_active = false; // Emergency shutdown
  }
  // Without any power, electric systems fail
  else if (!systems_powered)
  {
    sub->reactor_coolant_pumps_active = false;
    sub->reactor_steam_generator_active = false;
    sub->reactor_power_turbine_active = false;
  }

  // REALISTIC CASCADING SHUTDOWNS:

  // If coolant pumps fail while reactor is active and hot - EMERGENCY SHUTDOWN
  if (sub->reactor_active && !sub->reactor_coolant_pumps_active && sub->reactor_temp > 100.0f)
  {
    sub->reactor_active = false; // Emergency shutdown - no coolant = danger
  }

  // If steam generator fails - power generation stops
  if (!sub->reactor_steam_generator_active && sub->reactor_active)
  {
    // Reactor can stay on but won't generate power efficiently
    sub->reactor_power = MAX(0.0f, sub->reactor_power - 30.0f * deltaTime);
  }

  // If power turbine fails - no electrical generation
  if (!sub->reactor_power_turbine_active && sub->reactor_active)
  {
    // No electrical power generation from reactor
    // Battery won't charge even if reactor is running
  }

  // If containment fails - reactor becomes dangerous
  if (!sub->reactor_containment_active && sub->reactor_active && sub->reactor_temp > 200.0f)
  {
    // Force emergency shutdown if containment fails at high temps
    sub->reactor_active = false;
  }
}

static void updateLifeSupportSubsystems(SubmarineState *sub, float deltaTime)
{
  bool systems_powered = sub->battery_level > 5.0f || sub->backup_power_active;
  bool battery_overheated = sub->reactor_temp > 600.0f;

  // Life support sub-systems fail without power or when overheated
  if (battery_overheated && !sub->backup_power_active)
  {
    sub->oxygen_scrubbers_active = false;
    sub->oxygen_generator_active = false;
    sub->air_circulation_active = false;
    sub->oxygen_system_active = false;
  }
  else if (!systems_powered)
  {
    sub->oxygen_scrubbers_active = false;
    sub->oxygen_generator_active = false;
    sub->air_circulation_active = false;
    sub->oxygen_system_active = false;
  }

  // REALISTIC LIFE SUPPORT DEPENDENCIES:

  // Main O2 system requires at least 2 sub-systems active
  int active_subsystems = (sub->oxygen_scrubbers_active ? 1 : 0) +
                          (sub->oxygen_generator_active ? 1 : 0) +
                          (sub->air_circulation_active ? 1 : 0);

  // If main O2 system is on but not enough subsystems - automatic shutdown
  if (sub->oxygen_system_active && active_subsystems < 2)
  {
    sub->oxygen_system_active = false;
  }

  // If O2 generator fails - scrubbers become critical
  if (!sub->oxygen_generator_active && sub->oxygen_system_active)
  {
    // O2 generation drops significantly
    // Crew must rely more on scrubbers and emergency air
  }

  // If air circulation fails - CO2 buildup accelerates
  if (!sub->air_circulation_active && sub->oxygen_system_active)
  {
    // Scrubbers work overtime, O2 consumption increases
  }

  // Enhanced oxygen consumption based on sub-system status
  if (sub->oxygen_system_active && active_subsystems >= 2)
  {
    float generation_rate = 1.5f;

    // Bonus for having all systems active
    if (active_subsystems == 3)
    {
      generation_rate = 2.2f; // Full efficiency
    }
    else if (active_subsystems == 2)
    {
      generation_rate = 1.8f; // Reduced efficiency
    }

    sub->oxygen = MIN(100, sub->oxygen + generation_rate * deltaTime);
  }
  else if (sub->emergency_air_supply_active)
  {
    // Emergency air provides basic life support but limited
    float emergency_rate = 0.8f;
    sub->oxygen = MIN(100, sub->oxygen + emergency_rate * deltaTime);
  }
  else
  {
    // Faster consumption without proper life support
    float consumption_rate = 2.5f + (sub->nitrogen_level * 0.3f);

    // Hot reactor = crew stress = more O2 consumption
    if (sub->reactor_temp > 200.0f)
    {
      consumption_rate += (sub->reactor_temp - 200.0f) / 100.0f;
    }

    // No air circulation = CO2 buildup = much worse
    if (!sub->air_circulation_active && !sub->emergency_air_supply_active)
    {
      consumption_rate *= 2.0f; // Double consumption
    }

    sub->oxygen -= consumption_rate * deltaTime;
  }
}

static void updateNavigationSubsystems(SubmarineState *sub)
{
  bool systems_powered = sub->battery_level > 10.0f || sub->backup_power_active;
  bool battery_overheated = sub->reactor_temp > 600.0f;

  // Navigation systems fail without power or when overheated
  if (battery_overheated && !sub->backup_power_active)
  {
    sub->navigation_computer_active = false;
    sub->gyroscope_active = false;
    sub->depth_control_active = false;
    sub->autopilot_active = false;
  }
  else if (!systems_powered)
  {
    sub->navigation_computer_active = false;
    sub->gyroscope_active = false;
    sub->depth_control_active = false;
    sub->autopilot_active = false;
  }

  // REALISTIC NAVIGATION SYSTEM EFFECTS:

  // Without gyroscope - trim control becomes unstable
  if (!sub->gyroscope_active)
  {
    // Add random drift to trim angle
    static float drift_timer = 0;
    drift_timer += 0.016f;  // ~60fps
    if (drift_timer > 2.0f) // Every 2 seconds
    {
      sub->trim_angle += (rand() % 3 - 1) * 0.5f; // Random drift ±0.5°
      sub->trim_angle = MAX(-20.0f, MIN(20.0f, sub->trim_angle));
      drift_timer = 0;
    }
  }

  // Without navigation computer - no precise depth readings
  if (!sub->navigation_computer_active)
  {
    // Depth readings become inaccurate (this affects display only)
    // Could add noise to depth display in renderer
  }

  // Without depth control - ballast becomes manual only
  if (!sub->depth_control_active)
  {
    // Autopilot cannot function
    sub->autopilot_active = false;
    // Ballast control becomes less precise
  }

  // Ballast control affects ballast response
  if (!sub->ballast_control_active)
  {
    // Ballast operates at 50% efficiency
    // This is handled in updatePhysics
  }

  // Autopilot requires ALL navigation sub-systems
  bool navigation_operational = sub->navigation_computer_active &&
                                sub->gyroscope_active &&
                                sub->depth_control_active &&
                                sub->reactor_active;

  if (sub->autopilot_active && !navigation_operational)
  {
    sub->autopilot_active = false;
  }

  // IMPROVED AUTOPILOT with PID control
  if (sub->autopilot_active && navigation_operational)
  {
    static float integral_error = 0.0f;
    static float previous_error = 0.0f;

    float depth_error = sub->target_depth - sub->depth;

    // PID constants - tuned to prevent overshoot
    float Kp = 0.05f;
    float Ki = 0.01f;
    float Kd = 0.08f;

    // Calculate PID terms
    integral_error += depth_error * 0.016f;
    integral_error = MAX(-50.0f, MIN(50.0f, integral_error));

    float derivative_error = (depth_error - previous_error) / 0.016f;
    previous_error = depth_error;

    // PID output
    float control_output = (Kp * depth_error) + (Ki * integral_error) + (Kd * derivative_error);

    // Apply control with deadband to prevent oscillation
    if (fabsf(depth_error) > 2.0f)
    {
      if (control_output > 0)
      {
        sub->ballast_tanks_filled = true;
        sub->trim_angle = MIN(10.0f, fabsf(control_output) * 2.0f);
      }
      else if (control_output < 0)
      {
        sub->ballast_tanks_filled = false;
        sub->trim_angle = MAX(-10.0f, -fabsf(control_output) * 2.0f);
      }
    }
    else
    {
      // Within acceptable range - neutral trim
      sub->trim_angle *= 0.9f;
      if (fabsf(sub->trim_angle) < 0.5f)
        sub->trim_angle = 0.0f;
    }
  }
}

// Replace the updateReactor function with faster startup logic:

static void updateReactor(SubmarineState *sub, float deltaTime)
{
  bool systems_powered = sub->battery_level > 5.0f || sub->backup_power_active;

  // Reactor heating - FASTER STARTUP WITH REALISTIC EXPONENTIAL HEATING
  if (sub->reactor_active && !sub->reactor_control_rods_inserted)
  {
    // FAST STARTUP PHASE - when reactor is cold, heat up very quickly
    if (sub->reactor_temp < REACTOR_NORMAL_TEMP * 0.5f) // Below 50% of normal temp
    {
      // Rapid startup heating
      float startup_heating = REACTOR_WARMUP_RATE * deltaTime;
      sub->reactor_temp += startup_heating;
    }
    else
    {
      // Normal operation heating with exponential curve
      float base_heating = REACTOR_BASE_HEAT_RATE * deltaTime;

      // Exponential heating based on current temperature (runaway effect)
      float exponential_heating = sub->reactor_temp * REACTOR_EXPONENTIAL_FACTOR * deltaTime;

      // Total heating rate increases exponentially with temperature
      float total_heating = base_heating + exponential_heating;

      sub->reactor_temp += total_heating;
    }
  }
  else if (!sub->reactor_active || sub->reactor_control_rods_inserted)
  {
    // RESIDUAL/DECAY HEAT - reactors stay hot even when shut down
    if (sub->reactor_temp > 20.0f) // Only if reactor has been heated
    {
      // Decay heat decreases over time but is always present
      float decay_factor = MAX(0.1f, (sub->reactor_temp / REACTOR_NORMAL_TEMP));
      float decay_heat = REACTOR_RESIDUAL_HEAT_RATE * decay_factor * deltaTime;
      sub->reactor_temp += decay_heat;
    }
  }

  // Cooling systems
  float cooling_rate = 0.0f;

  // Primary cooling (coolant pumps)
  if (sub->reactor_coolant_pumps_active && systems_powered)
  {
    cooling_rate += REACTOR_COOL_RATE;
  }

  // Emergency cooling (more effective)
  if (sub->emergency_cooling_active)
  {
    cooling_rate += REACTOR_EMERGENCY_COOL_RATE;
  }

  // Manual cooling system
  if (sub->cooling_active)
  {
    cooling_rate += REACTOR_COOL_RATE * 0.8f;
  }

  // Apply cooling
  sub->reactor_temp -= cooling_rate * deltaTime;

  // Natural heat dissipation to hull (very slow)
  float heat_dissipation = (sub->reactor_temp - sub->water_temperature) * 0.01f * deltaTime;
  sub->reactor_temp -= heat_dissipation;

  // Transfer heat to hull
  if (sub->reactor_temp > sub->hull_temperature)
  {
    float heat_transfer = (sub->reactor_temp - sub->hull_temperature) * 0.005f * deltaTime;
    sub->hull_temperature += heat_transfer;
  }

  // Minimum temperature (ambient)
  sub->reactor_temp = MAX(sub->water_temperature, sub->reactor_temp);

  // AUTOMATIC SAFETY SYSTEMS
  if (sub->reactor_temp > REACTOR_SCRAM_TEMP && sub->reactor_active)
  {
    // Automatic reactor scram (emergency shutdown)
    sub->reactor_active = false;
    sub->reactor_control_rods_inserted = true;
  }

  // REACTOR DESTRUCTION
  if (sub->reactor_temp > REACTOR_MELTDOWN_TEMP && !sub->reactor_destroyed)
  {
    sub->reactor_destroyed = true;
    sub->reactor_active = false;
    sub->hull_integrity -= 50.0f;                       // Massive hull damage
    sub->reactor_temp = REACTOR_MELTDOWN_TEMP + 100.0f; // Stays very hot
  }

  // Power generation (only if all systems operational and not destroyed)
  // FASTER POWER GENERATION - start producing power at lower temperatures
  if (sub->reactor_active && !sub->reactor_destroyed &&
      sub->reactor_steam_generator_active &&
      sub->reactor_power_turbine_active &&
      systems_powered)
  {
    // Power output based on temperature efficiency - LOWER threshold for power generation
    float efficiency = 1.0f;
    if (sub->reactor_temp > REACTOR_CRITICAL_TEMP)
    {
      efficiency = 0.3f; // Severely reduced efficiency at high temps
    }
    else if (sub->reactor_temp > REACTOR_WARNING_TEMP)
    {
      efficiency = 0.7f; // Reduced efficiency
    }
    else if (sub->reactor_temp < 50.0f) // REDUCED from 100°C - start generating power sooner
    {
      efficiency = 0.2f; // Poor efficiency when cold
    }

    // FASTER power ramp-up - use a lower temperature threshold
    float temp_factor = sub->reactor_temp / (REACTOR_NORMAL_TEMP * 0.7f); // Use 70% of normal temp as 100% power
    sub->reactor_power = MIN(100.0f, temp_factor * 100.0f * efficiency);

    // Charge battery when reactor is producing power - LOWER threshold
    if (sub->reactor_power > 10.0f) // Reduced from 20% to 10%
    {
      float charge_rate = (sub->reactor_power / 100.0f) * BATTERY_CHARGE_RATE * deltaTime;
      sub->battery_level += charge_rate;
      sub->battery_level = MIN(100.0f, sub->battery_level);
    }
  }
  else
  {
    // No power generation
    sub->reactor_power = 0.0f;
  }
}
static void updateCoolingSystem(SubmarineState *sub, float deltaTime)
{
  bool systems_powered = sub->battery_level > 10.0f || sub->backup_power_active;
  bool cooling_systems_available = sub->reactor_coolant_pumps_active || sub->emergency_cooling_active;

  // Turn off cooling if reactor is not active
  if (!sub->reactor_active)
  {
    sub->cooling_active = false;
    return;
  }

  // ACTUALLY WORKING COOLING SYSTEM
  if (sub->cooling_active && systems_powered && cooling_systems_available && !sub->reactor_destroyed)
  {
    // STRONG cooling that actually maintains temperature
    float cooling_power = 500.0f; // Much stronger base cooling

    // Emergency cooling is VERY effective
    if (sub->emergency_cooling_active && sub->reactor_coolant_pumps_active)
    {
      cooling_power = 800.0f; // Extremely strong
    }
    else if (sub->emergency_cooling_active)
    {
      cooling_power = 650.0f; // Very strong
    }

    // Target temperature - stable at 120°C
    float target_temp = sub->water_temperature + 95.0f;

    // FORCE cooling towards target
    if (sub->reactor_temp > target_temp)
    {
      float temp_diff = sub->reactor_temp - target_temp;
      float cooling_rate = cooling_power + (temp_diff * 2.0f); // More cooling for higher temps
      sub->reactor_temp = MAX(target_temp, sub->reactor_temp - cooling_rate * deltaTime);
    }
  }
  else if (sub->cooling_active && (!systems_powered || !cooling_systems_available))
  {
    sub->cooling_active = false; // Cooling fails without power or pumps
  }
}
static void updateSonar(SubmarineState *sub, float deltaTime)
{
  bool systems_powered = sub->battery_level > 10.0f && sub->reactor_temp < 150.0f;

  if (sub->sonar_active && systems_powered)
  {
    sub->sonar_ping_timer += deltaTime;

    if (sub->sonar_ping_timer >= SONAR_PING_INTERVAL)
    {
      sub->sonar_ping_timer = 0.0f;

      if (audioInitialized && !IsSoundPlaying(sonarPing))
      {
        float depth_factor = 1.0f - (sub->depth / SONAR_RANGE) * 0.3f;
        depth_factor = MAX(0.3f, depth_factor);
        SetSoundVolume(sonarPing, 0.6f * depth_factor);
        PlaySound(sonarPing);
      }
    }
  }
  else
  {
    sub->sonar_ping_timer = 0.0f;
    if (sub->sonar_active && !systems_powered)
    {
      sub->sonar_active = false;
    }
  }
}

// Replace the updatePhysics function with this much more aggressive version:

static void updatePhysics(SubmarineState *sub, float deltaTime)
{
  // Ballast tank physics - more realistic timing
  // ONLY work if ballast control is active AND has power (battery > 0 OR backup power)
  bool ballast_has_power = sub->battery_level > 0.0f || sub->backup_power_active;

  if (sub->ballast_control_active && ballast_has_power)
  {
    if (sub->ballast_tanks_filled && sub->ballast_level < 100.0f)
    {
      sub->ballast_level = MIN(100.0f, sub->ballast_level + BALLAST_FILL_RATE * deltaTime);
    }
    else if (!sub->ballast_tanks_filled && sub->ballast_level > 0.0f)
    {
      sub->ballast_level = MAX(0.0f, sub->ballast_level - BALLAST_EMPTY_RATE * deltaTime);
    }
  }
  else if (!ballast_has_power)
  {
    // No power - ballast control stops working
    // But manual emergency blow still works (it's manual/pneumatic)
  }

  // Emergency ballast blow (always works - it's manual/pneumatic)
  if (sub->manual_ballast_blow_active && sub->ballast_level > 0.0f)
  {
    sub->ballast_level = MAX(0.0f, sub->ballast_level - BALLAST_EMPTY_RATE * 4.0f * deltaTime);
  }

  // MUCH MORE AGGRESSIVE BALLAST PHYSICS
  // Linear effect instead of exponential - much stronger
  float ballast_factor = (sub->ballast_level - 50.0f) / 50.0f; // Range: -1 to +1

  // DIRECT linear effect - much more powerful
  float ballast_weight = ballast_factor * 50.0f; // Up to ±25 m/s target speed (INCREASED!)

  // Hull breach flooding - makes submarine much heavier
  if (sub->hull_integrity < 50.0f)
  {
    float flood_weight = (50.0f - sub->hull_integrity) / 50.0f * 12.0f; // Up to +12 m/s down
    ballast_weight += flood_weight;
  }

  // Navigation precision affects control but not force
  float nav_precision = 1.0f;
  if (!sub->gyroscope_active)
    nav_precision *= 0.85f;
  if (!sub->depth_control_active)
    nav_precision *= 0.9f;
  if (!sub->ballast_control_active || !ballast_has_power)
    nav_precision *= 0.5f; // MAJOR impact when ballast control fails

  // Trim angle - more responsive
  float trim_effect = sinf(sub->trim_angle * DEG2RAD) * 6.0f * nav_precision; // INCREASED

  // Thrust - direct effect
  float thrust_effect = sub->thrust * 0.12f; // INCREASED

  // Calculate target vertical speed
  float target_speed = (ballast_weight + trim_effect + thrust_effect) * nav_precision;

  // MUCH MORE AGGRESSIVE submarine response
  float acceleration_rate = 8.0f; // DOUBLED - How fast submarine responds to ballast changes

  // Reduce water resistance effect
  float resistance = 1.0f + (fabsf(sub->vertical_speed) * 0.05f); // REDUCED resistance
  acceleration_rate /= resistance;

  // Much faster speed change
  float speed_diff = target_speed - sub->vertical_speed;
  sub->vertical_speed += speed_diff * acceleration_rate * deltaTime;

  // Less water drag - submarine moves more freely
  float drag_coefficient = 0.92f; // REDUCED drag (was 0.88f)
  sub->vertical_speed *= powf(drag_coefficient, deltaTime * 60.0f);

  // Update depth
  sub->depth += sub->vertical_speed * deltaTime;
  sub->depth = MAX(0.0f, MIN(MAX_DEPTH, sub->depth));

  // Surface effects
  if (sub->depth <= 0.0f)
  {
    sub->depth = 0.0f;
    sub->vertical_speed = MAX(0.0f, sub->vertical_speed);
    sub->emergency_surface = false;
  }

  // Emergency surface - EXTREMELY aggressive
  if (sub->emergency_surface)
  {
    sub->ballast_tanks_filled = false;
    sub->manual_ballast_blow_active = true;
    sub->thrust = -100.0f; // Maximum reverse thrust

    // Force VERY rapid emergency ascent
    if (sub->vertical_speed > -12.0f) // INCREASED from -8 m/s
    {
      sub->vertical_speed = -12.0f; // Force 12 m/s upward
    }
  }

  // Hull pressure damage
  float pressure_ratio = sub->depth / MAX_DEPTH;
  if (pressure_ratio > 0.8f && sub->hull_integrity > 0)
  {
    float damage_rate = (pressure_ratio - 0.8f) * 15.0f;
    sub->hull_integrity = MAX(0.0f, sub->hull_integrity - damage_rate * deltaTime);
  }

  // Hull breach flooding - MUCH more severe and realistic
  if (sub->hull_integrity < 25.0f)
  {
    // Water flooding rapidly increases ballast
    float breach_severity = (25.0f - sub->hull_integrity) / 25.0f;
    float flood_rate = breach_severity * 40.0f; // INCREASED flooding rate
    sub->ballast_level = MIN(100.0f, sub->ballast_level + flood_rate * deltaTime);

    // Power systems affected by flooding
    if (sub->hull_integrity < 10.0f && sub->battery_level > 0)
    {
      sub->battery_level = MAX(0.0f, sub->battery_level - 5.0f * deltaTime);
    }
  }

  // Speed calculation for display
  sub->speed = fabsf(sub->vertical_speed) * 3.6f;
}

static void updateNitrogenNarcosis(SubmarineState *sub, float deltaTime)
{
  // Nitrogen narcosis starts affecting crew at depths > 1000m
  if (sub->depth > NITROGEN_NARCOSIS_DEPTH)
  {
    float depth_factor = (sub->depth - NITROGEN_NARCOSIS_DEPTH) / 1000.0f;
    float narcosis_increase = depth_factor * 0.5f * deltaTime;
    sub->nitrogen_level = MIN(100.0f, sub->nitrogen_level + narcosis_increase);
  }
  else if (sub->nitrogen_level > 0)
  {
    // Recovery when surfacing
    sub->nitrogen_level = MAX(0.0f, sub->nitrogen_level - 2.0f * deltaTime);
  }

  // Nitrogen narcosis affects crew performance
  if (sub->nitrogen_level > 50.0f)
  {
    // High nitrogen levels cause systems to randomly fail
    // This could be expanded to affect specific systems
  }
}

void updateSubmarineState(SubmarineState *sub, float deltaTime)
{
  // Update all subsystems
  updateReactorSubsystems(sub, deltaTime);
  updateLifeSupportSubsystems(sub, deltaTime);
  updateNavigationSubsystems(sub);
  updateCoolingSystem(sub, deltaTime);
  updateReactor(sub, deltaTime);
  updatePowerAndEnvironment(sub, deltaTime);

  // Update physics
  updatePhysics(sub, deltaTime);

  // Update sonar
  updateSonar(sub, deltaTime);

  // Update nitrogen narcosis
  updateNitrogenNarcosis(sub, deltaTime);

  // Game over conditions
  if (sub->hull_integrity <= 0 || sub->oxygen <= 0)
  {
    // Game over logic could go here
  }
}