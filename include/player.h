#pragma once
#include "../lib/raylib/include/raylib.h"
#include "../include/screen.h"
#include "../include/map.h"

// TODO MULTI-MAP: Entity nanti bakal butuh info
// di map mana dia berada (map id / pointer ke map aktif)
// sementara doang (ini buat entity (contoh player, enemy, npc))
typedef struct
{
    TileCoordinate PlayerPosition;
    float MoveTimer;
    float MoveDelay;
} Entity;

// definisi struct entity ama tile khusus
extern Entity Player;
extern sTile Door;

// gak penting
void PlayerMovement(void);
void PlayerControl(void);
void PlayerCamera(void);

// penting
void UpdatePlayer(GameState *state);