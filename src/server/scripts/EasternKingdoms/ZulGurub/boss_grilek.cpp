/*
 * Copyright (C) 2008-2013 TrinityCore <http://www.trinitycore.org/>
 * Copyright (C) 2006-2009 ScriptDev2 <https://scriptdev2.svn.sourceforge.net/>
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
 * ToDo: Spellscript for Spikes (Rupture Line)
 * Special Sound entrys: 24238 | 24240 | 24239
 * ApplySpellImmunity Mask for Taunt doesnt work
*/

#include "ObjectMgr.h"
#include "ScriptMgr.h"
#include "ScriptedCreature.h"
#include "SpellScript.h"
#include "SpellAuraEffects.h"
#include "zulgurub.h"

enum Yells
{
    SAY_AGGRO                      = 0, // 24232 You seek me? we fight now!
    SAY_DEATH                      = 1, // 24233 Gri'lek wanders...no more...
    SAY_FIXATE_1                   = 2, // 24234 I COME FOR YOU. LITTLE MORTAL!
    SAY_FIXATE_2                   = 3, // 24235 I will catch you, small one.
    SAY_KILL_PLAYER_1              = 4, // 24236 Crushed!
    SAY_KILL_PLAYER_2              = 5, // 24237 You are weak!
    SAY_ROOT_1                     = 6, // 24241 Coward, you cannot run!
    SAY_ROOT_2                     = 7, // 24242 No, you cannot escape the rage of the Gurubashi!
    EMOTE_CHASE                    = 8  // Boss Emote Chasing Player  
};

enum Spells
{
    SPELL_AVATAR                   = 96618,
    SPELL_PURSUIT                  = 96631,
    SPELL_PURSUIT_BUFF             = 96306,
    SPELL_ENTANGLING_ROOTS         = 96633,
    SPELL_RUPTURE_LINE             = 96619, // Direct Damage & Vislual: 96620 // trigger 51422
    SPELL_RUPTURE_LINE_DAMAGE      = 96620,
};

enum Events
{
    EVENT_AVATAR                   = 1,
    EVENT_PURSUIT,
    EVENT_ENTANGLING_ROOTS,
    EVENT_RUPTURE_LINE
};

class boss_grilek : public CreatureScript
{
    public:
        boss_grilek() : CreatureScript("boss_grilek") { }

        struct boss_grilekAI : public BossAI
        {
            boss_grilekAI(Creature* creature) : BossAI(creature, DATA_GRILEK) { }

            void Reset()
            {
                _Reset();
                if (GameObject* forcefield = me->FindNearestGameObject(GO_THE_CACHE_OF_MADNESS_DOOR, 150.0f))
                    me->RemoveGameObject(forcefield, true);
            }

            void EnterCombat(Unit* /*who*/)
            {
                _EnterCombat();
                Talk(SAY_AGGRO);
                me->SummonGameObject(GO_THE_CACHE_OF_MADNESS_DOOR, -11938.6f, -1843.32f, 61.7272f, 0.0899053f, 0, 0, 0, 0, 0);
                events.ScheduleEvent(EVENT_AVATAR, 10000);
                events.ScheduleEvent(EVENT_ENTANGLING_ROOTS, 20000);
                events.ScheduleEvent(EVENT_RUPTURE_LINE, 30000);
            }

            void SetTempThreat(float threat)
            {
                _tempThreat = threat;
            }

            void _ResetThreat(Unit* target)
            {
                DoModifyThreatPercent(target, -100);
                me->AddThreat(target, _tempThreat);
            }

            void JustDied(Unit* /*killer*/)
            {
                _JustDied();
                Talk(SAY_DEATH);
                if (GameObject* forcefield = me->FindNearestGameObject(GO_THE_CACHE_OF_MADNESS_DOOR, 150.0f))
                    me->RemoveGameObject(forcefield, true);
            }

            void KilledUnit(Unit* victim)
            {
                if (victim->GetTypeId() == TYPEID_PLAYER)
                    Talk(RAND(SAY_KILL_PLAYER_1, SAY_KILL_PLAYER_2));
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
                        case EVENT_AVATAR:
                            DoCast(me, SPELL_AVATAR);
                            events.ScheduleEvent(EVENT_AVATAR, urand(35*IN_MILLISECONDS, 45*IN_MILLISECONDS));
                            events.ScheduleEvent(EVENT_PURSUIT, 500);
                            break;
                        case EVENT_PURSUIT:
                            Talk(RAND(SAY_FIXATE_1, SAY_FIXATE_2));
                            DoCastRandom(SPELL_PURSUIT, 150.0f);
                            break;
                        case EVENT_ENTANGLING_ROOTS:
                            Talk(RAND(SAY_ROOT_1, SAY_ROOT_2));
                            DoCastRandom(SPELL_ENTANGLING_ROOTS, 30.0f);
                            events.ScheduleEvent(EVENT_ENTANGLING_ROOTS, urand(20*IN_MILLISECONDS, 25*IN_MILLISECONDS));
                            break;
                        case EVENT_RUPTURE_LINE:
                            DoCastVictim(SPELL_RUPTURE_LINE);
                            events.ScheduleEvent(EVENT_RUPTURE_LINE, urand(30*IN_MILLISECONDS, 40*IN_MILLISECONDS));
                            break;
                    }
                }
                DoMeleeAttackIfReady();
            }
        private:
            float _tempThreat;
        };

        CreatureAI* GetAI(Creature* creature) const
        {
            return new boss_grilekAI(creature);
        }
};

