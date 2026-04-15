#pragma once
#include <raylib.h>
#include "map.h"
#include "screen.h"
#include "animation.h"

// ================================================================
// Player Class
// Handle semua behavior player: movement, collision, render,
// key binding, state management, inventory, dan hotbar.
//
// Workflow collision:
// - Collision dibaca dari object layer "collision" di Tiled
// - Setiap Rectangle di layer itu dianggap solid/blocked
// - CanMove() ngecek apakah posisi baru player nabrak salah satu rect itu
//
// Workflow spawn:
// - Spawn point dibaca dari object bernama "spawn" di Tiled
// - Init() otomatis taruh player di posisi spawn
// ================================================================

// nama layer & object di Tiled — sesuaiin kalau beda
#define COLLISION_LAYER_NAME "collision"
#define SPAWN_OBJECT_NAME "spawn"

// ================================================================
// Player State Enums
// ================================================================

// state utama player — menentukan behavior dan animasi
// enum state ini untuk sementara, nanti bisa dipindah ke animasi jika ada sprite untuk tiap state
typedef enum
{
    PLAYER_IDLE,
    PLAYER_MOVING,
    PLAYER_ATTACKING,
    PLAYER_DRINKING_POTION,
    PLAYER_INTERACTING,
    PLAYER_DEAD
} PlayerState;

// jenis slot hotbar — weapon atau potion
typedef enum
{
    SLOT_WEAPON,
    SLOT_POTION
} HotbarSlotType;

// ================================================================
// Hotbar Slot
// Struct untuk satu slot di hotbar (1-4)
// ================================================================
struct HotbarSlot
{
    HotbarSlotType type;       // weapon atau potion
    const char *name;          // nama item di slot
    bool isEmpty;              // apakah slot kosong
};

class Player
{
public:
    // inisialisasi player — load texture, baca spawn & collision dari Tiled
    void Init(void);

    // handle input dan movement per frame, cek collision sebelum apply posisi
    void Update(void);

    // render sprite player di posisi world saat ini
    void Render(void);

    // handle camera follow player dengan clamp ke world bounds
    void PlayerCamera(void);

    // wrapper per frame — dipanggil dari UpdateLogicAll()
    // urutan: HandleInput() → Update() → PlayerCamera()
    void Tick(void);

    // getter posisi player dalam pixel
    Vector2 GetPosition() { return Position; }

    // getter speed player — dipake debug panel
    float GetSpeed() { return Speed; }

    // getter state player — dipake debug panel
    PlayerState GetState() { return State; }

    // getter apakah player hidup
    bool IsAlive() { return bIsAlive; }

    // getter apakah inventory terbuka
    bool IsInventoryOpen() { return bInventoryOpen; }

    // getter apakah map terbuka
    bool IsMapOpen() { return bMapOpen; }

    // getter selected hotbar slot (0-3 untuk key 1-4)
    int GetSelectedSlot() { return SelectedHotbarSlot; }

    // getter hotbar slot info
    HotbarSlot GetHotbarSlot(int index);

    // render HUD (hotbar, state indicator, overlay UI) di screen space
    // dipanggil dari DrawRenderTexture() setelah EndMode2D()
    void RenderHUD(void);

private:
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

    // ---- Hotbar (slot 1-4) ----
    int SelectedHotbarSlot = 0; // 0-3 (key 1-4)
    HotbarSlot Hotbar[4];

    // ---- Action Timers ----
    float ActionTimer = 0.0f;        // timer untuk attack/potion animation
    float ActionDuration = 0.25f;     // durasi aksi dalam detik

    // ---- Input Handling ----
    void HandleInput(void);          // master input handler — dipanggil tiap frame
    void HandleMovement(void);       // handle WASD/Arrow movement
    void HandleHotbar(void);         // handle key 1-4 untuk switch slot
    void HandleActionKey(void);      // handle Space — context-sensitive action
    void HandleInteract(void);       // handle E — interact
    void HandleInventory(void);      // handle I — toggle inventory
    void HandleMap(void);            // handle M — toggle map
    void HandleTestDeath(void);      // handle K — test death
    void HandleTestRevive(void);     // handle R — test revive

    // ---- Collision ----
    // cek apakah posisi baru player nabrak collision rect
    // return false kalau nabrak, true kalau aman
    bool CanMove(Vector2 NewPos);

    // state animasi
    enum State currentState = IDLE;
    Direction currentDir = DOWN;
    int frame = 0;
    float frameTime = 0.0f;
    float frameSpeed = 0.15f;
    int walkFrameIndex = 0;

    bool isAttacking = false;
    bool isDead = false;

    // collision rectangles dari object layer Tiled
    // diisi pas Init() dari TilesonGetObjectsByType(COLLISION_LAYER_NAME)
    std::vector<Rectangle> CollisionRects;

    // ---- UI Rendering (private helpers) ----
    void RenderInventoryUI(void);    // render overlay inventory
    void RenderMapUI(void);          // render overlay map
    void RenderDeathOverlay(void);   // render overlay death screen
};

// global instance — bisa diakses file lain via extern
extern Player PlayerInstance;