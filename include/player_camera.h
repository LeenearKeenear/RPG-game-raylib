#pragma once
#include "frustum.h"

class Player;

// ================================================================
// PlayerCameraManager Class
// Handler component untuk mengatur posisi kamera agar mengikuti 
// player dan melakukan kalkulasi frustum culling (TileRange).
// ================================================================
class PlayerCameraManager
{
public:
    static void PlayerCamera(Player* player);
    static TileRange GetVisibleTileRange(Player* player);
};
