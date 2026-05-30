#pragma once
#include <set>
#include <vector>
#include <string>
#include "entity.h"
#include "enemy.h"

/**
 * @brief Sistem registri dan manajemen global untuk entitas.
 * Menangani inisialisasi, pembaruan (update), perenderan, dan persistensi status kematian.
 */
namespace Entities
{
    /** @brief Daftar semua enemy aktif */
    extern std::vector<Enemy *> EnemyRegistry;
    /** @brief Inisialisasi sistem entitas */
    void Init();
    /** @brief Update semua entitas yang aktif */
    void Update();
    /** @brief Render semua entitas yang aktif */
    int Render(Rectangle viewRect);
    /** @brief Bersihkan semua entitas */
    void Shutdown();

    /**
     * @brief Menambahkan entitas statis/persisten ke dalam registri.
     */
    void Add(Entity *entity);

    /**
     * @brief Menambahkan entitas dinamis (contoh: efek sementara, proyektil).
     */
    void AddDynamic(Entity *entity);

    void Clear(); // Hapus semua entitas dari registri

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

    /** @brief Ambil reference ke enemy registry */
    std::vector<Enemy *> &GetEnemyRegistry();

    /** @brief Ambil semua dead entry untuk save */
    const std::set<std::string> &GetDeadEntries();
    /** @brief Set dead entries (buat load) */
    void SetDeadEntries(const std::set<std::string> &entries);
}

/** @brief Render tile props (chest, trap, dll) */
void RenderTileProps(Rectangle viewRect);

/** @brief Clear tile props (chest, trap, dll) */
void ClearTileProps(void);