#include "../include/combat.h"
#include "../include/player.h"
#include "../include/input.h"
#include "../include/animation.h"
#include "../include/inventory.h"
#include "../include/entities.h"
#include "../include/map.h"
#include "../lib/raylib/include/raymath.h"
#include <cmath>

namespace Combat
{
    /**
     * @brief Melakukan deteksi tabrakan serangan terhadap semua entitas di registry
     */
    void PerformHitDetection(Player &player)
    {
        Vector2 playerCenter = {
            player.Position.x + player.HitboxOffsetX + player.HitboxWidth / 2,
            player.Position.y + player.HitboxOffsetY + player.HitboxHeight / 2
        };
        
        Rectangle attackHitbox;
        float reach = 32.0f;   // Jangkauan serangan ke depan
        float breadth = 64.0f; // Lebar serangan ke samping (tegak lurus)

        switch (player.Anim.direction)
        {
            case RIGHT:
                attackHitbox = { playerCenter.x + player.HitboxWidth / 2, playerCenter.y - breadth / 2, reach, breadth };
                break;
            case LEFT:
                attackHitbox = { playerCenter.x - player.HitboxWidth / 2 - reach, playerCenter.y - breadth / 2, reach, breadth };
                break;
            case DOWN:
                attackHitbox = { playerCenter.x - breadth / 2, playerCenter.y + player.HitboxHeight / 2, breadth, reach };
                break;
            case UP:
                attackHitbox = { playerCenter.x - breadth / 2, playerCenter.y - player.HitboxHeight / 2 - reach, breadth, reach };
                break;
        }



        for (auto entity : Entities::GetRegistry())
        {
            if (entity == &player) continue;
            if (!entity->IsActive || entity->Health <= 0) continue;

            if (CheckCollisionRecs(attackHitbox, entity->GetHitbox()))
            {
                entity->Health -= 25.0f; // Damage dasar
                TraceLog(LOG_INFO, "COMBAT: Player hit enemy with Rectangle Attack! Damage: 25. Enemy HP: %.1f", entity->Health);
            }
        }
    }

    void HandleCombat(Player &player)
    {
        if (player.Anim.isDead)
            return;

        if (player.Health <= 0)
        {
            player.Health = 0;
            PlayAnimation(player.Anim, DEAD, player.Anim.direction, PlayerAnimationSet);
            player.Anim.isDead = true;
            return;
        }

        if (player.ManaRegenTimer > 0)
        {
            player.ManaRegenTimer -= GetFrameTime();
        }
        else
        {
            if (player.Mana < player.MaxMana)
            {
                player.Mana += player.ManaRegenRate * GetFrameTime();
                if (player.Mana > player.MaxMana)
                    player.Mana = player.MaxMana;
            }
        }

        if (InputInstance.IsTestLoseHP())
        {
            player.Health -= 10.0f;
            if (player.Health < 0)
                player.Health = 0;
            TraceLog(LOG_INFO, "PLAYER: Test Health Decrease (%.1f)", player.Health);
        }

        if (InputInstance.IsLeftClickPressed() && !player.Anim.isAttacking)
        {
            PlayerAction action = InputInstance.ResolveAction();
            if (action == ACTION_ATTACK)
            {
                // Hitung arah bidikan dari kursor mouse (Raycast direction)
                Vector2 mouseWorld = GetScreenToWorld2D(GetVirtualMousePosition(player.State), camera);
                Vector2 playerCenter = {
                    player.Position.x + player.HitboxOffsetX + player.HitboxWidth / 2,
                    player.Position.y + player.HitboxOffsetY + player.HitboxHeight / 2
                };
                Vector2 attackDir = Vector2Normalize(Vector2Subtract(mouseWorld, playerCenter));

                // Tentukan arah hadap animasi berdasarkan sudut (Logika jam)
                float angle = atan2(attackDir.y, attackDir.x) * (180.0f / PI);
                Direction attackFaceDir;
                if (angle >= -135.0f && angle < -45.0f) {
                    attackFaceDir = UP;
                } else if (angle >= -45.0f && angle < 45.0f) {
                    attackFaceDir = RIGHT;
                } else if (angle >= 45.0f && angle < 135.0f) {
                    attackFaceDir = DOWN;
                } else {
                    attackFaceDir = LEFT;
                }

                // Update arah hadap karakter (selalu terjadi saat klik kiri)
                PlayAnimation(player.Anim, IDLE, attackFaceDir, PlayerAnimationSet);

                // Cek apakah mana cukup untuk melakukan serangan nyata
                if (player.Mana >= player.AttackManaCost)
                {
                    // Lakukan deteksi hit
                    PerformHitDetection(player);

                    player.Mana -= player.AttackManaCost;
                    player.ManaRegenTimer = player.ManaRegenDelay;

                    // INisialisasi Animasi Swing (Stardew Style)
                    player.Swing.active = true;
                    player.Swing.timer = 0;
                    player.Swing.center = playerCenter;
                    
                    float targetAngle = atan2(attackDir.y, attackDir.x) * (180.0f / PI);
                    player.Swing.startAngle = targetAngle - 90.0f; // Mulai dari 90 derajat di belakang target
                    player.Swing.sweepAngle = 180.0f;              // Ayunan 180 derajat
                    
                    // Aktifkan animasi attack pada sprite player
                    PlayAnimation(player.Anim, ATTACK, attackFaceDir, PlayerAnimationSet);
                    player.Anim.isAttacking = true;

                    TraceLog(LOG_INFO, "PLAYER: Attack aimed at (%.2f, %.2f)", attackDir.x, attackDir.y);
                }
                else
                {
                    TraceLog(LOG_WARNING, "PLAYER: Attack failed! Out of energy.");
                }
            }
        }
    }

