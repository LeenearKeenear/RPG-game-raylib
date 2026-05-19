#include "combat.h"
#include "screen.h"
#include "animation.h"
#include "entities.h"
#include "input.h"
#include "inventory.h"
#include "map.h"
#include "player.h"
#include "effects.h"
#include "propsbehavior.h"
#include "../lib/raylib/include/raymath.h"
#include <algorithm>
#include <cmath>
#include <string>

namespace Combat
{
    void PerformHitDetection(Player &player)
    {
        Vector2 playerCenter = {
            player.Position.x + player.GetHitboxOffsetX() + player.GetHitboxWidth() / 2,
            player.Position.y + player.GetHitboxOffsetY() + player.GetHitboxHeight() / 2};

        Rectangle attackHitbox;
        float reach = player.Swing.reach;
        float breadth = player.Swing.breadth;

        switch (player.Anim.direction)
        {
        case RIGHT:
            attackHitbox = {playerCenter.x + player.GetHitboxWidth() / 2,
                            playerCenter.y - breadth / 2, reach, breadth};
            break;
        case LEFT:
            attackHitbox = {playerCenter.x - player.GetHitboxWidth() / 2 - reach,
                            playerCenter.y - breadth / 2, reach, breadth};
            break;
        case DOWN:
            attackHitbox = {playerCenter.x - breadth / 2,
                            playerCenter.y + player.GetHitboxHeight() / 2, breadth, reach};
            break;
        case UP:
            attackHitbox = {playerCenter.x - breadth / 2,
                            playerCenter.y - player.GetHitboxHeight() / 2 - reach, breadth,
                            reach};
            break;
        }

        for (auto entity : Entities::GetRegistry())
        {
            Entity *playerAsEntity = &player;
            if (entity == playerAsEntity)
                continue;
            if (!entity->IsActive || entity->Health <= 0)
                continue;

            bool alreadyHit = false;
            for (void *ptr : player.Swing.damagedEntities)
            {
                if (ptr == (void *)entity)
                {
                    alreadyHit = true;
                    break;
                }
            }
            if (alreadyHit)
                continue;

            if (CheckCollisionRecs(attackHitbox, entity->GetHitbox()))
            {
                Vector2 entityCenter = {entity->Position.x + 16, entity->Position.y + 16};
                Vector2 knockDir = Vector2Normalize(Vector2Subtract(entityCenter, playerCenter));

                float damage = player.Swing.damage;
                Vector2 knockback = Vector2Scale(knockDir, player.Swing.knockbackForce);
                entity->TakeDamage(damage, knockback);

                Effects::AddDamage(entityCenter, damage);

                player.Swing.damagedEntities.push_back((void *)entity);
                TraceLog(LOG_INFO, "COMBAT: Pemain mengenai musuh! Damage: %.1f", damage);
            }
        }

        HitPropsByAttack(attackHitbox, PlayerInstance.GetHitbox(), &player);
    }

