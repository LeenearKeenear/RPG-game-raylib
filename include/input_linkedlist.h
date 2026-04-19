#pragma once

/**
 * @file input_linkedlist.h
 * @brief Circular Doubly Linked List for Hotbar Slots
 */

/**
 * @brief Slot aktif yang dipilih player (1-4)
 * @note SLOT_NONE = 0 berarti gak ada slot yang kepilih
 */
enum ItemSlot
{
    SLOT_NONE = 0,     /**< Gak ada slot aktif */
    SLOT_WEAPON_1 = 1, /**< Slot senjata 1 (key 1) */
    SLOT_WEAPON_2 = 2, /**< Slot senjata 2 (key 2) */
    SLOT_POTION_1 = 3, /**< Slot potion 1 (key 3) */
    SLOT_POTION_2 = 4  /**< Slot potion 2 (key 4) */
};

/**
 * @brief Node dalam Double Circular Linked List
 */
struct SlotNode
{
    ItemSlot slot;
    SlotNode* next;
    SlotNode* prev;
};

/**
 * @brief Manager Double Circular Linked List untuk Hotbar slots
 */
class HotbarList
{
public:
    HotbarList();
    ~HotbarList();

    /**
     * @brief Inisialisasi list dengan slot 1-4
     */
    void Initialize();

    /**
     * @brief Navigasi ke slot berikutnya (circular)
     * @return Slot baru yang aktif
     */
    ItemSlot GetNext();

    /**
     * @brief Navigasi ke slot sebelumnya (circular)
     * @return Slot baru yang aktif
     */
    ItemSlot GetPrev();

    /**
     * @brief Set current node ke slot tertentu (sinkronisasi keyboard)
     * @param slot Slot tujuan
     */
    void SetCurrentBySlot(ItemSlot slot);

    /**
     * @brief Ambil slot saat ini
     */
    ItemSlot GetCurrentSlot() const;

private:
    SlotNode* head;
    SlotNode* currentNode;

    /**
     * @brief Helper untuk membuat node baru
     */
    SlotNode* CreateNode(ItemSlot slot);
};
