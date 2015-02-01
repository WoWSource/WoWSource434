/*
* Copyright (C) 2010-2011 Project Trinity <http://www.projecttrinity.org/>
*
* This program is free software; you can redistribute it and/or modify it
* under the terms of the GNU General Public License as published by the
* Free Software Foundation; either version 2 of the License, or (at your
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

#include "ObjectMgr.h"
#include "ScriptMgr.h"
#include "ScriptedCreature.h"
#include "PoolMgr.h"
#include "Group.h"
#include "SpellInfo.h"
#include "baradin_hold.h"
#include "ObjectAccessor.h"
#include "InstanceScript.h"

enum Spells
{
    SPELL_EYES_OF_OCCUTHAR = 96920,
    SPELL_FOCUSED_FIRE     = 96884,
    SPELL_SEARING_SHADOWS  = 96913,
    SPELL_BERSERK          = 47008,
    SPELL_GAZE_OF_OCCUTHAR = 97028,
    SPELL_OCCU_DESTRUCTION = 96968
};

enum Events
{
    EVENT_EYES_OF_OCCUTHAR = 1,
    EVENT_FOCUSED_FIRE     = 2,
    EVENT_SEARING_SHADOWS  = 3,
    EVENT_BERSERK          = 4,
    EVENT_GAZE_OF_OCCUTHAR = 5,
    EVENT_AOE              = 6,
    EVENT_OCCU_DESTRUCT    = 7,
    EVENT_EYE_REMOVE       = 8
};

class boss_occuthar : public CreatureScript
{
    public:
        boss_occuthar() : CreatureScript("boss_occuthar") { }

        struct boss_occutharAI : public BossAI
        {
            boss_occutharAI(Creature* creature) : BossAI(creature, DATA_OCCUTHAR) { }

            void Reset()
            {
                _Reset();
                me->RemoveAurasDueToSpell(SPELL_BERSERK);
                events.ScheduleEvent(EVENT_EYES_OF_OCCUTHAR, 60000);
                events.ScheduleEvent(EVENT_FOCUSED_FIRE, 20000);
                events.ScheduleEvent(EVENT_SEARING_SHADOWS, 15000);
                instance->HandleGameObject(instance->GetData64(GO_OCCUTHAR_DOOR), true); // set me to true, when we release step by step
            }

            void EnterCombat(Unit* /*who*/)
            {
                events.ScheduleEvent(EVENT_EYES_OF_OCCUTHAR, 60000);
                events.ScheduleEvent(EVENT_FOCUSED_FIRE, 20000);
                events.ScheduleEvent(EVENT_SEARING_SHADOWS, 15000);
                instance->HandleGameObject(instance->GetData64(GO_OCCUTHAR_DOOR), false);
            }

            void JustDied(Unit* /*killer*/)
            {
                 instance->HandleGameObject(instance->GetData64(GO_OCCUTHAR_DOOR), true);
            }

            void SummEyes()
            {
                if (Unit* target = SelectTarget(SELECT_TARGET_RANDOM))
                me->SummonCreature(NPC_EYE_OF_OCCUTHAR, target->GetPositionX(), target->GetPositionY(), target->GetPositionZ());
            }

            void SummDummy()
            {
                if (Unit* target = SelectTarget(SELECT_TARGET_RANDOM, 0, NonTankTargetSelector(me)))
                me->SummonCreature(NPC_FOCUSED_FIRE_DUMMY, target->GetPositionX(), target->GetPositionY(), target->GetPositionZ());
            }

            void UpdateAI(uint32 const diff)
            {
                if (!UpdateVictim() || !CheckInRoom())
                    return;

                events.Update(diff);

                if (me->HasUnitState(UNIT_STATE_CASTING))
                    return;

                while (uint32 eventId = events.ExecuteEvent())
                {
                    switch (eventId)
                    {
                    case EVENT_EYES_OF_OCCUTHAR:
                        DoCast(SPELL_EYES_OF_OCCUTHAR);
                        SummEyes();
                        events.ScheduleEvent(EVENT_EYES_OF_OCCUTHAR, 60000);
                        break;
                    case EVENT_FOCUSED_FIRE:
                        SummDummy();
                        events.ScheduleEvent(EVENT_FOCUSED_FIRE, 20000);
                        break;
                    case EVENT_SEARING_SHADOWS:
                        DoCast(SPELL_SEARING_SHADOWS);
                        events.ScheduleEvent(EVENT_SEARING_SHADOWS, 15000);
                        break;
                        }
                    }
                DoMeleeAttackIfReady();
            }
    };
    CreatureAI* GetAI(Creature* creature) const
    {
        return new boss_occutharAI(creature);
    }
};

