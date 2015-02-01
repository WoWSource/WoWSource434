/*
* ToDo: 
* Find the correct Sound Texts from DBC's
* Find a better Way for his Archangel Cast
*/

#include "ScriptPCH.h"
#include "ScriptMgr.h"
#include "shadowfang_keep.h"

enum Spells
{
    // Normal Mode
    SPELL_ASPHYXOATE                       = 93423,
    SPELL_ASPHYXIATE_STOP                  = 93422,
    SPELL_PAIN_AND_SUFFERING               = 93581,
    SPELL_STAY_OF_EXECUTION                = 93468,

    // Heroic Mode
    SPELL_DARK_ARCHANGEL_FORM              = 93757,
    SPELL_CALAMITY                         = 93812,
    SPELL_WRACKING_PAIN                    = 93720
};

enum Events
{
    EVENT_ASPHYXOATE                       = 1,
    EVENT_PAIN_AND_SUFFERING,
    EVENT_STAY_OF_EXECUTION,
    EVENT_WRACKING_PAIN
};

enum Texts
{
    SAY_AGGRO   = 0,
    SAY_DEATH,
    SAY_KILL,
    SAY_ARCHANGEL,
    SAY_ASPHYXIATION,
    SAY_STAY_OF_EXECUTION,
    SAY_STAY_OF_EXECUTION_HEROIC,
};

class boss_baron_ashbury : public CreatureScript
{
    public:
        boss_baron_ashbury() : CreatureScript("boss_baron_ashbury") { }

        struct boss_baron_ashburyAI : public BossAI
        {
            boss_baron_ashburyAI(Creature* creature) : BossAI(creature, DATA_BARON_ASHBURY_EVENT) { }

            void Reset()
            {
                _Reset();
                enraged = false;
                me->RemoveAllAuras();
                instance->DoRemoveAurasDueToSpellOnPlayers(SPELL_WRACKING_PAIN);

                me->SetReactState(REACT_AGGRESSIVE);
            }

            void EnterCombat(Unit* /*who*/)
            {
                _EnterCombat();
                Talk(SAY_AGGRO);

                instance->HandleGameObject(instance->GetData64(GO_BARON_ASHBURY_DOOR), false);
                events.ScheduleEvent(EVENT_ASPHYXOATE, 21500);
                events.ScheduleEvent(EVENT_PAIN_AND_SUFFERING, 30000);
                if (IsHeroic())
                    events.ScheduleEvent(EVENT_WRACKING_PAIN, urand(14000, 16000));
            }

            void JustDied(Unit* /*killer*/)
            {
                _JustDied();
                Talk(SAY_DEATH);

                instance->DoRemoveAurasDueToSpellOnPlayers(SPELL_WRACKING_PAIN);
            }

            void KilledUnit(Unit* victim)
            {
                Talk(SAY_KILL);
            }

            void EnterEvadeMode()
            {
                _DespawnAtEvade();
                me->GetMotionMaster()->MoveTargetedHome();
                instance->DoRemoveAurasDueToSpellOnPlayers(SPELL_WRACKING_PAIN);
            }

            void DamageTaken(Unit* /*attacker*/, uint32& /*damage*/)
            {
                if (IsHeroic())
                    if(!enraged && !me->IsNonMeleeSpellCasted(false) && HealthBelowPct(20))
                    {
                        enraged = true;
                        Talk(SAY_ARCHANGEL);
                        DoCast(me, SPELL_DARK_ARCHANGEL_FORM);
                    }
            }

            void AttackStart(Unit* victim)
            {
                if (!me->GetCurrentSpell(CURRENT_CHANNELED_SPELL))
                    BossAI::AttackStart(victim);
            }