    void HandleCombat(Player &player)
    {
        if (player.Anim.isDead)
            return;

        if (InputInstance.IsInventoryOpen())
            return;

        if (InputInstance.IsLeftClickPressed())
        {
            player.Swing.pressRegistered = true;
        }
        if (!InputInstance.IsLeftClickDown())
        {
            player.Swing.pressRegistered = false;
        }

        if (player.Health <= 0)
        {
            player.Health = 0;
            PlayAnimation(player.Anim, DEAD, player.Anim.direction);
            player.Anim.isDead = true;
            return;
        }

        if (player.ManaRegenTimer > 0)
        {
            player.ManaRegenTimer -= Time::DELTA_TIME;
        }
        else
        {
            if (player.Mana < player.MaxMana)
            {
                player.Mana += player.ManaRegenRate * Time::DELTA_TIME;
                if (player.Mana > player.MaxMana)
                    player.Mana = player.MaxMana;
            }
        }

        if (player.Swing.pressRegistered && !player.Anim.isAttacking)
        {
            PlayerAction action = InputInstance.ResolveAction();
            if (action == ACTION_ATTACK)
            {
                Vector2 mouseWorld = GetScreenToWorld2D(GetVirtualMousePosition(player.State), camera);
                Vector2 playerCenter = {
                    player.Position.x + player.GetHitboxOffsetX() + player.GetHitboxWidth() / 2,
                    player.Position.y + player.GetHitboxOffsetY() + player.GetHitboxHeight() / 2};
                Vector2 attackDir = Vector2Normalize(Vector2Subtract(mouseWorld, playerCenter));

                float angle = atan2(attackDir.y, attackDir.x) * (180.0f / PI);
                Direction attackFaceDir;
                if (angle >= -135.0f && angle < -45.0f)
                    attackFaceDir = UP;
                else if (angle >= -45.0f && angle < 45.0f)
                    attackFaceDir = RIGHT;
                else if (angle >= 45.0f && angle < 135.0f)
                    attackFaceDir = DOWN;
                else
                    attackFaceDir = LEFT;

                Direction horizDir = (attackDir.x >= 0) ? RIGHT : LEFT;
                PlayAnimation(player.Anim, IDLE, horizDir);
                PlayAnimation(player.Anim, IDLE, attackFaceDir);

                float manaCost = Inventory::GetAttackManaCost(player);

                if (player.Mana >= manaCost)
                {
                    InventoryItem activeItem = Inventory::GetActiveHotbarItem(player);
                    if (activeItem.definitionId == -1 || itemDefs.GetById(activeItem.definitionId).category != ITEM_WEAPON)
                    {
                        Effects::AddLog("Tidak ada senjata!");
                        player.Swing.pressRegistered = false;
                        return;
                    }

                    player.Mana -= manaCost;
                    player.ManaRegenTimer = player.ManaRegenDelay;

                    player.Swing.active = true;
                    player.Swing.timer = 0;
                    player.Swing.damagedEntities.clear();
                    player.Swing.center = playerCenter;
                    player.Swing.raycastAngle = angle;

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

    void HandleRevive(Player &player)
    {
        if (player.Anim.isDead)
        {
            player.Anim.isDead = false;
            player.Anim.isAttacking = false;
            PlayAnimation(player.Anim, IDLE, player.Anim.direction);
            player.Health = player.MaxHealth;
            player.Mana = player.MaxMana;
            player.KnockbackVelocity = {0, 0};
            TraceLog(LOG_INFO, "PLAYER: Dihidupkan kembali!");
        }
    }

    void UpdateSwingAttack(Player &player, float dt)
    {
        if (!player.Swing.active)
            return;

        Vector2 playerCenter = {
            player.Position.x + player.GetHitboxOffsetX() + player.GetHitboxWidth() / 2,
            player.Position.y + player.GetHitboxOffsetY() + player.GetHitboxHeight() / 2};
        player.Swing.center = playerCenter;


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

            if (player.Swing.type == ATTACK_SLASH)
            {
                player.Swing.currentAngle = Slash(player.Swing.raycastAngle, progress);
                player.Swing.thrustOffset = 0.0f;
            }
            else if (player.Swing.type == ATTACK_THRUST)
            {
                player.Swing.thrustOffset = progress * 16.0f;
                player.Swing.currentAngle = player.Swing.startAngle;
            }
            else if (player.Swing.type == ATTACK_PIERCE)
            {
                player.Swing.thrustOffset = progress * 24.0f;
                player.Swing.currentAngle = player.Swing.startAngle;
            }
            else if (player.Swing.type == ATTACK_SLAM)
            {
                float slamProgress = progress * progress;
                player.Swing.currentAngle = player.Swing.startAngle + (slamProgress * player.Swing.sweepAngle);
                player.Swing.thrustOffset = slamProgress * 8.0f;
            }
            else
            {
                player.Swing.currentAngle = player.Swing.startAngle + (progress * player.Swing.sweepAngle);
                player.Swing.thrustOffset = 0.0f;
            }

            PerformHitDetection(player);
        }
    }

    void DrawSwingAttack(Player &player)
    {
        InventoryItem item = Inventory::GetActiveHotbarItem(player);
        if (item.definitionId == -1)
            return;

        const ItemDefinition &def = itemDefs.GetById(item.definitionId);
        if (def.category != ITEM_WEAPON)
            return;

        Vector2 visualPos;
        float drawAngle;
        float thrust = 0.0f;
        float rayAngle;
        float offsetRight = 0.0f;

        if (player.Swing.active)
        {
            visualPos = player.Swing.center;
            drawAngle = player.Swing.currentAngle;
            thrust = player.Swing.thrustOffset;
            rayAngle = player.Swing.raycastAngle;
        }
        else
        {
            visualPos = {
                player.Position.x + player.GetHitboxOffsetX() + player.GetHitboxWidth() / 2,
                player.Position.y + player.GetHitboxOffsetY() + player.GetHitboxHeight() / 2};

            static Direction lastHorizDir = RIGHT;
            if (player.Anim.direction == RIGHT)
                lastHorizDir = RIGHT;
            else if (player.Anim.direction == LEFT)
                lastHorizDir = LEFT;

            if (player.Anim.direction == RIGHT) {
                rayAngle = -9.0f; // Jam 3
                offsetRight = 0.8f;
            }
            else if (player.Anim.direction == LEFT)
                rayAngle = 189.0f; // Jam 9
            else if (player.Anim.direction == UP)
            {
                if (lastHorizDir == RIGHT) {
                    rayAngle = -60.0f; // Jam 1
                    offsetRight = 0.8f;   
                }
                else
                    rayAngle = -120.0f; // Jam 11
            }
            else if (player.Anim.direction == DOWN)
            {
                if (lastHorizDir == RIGHT) {
                    rayAngle = 60.0f; // Jam 5
                    offsetRight = 0.8f;
                }
                else
                    rayAngle = 120.0f; // Jam 7
            }
            else
            {
                rayAngle = 0.0f;
            }

            drawAngle = rayAngle;
            thrust = 0.0f;
        }

        float rad = rayAngle * (PI / 180.0f);
        visualPos.x += cosf(rad) * thrust;
        visualPos.y += sinf(rad) * thrust;

        const Frame &frame = GetFrame(def.spriteKey);

        Display display;
        display.position = visualPos;
        display.size = 32;
        display.offset = {0, 1 + offsetRight};
        display.origin = {17.0f, (float)(frame.height * display.size)};
        display.rotation = drawAngle + 90.0f;
        display.tint = WHITE;

        DrawFrame(frame, display);
    }

    void AddDamagePopup(Vector2 pos, float damage)
    {
        Effects::AddDamage(pos, damage);
    }
}
