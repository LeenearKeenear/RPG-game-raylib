#include "../include/mapLogic.h"
#include "../include/player.h"
#include "../include/debug.h"
#include "../include/map.h"
#include "../include/tiles.h"
#include "../include/animation.h"
#include "../include/input.h"
#include "../lib/raylib/include/raymath.h"
#include <cmath>

Player PlayerInstance;

Rectangle Player::GetPlayerHitboxAtPosition(Vector2 position)
{
    return BuildHitbox(position, HitboxOffsetX, HitboxOffsetY, HitboxWidth, HitboxHeight);
}

void Player::Init(GameState *state, const char *spawnObjectName)
{
    State = state;

    if (!isInitialized)
    {
        LoadTileTexture(TEXTURE_KNIGHT, "texture/knight.png");
        LoadTileTexture(TEXTURE_ITEMS,  "texture/test.png");

        MaxHealth = 100.0f;
        Health = MaxHealth;

        MaxMana = 100.0f;
        Mana = MaxMana;
        ManaRegenTimer = 0.0f;

        Hotbar[0] = {ITEM_WEAPON, "Iron Sword", 1, 10, 0, 6, 4};
        Hotbar[1] = {ITEM_WEAPON, "Wooden Bow", 1, 5, 0, 8, 4};
        Hotbar[2] = {ITEM_POTION, "Health Potion", 3, 0, 20, 7, 8};
        Hotbar[3] = {ITEM_POTION, "Mana Bread", 5, 0, 15, 10, 8};

        isInitialized = true;
        TraceLog(LOG_INFO, "Player: Global resources and stats initialized");
    }

    PlayAnimation(Anim, IDLE, DOWN, PlayerAnimationSet);

    CollisionRects.clear();
    CollisionPolygons.clear();

    if (tilesonMap == nullptr)
    {
        Position = {0.0f, 0.0f};
        Anim.position = Position;
        TraceLog(LOG_ERROR, "Player: tilesonMap is null during Init()");
        return;
    }

    Vector2 spawnPos;
    if (spawnObjectName != nullptr &&
        spawnObjectName[0] != '\0' &&
        TiledHelperFunction.TryGetObjectPositionByName(spawnObjectName, spawnPos))
    {
        Position = spawnPos;
        TraceLog(LOG_INFO, "Player: Spawn point '%s' found at (%.1f, %.1f)", spawnObjectName, Position.x, Position.y);
    }
    else if (TiledHelperFunction.TryGetObjectPositionByName(SPAWN_OBJECT_NAME, spawnPos))
    {
        Position = spawnPos;
        TraceLog(LOG_INFO, "Player: Default spawn point found at (%.1f, %.1f)", Position.x, Position.y);
    }
    else
    {
        Position = {
            ((float)tilesonMap->width * TILE_SIZE) / 2.0f,
            ((float)tilesonMap->height * TILE_SIZE) / 2.0f};
        TraceLog(LOG_WARNING, "Player: Spawn object '%s' not found, using default position",
                 (spawnObjectName != nullptr && spawnObjectName[0] != '\0') ? spawnObjectName : SPAWN_OBJECT_NAME);
    }

    Anim.position = Position;

    TiledHelper::CollisionResult collision;
    if (TiledHelperFunction.TryGetCollisionByLayerName(COLLISION_LAYER_NAME, collision))
    {
        CollisionRects = collision.rects;
        CollisionPolygons = collision.polygons;
    }

    TraceLog(LOG_INFO, "Player: Loaded %d collision rects", (int)CollisionRects.size());
    TraceLog(LOG_INFO, "Player: Loaded %d collision polygons", (int)CollisionPolygons.size());
}

