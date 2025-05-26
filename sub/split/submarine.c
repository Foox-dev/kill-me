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
      .sonar_ping_timer = 0
  };
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