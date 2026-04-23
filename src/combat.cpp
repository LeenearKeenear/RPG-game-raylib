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
    void PerformHitDetection(Player &player, Vector2 attackDir)
    {
        Vector2 playerCenter = {
            player.Position.x + player.HitboxOffsetX + player.HitboxWidth / 2,
            player.Position.y + player.HitboxOffsetY + player.HitboxHeight / 2
        };
        
        // Letakkan hitbox serangan di depan pemain sesuai arah bidikan kursor
        Vector2 hitboxCenter = Vector2Add(playerCenter, Vector2Scale(attackDir, 24.0f));
        Rectangle attackHitbox = { hitboxCenter.x - 16, hitboxCenter.y - 16, 32, 32 };

        for (auto entity : Entities::GetRegistry())
        {
            if (entity == &player) continue;
            if (!entity->IsActive || entity->Health <= 0) continue;

            if (CheckCollisionRecs(attackHitbox, entity->GetHitbox()))
            {
                entity->Health -= 25.0f; // Damage dasar
                TraceLog(LOG_INFO, "COMBAT: Player hit enemy! Damage: 25. Enemy HP: %.1f", entity->Health);
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
                if (player.Mana >= player.AttackManaCost)
                {
                    // Ambil arah bidikan dari kursor mouse (Raycast direction)
                    Vector2 mouseWorld = GetScreenToWorld2D(GetVirtualMousePosition(player.State), camera);
                    Vector2 playerCenter = {
                        player.Position.x + player.HitboxOffsetX + player.HitboxWidth / 2,
                        player.Position.y + player.HitboxOffsetY + player.HitboxHeight / 2
                    };
                    Vector2 attackDir = Vector2Normalize(Vector2Subtract(mouseWorld, playerCenter));

                    // Tentukan arah hadap animasi berdasarkan sudut (Logika jam)
                    // atan2 mengembalikan sudut dalam radian: 0 (Right), PI/2 (Down), -PI/2 (Up), PI/-PI (Left)
                    float angle = atan2(attackDir.y, attackDir.x) * (180.0f / PI);

                    Direction attackFaceDir;
                    if (angle >= -135.0f && angle < -45.0f) {
                        attackFaceDir = UP;    // -135 s/d -45 (90 derajat)
                    } else if (angle >= -45.0f && angle < 45.0f) {
                        attackFaceDir = RIGHT; // -45 s/d 45 (90 derajat)
                    } else if (angle >= 45.0f && angle < 135.0f) {
                        attackFaceDir = DOWN;  // 45 s/d 135 (90 derajat)
                    } else {
                        attackFaceDir = LEFT;  // Sisanya (90 derajat)
                    }

                    // Paksa update animasi agar pemain langsung menghadap ke arah bidikan
                    // Kita tidak mengubah player.Anim.direction secara manual agar PlayAnimation bisa mendeteksi perubahan
                    PlayAnimation(player.Anim, IDLE, attackFaceDir, PlayerAnimationSet);

                    // Animasi attack dinonaktifkan sesuai permintaan (dikomentari)
                    /*
                    PlayAnimation(player.Anim, ATTACK, player.Anim.direction, PlayerAnimationSet);
                    player.Anim.isAttacking = true;
                    */

                    // Langsung lakukan deteksi hit sesuai arah kursor
                    PerformHitDetection(player, attackDir);

                    player.Mana -= player.AttackManaCost;
                    player.ManaRegenTimer = player.ManaRegenDelay;
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
}
