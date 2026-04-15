#pragma once
#include <raylib.h>
#include <vector>
#include "map.h"
#include "screen.h"
#include "animation.h"
#include "frustum.h"

// ================================================================
// Player Class
// Handle semua behavior player: movement, collision, render,
// key binding, state management, inventory, dan hotbar.
// Logic dipecah dan ditangani oleh Component:
// - PlayerInput
// - PlayerUI
// - PlayerCameraManager
// ================================================================

#define COLLISION_LAYER_NAME "collision"
#define SPAWN_OBJECT_NAME "spawn"

typedef enum
{
    PLAYER_IDLE,
    PLAYER_MOVING,
    PLAYER_ATTACKING,
    PLAYER_DRINKING_POTION,
    PLAYER_INTERACTING,
    PLAYER_DEAD
} PlayerState;

typedef enum
{
    SLOT_WEAPON,
    SLOT_POTION
} HotbarSlotType;

struct HotbarSlot
{
    HotbarSlotType type;       
    const char *name;          
    bool isEmpty;              
};

// Deklarasi Handlers
class PlayerInput;
class PlayerUI;
class PlayerCameraManager;

class Player
{
public:
    void Init(void);
    void Update(void);
    void Render(void);
    void PlayerCamera(void);
    void Tick(void);

    Vector2 GetPosition() { return Position; }
    float GetSpeed() { return Speed; }
    PlayerState GetState() { return State; }
    bool IsAlive() { return bIsAlive; }
    bool IsInventoryOpen() { return bInventoryOpen; }
    bool IsMapOpen() { return bMapOpen; }
    int GetSelectedSlot() { return SelectedHotbarSlot; }
    HotbarSlot GetHotbarSlot(int index);
    TileRange GetVisibleTileRange(void);
    void RenderHUD(void);

private:
    friend class PlayerInput;
    friend class PlayerUI;
    friend class PlayerCameraManager;

    Vector2 Position;
    Vector2 Velocity;
    int TileSize = 32;
    float Speed = 4.0f;
    Texture2D CharTexture;

    // ---- State ----
    PlayerState State = PLAYER_IDLE;
    bool bIsAlive = true;
    bool bInventoryOpen = false;
    bool bMapOpen = false;
    
    // ---- Animation State ----
    float frameTime = 0.0f;
    float frameSpeed = 0.0f;
    int frame = 0;
    int walkFrameIndex = 0;
    enum State currentState = IDLE;
    Direction currentDir = DOWN;
    bool isAttacking = false;
    bool isDead = false;

    // ---- Hotbar (slot 1-4) ----
    int SelectedHotbarSlot = 0; 
    HotbarSlot Hotbar[4];

    // ---- Action Timers ----
    float ActionTimer = 0.0f;        
    float ActionDuration = 0.5f;     

    // ---- Collision ----
    bool CanMove(Vector2 NewPos);
    std::vector<Rectangle> CollisionRects;
};

extern Player PlayerInstance;