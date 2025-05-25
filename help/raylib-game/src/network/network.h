#ifndef NETWORK_H
#define NETWORK_H

#include <enet/enet.h>
#include <stdint.h>
#include <stdbool.h>

#define MAX_PLAYERS 8
#define PORT 1234

typedef struct {
    Vector3 position;
    Vector3 rotation;
    bool active;
    uint32_t clientId;
} NetworkPlayer;

void InitNetwork(void);
void CleanupNetwork(void);
void HostGame(void);
void JoinGame(const char *hostAddress);
void UpdateNetwork(Player *player);

#endif // NETWORK_H