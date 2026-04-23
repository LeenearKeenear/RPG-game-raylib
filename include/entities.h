#pragma once

/**
 * @namespace Entities
 * @brief Manager pusat untuk semua entitas game (Player, Musuh, NPC).
 * 
 * Arsitektur Scalable & Ultra-Lean:
 * - Modul ini hanya mengatur alur siklus hidup (Init, Update, Render).
 * - Logika detail didelegasikan ke masing-masing kelas entitas.
 */
namespace Entities {
    /**
     * @brief Inisialisasi awal sistem entitas.
     */
    void Init();

    /**
     * @brief Update logika semua entitas tiap frame.
     */
    void Update();

    /**
     * @brief Render semua entitas ke layar (world space).
     */
    void Render();

    /**
     * @brief Membersihkan resource entitas saat shutdown.
     */
    void Shutdown();
}