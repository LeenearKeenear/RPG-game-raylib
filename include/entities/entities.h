#pragma once
#include <vector>
#include <string>
#include <set>
#include "entity.h"
#include "enemy.h"

/**
 * @brief Sistem registri dan manajemen global untuk entitas.
 * Menangani inisialisasi, pembaruan (update), perenderan, dan persistensi status kematian.
 */
namespace Entities
{
    extern std::vector<Enemy *> EnemyRegistry;
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

    std::vector<Enemy *> &GetEnemyRegistry();

    /**
     * @brief Mendapatkan referensi ke set entitas yang sudah mati.
     * @return const ref ke DeadEntities set
     */
    const std::set<std::string> &GetDeadEntities();

    /**
     * @brief Mengganti seluruh isi DeadEntities set.
     * @param entities Set entitas mati baru
     */
    void SetDeadEntities(const std::set<std::string> &entities);

}

// master render untuk object tile kek chest, trap dll
void RenderTileProps(Rectangle viewRect);

// master clear untuk object tile kek chest, trap dll
void ClearTileProps(void);