            void MoveInLineOfSight(Unit* victim)
            {
                if (me->IsWithinDist2d(victim, 14.0f))
                    BossAI::MoveInLineOfSight(victim);
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
                    case EVENT_ASPHYXOATE:

                        if (me->HasSpellCooldown(SPELL_ASPHYXOATE))
                        {
                            events.ScheduleEvent(EVENT_ASPHYXOATE, 500);
                            break;
                        }
                        Talk(SAY_ASPHYXIATION);
                        DoCastAOE(SPELL_ASPHYXOATE, true);
                        me->SetFacingToObject(me->GetVictim());
                        me->SetReactState(REACT_PASSIVE);
                        me->AttackStop();
                        me->resetAttackTimer();

                        events.ScheduleEvent(EVENT_STAY_OF_EXECUTION, 7000);
                        events.ScheduleEvent(EVENT_ASPHYXOATE, urand(20000, 23000));
                        break;
                    case EVENT_PAIN_AND_SUFFERING:
                        if (me->HasSpellCooldown(SPELL_PAIN_AND_SUFFERING))
                        {
                            events.ScheduleEvent(EVENT_PAIN_AND_SUFFERING, 500);
                            break;
                        }
                        DoCastRandom(SPELL_PAIN_AND_SUFFERING, 50, true, 0, 1);
                        events.ScheduleEvent(EVENT_PAIN_AND_SUFFERING, 25000);
                        break;
                    case EVENT_STAY_OF_EXECUTION:
                        Talk(SAY_STAY_OF_EXECUTION);
                        if (IsHeroic())
                            Talk(SAY_STAY_OF_EXECUTION_HEROIC); // Ja, beide auf Heroisch. eines ist ein Emote.
                        me->SetReactState(REACT_AGGRESSIVE);
                        me->resetAttackTimer();
                        DoCast(me, SPELL_STAY_OF_EXECUTION);
                        break;
                    case EVENT_WRACKING_PAIN:
                        if (me->HasSpellCooldown(SPELL_WRACKING_PAIN))
                        {
                            events.ScheduleEvent(EVENT_WRACKING_PAIN, 500);
                            break;
                        }
                        DoCast(SPELL_WRACKING_PAIN);
                        events.ScheduleEvent(EVENT_WRACKING_PAIN, urand(24000, 26000));
                        break;
                    }
                }
                DoMeleeAttackIfReady();
            }
        private:
            bool enraged;
        };

        CreatureAI* GetAI(Creature* creature) const
        {
            return new boss_baron_ashburyAI(creature);
        }
};

class spell_ashbury_archangel : public SpellScriptLoader
{
public:
    spell_ashbury_archangel() : SpellScriptLoader("spell_ashbury_archangel") { }

    class spell_ashbury_archangel_SpellScript : public SpellScript
    {
        PrepareSpellScript(spell_ashbury_archangel_SpellScript);

        void HandleScript(SpellEffIndex effIndex)
        {
            if (Unit* target = GetHitUnit())
            {
                target->CastCustomSpell(uint32(GetEffectValue()),  SPELLVALUE_BASE_POINT0, 0, target, TRIGGERED_FULL_MASK);
                target->CastSpell(target, SPELL_CALAMITY, true);
            }
        }

        void Register()
        {
            OnEffectHitTarget += SpellEffectFn(spell_ashbury_archangel_SpellScript::HandleScript, EFFECT_0, SPELL_EFFECT_APPLY_AURA);
        }
    };

    SpellScript* GetSpellScript() const
    {
        return new spell_ashbury_archangel_SpellScript();
    }
};

class spell_ashbury_asphyxoate_periodic : public SpellScriptLoader
{
    public:
        spell_ashbury_asphyxoate_periodic() : SpellScriptLoader("spell_ashbury_asphyxoate_periodic") { }

        class spell_ashbury_asphyxoate_periodic_AuraScript : public AuraScript
        {
            PrepareAuraScript(spell_ashbury_asphyxoate_periodic_AuraScript);

            void HandleEffectApply(AuraEffect const* /*aurEff*/, AuraEffectHandleModes /*mode*/)
            {
                InitialHealth = GetTarget()->GetHealth();
            }

            void HandlePeriodic(AuraEffect const* aurEff)
            {
                PreventDefaultAction();

                if (aurEff->GetTickNumber() == 1)
                    GetTarget()->CastSpell((Unit*)NULL, SPELL_ASPHYXIATE_STOP);

                int32 damage = int32( (float)InitialHealth / 6.0f + 1.0f);

                if (damage >= int32(GetTarget()->GetHealth()))
                    damage = GetTarget()->GetHealth() - 1;

                 uint32 triggerSpell = GetSpellInfo()->Effects[aurEff->GetEffIndex()].TriggerSpell;
                 GetTarget()->CastCustomSpell(triggerSpell, SPELLVALUE_BASE_POINT0, damage, GetTarget(), true, NULL, aurEff, GetCasterGUID());

            }

            int32 InitialHealth;

            void Register()
            {
                OnEffectApply += AuraEffectApplyFn(spell_ashbury_asphyxoate_periodic_AuraScript::HandleEffectApply, EFFECT_2, SPELL_AURA_PERIODIC_TRIGGER_SPELL, AURA_EFFECT_HANDLE_REAL);
                OnEffectPeriodic += AuraEffectPeriodicFn(spell_ashbury_asphyxoate_periodic_AuraScript::HandlePeriodic, EFFECT_2, SPELL_AURA_PERIODIC_TRIGGER_SPELL);
            }
        };

        AuraScript* GetAuraScript() const
        {
            return new spell_ashbury_asphyxoate_periodic_AuraScript();
        }
};

void AddSC_boss_baron_ashbury()
{
    new boss_baron_ashbury();
    new spell_ashbury_archangel();
    new spell_ashbury_asphyxoate_periodic();
}