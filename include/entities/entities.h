#pragma once
#include <vector>
#include <string>
#include "entity.h"

/**
 * @brief Sistem registri dan manajemen global untuk entitas.
 * Menangani inisialisasi, pembaruan (update), perenderan, dan persistensi status kematian.
 */
namespace Entities
{
    void Init();                    ///< Inisialisasi sistem entitas
    void Update();                  ///< Update semua entitas yang aktif
    int Render(Rectangle viewRect); ///< Render semua entitas yang aktif
    void Shutdown();                ///< Bersihkan semua entitas

    /**
     * @brief Menambahkan entitas statis/persisten ke dalam registri.
     */
    void Add(Entity *entity);

    /**
     * @brief Menambahkan entitas dinamis (contoh: efek sementara, proyektil).
     */
    void AddDynamic(Entity *entity);

    void Clear(); ///< Hapus semua entitas dari registri

    /**
     * @brief Mendapatkan daftar semua entitas yang terdaftar saat ini.
     */
    const std::vector<Entity *> &GetRegistry();

    /**
     * @brief Mencatat entitas sebagai "mati" di map tertentu agar tidak muncul kembali (respawn).
     */
    void RegisterDeath(const std::string &mapPath, int objectId);

    /**
     * @brief Memeriksa apakah suatu entitas sudah pernah dibunuh sebelumnya di suatu map.
     */
    bool IsAlreadyDead(const std::string &mapPath, int objectId);
}

// master render untuk object tile kek chest, trap dll
void RenderTileProps(Rectangle viewRect);

// master clear untuk object tile kek chest, trap dll
void ClearTileProps(void);