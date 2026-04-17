#pragma once

/**
 * @file mapstack.h
 * @brief Map History Stack System
 *
 * Sistem stack buat nyimpen history perpindahan antar map.
 * Dipake buat fitur "go back" ke map sebelumnya.
 * Implementasi pake linked list manual.
 */

#include <string>

/*==============================================================================
 * MapSystem Namespace
 *==============================================================================*/

namespace MapSystem
{
    /*==========================================================================
     * MapHistoryEntry Struct
     *==========================================================================*/

    /**
     * @brief Satu entry di stack = satu map yang pernah dikunjungi
     * @note Nyimpen info map dan pintu masuk yang dipake
     */
    struct MapHistoryEntry
    {
        std::string mapPath;  /**< Path file map (.tmj) yang dikunjungi */
        std::string doorName; /**< Nama pintu/object yang dipake buat masuk ke map ini */
    };

    /*==========================================================================
     * MapStackNode Struct
     *==========================================================================*/

    /**
     * @brief Node untuk linked list manual (bukan pake std::stack)
     * @note Karena project pake C++ tapi hindari STL complex? atau alasan tertentu
     */
    struct MapStackNode
    {
        MapHistoryEntry data; /**< Data map history di node ini */
        MapStackNode *next;   /**< Pointer ke node berikutnya (ke arah bottom stack) */
    };

    /*==========================================================================
     * MapStack Class
     *==========================================================================*/

    /**
     * @brief Stack implementation buat history perpindahan map
     *
     * Cara pake:
     * - Push() pas pindah ke map baru
     * - Pop() pas mau kembali ke map sebelumnya
     * - Peek() buat liat map teratas tanpa ngeluarin
     *
     * Stack top = map yang paling baru dikunjungi
     * Stack bottom = map pertama kali
     */
    class MapStack
    {
    public:
        /** @brief Constructor - bikin stack kosong */
        MapStack();

        /** @brief Destructor - bersihin semua node dan free memory */
        ~MapStack();

        /**
         * @brief Push map baru ke stack
         * @param mapPath Path map yang dikunjungi
         * @param doorName Nama pintu/object yang dipake masuk
         */
        void Push(const std::string &mapPath, const std::string &doorName);

        /**
         * @brief Pop (hapus dan return) map teratas dari stack
         * @return MapHistoryEntry map yang baru aja di-pop
         * @warning Asumsikan stack gak kosong sebelum panggil Pop()
         */
        MapHistoryEntry Pop();

        /**
         * @brief Lihat map teratas tanpa ngeluarin dari stack
         * @return MapHistoryEntry map yang ada di top
         * @warning Asumsikan stack gak kosong sebelum panggil Peek()
         */
        MapHistoryEntry Peek() const;

        /**
         * @brief Cek apakah stack kosong
         * @return true kalo gak ada node, false kalo ada isinya
         */
        bool IsEmpty() const;

        /**
         * @brief Kosongkan seluruh stack (hapus semua node)
         */
        void Clear();

    private:
        MapStackNode *top; /**< Pointer ke node teratas (yang paling baru) */
        int size;          /**< Jumlah node dalam stack (opsional, kalo butuh) */
    };
}