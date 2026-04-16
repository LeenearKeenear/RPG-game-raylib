#pragma once
#include <string>

namespace MapSystem
{
    // satu entry di stack = satu map yang pernah dikunjungi
    struct MapHistoryEntry
    {
        std::string mapPath;
        std::string doorName;
    };

    // node untuk linked list manual
    struct MapStackNode
    {
        MapHistoryEntry data;
        MapStackNode *next;
    };

    class MapStack
    {
    public:
        MapStack();
        ~MapStack();

        void Push(const std::string &mapPath, const std::string &doorName);
        MapHistoryEntry Pop();
        MapHistoryEntry Peek() const;
        bool IsEmpty() const;
        void Clear();

    private:
        MapStackNode *top;
        int size;
    };
}