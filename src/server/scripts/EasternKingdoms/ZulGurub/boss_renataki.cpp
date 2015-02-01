/*
 * Copyright (C) 2008-2013 TrinityCore <http://www.trinitycore.org/>
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

/*
 * ToDo: Resolve Animated Hit issues after blade storm
*/

#include "ObjectMgr.h"
#include "ScriptMgr.h"
#include "ScriptedCreature.h"
#include "zulgurub.h"

enum Spells
{
    SPELL_AMBUSH                   = 96640,
    SPELL_DEADLY_POISON            = 96648,
    SPELL_FRENZY                   = 8269,
    SPELL_THOUSAND_BLADES          = 96646,
    SPELL_VANISH                   = 96639,
    SPELL_CHARGE_TARGET_DEST       = 72700
};

enum Events
{
    EVENT_DEADLY_POISON            = 1,
    EVENT_FRENZY,
    EVENT_AMBUSH,
    EVENT_AMBUSH_END,
    EVENT_THOUSAND_BLADES,
    EVENT_THOUSAND_BLADES_START,
    EVENT_THOUSAND_BLADES_END,
    EVENT_VANISH
};

enum Yells
{
    SAY_AMBUSH                    = 0,
    SAY_THOUSAND_BLADES           = 1,
    SAY_AGGRO                     = 2,
    SAY_KILL_PLAYER               = 3,
    SAY_DEATH                     = 4,
    EMOTE_FRENZY                  = 5
};

class boss_renataki : public CreatureScript
{
    public:
        boss_renataki() : CreatureScript("boss_renataki") { }

        struct boss_renatakiAI : public BossAI
        {
            boss_renatakiAI(Creature* creature) : BossAI(creature, DATA_RENATAKI)
            {
                _baseRun = creature->GetSpeedRate(MOVE_RUN);
            }

            void Reset()
            {
                _Reset();
                me->SetReactState(REACT_AGGRESSIVE);
                enrage = false;
                me->RemoveAllAuras();
                if (GameObject* forcefield = me->FindNearestGameObject(GO_THE_CACHE_OF_MADNESS_DOOR, 150.0f))
                    me->RemoveGameObject(forcefield, true);
            }

            void EnterCombat(Unit* /*who*/)
            {
                _EnterCombat();
                Talk(SAY_AGGRO);
                me->SummonGameObject(GO_THE_CACHE_OF_MADNESS_DOOR, -11938.6f, -1843.32f, 61.7272f, 0.0899053f, 0, 0, 0, 0, 0);
                events.ScheduleEvent(EVENT_DEADLY_POISON, 5000);
                events.ScheduleEvent(EVENT_VANISH, 15000);
                events.ScheduleEvent(EVENT_THOUSAND_BLADES, 33000);
            }

            void JustDied(Unit* /*killer*/)
            {
                _JustDied();
                Talk(SAY_DEATH);
                if (GameObject* forcefield = me->FindNearestGameObject(GO_THE_CACHE_OF_MADNESS_DOOR, 150.0f))
                    me->RemoveGameObject(forcefield, true);
            }

            void DamageTaken(Unit* /*done_by*/, uint32 & /*damage*/)
            {
                if (!enrage && me->GetHealthPct() <= 30.0f)
                {
                    enrage = true;
                    Talk(EMOTE_FRENZY);
                    me->CastSpell(me, SPELL_FRENZY, false);
                }
            }

            void KilledUnit(Unit* victim)
            {
                if (victim->GetTypeId() == TYPEID_PLAYER)
                    Talk(SAY_KILL_PLAYER);
            }

            void UpdateAI(uint32 const diff)
            {
                if(!UpdateVictim())
                    return;

                events.Update(diff);

                if(me->HasUnitState(UNIT_STATE_CASTING))
                    return;

                if(uint32 eventId = events.ExecuteEvent())
                {
                    switch(eventId)
                    {
                        case EVENT_DEADLY_POISON:
                            DoCastVictim(SPELL_DEADLY_POISON);
                            events.ScheduleEvent(EVENT_DEADLY_POISON, urand(30*IN_MILLISECONDS, 40*IN_MILLISECONDS));
                            break;
                        case EVENT_VANISH:
                            DoCast(me, SPELL_VANISH);
                            events.ScheduleEvent(EVENT_AMBUSH, 1000);
                            events.ScheduleEvent(EVENT_VANISH, urand(40*IN_MILLISECONDS, 45*IN_MILLISECONDS));
                            break;
                        case EVENT_AMBUSH:
                             Talk(SAY_AMBUSH);
                             if (Unit* target = SelectTarget(SELECT_TARGET_FARTHEST, 0))
                                 DoCast(target, SPELL_AMBUSH);
                            events.ScheduleEvent(EVENT_AMBUSH_END, 1000);
                            break;
                        case EVENT_AMBUSH_END:
                            me->RemoveAura(SPELL_VANISH);
                            me->HandleEmoteCommand(EMOTE_ONESHOT_LAUGH);
                            me->SetReactState(REACT_AGGRESSIVE);
                            break;
                        case EVENT_THOUSAND_BLADES:
                            Talk(SAY_THOUSAND_BLADES);
                            me->CastSpell(me, SPELL_THOUSAND_BLADES, false);
                            events.ScheduleEvent(EVENT_THOUSAND_BLADES, 40*IN_MILLISECONDS);
                            events.ScheduleEvent(EVENT_THOUSAND_BLADES_START, 3000);
                            events.ScheduleEvent(EVENT_THOUSAND_BLADES_END, urand(10000, 15000));
                            break;
                       case EVENT_THOUSAND_BLADES_START:
                       {
                            if (Unit* target = SelectTarget(SELECT_TARGET_FARTHEST, 0, 50.0f, true))
                                me->CastSpell(target, SPELL_CHARGE_TARGET_DEST, true);

                            if (me->HasAura(SPELL_THOUSAND_BLADES))
                                events.ScheduleEvent(EVENT_THOUSAND_BLADES_START, urand(1000, 2000));
                            break;
                       }
                        case EVENT_THOUSAND_BLADES_END:
                            me->RemoveAura(SPELL_THOUSAND_BLADES);
                            me->SetSpeed(MOVE_RUN, _baseRun, true);
                            break;
                        default:
                            break;
                    }
                }
                DoMeleeAttackIfReady();
                EnterEvadeIfOutOfCombatArea(diff);
            }
        private:
            float _baseRun;
            bool enrage;
        };

        CreatureAI* GetAI(Creature* creature) const
        {
            return new boss_renatakiAI(creature);
        }
};

class spell_thousand_blades_damage : public SpellScriptLoader
{
    public:
        spell_thousand_blades_damage() : SpellScriptLoader("spell_thousand_blades_damage") { }

    private:
        class spell_thousand_blades_damage_SpellScript : public SpellScript
        {
            PrepareSpellScript(spell_thousand_blades_damage_SpellScript);

            void CalculateDamage(SpellEffIndex /*effIndex*/)
            {
                float dist = 1.0f - GetCaster()->GetDistance(GetHitUnit()) / 100;
                SetHitDamage(int32(GetHitDamage() * dist));
            }

            void Register()
            {
                OnEffectHitTarget += SpellEffectFn(spell_thousand_blades_damage_SpellScript::CalculateDamage, EFFECT_0, SPELL_EFFECT_SCHOOL_DAMAGE);
            }
        };

        SpellScript* GetSpellScript() const
        {
            return new spell_thousand_blades_damage_SpellScript();
        }
};

void AddSC_boss_renataki()
{
    new boss_renataki();
    new spell_thousand_blades_damage();
}
