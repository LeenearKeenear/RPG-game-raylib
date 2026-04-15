#include "../include/player_camera.h"
#include "../include/player.h"
#include "../include/map.h"
#include "../include/screen.h"
#include "../include/debug.h"
#include <cmath>
#include <raylib.h>

void PlayerCameraManager::PlayerCamera(Player* player)
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

    camera.target.x = player->GetPosition().x + (TILE_SIZE / 2.0f);
    camera.target.y = player->GetPosition().y + (TILE_SIZE / 2.0f);

    float halfW = (GameScreenWidth / 2.0f) / camera.zoom;
    float halfH = (GameScreenHeight / 2.0f) / camera.zoom;

    if (camera.target.x < halfW)
        camera.target.x = halfW;
    if (camera.target.y < halfH)
        camera.target.y = halfH;
    if (camera.target.x > MapW - halfW)
        camera.target.x = MapW - halfW;
    if (camera.target.y > MapH - halfH)
        camera.target.y = MapH - halfH;
}

TileRange PlayerCameraManager::GetVisibleTileRange(Player* player)
{
    Vector2 worldMin = GetScreenToWorld2D({0.0f, 0.0f}, camera);
    Vector2 worldMax = GetScreenToWorld2D({(float)GameScreenWidth, (float)GameScreenHeight}, camera);

    TileRange range;

    range.minX = (int)floorf(worldMin.x / TILE_SIZE) - 1;
    range.minY = (int)floorf(worldMin.y / TILE_SIZE) - 1;
    range.maxX = (int)ceilf(worldMax.x  / TILE_SIZE) + 1;
    range.maxY = (int)ceilf(worldMax.y  / TILE_SIZE) + 1;

    if (range.minX < 0)                       range.minX = 0;
    if (range.minY < 0)                       range.minY = 0;
    if (range.maxX > tilesonMap->width)       range.maxX = tilesonMap->width;
    if (range.maxY > tilesonMap->height)      range.maxY = tilesonMap->height;

    return range;
}
