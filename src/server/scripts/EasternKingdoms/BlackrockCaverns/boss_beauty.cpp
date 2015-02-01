/*
 * Copyright (C) 2011-2012 Project SkyFire <http://www.projectskyfire.org/>
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License as published by the
 * Free Software Foundation; either version 3 of the License, or (at your
 * option) any later version.
 *
 * This program is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
 * FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for
 * more details.
 *
 * You should have received a copy of the GNU General Public License along
 * with this program. If not, see <http://www.gnu.org/licenses/>.
 */

#include "ScriptPCH.h"
#include "blackrock_caverns.h"

enum eBeautySpells
{
    SPELL_BERSKER           = 82395,
    SPELL_BERSERKER_CHARGE  = 76030,
    SPELL_FLAMEBREAKER      = 76032,
    SPELL_MAGMA_SPIT        = 76031,
    SPELL_TERRIFYING_ROAR   = 76028,
};

class boss_beauty: public CreatureScript
{
public:
    boss_beauty() : CreatureScript("boss_beauty") { }

    CreatureAI* GetAI(Creature* creature) const
    {
        return new boss_beautyAI (creature);
    }

    struct boss_beautyAI : public ScriptedAI
    {
        boss_beautyAI(Creature* creature) : ScriptedAI(creature)
        {
            instance = creature->GetInstanceScript();
        }

        InstanceScript* instance;

        uint32 BerserkerChargeTimer;
        uint32 FlamebreakerTimer;
        uint32 MagmaSpitTimer;
        uint32 TerrifyingRoarTimer;

        void Reset()
        {
            BerserkerChargeTimer = urand(15000, 25000);
            TerrifyingRoarTimer = urand(20000, 30000);
            MagmaSpitTimer = urand(5000, 20000);
            FlamebreakerTimer = urand(10000, 20000);

            if (IsHeroic())
            {
                if (Creature * first = GetClosestCreatureWithEntry(me, NPC_BUSTER, 100.0f))
                    if (first->isDead())
                        first->Respawn();

                if (Creature * second = GetClosestCreatureWithEntry(me, NPC_LUCKY, 100.0f))
                    if (second->isDead())
                        second->Respawn();
            }
        }

        void JustDied(Unit* /*Kill*/)
        {
            if (instance)
                instance->SetData(BOSS_BEAUTY, DONE);
        }

        void EnterCombat(Unit* /*target*/)
        {
            if (instance)
                instance->SetData(BOSS_BEAUTY, IN_PROGRESS);

            if (IsHeroic()) // Put pups in combat on heroic mode.
            {
                if (Creature * first = GetClosestCreatureWithEntry(me, NPC_BUSTER, 100.0f))
                    DoZoneInCombat(first);

                if (Creature * second = GetClosestCreatureWithEntry(me, NPC_LUCKY, 100.0f))
                    DoZoneInCombat(second);
            }
        }

        void UpdateAI(uint32 const Diff)
        {
            if (!UpdateVictim())
                return;

            if (TerrifyingRoarTimer <= Diff)
            {
                if (me->IsNonMeleeSpellCasted(false))
                    return;

                me->CastSpell(me, SPELL_TERRIFYING_ROAR, false);

                TerrifyingRoarTimer = urand(20000, 30000);
            }
            else
                TerrifyingRoarTimer -= Diff;

            if (FlamebreakerTimer <= Diff)
            {
                if (me->IsNonMeleeSpellCasted(false))
                    return;

                me->CastSpell(me, SPELL_FLAMEBREAKER, false);

                FlamebreakerTimer = urand(15000, 25000);
            }
            else
                FlamebreakerTimer -= Diff;


            if (MagmaSpitTimer <= Diff)
            {
                if (me->IsNonMeleeSpellCasted(false))
                    return;

                if (Unit * pTarget = SelectTarget(SELECT_TARGET_RANDOM, 0, 60.0f, true))
                    me->CastSpell(pTarget, SPELL_MAGMA_SPIT, false);

                MagmaSpitTimer = urand(10000, 16000);
            }
            else
                MagmaSpitTimer -= Diff;

            if (BerserkerChargeTimer <= Diff)
            {
                if (me->IsNonMeleeSpellCasted(false))
                    return;

                if (Unit * pTarget = SelectTarget(SELECT_TARGET_FARTHEST, 0, 60.0f, true))
                    me->CastSpell(pTarget, SPELL_BERSERKER_CHARGE, false);

                BerserkerChargeTimer = urand(20000, 25000);
            }
            else
                BerserkerChargeTimer -= Diff;

            DoMeleeAttackIfReady();
        }
    };
};

enum eBeautyPupSpells
{
    SPELL_LAVA_DROOL                = 76628,
    SPELL_LITTLE_BIG_FLAME_BREATH   = 76665,
};

class boss_beauty_pup: public CreatureScript
{
public:
    boss_beauty_pup() : CreatureScript("boss_beauty_pup") { }

    CreatureAI* GetAI(Creature* creature) const
    {
        return new boss_beauty_pupAI (creature);
    }

    struct boss_beauty_pupAI : public ScriptedAI
    {
        boss_beauty_pupAI(Creature* creature) : ScriptedAI(creature)
        {
        }

        uint32 LavaDroolTimer;
        uint32 LittleBigFlameBreathTimer;

        void Reset()
        {
            LavaDroolTimer = urand(15000, 25000);
            LittleBigFlameBreathTimer = urand(10000, 20000);
        }

        void EnterCombat(Unit* /*who*/)
        {
            if(IsHeroic())
                if (Creature * beauty = GetClosestCreatureWithEntry(me, BOSS_BEAUTY, 100.0f))
                    DoZoneInCombat(beauty);
        }

        void JustDied(Unit* /*Kill*/)
        {
            // Apply berserker aura if Runty was killed before boss
            if (me->GetEntry() == NPC_RUNTY)
                if (Creature * beauty = GetClosestCreatureWithEntry(me, BOSS_BEAUTY, 100.0f))
                    beauty->CastSpell(beauty, SPELL_BERSKER, true);
        }

        void UpdateAI(uint32 const Diff)
        {
            if (!UpdateVictim())
                return;

            if (LavaDroolTimer <= Diff)
            {
                if (me->IsNonMeleeSpellCasted(false))
                    return;

                if (Unit * pTarget = SelectTarget(SELECT_TARGET_RANDOM, 0, 60.0f, true))
                    me->CastSpell(pTarget, SPELL_LAVA_DROOL, false);

                LavaDroolTimer = urand(15000, 25000);
            }
            else
                LavaDroolTimer -= Diff;

            if (LittleBigFlameBreathTimer <= Diff)
            {
                if (me->IsNonMeleeSpellCasted(false))
                    return;

                DoCastVictim(SPELL_LITTLE_BIG_FLAME_BREATH);

                LittleBigFlameBreathTimer = urand(12000, 18000);
            }
            else
                LittleBigFlameBreathTimer -= Diff;

            DoMeleeAttackIfReady();
        }
    };
};

void AddSC_boss_beauty()
{
    new boss_beauty();
    new boss_beauty_pup();
}
