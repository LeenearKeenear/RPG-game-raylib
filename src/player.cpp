#include "../include/player.h"
#include <cmath>

void Player::Init(Map *Map)
{
    MapRef = Map;
    TilesetRef = Map->GetTilesetRef();
    Position = {160.0f, 160.0f};
    TileCharacter = CHARACTER1;
}

void Player::Update()
{
    Velocity = {0, 0};

    if (IsKeyDown(KEY_UP))    Velocity.y -= 1;
    if (IsKeyDown(KEY_DOWN))  Velocity.y += 1;
    if (IsKeyDown(KEY_LEFT))  Velocity.x -= 1;
    if (IsKeyDown(KEY_RIGHT)) Velocity.x += 1;

    float Length = sqrtf(Velocity.x * Velocity.x + Velocity.y * Velocity.y);
    if (Length != 0)
    {
        Velocity.x /= Length;
        Velocity.y /= Length;
    }

    float NewX = Position.x + Velocity.x * Speed;
    float NewY = Position.y + Velocity.y * Speed;

    TilePos checkPos;
    checkPos.x = (int)(NewX / TileSize);
    checkPos.y = (int)(NewY / TileSize);

    if (CanMove(checkPos))
    {
        Position.x = NewX;
        Position.y = NewY;
    }
}

void Player::Render()
{
    TilesetRef->RenderChar(TileCharacter, {(int)Position.x, (int)Position.y});
}

bool Player::CanMove(TilePos NewTilePos)
{
    if (NewTilePos.x < 0 || NewTilePos.y < 0 ||
        NewTilePos.x >= MapRef->GetWidth() ||
        NewTilePos.y >= MapRef->GetHeight())
        return false;

    TileMapType Tile = MapRef->Tiles[NewTilePos.y][NewTilePos.x];
    TileDef &Def = TilesetRef->GetTileMapType(Tile);
    return !Def.Blocked;
}