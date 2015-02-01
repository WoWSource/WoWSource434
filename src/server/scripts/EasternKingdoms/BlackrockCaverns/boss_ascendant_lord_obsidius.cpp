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

enum eObsidiusYells
{
    SAY_AGGRO                   = 0,
    SAY_TRANSFORM               = 1,
    SAY_KILL                    = 2,
    SAY_DEATH                   = 3,
};

enum eObsidiusSpells
{
    SPELL_STONE_BLOW            = 76185,
    SPELL_TWILIGHT_CORRUPTION   = 76188,
    SPELL_THUNDERCLAP           = 76186,
    SPELL_STOP_HEART            = 82393,

    SPELL_TRANSFORMATION_1      = 76200, // Scale
    SPELL_TRANSFORMATION_2      = 76242, // Unattackable
    SPELL_TRANSFORMATION_3      = 76274, // Stun

    SPELL_TRANSFORMATION_TRIG   = 76211,
};

class boss_ascendant_lord_obsidius: public CreatureScript
{
public:
    boss_ascendant_lord_obsidius() : CreatureScript("boss_ascendant_lord_obsidius") { }

    CreatureAI* GetAI(Creature* creature) const
    {
        return new boss_ascendant_lord_obsidiusAI (creature);
    }

    struct boss_ascendant_lord_obsidiusAI : public ScriptedAI
    {
        boss_ascendant_lord_obsidiusAI(Creature* creature) : ScriptedAI(creature)
        {
            instance = creature->GetInstanceScript();
        }

        InstanceScript* instance;

        uint32 StoneBlowTimer;
        uint32 TwilightCorruptionTimer;
        uint32 ThunderclapTimer;
        bool TransformationCounter;

        void Reset()
        {
            TransformationCounter = false;

            ThunderclapTimer = urand(12000, 18000);
            StoneBlowTimer = urand(15000, 25000);
            TwilightCorruptionTimer = urand(10000, 20000);

            std::list<Creature*> shadowsList;
            GetCreatureListWithEntryInGrid(shadowsList, me, NPC_SHADOW_OF_OBSIDIUS, 60.0f);
            if (shadowsList.empty())
                return;

            for (std::list<Creature*>::iterator itr = shadowsList.begin(); itr != shadowsList.end(); ++itr)
                if ((*itr)->isDead())
                    (*itr)->Respawn();
        }

        void KilledUnit(Unit* /*victim*/)
        {
            Talk(SAY_KILL);
        }

        void JustDied(Unit* /*Kill*/)
        {
            std::list<Creature*> shadowsList;
            GetCreatureListWithEntryInGrid(shadowsList, me, NPC_SHADOW_OF_OBSIDIUS, 60.0f);
            if (shadowsList.empty())
                return;

            for (std::list<Creature*>::iterator itr = shadowsList.begin(); itr != shadowsList.end(); ++itr)
                (*itr)->DisappearAndDie();

            Talk(SAY_DEATH);

            if (instance)
                instance->SetData(BOSS_ASCENDANT_LORD_OBSIDIUS, DONE);
        }

        void EnterCombat(Unit* /*Ent*/)
        {
            Talk(SAY_AGGRO);

            if (instance)
                instance->SetData(BOSS_ASCENDANT_LORD_OBSIDIUS, IN_PROGRESS);
        }

        void UpdateAI(uint32 const Diff)
        {
            if (!UpdateVictim())
                return;

            // Transformation on 66% and 33% of health
            if ((me->GetHealthPct() < 66.0f && !TransformationCounter && me->GetHealthPct() > 33.0f) || (me->GetHealthPct() < 33.0f && TransformationCounter))
            {
                TransformationCounter = !TransformationCounter;

                std::list<Creature*> shadowsList;
                GetCreatureListWithEntryInGrid(shadowsList, me, NPC_SHADOW_OF_OBSIDIUS, 60.0f);
                if (shadowsList.empty())
                    return;

                std::list<Creature*>::iterator current = shadowsList.begin();
                std::advance(current, urand(0, shadowsList.size() - 1));
                Creature * toTransform = *current;

                Position Pos;
                toTransform->GetPosition(&Pos);

                toTransform->NearTeleportTo(me->GetPositionX(), me->GetPositionY(), me->GetPositionZ(), me->GetOrientation());
                me->NearTeleportTo(Pos.GetPositionX(), Pos.GetPositionY(), Pos.GetPositionZ(), Pos.GetOrientation());

                // Needed to prevent continual visual movement on the stun
                toTransform->GetMotionMaster()->MoveIdle();
                me->GetMotionMaster()->MoveIdle();

                me->CastSpell(toTransform, SPELL_TRANSFORMATION_1, true);
                me->CastSpell(toTransform, SPELL_TRANSFORMATION_3, true);
                me->CastSpell(toTransform, SPELL_TRANSFORMATION_2, true);

                // Return to chase movement
                me->GetMotionMaster()->MoveChase(me->GetVictim());
                toTransform->GetMotionMaster()->MoveChase(me->GetVictim());

                Talk(SAY_TRANSFORM);
            }

            if (StoneBlowTimer <= Diff)
            {
                if (me->IsNonMeleeSpellCasted(false))
                    return;

                DoCastVictim(SPELL_STONE_BLOW);

                StoneBlowTimer = urand(20000, 25000);
            }
            else
                StoneBlowTimer -= Diff;

            if (TwilightCorruptionTimer <= Diff)
            {
                if (me->IsNonMeleeSpellCasted(false))
                    return;

                if (Unit * pTarget = SelectTarget(SELECT_TARGET_RANDOM, 0, 60.0f, true))
                    me->CastSpell(pTarget, SPELL_TWILIGHT_CORRUPTION, false);

                TwilightCorruptionTimer = urand(20000, 25000);
            }
            else
                TwilightCorruptionTimer -= Diff;

            if (IsHeroic())
            {
                if (ThunderclapTimer <= Diff)
                {
                    if (me->IsNonMeleeSpellCasted(false))
                        return;

                    me->CastSpell(me, SPELL_THUNDERCLAP, false);

                    ThunderclapTimer = urand(20000, 25000);
                }
                else
                    ThunderclapTimer -= Diff;
            }

            DoMeleeAttackIfReady();
        }
    };
};

void AddSC_boss_ascendant_lord_obsidius()
{
    new boss_ascendant_lord_obsidius();
}