#include "../include/combat.h"
#include "../include/animation.h"
#include "../include/entities.h"
#include "../include/input.h"
#include "../include/inventory.h"
#include "../include/map.h"
#include "../include/player.h"
#include "../include/tiles.h"
#include "../include/effects.h"
#include "../lib/raylib/include/raymath.h"
#include <algorithm>
#include <cmath>
#include <string>

namespace Combat
{
    /**
     * Pembantu internal untuk mendeteksi tabrakan antara serangan pemain dan entitas dunia.
     * Menghitung hitbox dinamis berdasarkan arah hadap pemain dan jangkauan senjata.
     */
    void PerformHitDetection(Player &player)
    {
        Vector2 playerCenter = {
            player.Position.x + player.HitboxOffsetX + player.HitboxWidth / 2,
            player.Position.y + player.HitboxOffsetY + player.HitboxHeight / 2};

        Rectangle attackHitbox;
        float reach = player.Swing.reach;     ///< Jarak serangan ke depan
        float breadth = player.Swing.breadth; ///< Lebar serangan ke samping

        // Menghasilkan persegi panjang hitbox serangan berdasarkan arah
        switch (player.Anim.direction)
        {
        case RIGHT:
            attackHitbox = {playerCenter.x + player.HitboxWidth / 2,
                            playerCenter.y - breadth / 2, reach, breadth};
            break;
        case LEFT:
            attackHitbox = {playerCenter.x - player.HitboxWidth / 2 - reach,
                            playerCenter.y - breadth / 2, reach, breadth};
            break;
        case DOWN:
            attackHitbox = {playerCenter.x - breadth / 2,
                            playerCenter.y + player.HitboxHeight / 2, breadth, reach};
            break;
        case UP:
            attackHitbox = {playerCenter.x - breadth / 2,
                            playerCenter.y - player.HitboxHeight / 2 - reach, breadth,
                            reach};
            break;
        }

        // Memeriksa semua entitas aktif untuk tabrakan
        for (auto entity : Entities::GetRegistry())
        {
            if (entity == &player) continue;
            if (!entity->IsActive || entity->Health <= 0) continue;

            // Memastikan kita tidak memukul entitas yang sama berkali-kali dalam satu ayunan/tusukan
            bool alreadyHit = false;
            for (void *ptr : player.Swing.damagedEntities)
            {
                if (ptr == (void *)entity)
                {
                    alreadyHit = true;
                    break;
                }
            }
            if (alreadyHit) continue;

            if (CheckCollisionRecs(attackHitbox, entity->GetHitbox()))
            {
                // Menghitung arah dorongan balik (knockback) menjauhi pemain
                Vector2 entityCenter = {entity->Position.x + 16, entity->Position.y + 16};
                Vector2 knockDir = Vector2Normalize(Vector2Subtract(entityCenter, playerCenter));

                float damage = player.Swing.damage;
                Vector2 knockback = Vector2Scale(knockDir, player.Swing.knockbackForce);
                entity->TakeDamage(damage, knockback);

                // Umpan balik visual
                Effects::AddDamage(entityCenter, damage);

                player.Swing.damagedEntities.push_back((void *)entity);
                TraceLog(LOG_INFO, "COMBAT: Pemain mengenai musuh! Damage: %.1f", damage);
            }
        }
    }

    /**
     * Titik masuk utama untuk memproses logika pertarungan pemain.
     * Menangani regenerasi mana, transisi status, dan pemicuan serangan.
     */
    void HandleCombat(Player &player)
    {
        if (player.Anim.isDead) return;

        // Penyaringan Input: Pastikan serangan hanya terpicu jika klik dimulai dalam status yang valid
        if (InputInstance.IsLeftClickPressed())
        {
            player.Swing.pressRegistered = true;
        }
        if (!InputInstance.IsLeftClickDown())
        {
            player.Swing.pressRegistered = false;
        }

        // Menangani transisi kematian pemain
        if (player.Health <= 0)
        {
            player.Health = 0;
            PlayAnimation(player.Anim, DEAD, player.Anim.direction, PlayerAnimationSet);
            player.Anim.isDead = true;
            return;
        }

        // Logika Regenerasi Mana
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

        // Eksekusi Serangan
        if (player.Swing.pressRegistered && !player.Anim.isAttacking)
        {
            PlayerAction action = InputInstance.ResolveAction();
            if (action == ACTION_ATTACK)
            {
                // Menghitung arah bidikan berdasarkan posisi mouse di dunia (world space)
                Vector2 mouseWorld = GetScreenToWorld2D(GetVirtualMousePosition(player.State), camera);
                Vector2 playerCenter = {
                    player.Position.x + player.HitboxOffsetX + player.HitboxWidth / 2,
                    player.Position.y + player.HitboxOffsetY + player.HitboxHeight / 2};
                Vector2 attackDir = Vector2Normalize(Vector2Subtract(mouseWorld, playerCenter));

                // Menentukan arah hadap animasi berdasarkan sudut bidikan
                float angle = atan2(attackDir.y, attackDir.x) * (180.0f / PI);
                Direction attackFaceDir;
                if (angle >= -135.0f && angle < -45.0f) attackFaceDir = UP;
                else if (angle >= -45.0f && angle < 45.0f) attackFaceDir = RIGHT;
                else if (angle >= 45.0f && angle < 135.0f) attackFaceDir = DOWN;
                else attackFaceDir = LEFT;

                // Mengunci arah karakter ke arah bidikan
                PlayAnimation(player.Anim, IDLE, attackFaceDir, PlayerAnimationSet);

                float manaCost = Inventory::GetAttackManaCost(player);

                // Pemeriksaan Resource (Mana)
                if (player.Mana >= manaCost)
                {
                    player.Mana -= manaCost;
                    player.ManaRegenTimer = player.ManaRegenDelay;

                    // Inisialisasi Status Serangan
                    player.Swing.active = true;
                    player.Swing.timer = 0;
                    player.Swing.damagedEntities.clear();
                    player.Swing.center = playerCenter;

                    Inventory::SetupAttackStats(player, attackFaceDir);

                    player.Anim.isAttacking = true;
                    TraceLog(LOG_INFO, "PLAYER: Serangan diarahkan ke (%.2f, %.2f)", attackDir.x, attackDir.y);
                }
                else
                {
                    Effects::AddLog("Stamina tidak cukup!");
                    player.Swing.pressRegistered = false; 
                    TraceLog(LOG_WARNING, "PLAYER: Serangan gagal! Mana habis.");
                }
            }
        }
    }

