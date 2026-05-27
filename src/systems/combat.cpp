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
    void Update(Player &player)
    {
        HandleDead(player);
        if (player.Anim.isDead)
            return;

        if (InputInstance.IsInventoryOpen())
            return;

        HandleStamina(player);
        HandleAttack(player);
    }

    void HandleDead(Player &player)
    {
        if (player.Health <= 0)
        {
            player.Health = 0;
            PlayAnimation(player.Anim, DEAD, player.Anim.direction);
            player.Anim.isDead = true;
        }
    }

    void HandleStamina(Player &player)
    {
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
    }

    void HandleAttack(Player &player)
    {
        if (InputInstance.IsLeftClickPressed())
        {
            player.attack.pressHeld = true;
        }
        if (!InputInstance.IsLeftClickDown())
        {
            player.attack.pressHeld = false;
        }

        if (player.attack.pressHeld && !player.Anim.isAttacking)
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
                        player.attack.pressHeld = false;
                        return;
                    }

                    player.Mana -= manaCost;
                    player.ManaRegenTimer = player.ManaRegenDelay;

                    player.attack.active = true;
                    player.attack.timer = 0;
                    player.attack.damagedEntities.clear();
                    player.attack.center = playerCenter;
                    player.attack.raycastAngle = angle;

                    Inventory::SetupAttackStats(player, attackFaceDir);

                    player.Anim.isAttacking = true;
                    TraceLog(LOG_INFO, "PLAYER: Serangan diarahkan ke (%.2f, %.2f)", attackDir.x, attackDir.y);
                }
                else
                {
                    Effects::AddLog("Stamina tidak cukup!");
                    player.attack.pressHeld = false;
                    TraceLog(LOG_WARNING, "PLAYER: Serangan gagal! Mana habis.");
                }
            }
        }
    }

    // void PerformHitDetection(Player &player)
    // {
    //     if (!player.attack.active || !player.attack.weapon)
    //         return;

    //     Vector2 playerCenter = {
    //         player.Position.x + player.GetHitboxOffsetX() + player.GetHitboxWidth() / 2,
    //         player.Position.y + player.GetHitboxOffsetY() + player.GetHitboxHeight() / 2};

    //     Rectangle attackHitbox;
    //     float reach = player.attack.weapon->reach;
    //     float breadth = player.attack.weapon->breadth;

    //     switch (player.Anim.direction)
    //     {
    //     case RIGHT:
    //         attackHitbox = {playerCenter.x + player.GetHitboxWidth() / 2,
    //                         playerCenter.y - breadth / 2, reach, breadth};
    //         break;
    //     case LEFT:
    //         attackHitbox = {playerCenter.x - player.GetHitboxWidth() / 2 - reach,
    //                         playerCenter.y - breadth / 2, reach, breadth};
    //         break;
    //     case DOWN:
    //         attackHitbox = {playerCenter.x - breadth / 2,
    //                         playerCenter.y + player.GetHitboxHeight() / 2, breadth, reach};
    //         break;
    //     case UP:
    //         attackHitbox = {playerCenter.x - breadth / 2,
    //                         playerCenter.y - player.GetHitboxHeight() / 2 - reach, breadth,
    //                         reach};
    //         break;
    //     }

    //     for (auto entity : Entities::GetRegistry())
    //     {
    //         Entity *playerAsEntity = &player;
    //         if (entity == playerAsEntity)
    //             continue;
    //         if (!entity->IsActive || entity->Health <= 0)
    //             continue;

    //         bool alreadyHit = false;
    //         for (Entity *ptr : player.attack.damagedEntities)
    //         {
    //             if (ptr == entity)
    //             {
    //                 alreadyHit = true;
    //                 break;
    //             }
    //         }
    //         if (alreadyHit)
    //             continue;

    //         if (CheckCollisionRecs(attackHitbox, entity->GetHitbox()))
    //         {
    //             Vector2 entityCenter = {entity->Position.x + FRAME_SIZE / 2, entity->Position.y + FRAME_SIZE / 2};
    //             Vector2 knockDir = Vector2Normalize(Vector2Subtract(entityCenter, playerCenter));

    //             float damage = player.attack.weapon->damage;
    //             Vector2 knockback = Vector2Scale(knockDir, player.attack.weapon->knockbackForce);
    //             entity->TakeDamage(damage, knockback);

    //             Effects::AddDamage(entityCenter, damage);

    //             player.attack.damagedEntities.push_back(entity);
    //             TraceLog(LOG_INFO, "COMBAT: Pemain mengenai musuh! Damage: %.1f", damage);
    //         }
    //     }

    //     HitPropsByAttack(attackHitbox, PlayerInstance.GetHitbox(), &player);
    // }

    // void HandleCombat(Player &player)
    // {
    //     if (player.Anim.isDead)
    //         return;

    //     if (InputInstance.IsInventoryOpen())
    //         return;

    //     if (InputInstance.IsLeftClickPressed())
    //     {
    //         player.attack.pressHeld = true;
    //     }
    //     if (!InputInstance.IsLeftClickDown())
    //     {
    //         player.attack.pressHeld = false;
    //     }

    //     if (player.Health <= 0)
    //     {
    //         player.Health = 0;
    //         PlayAnimation(player.Anim, DEAD, player.Anim.direction);
    //         player.Anim.isDead = true;
    //         return;
    //     }

    //     if (player.ManaRegenTimer > 0)
    //     {
    //         player.ManaRegenTimer -= Time::DELTA_TIME;
    //     }
    //     else
    //     {
    //         if (player.Mana < player.MaxMana)
    //         {
    //             player.Mana += player.ManaRegenRate * Time::DELTA_TIME;
    //             if (player.Mana > player.MaxMana)
    //                 player.Mana = player.MaxMana;
    //         }
    //     }

    //     if (player.attack.pressHeld && !player.Anim.isAttacking)
    //     {
    //         PlayerAction action = InputInstance.ResolveAction();
    //         if (action == ACTION_ATTACK)
    //         {
    //             Vector2 mouseWorld = GetScreenToWorld2D(GetVirtualMousePosition(player.State), camera);
    //             Vector2 playerCenter = {
    //                 player.Position.x + player.GetHitboxOffsetX() + player.GetHitboxWidth() / 2,
    //                 player.Position.y + player.GetHitboxOffsetY() + player.GetHitboxHeight() / 2};
    //             Vector2 attackDir = Vector2Normalize(Vector2Subtract(mouseWorld, playerCenter));

    //             float angle = atan2(attackDir.y, attackDir.x) * (180.0f / PI);
    //             Direction attackFaceDir;
    //             if (angle >= -135.0f && angle < -45.0f)
    //                 attackFaceDir = UP;
    //             else if (angle >= -45.0f && angle < 45.0f)
    //                 attackFaceDir = RIGHT;
    //             else if (angle >= 45.0f && angle < 135.0f)
    //                 attackFaceDir = DOWN;
    //             else
    //                 attackFaceDir = LEFT;

    //             Direction horizDir = (attackDir.x >= 0) ? RIGHT : LEFT;
    //             PlayAnimation(player.Anim, IDLE, horizDir);
    //             PlayAnimation(player.Anim, IDLE, attackFaceDir);

    //             float manaCost = Inventory::GetAttackManaCost(player);

    //             if (player.Mana >= manaCost)
    //             {
    //                 InventoryItem activeItem = Inventory::GetActiveHotbarItem(player);
    //                 if (activeItem.definitionId == -1 || itemDefs.GetById(activeItem.definitionId).category != ITEM_WEAPON)
    //                 {
    //                     Effects::AddLog("Tidak ada senjata!");
    //                     player.attack.pressHeld = false;
    //                     return;
    //                 }

    //                 player.Mana -= manaCost;
    //                 player.ManaRegenTimer = player.ManaRegenDelay;

    //                 player.attack.active = true;
    //                 player.attack.timer = 0;
    //                 player.attack.damagedEntities.clear();
    //                 player.attack.center = playerCenter;
    //                 player.attack.raycastAngle = angle;

    //                 Inventory::SetupAttackStats(player, attackFaceDir);

    //                 player.Anim.isAttacking = true;
    //                 TraceLog(LOG_INFO, "PLAYER: Serangan diarahkan ke (%.2f, %.2f)", attackDir.x, attackDir.y);
    //             }
    //             else
    //             {
    //                 Effects::AddLog("Stamina tidak cukup!");
    //                 player.attack.pressHeld = false;
    //                 TraceLog(LOG_WARNING, "PLAYER: Serangan gagal! Mana habis.");
    //             }
    //         }
    //     }
    // }

    // void HandleRevive(Player &player)
    // {
    //     if (player.Anim.isDead)
    //     {
    //         player.Anim.isDead = false;
    //         player.Anim.isAttacking = false;
    //         PlayAnimation(player.Anim, IDLE, player.Anim.direction);
    //         player.Health = player.MaxHealth;
    //         player.Mana = player.MaxMana;
    //         player.KnockbackVelocity = {0, 0};
    //         TraceLog(LOG_INFO, "PLAYER: Dihidupkan kembali!");
    //     }
    // }

    // void UpdateSwingAttack(Player &player, float dt)
    // {
    //     if (!player.attack.active || !player.attack.weapon)
    //         return;

    //     Vector2 playerCenter = {
    //         player.Position.x + player.GetHitboxOffsetX() + player.GetHitboxWidth() / 2,
    //         player.Position.y + player.GetHitboxOffsetY() + player.GetHitboxHeight() / 2};
    //     player.attack.center = playerCenter;


    //     player.attack.timer += dt;
    //     if (player.attack.timer >= player.attack.weapon->duration)
    //     {
    //         player.attack.active = false;
    //         player.attack.timer = 0;
    //         player.Anim.isAttacking = false;
    //     }
    //     else
    //     {
    //         float progress = player.attack.timer / player.attack.weapon->duration;

    //         float baseAngle = 0.0f;
    //         switch (player.Anim.direction)
    //         {
    //         case RIGHT: baseAngle = 0.0f; break;
    //         case DOWN:  baseAngle = 90.0f; break;
    //         case LEFT:  baseAngle = 180.0f; break;
    //         case UP:    baseAngle = -90.0f; break;
    //         }
    //         float startAngle = baseAngle + player.attack.weapon->startAngleOffset;

    //         if (player.attack.weapon->attackType == ATTACK_SLASH)
    //         {
    //             player.attack.currentAngle = Slash(player.attack.raycastAngle, progress);
    //             player.attack.thrustOffset = 0.0f;
    //         }
    //         else if (player.attack.weapon->attackType == ATTACK_THRUST)
    //         {
    //             player.attack.thrustOffset = progress * 16.0f;
    //             player.attack.currentAngle = startAngle;
    //         }
    //         else if (player.attack.weapon->attackType == ATTACK_PIERCE)
    //         {
    //             player.attack.thrustOffset = progress * 24.0f;
    //             player.attack.currentAngle = startAngle;
    //         }
    //         else if (player.attack.weapon->attackType == ATTACK_SLAM)
    //         {
    //             float slamProgress = progress * progress;
    //             player.attack.currentAngle = startAngle + (slamProgress * player.attack.weapon->sweepAngle);
    //             player.attack.thrustOffset = slamProgress * 8.0f;
    //         }
    //         else
    //         {
    //             player.attack.currentAngle = startAngle + (progress * player.attack.weapon->sweepAngle);
    //             player.attack.thrustOffset = 0.0f;
    //         }

    //         PerformHitDetection(player);
    //     }
    // }

    // void DrawSwingAttack(Player &player)
    // {
    //     InventoryItem item = Inventory::GetActiveHotbarItem(player);
    //     if (item.definitionId == -1)
    //         return;

    //     const ItemDefinition &def = itemDefs.GetById(item.definitionId);
    //     if (def.category != ITEM_WEAPON)
    //         return;

    //     Vector2 visualPos;
    //     float drawAngle;
    //     float thrust = 0.0f;
    //     float rayAngle;
    //     float offsetRight = 0.0f;

    //     if (player.attack.active)
    //     {
    //         visualPos = player.attack.center;
    //         drawAngle = player.attack.currentAngle;
    //         thrust = player.attack.thrustOffset;
    //         rayAngle = player.attack.raycastAngle;
    //     }
    //     else
    //     {
    //         visualPos = {
    //             player.Position.x + player.GetHitboxOffsetX() + player.GetHitboxWidth() / 2,
    //             player.Position.y + player.GetHitboxOffsetY() + player.GetHitboxHeight() / 2};

    //         static Direction lastHorizDir = RIGHT;
    //         if (player.Anim.direction == RIGHT)
    //             lastHorizDir = RIGHT;
    //         else if (player.Anim.direction == LEFT)
    //             lastHorizDir = LEFT;

    //         if (player.Anim.direction == RIGHT) {
    //             rayAngle = -9.0f; // Jam 3
    //             offsetRight = 0.8f;
    //         }
    //         else if (player.Anim.direction == LEFT)
    //             rayAngle = 189.0f; // Jam 9
    //         else if (player.Anim.direction == UP)
    //         {
    //             if (lastHorizDir == RIGHT) {
    //                 rayAngle = -60.0f; // Jam 1
    //                 offsetRight = 0.8f;   
    //             }
    //             else
    //                 rayAngle = -120.0f; // Jam 11
    //         }
    //         else if (player.Anim.direction == DOWN)
    //         {
    //             if (lastHorizDir == RIGHT) {
    //                 rayAngle = 60.0f; // Jam 5
    //                 offsetRight = 0.8f;
    //             }
    //             else
    //                 rayAngle = 120.0f; // Jam 7
    //         }
    //         else
    //         {
    //             rayAngle = 0.0f;
    //         }

    //         drawAngle = rayAngle;
    //         thrust = 0.0f;
    //     }

    //     float rad = rayAngle * (PI / 180.0f);
    //     visualPos.x += cosf(rad) * thrust;
    //     visualPos.y += sinf(rad) * thrust;

    //     const Frame &frame = GetFrame(def.spriteKey);

    //     Display display;
    //     display.position = visualPos;
    //     display.size = 32;
    //     display.offset = {0, 1 + offsetRight};
    //     display.origin = {17.0f, (float)(frame.height * display.size)};
    //     display.rotation = drawAngle + 90.0f;
    //     display.tint = WHITE;

    //     DrawFrame(frame, display);

    //     if (player.attack.active && player.attack.weapon && player.attack.weapon->attackType == ATTACK_SLASH && (def.spriteKey == "sword1" || def.spriteKey == "sword2"))
    //     {
    //         float progress = player.attack.timer / player.attack.weapon->duration;
    //         std::string slashSpriteKey;
            
    //         if (progress >= 1.0f / 3.0f && progress < 2.0f / 3.0f)
    //         {
    //             slashSpriteKey = (def.spriteKey == "sword1") ? "slashLightGraySmall1" : "slashSilverBlueMedium1";
    //         }
    //         else if (progress >= 2.0f / 3.0f)
    //         {
    //             slashSpriteKey = (def.spriteKey == "sword1") ? "slashLightGraySmall2" : "slashSilverBlueMedium2";
    //         }

    //         if (!slashSpriteKey.empty())
    //         {
    //             const Frame &slashFrame = GetFrame(slashSpriteKey);
    //             float radRay = rayAngle * (PI / 180.0f);
    //             Vector2 slashPos = player.attack.center;
    //             float slashDist = player.attack.weapon->reach * 0.65f;
    //             slashPos.x += cosf(radRay) * slashDist;
    //             slashPos.y += sinf(radRay) * slashDist;

    //             if (progress >= 2.0f / 3.0f)
    //             {
    //                 float shiftAngleRad = (rayAngle - 90.0f) * (PI / 180.0f);
    //                 float H = (def.spriteKey == "sword1") ? 26.0f : 37.0f;
    //                 float backDist = (def.spriteKey == "sword1") ? 16.0f : 19.5f;
    //                 slashPos.x += cosf(shiftAngleRad) * H;
    //                 slashPos.y += sinf(shiftAngleRad) * H;

    //                 // Additional shift back (opposite to attack direction)
    //                 slashPos.x -= cosf(radRay) * backDist;
    //                 slashPos.y -= sinf(radRay) * backDist;
    //             }

    //             Display slashDisplay;
    //             slashDisplay.position = slashPos;
    //             slashDisplay.size = 32;
    //             slashDisplay.offset = {0, 0};
    //             slashDisplay.origin = {
    //                 (float)slashFrame.width * slashDisplay.size / 2.0f,
    //                 (float)slashFrame.height * slashDisplay.size / 2.0f
    //             };
    //             slashDisplay.rotation = rayAngle + 90.0f;
    //             slashDisplay.tint = WHITE;

    //             DrawFrame(slashFrame, slashDisplay);
    //         }
    //     }
    // }

    // void AddDamagePopup(Vector2 pos, float damage)
    // {
    //     Effects::AddDamage(pos, damage);
    // }
}