class spell_grilek_pursuit : public SpellScriptLoader
{
    public:
        spell_grilek_pursuit() : SpellScriptLoader("spell_grilek_pursuit") { }

        class spell_grilek_pursuit_SpellScript : public SpellScript
        {
            PrepareSpellScript(spell_grilek_pursuit_SpellScript);

            void HandleScriptEffect(SpellEffIndex /*effIndex*/)
            {
                if (GetCaster())
                    if (Creature* grilek = GetCaster()->ToCreature())
                    {
                        if (Unit* target = grilek->AI()->SelectTarget(SELECT_TARGET_FARTHEST, 0, 150.0f, true))
                        {
                            grilek->AI()->Talk(EMOTE_CHASE, target->GetGUID());
                            CAST_AI(boss_grilek::boss_grilekAI, grilek->AI())->SetTempThreat(grilek->getThreatManager().getThreat(target));
                            grilek->AddThreat(target, float(GetEffectValue()));
                            target->AddThreat(grilek, float(GetEffectValue()));
                        }
                    }
            }

            void Register()
            {
                OnEffectHitTarget += SpellEffectFn(spell_grilek_pursuit_SpellScript::HandleScriptEffect, EFFECT_1, SPELL_EFFECT_SCRIPT_EFFECT);
            }
        };

        class spell_grilek_pursuit_AuraScript : public AuraScript
        {
            PrepareAuraScript(spell_grilek_pursuit_AuraScript);

            void HandleExtraEffect(AuraEffect const* /*aurEff*/, AuraEffectHandleModes /*mode*/)
            {
                if (Unit* caster = GetCaster())
                    if (Creature* creCaster = caster->ToCreature())
                        CAST_AI(boss_grilek::boss_grilekAI, creCaster->AI())->_ResetThreat(GetTarget());
            }

            void Register()
            {
                AfterEffectRemove += AuraEffectRemoveFn(spell_grilek_pursuit_AuraScript::HandleExtraEffect, EFFECT_0, SPELL_AURA_DUMMY, AURA_EFFECT_HANDLE_REAL);
            }
        };

        SpellScript* GetSpellScript() const
        {
            return new spell_grilek_pursuit_SpellScript();
        }

        AuraScript* GetAuraScript() const
        {
            return new spell_grilek_pursuit_AuraScript();
        }
};

class spell_grilek_pursuit_buff_state : public SpellScriptLoader
{
    public:
        spell_grilek_pursuit_buff_state() : SpellScriptLoader("spell_grilek_pursuit_buff_state") { }

        class spell_grilek_pursuit_buff_state_AuraScript : public AuraScript
        {
            PrepareAuraScript(spell_grilek_pursuit_buff_state_AuraScript);

            void OnApply(AuraEffect const* /*aurEff*/, AuraEffectHandleModes /*mode*/)
            {
//                GetTarget()->ApplySpellImmune(0, IMMUNITY_STATE, SPELL_AURA_MOD_TAUNT, true);
//                GetTarget()->ApplySpellImmune(0, IMMUNITY_EFFECT, SPELL_EFFECT_ATTACK_ME, true);
                GetTarget()->ApplySpellImmune(0, IMMUNITY_ID, 56222, true); // Dark Command - DeathKnight
                GetTarget()->ApplySpellImmune(0, IMMUNITY_ID, 355, true);   // Taunt - Warrior
                GetTarget()->ApplySpellImmune(0, IMMUNITY_ID, 62124, true); // Reckoning - Paladin
            }

            void OnRemove(AuraEffect const* /*aurEff*/, AuraEffectHandleModes /*mode*/)
            {
//                GetTarget()->ApplySpellImmune(0, IMMUNITY_STATE, SPELL_AURA_MOD_TAUNT, false);
//                GetTarget()->ApplySpellImmune(0, IMMUNITY_EFFECT, SPELL_EFFECT_ATTACK_ME, false);
                GetTarget()->ApplySpellImmune(0, IMMUNITY_ID, 56222, false); // Dark Command - DeathKnight
                GetTarget()->ApplySpellImmune(0, IMMUNITY_ID, 355, false);   // Taunt - Warrior
                GetTarget()->ApplySpellImmune(0, IMMUNITY_ID, 62124, false); // Reckoning - Paladin
            }

            void Register()
            {
                OnEffectApply += AuraEffectApplyFn(spell_grilek_pursuit_buff_state_AuraScript::OnApply, EFFECT_2, SPELL_AURA_LINKED, AURA_EFFECT_HANDLE_REAL);
                OnEffectRemove += AuraEffectRemoveFn(spell_grilek_pursuit_buff_state_AuraScript::OnRemove, EFFECT_2, SPELL_AURA_LINKED, AURA_EFFECT_HANDLE_REAL);
            }
        };

        AuraScript* GetAuraScript() const
        {
            return new spell_grilek_pursuit_buff_state_AuraScript();
        }
};

void AddSC_boss_grilek()
{
    new boss_grilek();
    new spell_grilek_pursuit();
    new spell_grilek_pursuit_buff_state();
}
