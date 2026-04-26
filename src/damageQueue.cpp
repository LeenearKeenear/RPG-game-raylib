#include "../include/damageQueue.h"
#include "../lib/raylib/include/raymath.h"
#include <string>

DamageQueue::DamageQueue() : head(nullptr), tail(nullptr) {}

DamageQueue::~DamageQueue() {
    while (!IsEmpty()) {
        Dequeue();
    }
}

void DamageQueue::Enqueue(DamagePopup popup) {
    DamageNode* newNode = new DamageNode();
    newNode->data = popup;
    newNode->next = nullptr;

    if (tail == nullptr) {
        head = tail = newNode;
    } else {
        tail->next = newNode;
        tail = newNode;
    }
}

void DamageQueue::Dequeue() {
    if (head == nullptr) return;

    DamageNode* temp = head;
    head = head->next;

    if (head == nullptr) {
        tail = nullptr;
    }

    delete temp;
}

bool DamageQueue::IsEmpty() const {
    return head == nullptr;
}

void DamageQueue::Update(float dt) {
    DamageNode* current = head;
    
    // Update all popups in the queue
    while (current != nullptr) {
        if (current->data.active) {
            current->data.timer += dt;
            if (current->data.timer >= current->data.duration) {
                current->data.active = false;
            } else {
                // Move popup with physics (Gravity & Friction)
                AnimEffects::ApplyPhysics(current->data.position, current->data.velocity, 0.1f, 0.95f, dt);
            }
        }
        current = current->next;
    }

    // Since it's a queue, we only remove from the front when it's no longer active.
    // This assumes they are added in chronological order and have similar durations.
    while (head != nullptr && !head->data.active) {
        Dequeue();
    }
}

void DamageQueue::Draw() {
    DamageNode* current = head;
    while (current != nullptr) {
        if (current->data.active) {
            float alpha = AnimEffects::CalculateFadeOut(current->data.timer, current->data.duration);
            Color color = Fade(YELLOW, alpha);
            
            std::string dmgStr = std::to_string((int)current->data.damage);
            Vector2 textPos = { 
                current->data.position.x - MeasureText(dmgStr.c_str(), 10) / 2.0f, 
                current->data.position.y 
            };
            
            DrawText(dmgStr.c_str(), (int)textPos.x, (int)textPos.y, 10, color);
        }
        current = current->next;
    }
}
