#include "../include/entities.h"
#include <vector>
#include <algorithm>
#include <set>
#include <string>

namespace Entities
{
    /**
     * @brief Daftar utama semua entitas yang saat ini ada di dalam scene (statis dan dinamis).
     */
    static std::vector<Entity *> Registry;

    /**
     * @brief Daftar entitas yang dimiliki oleh registri (dialokasikan secara dinamis).
     * Entitas-entitas ini akan dihapus saat Clear() atau Shutdown() dipanggil.
     */
    static std::vector<Entity *> DynamicRegistry;

    /**
     * @brief Set ID unik (map + indeks objek) untuk entitas yang tidak boleh muncul kembali (respawn).
     */
    static std::set<std::string> DeadEntities;

    void Init()
    {
        Registry.clear();
        DynamicRegistry.clear();
    }

    /**
     * Mengirimkan panggilan Update ke semua entitas yang aktif.
     */
    void Update()
    {
        for (size_t i = 0; i < Registry.size(); i++)
        {
            if (Registry[i] && Registry[i]->IsActive)
            {
                Registry[i]->Update();
            }
        }
    }

    /**
     * Mengurutkan entitas berdasarkan koordinat Y sebelum di-render untuk mencapai kedalaman pseudo-3D (Y-sorting).
     */
    void Render()
    {
        // Depth sorting: Entitas dengan nilai Y lebih besar akan di-render terakhir (tampil di depan)
        std::sort(Registry.begin(), Registry.end(), [](Entity *a, Entity *b) {
            if (!a) return false;
            if (!b) return true;
            return a->Position.y < b->Position.y;
        });

        for (auto entity : Registry)
        {
            if (entity && entity->IsActive)
            {
                entity->Render();
            }
        }
    }

    /**
     * Membersihkan semua entitas yang dikelola.
     */
    void Shutdown()
    {
        Clear();
    }

    /**
     * Mendaftarkan entitas yang sudah ada sebelumnya (contoh: Player) tanpa mengambil kepemilikan memori.
     */
    void Add(Entity *entity)
    {
        if (entity)
        {
            Registry.push_back(entity);
        }
    }

    /**
     * Menambahkan entitas dan mengambil alih kepemilikan memorinya.
     */
    void AddDynamic(Entity *entity)
    {
        if (entity)
        {
            DynamicRegistry.push_back(entity);
            Registry.push_back(entity);
        }
    }

    /**
     * Membersihkan scene saat ini. Menghapus semua entitas dinamis.
     */
    void Clear()
    {
        for (auto entity : DynamicRegistry)
        {
            delete entity;
        }
        DynamicRegistry.clear();
        Registry.clear();
    }

    /**
     * Mengembalikan daftar entitas global untuk pemeriksaan tabrakan/interaksi.
     */
    const std::vector<Entity *> &GetRegistry()
    {
        return Registry;
    }

    /**
     * Menandai entitas sebagai mati untuk mencegahnya muncul kembali saat map dimuat ulang.
     */
    void RegisterDeath(const std::string& mapPath, int objectId)
    {
        DeadEntities.insert(mapPath + "_" + std::to_string(objectId));
    }

    /**
     * Memeriksa apakah objek map tertentu sudah tercatat sebagai mati.
     */
    bool IsAlreadyDead(const std::string& mapPath, int objectId)
    {
        return DeadEntities.find(mapPath + "_" + std::to_string(objectId)) != DeadEntities.end();
    }
}
