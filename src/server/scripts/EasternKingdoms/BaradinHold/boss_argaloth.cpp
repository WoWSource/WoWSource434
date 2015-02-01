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

#include "ScriptPCH.h"
#include "baradin_hold.h"

enum Spells
{
    SPELL_BERSERK               = 47008,
    SPELL_CONSUMING_DARKNESS    = 88954,
    SPELL_METEOR_SLASH          = 88942,
    SPELL_METEOR_SLASH_VISUAL   = 88949,
    SPELL_FEL_FIRESTORM         = 88972,
};

enum Events
{
    EVENT_BERSERK               = 1,
    EVENT_CONSUMING_DARKNESS    = 2,
    EVENT_METEOR_SLASH          = 3,
};

class boss_argaloth: public CreatureScript
{
    public:
        boss_argaloth() : CreatureScript("boss_argaloth")
        {
        }

        struct boss_argalothAI: public BossAI
        {
            boss_argalothAI(Creature* creature) : BossAI(creature, BOSS_ARGALOTH)
            {
            }

            bool felFirestorm;

            void EnterCombat(Unit* /*who*/)
            {
                _EnterCombat();

                if(instance)
                {
                   instance->HandleGameObject(instance->GetData64(GO_ARGALOTH_DOOR), false);
                }
            }

            void Reset()
            {
                _Reset();
                me->RemoveAurasDueToSpell(SPELL_BERSERK);
                events.ScheduleEvent(EVENT_BERSERK, 300000);
                events.ScheduleEvent(EVENT_CONSUMING_DARKNESS, 14000);
                events.ScheduleEvent(EVENT_METEOR_SLASH, 10000);
                felFirestorm = false;

                if(instance)
                {
                   instance->HandleGameObject(instance->GetData64(GO_ARGALOTH_DOOR), true);
                }
            }

            void JustDied(Unit* /*killer*/)
            {
                if(instance)
                {
                   instance->HandleGameObject(instance->GetData64(GO_ARGALOTH_DOOR), true);
                }
            }

            void UpdateAI(uint32 const diff)
            {
                if (!UpdateVictim())
                    return;

                if ((HealthBelowPct(66) && HealthAbovePct(33) && !felFirestorm) || (HealthBelowPct(33) && felFirestorm))
                {
                    DoCast(SPELL_FEL_FIRESTORM);
                    felFirestorm = !felFirestorm;
                    me->SetReactState(REACT_PASSIVE);
                    me->AttackStop();
                }

                if (me->HasAura(SPELL_FEL_FIRESTORM))
                    return;

                events.Update(diff);

                if (me->HasUnitState(UNIT_STATE_CASTING))
                    return;

                while (uint32 eventId = events.ExecuteEvent())
                {
                    switch (eventId)
                    {
                        case EVENT_CONSUMING_DARKNESS:
                            me->CastSpell(me, SPELL_CONSUMING_DARKNESS, false);
                            events.RescheduleEvent(EVENT_CONSUMING_DARKNESS, 22000);
                            break;
                        case EVENT_METEOR_SLASH:
                            DoCast(SPELL_METEOR_SLASH);
                            me->CastSpell(me, SPELL_METEOR_SLASH_VISUAL, true);
                            events.RescheduleEvent(EVENT_METEOR_SLASH, urand(15000, 20000));
                            break;
                        case EVENT_BERSERK:
                            DoCast(me, SPELL_BERSERK);
                            events.CancelEvent(EVENT_BERSERK);
                            break;
                    }
                }

                DoMeleeAttackIfReady();
            }
         };

        CreatureAI* GetAI(Creature* creature) const
        {
            return new boss_argalothAI(creature);
        }
};

class spell_argaloth_fel_firestorm : public SpellScriptLoader
{
    class script_impl : public AuraScript
    {
        PrepareAuraScript(script_impl);

        void OnRemove(AuraEffect const*, AuraEffectHandleModes)
        {
            Unit* const caster = GetCaster();
            if (!caster || !caster->ToCreature())
                return;

            std::list<Creature*> triggerList;
            caster->GetCreatureListWithEntryInGrid(triggerList, NPC_FEL_FIRESTORM_TRIGGER, 100.0f);
            for (std::list<Creature*>::iterator itr = triggerList.begin(); itr != triggerList.end(); ++itr)
                if (Creature* const trigger = *itr)
                    trigger->DespawnOrUnsummon(1000);
            caster->ToCreature()->SetReactState(REACT_AGGRESSIVE);
        }

        void Register()
        {
            OnEffectRemove += AuraEffectRemoveFn(script_impl::OnRemove, EFFECT_0, SPELL_AURA_PERIODIC_TRIGGER_SPELL, AURA_EFFECT_HANDLE_REAL);
        }
    };

public:
    spell_argaloth_fel_firestorm()
        : SpellScriptLoader("spell_argaloth_fel_firestorm")
    {
    }

    AuraScript* GetAuraScript() const
    {
        return new script_impl;
    }
};

void AddSC_boss_argaloth()
{
    new boss_argaloth();
    new spell_argaloth_fel_firestorm();
}