#include "../include/movement.h"
#include "../include/player.h"
#include "../include/mapLogic.h"
#include "../include/input.h"
#include "../include/tiles.h"
#include "../include/debug.h"
#include <cmath>

namespace Movement
{
    void HandleMovement(Player &player)
    {
        player.Velocity = {0, 0};
        bool moving = false;
        Direction nextDir = player.Anim.direction;

        if (InputInstance.IsMoveUp())
        {
            player.Velocity.y -= 1;
            nextDir = UP;
            moving = true;
        }
        if (InputInstance.IsMoveDown())
        {
            player.Velocity.y += 1;
            nextDir = DOWN;
            moving = true;
        }
        if (InputInstance.IsMoveLeft())
        {
            player.Velocity.x -= 1;
            nextDir = LEFT;
            moving = true;
        }
        if (InputInstance.IsMoveRight())
        {
            player.Velocity.x += 1;
            nextDir = RIGHT;
            moving = true;
        }

        ::State nextState = moving ? WALK : IDLE;
        PlayAnimation(player.Anim, nextState, nextDir, PlayerAnimationSet);

        float Length = sqrtf(player.Velocity.x * player.Velocity.x + player.Velocity.y * player.Velocity.y);
        if (Length != 0)
        {
            player.Velocity.x /= Length;
            player.Velocity.y /= Length;
        }

        Vector2 NewPos = {
            player.Position.x + player.Velocity.x * player.Speed,
            player.Position.y + player.Velocity.y * player.Speed};

        if (CanMove(player, NewPos))
        {
            player.Position = NewPos;
        }

        player.Anim.position = player.Position;
    }

    void UpdateCamera(Player &player)
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

        camera.target.x = player.Position.x + (TILE_SIZE / 2.0f);
        camera.target.y = player.Position.y + (TILE_SIZE / 2.0f);

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

    bool CanMove(Player &player, Vector2 newPosition)
    {
        Rectangle hitbox = BuildHitbox(newPosition, player.HitboxOffsetX, player.HitboxOffsetY, player.HitboxWidth, player.HitboxHeight);

        if (!IsWithinWorldBounds(hitbox, tilesonMap->width * TILE_SIZE, tilesonMap->height * TILE_SIZE))
            return false;

        if (CheckCollisionAgainstRects(hitbox, player.CollisionRects))
            return false;

        if (CheckCollisionAgainstPolygons(hitbox, player.CollisionPolygons))
            return false;

        return true;
    }
}
