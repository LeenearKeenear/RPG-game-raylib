#include "../include/combat.h"
#include "../include/player.h"
#include "../include/input.h"
#include "../include/animation.h"
#include "../include/inventory.h"

namespace Combat
{
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
                    PlayAnimation(player.Anim, ATTACK, player.Anim.direction, PlayerAnimationSet);
                    player.Anim.isAttacking = true;
                    player.Mana -= player.AttackManaCost;
                    player.ManaRegenTimer = player.ManaRegenDelay;
                    TraceLog(LOG_INFO, "PLAYER: Attack! (slot %d) - Mana: %.1f", (int)InputInstance.GetActiveSlot(), player.Mana);
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
