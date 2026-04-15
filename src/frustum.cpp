#include "../include/frustum.h"
#include "../include/map.h"
#include "../include/player.h"
#include "../lib/raylib/include/raylib.h"

int lastTilesRendered = 0;
TileRange currentVisibleRange = {0, 0, 0, 0};

// ================================================================
// RenderMapCulled()
// Pengganti RenderMap() dari map.h — render tilemap dengan frustum culling.
//
// Cara kerja:
// 1. Minta PlayerInstance.GetVisibleTileRange() buat dapet range tile visible
// 2. BeginMode2D — masuk ke camera space
// 3. Loop hanya dari tileMin ke tileMax (bukan seluruh map)
// 4. Sisa logika render tile sama persis dengan RenderMap()
//
// map.cpp dan map.h TIDAK diubah sama sekali.
// tapi sementara render map di screen_handler.cpp diganti ke sini
// nanti lu bisa nyesuaian lagi isi map dari file frustum ini
// ================================================================
void RenderMapCulled(void)
{
    // skip kalau map atau texture belum siap
    if (tilesonMap == nullptr || tilesonMap->tilesetTexture.id == 0)
        return;

    // dapatkan range tile yang visible dari inti logic frustum culling di player
    currentVisibleRange = PlayerInstance.GetVisibleTileRange();

    lastTilesRendered = 0; // reset counter per frame

    BeginMode2D(camera);

    for (int l = 0; l < tilesonMap->layerCount; l++)
    {
        for (int y = currentVisibleRange.minY; y < currentVisibleRange.maxY; y++)
        {
            for (int x = currentVisibleRange.minX; x < currentVisibleRange.maxX; x++)
            {
                lastTilesRendered++; // hitung berapa tile yang diproses loop
                int tileId = tilesonMap->tiles[l][(y * tilesonMap->width) + x];
                if (tileId == 0) // tile ID 0 = kosong, skip
                    continue;

                int spacing    = tilesonMap->tilesetSpacing;
                int firstgid   = tilesonMap->tilesetFirstgid;
                int adjustedId = tileId - firstgid; // ID relatif ke tileset
                int tilesetCols = tilesonMap->tilesetCols;

                // hitung posisi potongan tile di spritesheet
                int srcX = (adjustedId % tilesetCols) * (TILE_SIZE + spacing);
                int srcY = (adjustedId / tilesetCols) * (TILE_SIZE + spacing);

                Rectangle srcRec = {(float)srcX, (float)srcY, (float)TILE_SIZE, (float)TILE_SIZE};
                Rectangle dstRec = {(float)(x * TILE_SIZE), (float)(y * TILE_SIZE), (float)TILE_SIZE, (float)TILE_SIZE};

                DrawTexturePro(tilesonMap->tilesetTexture, srcRec, dstRec, (Vector2){0, 0}, 0.0F, WHITE);
            }
        }
    }

    EndMode2D();
}