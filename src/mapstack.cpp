#include "../include/mapstack.h"

namespace MapSystem
{
    MapStack::MapStack() : top(nullptr), size(0) {}

    MapStack::~MapStack()
    {
        Clear();
    }

    void MapStack::Push(const std::string &mapPath, const std::string &doorName)
    {
        MapStackNode *newNode = new MapStackNode();
        newNode->data = {mapPath, doorName};
        newNode->next = top;
        top = newNode;
        size++;
    }

    MapHistoryEntry MapStack::Pop()
    {
        if (IsEmpty())
            return {"", ""};

        MapStackNode *temp = top;
        MapHistoryEntry entry = temp->data;
        top = top->next;
        delete temp;
        size--;
        return entry;
    }

    MapHistoryEntry MapStack::Peek() const
    {
        if (IsEmpty())
            return {"", ""};
        return top->data;
    }

    bool MapStack::IsEmpty() const
    {
        return top == nullptr;
    }

    void MapStack::Clear()
    {
        // hapus semua node satu per satu
        while (!IsEmpty())
            Pop();
    }
}