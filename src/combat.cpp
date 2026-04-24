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
        
        float attackRadius = 40.0f;
        float attackAngleRange = 45.0f; // 1/8 lingkaran
        
        // Tentukan sudut pusat berdasarkan direction player
        float centerAngle = 0.0f;
        switch (player.Anim.direction)
        {
            case RIGHT: centerAngle = 0.0f;   break;
            case DOWN:  centerAngle = 90.0f;  break;
            case LEFT:  centerAngle = 180.0f; break;
            case UP:    centerAngle = -90.0f; break;
        }

        for (auto entity : Entities::GetRegistry())
        {
            if (entity == &player) continue;
            if (!entity->IsActive || entity->Health <= 0) continue;

            Rectangle enemyHitbox = entity->GetHitbox();
            Vector2 enemyCenter = {
                enemyHitbox.x + enemyHitbox.width / 2,
                enemyHitbox.y + enemyHitbox.height / 2
            };

            // 1. Cek jarak (Radius)
            float dist = Vector2Distance(playerCenter, enemyCenter);
            if (dist <= attackRadius)
            {
                // 2. Cek sudut (Angle)
                Vector2 toEnemy = Vector2Subtract(enemyCenter, playerCenter);
                float angleToEnemy = atan2(toEnemy.y, toEnemy.x) * (180.0f / PI);
                
                // Normalisasi selisih sudut agar berada di rentang [-180, 180]
                float angleDiff = angleToEnemy - centerAngle;
                while (angleDiff > 180) angleDiff -= 360;
                while (angleDiff < -180) angleDiff += 360;

                if (fabs(angleDiff) <= attackAngleRange / 2.0f)
                {
                    entity->Health -= 25.0f; // Damage dasar
                    TraceLog(LOG_INFO, "COMBAT: Player hit enemy with Sector Attack! Damage: 25. Enemy HP: %.1f", entity->Health);
                }
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

                    // Langsung lakukan deteksi hit sesuai arah hadap pemain
                    PerformHitDetection(player);

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
