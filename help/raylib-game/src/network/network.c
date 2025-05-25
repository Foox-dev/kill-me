#include "network.h"
#include <stdio.h>
#include <string.h>
#include <enet/enet.h>

static ENetHost *host = NULL;
static ENetPeer *peer = NULL;
static bool isHost = false;
static bool isConnected = false;
static uint32_t clientId = 0;

void InitNetwork(void) {
    if (enet_initialize() != 0) {
        printf("Failed to initialize ENet.\n");
        return;
    }
}

void CleanupNetwork(void) {
    if (peer) {
        enet_peer_disconnect(peer, 0);
        peer = NULL;
    }

    if (host) {
        enet_host_destroy(host);
        host = NULL;
    }

    isHost = false;
    isConnected = false;
    clientId = 0;
}

void HostGame(void) {
    CleanupNetwork();

    ENetAddress address;
    address.host = ENET_HOST_ANY;
    address.port = PORT;

    host = enet_host_create(&address, MAX_PLAYERS, 2, 0, 0);
    if (host == NULL) {
        printf("Failed to create host.\n");
        return;
    }

    isHost = true;
    clientId = 0;
}

void JoinGame(const char *hostAddress) {
    CleanupNetwork();

    host = enet_host_create(NULL, 1, 2, 0, 0);
    if (host == NULL) {
        printf("Failed to create client.\n");
        return;
    }

    ENetAddress address;
    enet_address_set_host(&address, hostAddress);
    address.port = PORT;

    peer = enet_host_connect(host, &address, 2, 0);
    if (peer == NULL) {
        printf("Failed to connect to host.\n");
        enet_host_destroy(host);
        host = NULL;
        return;
    }
}

void UpdateNetwork(Player *player) {
    if (host == NULL) return;

    ENetEvent event;
    while (enet_host_service(host, &event, 0) > 0) {
        switch (event.type) {
            case ENET_EVENT_TYPE_CONNECT:
                printf("New client connected: %u\n", event.peer->connectID);
                if (!isHost) {
                    clientId = 1;
                    isConnected = true;
                } else {
                    event.peer->data = (void *)(uintptr_t)1;
                }
                break;

            case ENET_EVENT_TYPE_RECEIVE:
                // Handle received data
                enet_packet_destroy(event.packet);
                break;

            case ENET_EVENT_TYPE_DISCONNECT:
                printf("Client disconnected: %u\n", event.peer->connectID);
                break;
        }
    }

    // Send player data
    if (isConnected || isHost) {
        NetworkPlayer netPlayer = { .position = player->position, .rotation = player->rotation, .active = true, .clientId = clientId };
        ENetPacket *packet = enet_packet_create(&netPlayer, sizeof(NetworkPlayer), ENET_PACKET_FLAG_RELIABLE);
        if (packet) {
            if (isHost) {
                enet_host_broadcast(host, 0, packet);
            } else if (peer) {
                enet_peer_send(peer, 0, packet);
            }
        }
    }
}