    void HandleRevive(Player &player)
    {
        if (player.Anim.isDead)
        {
            player.Anim.isDead = false;
            player.Anim.isAttacking = false;
            PlayAnimation(player.Anim, IDLE, player.Anim.direction, PlayerAnimationSet);
            player.Health = player.MaxHealth;
            player.Mana = player.MaxMana;
            TraceLog(LOG_INFO, "PLAYER: Revived!");
        }
    }

    void UpdateSwingAttack(Player &player, float dt)
    {
        if (!player.Swing.active) return;

        player.Swing.timer += dt;
        if (player.Swing.timer >= player.Swing.duration)
        {
            player.Swing.active = false;
            player.Swing.timer = 0;
        }
        else
        {
            // Progress ayunan dari 0 ke 1
            float progress = player.Swing.timer / player.Swing.duration;
            // Gunakan sine easing untuk swing yang lebih mulus (cepat di tengah)
            float easedProgress = sinf(progress * PI / 2.0f); 
            player.Swing.currentAngle = player.Swing.startAngle + (easedProgress * player.Swing.sweepAngle);
        }
    }

    void DrawSwingAttack(Player &player)
    {
        if (!player.Swing.active) return;

        // 1. Gambar Slash Trail (Multiple fading arcs)
        float progress = player.Swing.timer / player.Swing.duration;
        int trailSegments = 8;
        float segmentAngle = player.Swing.sweepAngle / trailSegments;
        
        for (int i = 0; i < trailSegments; i++)
        {
            float segmentProgress = (float)i / trailSegments;
            if (segmentProgress > progress) break;

            // Fade out segment yang lebih lama
            float alpha = (segmentProgress / progress) * 0.5f; 
            Color color = Fade(SKYBLUE, alpha);
            
            float startAngle = player.Swing.startAngle + (i * segmentAngle);
            // DrawCircleSector di Raylib: 0 = kanan, searah jarum jam
            // Kita kurangi 90 karena startAngle kita berbasis atan2 (0 = kanan)
            DrawCircleSector(player.Swing.center, 40.0f, startAngle, startAngle + segmentAngle, 10, color);
        }

        // 2. Gambar Weapon Sprite yang berputar
        ItemSlot activeSlot = InputInstance.GetActiveSlot();
        InventoryItem item = player.Hotbar[(int)activeSlot - 1];
        
        if (item.type == ITEM_WEAPON)
        {
            Rectangle src = GetFrame(item.iconX, item.iconY);
            Rectangle dest = { player.Swing.center.x, player.Swing.center.y, 32, 32 };
            // Origin {16, 32} berarti putar di dasar bawah sprite (gagang)
            Vector2 origin = { 16, 32 }; 
            
            // Render dengan rotasi (ditambah 90 agar tegak lurus dengan arah ayunan)
            DrawTexturePro(TexturesMap[TEXTURE_ITEMS], src, dest, origin, player.Swing.currentAngle + 90.0f, WHITE);
        }
    }
}

