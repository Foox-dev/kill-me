#ifndef CONFIG_H
#define CONFIG_H

// Screen configuration
#define SCREEN_WIDTH 800
#define SCREEN_HEIGHT 600

// Physics constants
#define GRAVITY 9.81f
#define JUMP_FORCE 5.0f
#define GROUND_LEVEL 1.0f

// Network configuration
#define MAX_PLAYERS 8
#define PORT 1234
#define TICK_RATE 60
#define NETWORK_UPDATE_TIME (1.0f / TICK_RATE)

#endif // CONFIG_H