/*
* ToDo:
* Find the correct Sound-Yells
*/

#include "ScriptPCH.h"
#include "SpellAuraEffects.h"
#include "shadowfang_keep.h"

enum Spells
{
    SPELL_CONJURE_FROST_MIXTURE            = 93505,
    SPELL_CONJURE_POISONOUS_MIXTURE        = 93697,
    SPELL_CONJURE_MYSTERY_TOXIN            = 93563,
    SPELL_TOXIC_COAGULANT_GREEN            = 93572,
    SPELL_TOXIC_COAGULANT_RED              = 93573,
    SPELL_ICE_SHARDS                       = 93527,
    SPELL_TOXIC_COAGULANT_TRIGGERED        = 93617,
    SPELL_FULLY_COAGULATED                 = 93660
};

enum Events
{
    EVENT_CONJURE_POISONOUS_MIXTURE        = 1,
    EVENT_CONJURE_FROST_MIXTURE,
    EVENT_CONJURE_MYSTERY_TOXIN,
    EVENT_ICE_SHARDS,
};

enum Texts
{
    SAY_AGGRO                              = 0,
    SAY_DEATH                              = 1,
    SAY_KILLER_1                           = 2,
    SAY_KILLER_2                           = 3,
};

class boss_lord_walden : public CreatureScript
{
    public:
        boss_lord_walden() : CreatureScript("boss_lord_walden") { }

        struct boss_lord_waldenAI : public BossAI
        {
            boss_lord_waldenAI(Creature* creature) : BossAI(creature, DATA_LORD_WALDEN_EVENT) { }

            void Reset()
            {
                _Reset();
            }

            void EnterCombat(Unit* /*who*/)
            {
                _EnterCombat();
                Talk(SAY_AGGRO);
                events.ScheduleEvent(EVENT_CONJURE_POISONOUS_MIXTURE, 5000);
                events.ScheduleEvent(EVENT_CONJURE_FROST_MIXTURE, urand(10000, 20000));
                events.ScheduleEvent(EVENT_CONJURE_MYSTERY_TOXIN,  25000);
            }

            void JustDied(Unit* /*killer*/)
            {
                Talk(SAY_DEATH);
                _JustDied();

                instance->HandleGameObject(instance->GetData64(GO_LORD_WALDEN_DOOR), true);
            }

            void KilledUnit(Unit* victim)
            {
                Talk(RAND(SAY_KILLER_1, SAY_KILLER_2));
            }

            void JustSummoned(Creature* summon)
            {
                BossAI::JustSummoned(summon);

                if (summon->GetEntry() == NPC_TOXIN_TRIGGER)
                {
                    float x, y, z;
                    me->GetPosition(x, y, z);
                    summon->SetCanFly(true);
                    summon->SetDisableGravity(true);
                    summon->NearTeleportTo(x, y, z+5.0f, 0.0f);

                    if (HealthAbovePct(35))
                    {
                        if (roll_chance_i(75))
                            summon->CastSpell(summon, SPELL_TOXIC_COAGULANT_GREEN, false);
                        else
                            summon->CastSpell(summon, SPELL_TOXIC_COAGULANT_RED, false);
                    }
                    else
                    {
                        if (roll_chance_i(75))
                            summon->CastSpell(summon, SPELL_TOXIC_COAGULANT_RED, false);
                        else
                            summon->CastSpell(summon, SPELL_TOXIC_COAGULANT_GREEN, false);
                    }
                }
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
                    case EVENT_ICE_SHARDS:
                        DoCast(SPELL_ICE_SHARDS);
                        break;
                    case EVENT_CONJURE_MYSTERY_TOXIN:
                    if (IsHeroic())
                        DoCast(SPELL_CONJURE_MYSTERY_TOXIN);
                        events.ScheduleEvent(EVENT_CONJURE_MYSTERY_TOXIN, 45000);
                        events.ScheduleEvent(EVENT_ICE_SHARDS, 10000);
                        break;
                    case EVENT_CONJURE_FROST_MIXTURE:
                    if (IsHeroic())
                        DoCastRandom(SPELL_CONJURE_FROST_MIXTURE, 50);
                        events.ScheduleEvent(EVENT_CONJURE_FROST_MIXTURE, urand(10000, 20000));
                        break;
                    case EVENT_CONJURE_POISONOUS_MIXTURE:
                        DoCastRandom(SPELL_CONJURE_POISONOUS_MIXTURE, 50.0f);
                        events.ScheduleEvent(EVENT_CONJURE_POISONOUS_MIXTURE, urand(15000, 30000));
                        break;
                    }
                }