void Player::Update()
{
    InputInstance.PollInput();
    InputInstance.UpdateState();

    if (InputInstance.IsRevive())
    {
        HandleRevive();
        return;
    }

    if (Anim.isDead)
        return;

    if (Health <= 0)
    {
        Health = 0;
        PlayAnimation(Anim, DEAD, Anim.direction, PlayerAnimationSet);
        Anim.isDead = true;
        return;
    }

    if (ManaRegenTimer > 0)
    {
        ManaRegenTimer -= GetFrameTime();
    }
    else
    {
        if (Mana < MaxMana)
        {
            Mana += ManaRegenRate * GetFrameTime();
            if (Mana > MaxMana)
                Mana = MaxMana;
        }
    }

    if (InputInstance.IsTestLoseHP())
    {
        Health -= 10.0f;
        if (Health < 0)
            Health = 0;
        TraceLog(LOG_INFO, "PLAYER: Test Health Decrease (%.1f)", Health);
    }

    if (InputInstance.IsLeftClickPressed() && !Anim.isAttacking)
    {
        HandleAction();
        if (Anim.isAttacking)
            return;
    }

    if (InputInstance.IsGoBack())
    {
        pendingSwitchMap = false;
        pendingGoBack = true;
        return;
    }

    if (Anim.isAttacking)
        return;

    if (InputInstance.IsInteract())
    {
        TraceLog(LOG_INFO, "PLAYER: Interact triggered");
    }

    Velocity = {0, 0};
    bool moving = false;
    Direction nextDir = Anim.direction;
    if (InputInstance.IsMoveUp())    { Velocity.y -= 1; nextDir = UP;    moving = true; }
    if (InputInstance.IsMoveDown())  { Velocity.y += 1; nextDir = DOWN;  moving = true; }
    if (InputInstance.IsMoveLeft())  { Velocity.x -= 1; nextDir = LEFT;  moving = true; }
    if (InputInstance.IsMoveRight()) { Velocity.x += 1; nextDir = RIGHT; moving = true; }

    ::State nextState = moving ? WALK : IDLE;
    PlayAnimation(Anim, nextState, nextDir, PlayerAnimationSet);

    float Length = sqrtf(Velocity.x * Velocity.x + Velocity.y * Velocity.y);
    if (Length != 0)
    {
        Velocity.x /= Length;
        Velocity.y /= Length;
    }

    Vector2 NewPos = {
        Position.x + Velocity.x * Speed,
        Position.y + Velocity.y * Speed};

    if (CanMove(NewPos))
        Position = NewPos;

    Anim.position = Position;

    RayCasting();
    CheckDoorInteraction();
    CheckPropInteraction();
}

void Player::Render(void)
{
    DrawAnimation(Anim, TEXTURE_KNIGHT);
}

void Player::Tick()
{
    Update();

    if (PlayerInstance.pendingGoBack)
    {
        PlayerInstance.pendingGoBack = false;
        GoBack();
        return;
    }

    if (PlayerInstance.pendingSwitchMap)
    {
        PlayerInstance.pendingSwitchMap = false;
        SwitchMap(PlayerInstance.pendingMapPath.c_str(), PlayerInstance.pendingDoorName.c_str());
    }

    UpdateAnimation(Anim, GetFrameTime());

    PlayerCamera();
}

void Player::PlayerCamera(void)
{
    float MapW = (float)tilesonMap->width * TILE_SIZE;
    float MapH = (float)tilesonMap->height * TILE_SIZE;

    const int MinMapTileZoom = 15;
    float AutoZoom = (float)GameScreenWidth / (MinMapTileZoom * TILE_SIZE);
    float FixedZoom = 2.0f;
    const float CameraZoom = (tilesonMap->width <= MinMapTileZoom || tilesonMap->height <= MinMapTileZoom)
                                 ? AutoZoom
                                 : FixedZoom;

    if (!isDebugMode)
        camera.zoom = CameraZoom;

    camera.target.x = PlayerInstance.GetPosition().x + (TILE_SIZE / 2.0f);
    camera.target.y = PlayerInstance.GetPosition().y + (TILE_SIZE / 2.0f);

    float halfW = (GameScreenWidth / 2.0f) / camera.zoom;
    float halfH = (GameScreenHeight / 2.0f) / camera.zoom;

    if (MapW <= halfW * 2.0f)
        camera.target.x = MapW / 2.0f;
    else
    {
        if (camera.target.x < halfW)
            camera.target.x = halfW;
        if (camera.target.x > MapW - halfW)
            camera.target.x = MapW - halfW;
    }

    if (MapH <= halfH * 2.0f)
        camera.target.y = MapH / 2.0f;
    else
    {
        if (camera.target.y < halfH)
            camera.target.y = halfH;
        if (camera.target.y > MapH - halfH)
            camera.target.y = MapH - halfH;
    }
}

bool Player::CanMove(Vector2 newPosition)
{
    Rectangle hitbox = GetPlayerHitboxAtPosition(newPosition);

    if (!IsWithinWorldBounds(hitbox, tilesonMap->width * TILE_SIZE, tilesonMap->height * TILE_SIZE))
        return false;

    if (CheckCollisionAgainstRects(hitbox, CollisionRects))
        return false;

    if (CheckCollisionAgainstPolygons(hitbox, CollisionPolygons))
        return false;

    return true;
}

void Player::RayCasting()
{
    Vector2 playerCenter = {
        Position.x + HitboxOffsetX + HitboxWidth / 2,
        Position.y + HitboxOffsetY + HitboxHeight / 2};

    Vector2 mouseWorld = GetScreenToWorld2D(GetVirtualMousePosition(State), camera);

    Vector2 aimDir = Vector2Normalize(Vector2Subtract(mouseWorld, playerCenter));

    std::vector<MapObject> propObjects;
    for (auto &obj : tilesonMap->Objects)
    {
        if (obj.layerName == OBJECT_LAYER_NAME)
            propObjects.push_back(obj);
    }

    LastHit = Ray.Cast(playerCenter, aimDir, INTERACT_RANGE, propObjects);
}

