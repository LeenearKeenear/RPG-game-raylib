/**
 * @file mapstack.cpp
 * @brief Implementasi dari Map History Stack System
 *
 * Implementasi dari class MapStack yang dideklarasikan di mapstack.h
 * Handle stack untuk riwayat perpindahan antar map (fitur go back).
 */

#include "../include/mapstack.h"

/*==============================================================================
 * MapSystem Namespace Implementation
 *==============================================================================*/

namespace MapSystem
{
    /*==========================================================================
     * Constructor & Destructor
     *==========================================================================*/

    /**
     * @brief Constructor - bikin stack kosong
     */
    MapStack::MapStack() : top(nullptr), size(0) {}

    /**
     * @brief Destructor - bersihin semua node
     */
    MapStack::~MapStack()
    {
        Clear();
    }

    /*==========================================================================
     * Stack Operations
     *==========================================================================*/

    /**
     * @brief Push map baru ke stack
     * @param mapPath Path map yang dikunjungi
     * @param doorName Nama pintu yang dipake masuk
     * @note Node baru jadi top, next指向 top lama
     */
    void MapStack::Push(const std::string &mapPath, const std::string &doorName)
    {
        // Bikin node baru
        MapStackNode *newNode = new MapStackNode();
        newNode->data = {mapPath, doorName};

        // Node baru pointing ke top lama
        newNode->next = top;

        // Update top ke node baru
        top = newNode;
        size++;
    }

    /**
     * @brief Pop (hapus dan return) map teratas dari stack
     * @return MapHistoryEntry map yang baru di-pop
     * @note Kalo stack kosong, return entry kosong
     */
    MapHistoryEntry MapStack::Pop()
    {
        if (IsEmpty())
            return {"", ""}; // return entry kosong kalo stack kosong

        // Simpen node top yang mau dihapus
        MapStackNode *temp = top;
        MapHistoryEntry entry = temp->data;

        // Pindahin top ke node berikutnya
        top = top->next;

        // Hapus node lama
        delete temp;
        size--;

        return entry;
    }

    /**
     * @brief Lihat map teratas tanpa ngeluarin dari stack
     * @return MapHistoryEntry map yang ada di top
     * @note Kalo stack kosong, return entry kosong
     */
    MapHistoryEntry MapStack::Peek() const
    {
        if (IsEmpty())
            return {"", ""}; // return entry kosong kalo stack kosong
        return top->data;
    }

    /**
     * @brief Cek apakah stack kosong
     * @return true kalo top == nullptr (gak ada node)
     */
    bool MapStack::IsEmpty() const
    {
        return top == nullptr;
    }

    /**
     * @brief Kosongkan seluruh stack (hapus semua node)
     * @note Panggil Pop() berulang sampai stack kosong
     */
    void MapStack::Clear()
    {
        // hapus semua node satu per satu
        while (!IsEmpty())
            Pop();
    }
}