#pragma once
#include "raylib.h"

struct DamagePopup {
    Vector2 position;
    float damage;
    float timer;
    float duration;
    Vector2 velocity;
    bool active;
};

struct DamageNode {
    DamagePopup data;
    DamageNode* next;
};

class DamageQueue {
public:
    DamageQueue();
    ~DamageQueue();

    // Core Queue Operations
    void Enqueue(DamagePopup popup);
    void Dequeue();
    bool IsEmpty() const;

    // Management Methods
    void Update(float dt);
    void Draw();

private:
    DamageNode* head;
    DamageNode* tail;
};