                DoMeleeAttackIfReady();
            }
        };

        CreatureAI* GetAI(Creature* creature) const
        {
            return new boss_lord_waldenAI(creature);
        }
};

class NotMovingFilter
{
public:
    bool operator()(WorldObject* target) const
    {
        return target->ToUnit() && target->ToUnit()->isMoving() == false;
    }
};

class spell_toxic_coagulant_red : public SpellScriptLoader
{
public:
    spell_toxic_coagulant_red() : SpellScriptLoader("spell_toxic_coagulant_red") { }

    class spell_toxic_coagulant_red_SpellScript : public SpellScript
    {
        PrepareSpellScript(spell_toxic_coagulant_red_SpellScript);

        void FilterTargetsInitial(std::list<WorldObject*>& unitList)
        {
            unitList.remove_if(NotMovingFilter());
             
            targetList.clear();
            targetList.insert(targetList.end(), unitList.begin(), unitList.end());
        }

        void FilterTargetsSubsequential(std::list<WorldObject*>& unitList)
        {
            unitList.clear();
            unitList.insert(unitList.end(), targetList.begin(), targetList.end());
        }

        void Register()
        {
            OnObjectAreaTargetSelect += SpellObjectAreaTargetSelectFn(spell_toxic_coagulant_red_SpellScript::FilterTargetsInitial, EFFECT_0, TARGET_UNIT_SRC_AREA_ENEMY);
            OnObjectAreaTargetSelect += SpellObjectAreaTargetSelectFn(spell_toxic_coagulant_red_SpellScript::FilterTargetsSubsequential, EFFECT_1, TARGET_UNIT_SRC_AREA_ENEMY);
            OnObjectAreaTargetSelect += SpellObjectAreaTargetSelectFn(spell_toxic_coagulant_red_SpellScript::FilterTargetsSubsequential, EFFECT_2, TARGET_UNIT_SRC_AREA_ENEMY);
        }
        std::list<WorldObject*> targetList;
    };

    SpellScript* GetSpellScript() const
    {
        return new spell_toxic_coagulant_red_SpellScript();
    }
};

class spell_toxic_coagulant_green : public SpellScriptLoader
{
public:
    spell_toxic_coagulant_green() : SpellScriptLoader("spell_toxic_coagulant_green") { }

    class spell_toxic_coagulant_green_AuraScript : public AuraScript
    {
        PrepareAuraScript(spell_toxic_coagulant_green_AuraScript)

        void HandleEffectPeriodic(AuraEffect const* /*aurEff*/)
        {
            Unit* target = GetTarget();
            if (!target)
                return;

            if (Aura* aura = GetAura())
                if (aura->GetStackAmount() == 3)
                {
                    target->CastSpell(target, SPELL_FULLY_COAGULATED);
                    aura->Remove(AURA_REMOVE_BY_DEFAULT);
                }
        }

        void Register()
        {
            OnEffectPeriodic += AuraEffectPeriodicFn(spell_toxic_coagulant_green_AuraScript::HandleEffectPeriodic, EFFECT_1, SPELL_AURA_PERIODIC_DAMAGE);
        }
    };

    AuraScript* GetAuraScript() const
    {
        return new spell_toxic_coagulant_green_AuraScript();
    }
};

void AddSC_boss_lord_walden()
{
    new boss_lord_walden();
    new spell_toxic_coagulant_red();
    new spell_toxic_coagulant_green();
}