#pragma once

// ================================================================
// Frustum Culling — Tilemap Rendering
//
// Hanya render tile yang benar-benar masuk ke camera viewport.
// Kalkulasi visible tile range dilakukan di player.cpp (GetVisibleTileRange()),
// lalu dipakai di sini untuk batasi loop render.
//
// Gunakan RenderMapCulled() sebagai pengganti RenderMap() dari map.h.
// ================================================================

// hasil kalkulasi frustum: range index tile yang visible di layar
struct TileRange
{
    int minX; // kolom tile paling kiri yang visible
    int minY; // baris tile paling atas yang visible
    int maxX; // kolom tile paling kanan yang visible (exclusive)
    int maxY; // baris tile paling bawah yang visible (exclusive)
};

// render tilemap dengan frustum culling — pengganti RenderMap()
void RenderMapCulled(void);

// data yang bisa dibaca oleh sistem debug
extern int lastTilesRendered;
extern TileRange currentVisibleRange;

