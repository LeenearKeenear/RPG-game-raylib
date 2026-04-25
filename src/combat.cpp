#include "../include/combat.h"
#include "../include/input.h"
#include "../include/animation.h"
#include "../include/inventory.h"
#include "../include/entities.h"
#include "../include/map.h"
#include "../include/player.h"
#include "../include/tiles.h"
#include "../lib/raylib/include/raymath.h"
#include <cmath>
#include <string>
#include <algorithm>

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
        float reach = player.Swing.reach;     // Jangkauan serangan ke depan
        float breadth = player.Swing.breadth; // Lebar serangan ke samping (tegak lurus)

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

            // Cek apakah entitas sudah pernah terkena damage di ayunan ini
            bool alreadyHit = false;
            for (void* ptr : player.Swing.damagedEntities)
            {
                if (ptr == (void*)entity)
                {
                    alreadyHit = true;
                    break;
                }
            }
            if (alreadyHit) continue;

            if (CheckCollisionRecs(attackHitbox, entity->GetHitbox()))
            {
                // Hitung arah knockback
                Vector2 entityCenter = {
                    entity->Position.x + 16, // Asumsi 32x32 entity
                    entity->Position.y + 16
                };
                Vector2 knockDir = Vector2Normalize(Vector2Subtract(entityCenter, playerCenter));
                
                float damage = 25.0f;
                entity->TakeDamage(damage, knockDir); 
                
                // Tambahkan Damage Popup
                AddDamagePopup(entityCenter, damage);

                player.Swing.damagedEntities.push_back((void*)entity);
                TraceLog(LOG_INFO, "COMBAT: Player hit enemy! Damage: %.1f", damage);
            }
        }
    }

    void HandleCombat(Player &player)
    {
        if (player.Anim.isDead)
            return;

        // Update status pressRegistered: Hanya izinkan serangan jika klik dimulai saat sudah di state PLAY
        if (InputInstance.IsLeftClickPressed()) {
            player.Swing.pressRegistered = true;
        }
        if (!InputInstance.IsLeftClickDown()) {
            player.Swing.pressRegistered = false;
        }

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
            player.TakeDamage(10.0f);
            TraceLog(LOG_INFO, "PLAYER: Test Health Decrease (%.1f)", player.Health);
        }

        if (player.Swing.pressRegistered && !player.Anim.isAttacking)
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
                    player.Mana -= player.AttackManaCost;
                    player.ManaRegenTimer = player.ManaRegenDelay;

                    // INisialisasi Animasi Swing (Stardew Style)
                    player.Swing.active = true;
                    player.Swing.timer = 0;
                    player.Swing.damagedEntities.clear(); // Reset list musuh yang terkena hit
                    player.Swing.center = playerCenter;
                    switch (attackFaceDir)
                    {
                        case UP:    player.Swing.center.y -= 8; break;
                        case DOWN:  player.Swing.center.y += 8; break;
                        case LEFT:  player.Swing.center.x -= 8; break;
                        case RIGHT: player.Swing.center.x += 8; break;
                    }
                    
                    float baseAngle = 0.0f;
                    switch (attackFaceDir)
                    {
                        case RIGHT: baseAngle = 0.0f; break;
                        case DOWN:  baseAngle = 90.0f; break;
                        case LEFT:  baseAngle = 180.0f; break;
                        case UP:    baseAngle = -90.0f; break;
                    }

                    player.Swing.baseAngle = baseAngle;
                    ItemSlot activeSlot = InputInstance.GetActiveSlot();

                    if (activeSlot == SLOT_WEAPON_1) // Sword - Thrust
                    {
                        player.Swing.type = ATTACK_THRUST;
                        player.Swing.duration = 0.35f;
                        player.Swing.reach = 40.0f;
                        player.Swing.breadth = 16.0f;
                        player.Swing.startAngle = baseAngle;
                        player.Swing.sweepAngle = 0.0f; // No sweep for thrust
                    }
                    else // Axe or default - Slash
                    {
                        player.Swing.type = ATTACK_SLASH;
                        player.Swing.duration = 0.75f;
                        player.Swing.reach = 32.0f;
                        player.Swing.breadth = 52.0f;
                        player.Swing.startAngle = baseAngle + 55.0f;
                        player.Swing.sweepAngle = -95.0f;
                    }
                    
                    // Gunakan animasi IDLE saat menyerang (sesuai permintaan user)
                    PlayAnimation(player.Anim, IDLE, attackFaceDir, PlayerAnimationSet);
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
            player.KnockbackVelocity = {0, 0}; // Riset knockback saat revive
            TraceLog(LOG_INFO, "PLAYER: Revived!");
        }
    }

    void UpdateSwingAttack(Player &player, float dt)
    {
        if (!player.Swing.active) return;

        // Update center agar mengikuti player (solusi bug sprite tertinggal saat knockback)
        Vector2 playerCenter = {
            player.Position.x + player.HitboxOffsetX + player.HitboxWidth / 2,
            player.Position.y + player.HitboxOffsetY + player.HitboxHeight / 2
        };
        player.Swing.center = playerCenter;
        switch (player.Anim.direction)
        {
            case UP:    player.Swing.center.y -= 8; break;
            case DOWN:  player.Swing.center.y += 8; break;
            case LEFT:  player.Swing.center.x -= 8; break;
            case RIGHT: player.Swing.center.x += 8; break;
        }

        player.Swing.timer += dt;
        if (player.Swing.timer >= player.Swing.duration)
        {
            player.Swing.active = false;
            player.Swing.timer = 0;
            player.Anim.isAttacking = false; // Reset status attacking agar bisa bergerak lagi
        }
        else
        {
            // Progress ayunan dari 0 ke 1
            float progress = player.Swing.timer / player.Swing.duration;

            if (player.Swing.type == ATTACK_THRUST)
            {
                // Animasi tusukan: maju dan mundur menggunakan sine wave
                player.Swing.thrustOffset = sinf(progress * PI) * 16.0f;
                player.Swing.currentAngle = player.Swing.startAngle;
            }
            else
            {
                // Gunakan sine easing untuk swing yang lebih mulus (cepat di tengah)
                float easedProgress = sinf(progress * PI / 2.0f); 
                player.Swing.currentAngle = player.Swing.startAngle + (easedProgress * player.Swing.sweepAngle);
                player.Swing.thrustOffset = 0.0f;
            }

            // Deteksi hit setiap frame selama ayunan aktif
            PerformHitDetection(player);
        }
    }

    void DrawSwingAttack(Player &player)
    {
        if (!player.Swing.active) return;

        // 1. Gambar Slash Trail (Removed by request)
        /*
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
        */

        // 2. Gambar Weapon Sprite yang berputar
        ItemSlot activeSlot = InputInstance.GetActiveSlot();
        InventoryItem item = player.Hotbar[(int)activeSlot - 1];
        
        if (item.type == ITEM_WEAPON)
        {
            Rectangle src = GetFrame(item.iconX, item.iconY);
            
            // Tentukan posisi visual dengan offset thrust
            Vector2 visualPos = player.Swing.center;
            if (player.Swing.type == ATTACK_THRUST)
            {
                float rad = player.Swing.baseAngle * (PI / 180.0f);
                visualPos.x += cosf(rad) * player.Swing.thrustOffset;
                visualPos.y += sinf(rad) * player.Swing.thrustOffset;
            }

            Rectangle dest = { visualPos.x, visualPos.y, 20, 20 };
            // Origin {0, 24} untuk tile diagonal (handle di pojok kiri bawah)
            Vector2 origin = { 0, 24 }; 
            
            // Render dengan rotasi (ditambah 45 karena sprite sudah miring 45 derajat secara default)
            DrawTexturePro(TexturesMap[TEXTURE_ITEMS], src, dest, origin, player.Swing.currentAngle + 45.0f, WHITE);
        }
    }

    // --- Damage Popup System ---
    static DamageQueue Popups;

    void AddDamagePopup(Vector2 pos, float damage)
    {
        DamagePopup p;
        p.position = pos;
        p.damage = damage;
        p.timer = 0;
        p.duration = 1.0f;
        p.velocity = { (float)GetRandomValue(-20, 20) / 10.0f, -2.0f };
        p.active = true;
        Popups.Enqueue(p);
    }

    void UpdateDamagePopups(float dt)
    {
        Popups.Update(dt);
    }

    void DrawDamagePopups()
    {
        Popups.Draw();
    }
}