void Player::HandleAction(void)
{
    PlayerAction action = InputInstance.ResolveAction();

    switch (action)
    {
    case ACTION_ATTACK:
        if (Mana >= AttackManaCost)
        {
            PlayAnimation(Anim, ATTACK, Anim.direction, PlayerAnimationSet);
            Anim.isAttacking = true;
            Mana -= AttackManaCost;
            ManaRegenTimer = ManaRegenDelay;
            TraceLog(LOG_INFO, "PLAYER: Attack! (slot %d) - Mana: %.1f", (int)InputInstance.GetActiveSlot(), Mana);
        }
        else
        {
            TraceLog(LOG_WARNING, "PLAYER: Attack failed! Out of energy.");
        }
        break;

    case ACTION_DRINK_POTION:
    {
        int slotIdx = (int)InputInstance.GetActiveSlot() - 1;
        if (slotIdx >= 0 && slotIdx < 4)
        {
            if (Hotbar[slotIdx].type == ITEM_POTION && Hotbar[slotIdx].amount > 0)
            {
                UsePotion(slotIdx);
                TraceLog(LOG_INFO, "PLAYER: Drink potion! %s (slot %d)", Hotbar[slotIdx].name.c_str(), (int)InputInstance.GetActiveSlot());
            }
            else
            {
                TraceLog(LOG_INFO, "PLAYER: No potion in slot %d!", (int)InputInstance.GetActiveSlot());
            }
        }
        break;
    }

    case ACTION_EQUIP_UNEQUIP:
        TraceLog(LOG_INFO, "PLAYER: Equip/Unequip from inventory!");
        break;

    case ACTION_NONE:
    default:
        break;
    }
}

void Player::UsePotion(int slotIndex)
{
    if (Hotbar[slotIndex].type != ITEM_POTION || Hotbar[slotIndex].amount <= 0)
        return;

    if (Hotbar[slotIndex].name.find("Mana") != std::string::npos)
    {
        Mana += Hotbar[slotIndex].healValue;
        if (Mana > MaxMana)
            Mana = MaxMana;
        TraceLog(LOG_INFO, "PLAYER: Healed Mana by %d! Current: %.1f", Hotbar[slotIndex].healValue, Mana);
    }
    else
    {
        Health += Hotbar[slotIndex].healValue;
        if (Health > MaxHealth)
            Health = MaxHealth;
        TraceLog(LOG_INFO, "PLAYER: Healed Health by %d! Current: %.1f", Hotbar[slotIndex].healValue, Health);
    }

    Hotbar[slotIndex].amount--;
    if (Hotbar[slotIndex].amount <= 0)
    {
        Hotbar[slotIndex] = {ITEM_NONE, "", 0, 0, 0, 0, 0};
    }
}

void Player::HandleRevive(void)
{
    if (Anim.isDead)
    {
        Anim.isDead = false;
        Anim.isAttacking = false;
        PlayAnimation(Anim, IDLE, Anim.direction, PlayerAnimationSet);
        Health = MaxHealth;
        Mana = MaxMana;
        TraceLog(LOG_INFO, "PLAYER: Revived!");
    }
}

void Player::CheckDoorInteraction(void)
{
    Rectangle playerHitbox = GetPlayerHitboxAtPosition(Position);

    std::vector<MapObject *> doors = TiledHelperFunction.GetObjectsByType(DOOR_TYPE_OBJECT_NAME);

    for (const auto &door : doors)
    {
        if (!CheckCollisionRecs(playerHitbox, door->bounds))
            continue;

        if (!InputInstance.IsInteract())
            return;

        auto mapIt = door->properties.find("target_map");
        if (mapIt == door->properties.end())
        {
            TraceLog(LOG_WARNING, "Door '%s' has no target_map property", door->name.c_str());
            return;
        }

        auto doorIt = door->properties.find("target_door");
        if (doorIt == door->properties.end())
        {
            TraceLog(LOG_WARNING, "Door '%s' has no target_door property", door->name.c_str());
            return;
        }

        std::string targetMap = mapIt->second.getValue<std::string>();
        std::string targetDoor = doorIt->second.getValue<std::string>();

        pendingSwitchMap = true;
        pendingMapPath = targetMap;
        pendingDoorName = targetDoor;
        return;
    }
}

void Player::CheckPropInteraction(void)
{
    if (!LastHit.hit)
        return;
    if (!InputInstance.IsInteract())
        return;

    const std::string &type = LastHit.object->type;
    TraceLog(LOG_INFO, "Interacting with: '%s' (type: %s)", LastHit.object->name.c_str(), type.c_str());

    if (type == CHEST_TYPE_OBJECT_NAME)
    {
        TraceLog(LOG_INFO, "Opening chest: '%s'", LastHit.object->name.c_str());
    }
    else if (type == "npc")
    {
    }
    else
    {
        TraceLog(LOG_WARNING, "Unknown prop type: %s", type.c_str());
    }
}