    /**
     * Logika untuk memulihkan status pemain saat dihidupkan kembali (resurrection).
     */
    void HandleRevive(Player &player)
    {
        if (player.Anim.isDead)
        {
            player.Anim.isDead = false;
            player.Anim.isAttacking = false;
            PlayAnimation(player.Anim, IDLE, player.Anim.direction, PlayerAnimationSet);
            player.Health = player.MaxHealth;
            player.Mana = player.MaxMana;
            player.KnockbackVelocity = {0, 0};
            TraceLog(LOG_INFO, "PLAYER: Dihidupkan kembali!");
        }
    }

    /**
     * Memperbarui status fisik dari ayunan/tusukan senjata yang aktif.
     * Menghitung rotasi dengan easing dan melakukan deteksi hit terus-menerus.
     */
    void UpdateSwingAttack(Player &player, float dt)
    {
        if (!player.Swing.active) return;

        // Pastikan pusat senjata tetap terkunci pada posisi pemain (bahkan saat terkena knockback)
        Vector2 playerCenter = {
            player.Position.x + player.HitboxOffsetX + player.HitboxWidth / 2,
            player.Position.y + player.HitboxOffsetY + player.HitboxHeight / 2};
        player.Swing.center = playerCenter;

        ItemSlot activeSlot = InputInstance.GetActiveSlot();
        if (activeSlot == SLOT_WEAPON_1)
        {
            switch (player.Anim.direction)
            {
            case UP:    player.Swing.center.x += 3; break;
            case DOWN:  player.Swing.center.x -= 3; break;
            case LEFT:  player.Swing.center.y -= 3; break;
            case RIGHT: player.Swing.center.y += 3; break;
            }
        }
        else
        {
            switch (player.Anim.direction)
            {
            case UP:    player.Swing.center.y -= 20; player.Swing.center.x += 1; break;
            case DOWN:  player.Swing.center.y += 20; player.Swing.center.x -= 1; break;
            case LEFT:  player.Swing.center.x -= 20; player.Swing.center.y -= 1; break;
            case RIGHT: player.Swing.center.x += 20; player.Swing.center.y += 1; break;
            }
        }

        player.Swing.timer += dt;
        if (player.Swing.timer >= player.Swing.duration)
        {
            player.Swing.active = false;
            player.Swing.timer = 0;
            player.Anim.isAttacking = false;
        }
        else
        {
            float progress = player.Swing.timer / player.Swing.duration;

            if (player.Swing.type == ATTACK_THRUST)
            {
                player.Swing.thrustOffset = AnimEffects::CalculateThrustOffset(progress, 16.0f);
                player.Swing.currentAngle = player.Swing.startAngle;
            }
            else
            {
                player.Swing.currentAngle = AnimEffects::CalculateSlashRotation(progress, player.Swing.startAngle, player.Swing.sweepAngle);
                player.Swing.thrustOffset = 0.0f;
            }

            // Deteksi hit terus-menerus selama jendela aktif
            PerformHitDetection(player);
        }
    }

    /**
     * Me-render sprite senjata berdasarkan status dan tipe ayunan saat ini.
     */
    void DrawSwingAttack(Player &player)
    {
        if (!player.Swing.active) return;

        InventoryItem item = Inventory::GetActiveHotbarItem(player);

        if (item.type == ITEM_WEAPON)
        {

            // Menghitung posisi visual dengan offset tusukan searah
            Vector2 visualPos = player.Swing.center;
            if (player.Swing.type == ATTACK_THRUST)
            {
                float rad = player.Swing.baseAngle * (PI / 180.0f);
                visualPos.x += cosf(rad) * player.Swing.thrustOffset;
                visualPos.y += sinf(rad) * player.Swing.thrustOffset;
            }

            Rectangle dest = {visualPos.x, visualPos.y, 20, 20};
            // Titik asal (Origin) {0, 24} untuk tile 32x32 yang digunakan sebagai sprite 20x20 agar rotasi dari posisi tangan
            Vector2 origin = {0, 24};

            // Sprite dimiringkan 45 derajat di sumber, jadi kita sesuaikan
            DrawTileTexture(TEXTURE_ITEMS, item.iconX, item.iconY, dest, origin, player.Swing.currentAngle + 45.0f);
        }
    }

    void AddDamagePopup(Vector2 pos, float damage)
    {
        Effects::AddDamage(pos, damage);
    }
} // namespace Combat
