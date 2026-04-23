#include "../include/entities.h"
#include "../include/player.h"

namespace Entities {

    void Init() {
        // Player Init sudah ditangani di screen_handler.cpp
        // Di sini bisa ditambahkan init untuk manager lain (misal: EnemyManager)
    }

    void Update() {
        // Update Player
        PlayerInstance.Tick();

        // Placeholder untuk skalabilitas masa depan:
        // UpdateEnemies();
        // UpdateNPCs();
    }

    void Render() {
        // Render Player
        PlayerInstance.Render();

        // Placeholder untuk skalabilitas masa depan:
        // RenderEnemies();
        // RenderNPCs();
    }

    void Shutdown() {
        // Cleanup resource jika diperlukan
    }
}