class npc_eye_of_occuthar : public CreatureScript
{
    public:
        npc_eye_of_occuthar() : CreatureScript("npc_eye_of_occuthar") { }

        struct npc_eye_of_occutharAI : public BossAI
        {
            npc_eye_of_occutharAI(Creature* creature) : BossAI(creature, DATA_EYE_OF_OCCUTHAR)
            {
            }

            void reset()
            {
                events.ScheduleEvent(EVENT_GAZE_OF_OCCUTHAR, 8000);
            }

            void EnterCombat(Unit* /*who*/)
            {
                events.ScheduleEvent(EVENT_GAZE_OF_OCCUTHAR, 1000);
            }

            void UpdateAI(uint32 const diff)
            {
                if (!UpdateVictim() || !CheckInRoom())
                    return;

                events.Update(diff);

                if(me->HasUnitState(UNIT_STATE_CASTING) && !me->GetCurrentSpell(CURRENT_CHANNELED_SPELL))
                    return;

                while (uint32 eventId = events.ExecuteEvent())
                {
                    switch (eventId)
                    {
                    case EVENT_GAZE_OF_OCCUTHAR:
                        if (Unit* target = SelectTarget(SELECT_TARGET_RANDOM))
                            DoCast(target, SPELL_GAZE_OF_OCCUTHAR);
                        events.ScheduleEvent(EVENT_OCCU_DESTRUCT, 10000);
                        break;
                    case EVENT_OCCU_DESTRUCT:
                        DoCast(me, SPELL_OCCU_DESTRUCTION, false);
                        events.ScheduleEvent(EVENT_EYE_REMOVE, 1500);
                        break;
                    case EVENT_EYE_REMOVE:
                        me->DisappearAndDie();
                        break;
                        }
                    }
                }

            protected:
            EventMap Events;
        };

        CreatureAI* GetAI(Creature* creature) const
        {
            return new npc_eye_of_occutharAI(creature);
        }
};

class npc_focused_fire_dummy : public CreatureScript
{
    public:
        npc_focused_fire_dummy() : CreatureScript("npc_focused_fire_dummy") { }

        struct npc_focused_fire_dummyAI : public BossAI
        {
            npc_focused_fire_dummyAI(Creature* creature) : BossAI(creature, DATA_FOCUSED_FIRE_DUMMY)
            {
            }

            void EnterCombat(Unit* who)
            {
                events.ScheduleEvent(EVENT_AOE, 1000);
            }

            void UpdateAI(uint32 const diff)
            {
                if (!UpdateVictim() || !CheckInRoom())
                    return;

                events.Update(diff);

                if (me->HasUnitState(UNIT_STATE_CASTING))
                    return;

                while (uint32 eventId = events.ExecuteEvent())
                {
                    switch (eventId)
                    {
                    case EVENT_AOE:
                        DoCast(SPELL_FOCUSED_FIRE);
                        break;
                        }
                    }
                }

            protected:
            EventMap Events;
        };

        CreatureAI* GetAI(Creature* creature) const
        {
            return new npc_focused_fire_dummyAI(creature);
        }
};

void AddSC_boss_occuthar()
{
    new boss_occuthar();
    new npc_eye_of_occuthar();
    new npc_focused_fire_dummy();
}