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

#include "ObjectMgr.h"
#include "ScriptMgr.h"
#include "ScriptedCreature.h"
#include "Spell.h"
#include "zulgurub.h"

/*
 * ToDo: Still bugged melee attack visual after camouflage
 * Wee need Retail infos for the event: EVENT_RAGE in which seconds he stack this buff
 * Is SAY_BERSERK for EVENT_RAGE?
*/

/*
More Sounds during the Fight with Kilnara, but idk where to put them all in (exctracted from DBCs)

24279: VO_ZG2_KILNARA_LAUGH_02
24280: VO_ZG2_KILNARA_LAUGH01
*/

enum Yells
{
    SAY_AGGRO                      = 0,
    SAY_WAVE_OF_AGONY              = 1,
    SAY_TRANSFROM_1                = 2,
    SAY_TRANSFORM_2                = 3,
    SAY_TRANSFROM_EMOTE            = 4,
    SAY_PLAYER_KILL_BET_1          = 5,
    SAY_PLAYER_KILL_BET_2          = 6,
    SAY_BERSERK                    = 7, // where is berserk called? enrage or Rage of the Ancients
    SAY_DEATH                      = 8,

    // Transformed
    SAY_BERSERK_BET                = 9,
    SAY_PLAYER_KILL_1              = 10,
    SAY_PLAYER_KILL_2              = 11,
};

enum Spells
{
    // Phase 1
    SPELL_LASH_OF_ANGUISH               = 96423,
    SPELL_SHADOW_BOLT                   = 96516,
    SPELL_TEARS_OF_BLOOD                = 96438,
    SPELL_WAIL_OF_SORROW                = 96909,
    SPELL_KILNARA_HOVER                 = 97428,
    SPELL_WAVE_OF_AGONY_TARGET_SELECTOR = 98269, // Cast on all players, selects the victim of SPELL_WAVE_OF_AGONY_TRIGGER_SUMMON
    SPELL_WAVE_OF_AGONY_SUMMON          = 96457, // Lets her channel and summon the wave
    SPELL_WAVE_OF_AGONY_DAMAGE          = 96460, // Deals damage in a 70 yards cone.
    SPELL_WAVE_OF_AGONY_DAMAGE_TRIGGER  = 96542,
    SPELL_WAVE_OF_AGONY_TRIGGER_SUMMON  = 96461, // Cast by the target player, she casts SPELL_WAVE_OF_AGONY_DAMAGE on it.
    SPELL_WAVE_OF_AGONY_TRACK_TARGET    = 96465,


    // Phase 2
    SPELL_CAMOUFLAGE               = 96594,
    SPELL_RAGE_OF_THE_ANCIENTS     = 96531,
    SPELL_RAVAGE                   = 96592,
    SPELL_TRANSFORM                = 96559,
    SPELL_TRANSFORM_BUFF           = 96559,
    SPELL_CAVE_IN                  = 97380,
    SPELL_CAVE_IN_VISUAL           = 81309,
    SPELL_PRIMAL_AWAKENING         = 96530,

    // Trash
    SPELL_BLOOD_FRENZY             = 98239,
    SPELL_GAPING_WOUND             = 97357
};

enum Misc
{
    EQUIP_ID_STAFF                 = 43219,
    NPC_WAVE_DUMMY                 = 52160,
    NPC_WAVE_VISUAL                = 52147,
    
};

enum Actions
{
    ACTION_RESUME_ATTACK            = 1,
};

enum SoundIds
{
    SOUND_TRANSFORM                = 24056,
    SOUND_CRY                      = 24272
};

enum Events
{
    EVENT_CAMOUFLAGE               = 1,
    EVENT_LASH_OF_ANGUISH,
    EVENT_RAGE_OF_THE_ANCIENTS,
    EVENT_RAVAGE,
    EVENT_SHADOW_BOLT,
    EVENT_TEARS_OF_BLOOD,
    EVENT_WAIL_OF_SORROW,
    EVENT_TRANSFORM,
    EVENT_CALL_KITTEN,
    EVENT_RAGE,
    EVENT_WAVE_OF_AGONY,

    // Trash
    EVENT_BLOOD_FRENZY,
    EVENT_GAPING_WOUND
};

class DelayedAttackStartEvent : public BasicEvent
{
public:
    DelayedAttackStartEvent(Creature* owner) : _owner(owner) { }
    
    bool Execute(uint64 /*e_time*/, uint32 /*p_time*/)
    {
        _owner->SetReactState(REACT_DEFENSIVE);
        _owner->AI()->AttackStart(_owner->SelectVictim());
        return true;
    }
    
private:
    Creature* _owner;
};

class boss_kilnara : public CreatureScript
{
    public:
        boss_kilnara() : CreatureScript("boss_kilnara") { }

        struct boss_kilnaraAI : public BossAI
        {
            boss_kilnaraAI(Creature* creature) : BossAI(creature, DATA_KILNARA) { }

            void Reset()
            {
                _Reset();
                me->AddAura(SPELL_KILNARA_HOVER, me);
                me->SetReactState(REACT_DEFENSIVE);
                transform = false;
                camouflage = false;
                CastingWave = false;
                me->RemoveAura(SPELL_TRANSFORM_BUFF);
                SetEquipmentSlots(true, EQUIP_ID_STAFF);
                if (GameObject* forcefield = me->FindNearestGameObject(GO_FORCEFIELD, 150.0f))
                    me->RemoveGameObject(forcefield, true);
            }

            void EnterCombat(Unit* /*who*/)
            {
                _EnterCombat();
                Talk(SAY_AGGRO);
                me->SummonGameObject(GO_FORCEFIELD, -11494.5f, -1613.67f, 47.3295f, 4.71461f, 0, 0, 0, 0, 0);
                me->RemoveAura(SPELL_KILNARA_HOVER);
                events.ScheduleEvent(EVENT_TEARS_OF_BLOOD, 12000);
                events.ScheduleEvent(EVENT_WAIL_OF_SORROW, 20000);
                events.ScheduleEvent(EVENT_SHADOW_BOLT, 500);
                events.ScheduleEvent(EVENT_WAVE_OF_AGONY, 23000);
            }

            void CaveIn()
            {
                std::list<Creature*> templist;
                float x, y, z;
                me->GetPosition(x, y, z);

                {
                    CellCoord pair(Trinity::ComputeCellCoord(x, y));
                    Cell cell(pair);
                    cell.SetNoCreate();

                    Trinity::AllCreaturesOfEntryInRange check(me, NPC_CAVE_IN_TRIGGER, 150);
                    Trinity::CreatureListSearcher<Trinity::AllCreaturesOfEntryInRange> searcher(me, templist, check);
                    TypeContainerVisitor<Trinity::CreatureListSearcher<Trinity::AllCreaturesOfEntryInRange>, GridTypeMapContainer> cSearcher(searcher);
                    cell.Visit(pair, cSearcher, *me->GetMap(), *me, me->GetGridActivationRange());
                }

                if (templist.empty())
                return;

                for (std::list<Creature*>::const_iterator i = templist.begin(); i != templist.end(); ++i)
                    (*i)->CastSpell(*i, SPELL_CAVE_IN_VISUAL, false);
            }

            void JustDied(Unit* /*killer*/)
            {
                _JustDied();
                me->RemoveAura(SPELL_KILNARA_HOVER);
                Talk(SAY_DEATH);
                if (GameObject* forcefield = me->FindNearestGameObject(GO_FORCEFIELD, 150.0f))
                    me->RemoveGameObject(forcefield, true);
            }
            
            void JustSummoned(Creature* summon)
            {
                summons.Summon(summon);
                
                switch(summon->GetEntry())
                {
                    case NPC_WAVE_DUMMY:
                        summon->SetControlled(true, UNIT_STATE_STUNNED);
                        DoCast(summon, SPELL_WAVE_OF_AGONY_SUMMON);
                        me->SetReactState(REACT_PASSIVE);
                        me->AttackStop();
                        me->StopMoving();
                        
                        if (!me->IsWithinLOSInMap(summon))
                        {
                            Position pos;
                            pos.Relocate(me);
                            me->MovePositionToFirstCollision(pos, 2.0f, me->GetAngle(summon) - me->GetOrientation());
                            summon->NearTeleportTo(pos);
                        }
                        break;
                    case NPC_WAVE_VISUAL:
                        summon->SetControlled(true, UNIT_STATE_STUNNED);
                        summon->DespawnOrUnsummon(3500);
                        me->SetFacingToObject(summon);
                        break;
                }
            }

            void KilledUnit(Unit* victim)
            {
                if (victim->GetTypeId() == TYPEID_PLAYER && HealthAbovePct(50))
                    Talk(RAND(SAY_PLAYER_KILL_1, SAY_PLAYER_KILL_2));

                if (victim->GetTypeId() == TYPEID_PLAYER && me->HasAura(SPELL_TRANSFORM_BUFF))
                    Talk(RAND(SAY_PLAYER_KILL_BET_1, SAY_PLAYER_KILL_BET_2));
            }
            
            void DoAction(int32 const ActionID)
            {
                if (ActionID == ACTION_RESUME_ATTACK)
                {
                    me->m_Events.AddEvent(new DelayedAttackStartEvent(me), me->m_Events.CalculateTime(1000));
                    events.DelayEvents(1000);
                    CastingWave = false;
                }
            }

            void DamageTaken(Unit* /*attacker*/, uint32& /*damage*/)
            {
                if(!transform && !CastingWave && me->GetReactState() != REACT_PASSIVE && !me->IsNonMeleeSpellCasted(false) && HealthBelowPct(50))
                {
                    transform = true;
                    Talk(RAND(SAY_TRANSFROM_1, SAY_TRANSFORM_2));
                    Talk(SAY_TRANSFROM_EMOTE);
                    me->AttackStop();
                    me->SetUInt32Value(UNIT_NPC_EMOTESTATE, EMOTE_STATE_SPELL_CHANNEL_OMNI);
                    me->SetReactState(REACT_PASSIVE);
                    DoCast(me, SPELL_CAVE_IN);
                    CaveIn();
                    events.CancelEvent(EVENT_TEARS_OF_BLOOD);
                    events.CancelEvent(EVENT_WAIL_OF_SORROW);
                    events.CancelEvent(EVENT_SHADOW_BOLT);
                    events.CancelEvent(EVENT_LASH_OF_ANGUISH);
                    events.CancelEvent(EVENT_WAVE_OF_AGONY);
                    events.RescheduleEvent(EVENT_TRANSFORM, 6500);
                }
                if(!camouflage && !me->IsNonMeleeSpellCasted(false) && HealthBelowPct(15))
                {
                    camouflage = true;
                    DoCast(me, SPELL_CAMOUFLAGE);
                    events.ScheduleEvent(EVENT_CAMOUFLAGE, 5000);
                }
            }
            
            void AttackStart(Unit* who)
            {
                if (me->GetCurrentSpell(CURRENT_CHANNELED_SPELL))
                    return;
                
                BossAI::AttackStart(who);
            }
            void UpdateAI(uint32 const diff)
            {
                if(!UpdateVictim())
                    return;

                events.Update(diff);

                if(me->HasUnitState(UNIT_STATE_CASTING) || CastingWave)
                    return;

                if(uint32 eventId = events.ExecuteEvent())
                {
                    switch(eventId)
                    {
                        case EVENT_TEARS_OF_BLOOD:
                            if (me->HasSpellCooldown(SPELL_TEARS_OF_BLOOD))
                                events.ScheduleEvent(EVENT_TEARS_OF_BLOOD, 1);
                            else
                            {
                                me->CastSpell(me, SPELL_TEARS_OF_BLOOD, TRIGGERED_IGNORE_SPELL_AND_CATEGORY_CD);
                                me->AttackStop();
                                me->StopMoving();
                                DoPlaySoundToSet(me, SOUND_CRY);
                                events.ScheduleEvent(EVENT_TEARS_OF_BLOOD, 38*IN_MILLISECONDS);
                            }
                            break;
                        case EVENT_WAIL_OF_SORROW:
                            if (me->HasSpellCooldown(SPELL_WAIL_OF_SORROW))
                                events.ScheduleEvent(EVENT_WAIL_OF_SORROW, 1);
                            else
                            {
                                DoCastAOE(SPELL_WAIL_OF_SORROW);
                                events.ScheduleEvent(EVENT_WAIL_OF_SORROW,  45*IN_MILLISECONDS);
                            }
                            break;
                        case EVENT_SHADOW_BOLT:
                            DoCastVictim(SPELL_SHADOW_BOLT);
                            events.ScheduleEvent(EVENT_SHADOW_BOLT, 3*IN_MILLISECONDS);
                            break;
                        case EVENT_LASH_OF_ANGUISH:
                            DoCastRandom(SPELL_LASH_OF_ANGUISH, 50.0f);
                            events.ScheduleEvent(EVENT_LASH_OF_ANGUISH, urand(25*IN_MILLISECONDS, 30*IN_MILLISECONDS));
                            break;
                        case EVENT_WAVE_OF_AGONY:
                            if (me->HasSpellCooldown(SPELL_WAVE_OF_AGONY_TARGET_SELECTOR))
                                events.ScheduleEvent(EVENT_WAVE_OF_AGONY, 1);
                            else
                            {
                                Talk(SAY_WAVE_OF_AGONY);
                                CastingWave = true;
                                DoCastAOE(SPELL_WAVE_OF_AGONY_TARGET_SELECTOR);
                                events.ScheduleEvent(EVENT_WAVE_OF_AGONY, urand(48*IN_MILLISECONDS, 50*IN_MILLISECONDS));
                            }
                            break;
                        case EVENT_TRANSFORM:
                            DoCast(me, SPELL_TRANSFORM);
                            DoPlaySoundToSet(me, SOUND_TRANSFORM);
                            SetEquipmentSlots(false, 0, 0,0);
                            events.ScheduleEvent(EVENT_CALL_KITTEN, 2000);
                            break;
                        case EVENT_CALL_KITTEN:
                            me->SetReactState(REACT_AGGRESSIVE);
                            me->SetUInt32Value(UNIT_NPC_EMOTESTATE, EMOTE_ONESHOT_NONE);
                            DoCastAOE(SPELL_PRIMAL_AWAKENING);
                            events.ScheduleEvent(EVENT_RAGE, 2000);
                            events.ScheduleEvent(EVENT_RAVAGE, 10000);
                            break;
                        case EVENT_RAGE:
                            Talk(SAY_BERSERK_BET);
                            DoCast(me, SPELL_RAGE_OF_THE_ANCIENTS);
                            events.ScheduleEvent(EVENT_RAGE, urand(10*IN_MILLISECONDS, 15*IN_MILLISECONDS));
                            break;
                        case EVENT_RAVAGE:
                            DoCastVictim(SPELL_RAVAGE);
                            events.ScheduleEvent(EVENT_RAVAGE, urand(15*IN_MILLISECONDS, 18*IN_MILLISECONDS));
                            break;
                        case EVENT_CAMOUFLAGE:
                            if (Unit* target = SelectTarget(SELECT_TARGET_RANDOM, 0, NonTankTargetSelector(me))) //Guide says she only attacks non Tank targets
                                DoCast(target, SPELL_RAVAGE);
                            me->RemoveAura(SPELL_CAMOUFLAGE);
                            break;
                        default:
                            break;
                    }
                }
                DoMeleeAttackIfReady();
            }
    private:
        bool transform;
        bool camouflage;
        bool CastingWave;
        };

        CreatureAI* GetAI(Creature* creature) const
        {
            return new boss_kilnaraAI(creature);
        }
};

class mob_pride_of_bethekk : public CreatureScript
{
public:
    mob_pride_of_bethekk() : CreatureScript("mob_pride_of_bethekk") { }

    struct mob_pride_of_bethekkAI : public ScriptedAI
    {
        mob_pride_of_bethekkAI(Creature* creature) : ScriptedAI(creature) { }

        void Reset()
        {
            events.Reset();
            me->SetReactState(REACT_DEFENSIVE);
        }

        void EnterCombat(Unit* /*who*/)
        {
            events.ScheduleEvent(EVENT_BLOOD_FRENZY, 20000);
            events.ScheduleEvent(EVENT_GAPING_WOUND, 5000);

            if (Creature* kilnara = me->FindNearestCreature(NPC_KILNARA, 50.0f, true))
            {
                if (Unit* player = me->FindNearestPlayer(100.0f, true))
                    kilnara->AI()->AttackStart(player);
            }
        }

        void SpellHit(Unit* caster, SpellInfo const* spell)
        {
            if (spell->Id == SPELL_PRIMAL_AWAKENING)
            {
                if (Unit* player = me->FindNearestPlayer(100.0f, true))
                {
                    me->SetReactState(REACT_AGGRESSIVE);
                    me->Attack(player, false);
                }
            }
        }

        void UpdateAI(const uint32 diff)
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
                case EVENT_BLOOD_FRENZY:
                    DoCast(me, SPELL_BLOOD_FRENZY);
                    events.ScheduleEvent(EVENT_BLOOD_FRENZY, urand(70*IN_MILLISECONDS, 75*IN_MILLISECONDS));
                    break;
                case EVENT_GAPING_WOUND:
                    DoCastVictim(SPELL_GAPING_WOUND);
                    events.ScheduleEvent(EVENT_GAPING_WOUND, urand(10*IN_MILLISECONDS, 15*IN_MILLISECONDS));
                    break;
                }
            }
            DoMeleeAttackIfReady();
        }
    private:
        EventMap events;
    };

    CreatureAI* GetAI(Creature* creature) const
    {
        return new mob_pride_of_bethekkAI (creature);
    }
};

class spell_kilnara_tears_of_blood_trigger : public SpellScriptLoader
{
public:
    spell_kilnara_tears_of_blood_trigger() : SpellScriptLoader("spell_kilnara_tears_of_blood_trigger") { }
    
    class spell_kilnara_tears_of_blood_trigger_SpellScript : public SpellScript
    {
        PrepareSpellScript(spell_kilnara_tears_of_blood_trigger_SpellScript);
        
        void HandleEffectDummy(SpellEffIndex effIndex)
        {
            PreventHitDefaultEffect(effIndex);
            GetCaster()->CastSpell(GetCaster(), GetSpellInfo()->Effects[effIndex].TriggerSpell);
            
        }
        
        void Register()
        {
            OnEffectHitTarget += SpellEffectFn(spell_kilnara_tears_of_blood_trigger_SpellScript::HandleEffectDummy, EFFECT_1, SPELL_EFFECT_TRIGGER_SPELL);
        }
    };
    
    SpellScript* GetSpellScript() const
    {
        return new spell_kilnara_tears_of_blood_trigger_SpellScript();
    }
};

class spell_kilnara_wave_selector : public SpellScriptLoader
{
public:
    spell_kilnara_wave_selector() : SpellScriptLoader("spell_kilnara_wave_selector") { }
    
    class spell_kilnara_wave_selector_SpellScript : public SpellScript
    {
        PrepareSpellScript(spell_kilnara_wave_selector_SpellScript);
        
        void SelectTarget(std::list<WorldObject*>& tarList)
        {
            if (tarList.empty())
                return;
            
            Trinity::Containers::RandomResizeList(tarList, 1);
            
            if (Unit* target = tarList.front()->ToUnit())
                target->CastSpell(target, SPELL_WAVE_OF_AGONY_TRIGGER_SUMMON, true, NULL, NULL,  GetCaster()->GetGUID());
        }
        
        void Register()
        {
            OnObjectAreaTargetSelect += SpellObjectAreaTargetSelectFn(spell_kilnara_wave_selector_SpellScript::SelectTarget, EFFECT_0, TARGET_UNIT_SRC_AREA_ENEMY);
        }
    };
    
    SpellScript* GetSpellScript() const
    {
        return new spell_kilnara_wave_selector_SpellScript();
    }
};

class DistanceCheck
{
public:
    explicit DistanceCheck(Unit* _caster) : caster(_caster) { }
    
    bool operator() (WorldObject* unit) const
    {
        return unit->isInBack(caster);
    }
    
    Unit* caster;
};

class spell_kilnara_wave_damage : public SpellScriptLoader
{
public:
    spell_kilnara_wave_damage() : SpellScriptLoader("spell_kilnara_wave_damage") { }
    
    class spell_kilnara_wave_damage_SpellScript : public SpellScript
    {
        PrepareSpellScript(spell_kilnara_wave_damage_SpellScript);
        
        void FilterTargetsInitial(std::list<WorldObject*>& unitList)
        {
            unitList.remove_if(DistanceCheck(GetCaster()));            
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
            OnObjectAreaTargetSelect += SpellObjectAreaTargetSelectFn(spell_kilnara_wave_damage_SpellScript::FilterTargetsInitial, EFFECT_0, TARGET_UNIT_CONE_ENEMY_104);
            OnObjectAreaTargetSelect += SpellObjectAreaTargetSelectFn(spell_kilnara_wave_damage_SpellScript::FilterTargetsSubsequential, EFFECT_1, TARGET_UNIT_CONE_ENEMY_104);
        }
        std::list<WorldObject*> targetList;
    };
    
    SpellScript* GetSpellScript() const
    {
        return new spell_kilnara_wave_damage_SpellScript();
    }
};

class spell_kilnara_wave_summon : public SpellScriptLoader
{
public:
    spell_kilnara_wave_summon() : SpellScriptLoader("spell_kilnara_wave_summon") { }
    
    class spell_kilnara_wave_summon_AuraScript : public AuraScript
    {
        PrepareAuraScript(spell_kilnara_wave_summon_AuraScript);
        
        void HandlePeriodic(AuraEffect const* /*aurEff*/)
        {
            PreventDefaultAction();
            if (Creature* target = GetOwner()->ToCreature())
            {
                if (Creature* caster = GetCaster()->ToCreature())
                {
                    if (caster->IsAIEnabled)
                    {
                        caster->SetFacingToObject(target);
                        caster->CastSpell((Unit*)NULL, SPELL_WAVE_OF_AGONY_TRACK_TARGET, true);
                        caster->CastSpell(target, SPELL_WAVE_OF_AGONY_DAMAGE_TRIGGER, true);
                    }
                }
            }
            
        }
        
        void Register()
        {
            OnEffectPeriodic += AuraEffectPeriodicFn(spell_kilnara_wave_summon_AuraScript::HandlePeriodic, EFFECT_0, SPELL_AURA_PERIODIC_DUMMY);
        }
    };
    
    AuraScript* GetAuraScript() const
    {
        return new spell_kilnara_wave_summon_AuraScript();
    }
};

class spell_kilnara_wave_launch : public SpellScriptLoader
{
public:
    spell_kilnara_wave_launch() : SpellScriptLoader("spell_kilnara_wave_launch") { }
    
    class spell_kilnara_wave_launch_AuraScript : public AuraScript
    {
        PrepareAuraScript(spell_kilnara_wave_launch_AuraScript);
        
        void HandlePeriodic(AuraEffect const* /*aurEff*/)
        {
            PreventDefaultAction();
            if (Creature* caster = GetCaster()->ToCreature())
            {
                if (caster->IsAIEnabled)
                    caster->AI()->DoAction(ACTION_RESUME_ATTACK);
            }
            
        }
        
        void Register()
        {
            OnEffectPeriodic += AuraEffectPeriodicFn(spell_kilnara_wave_launch_AuraScript::HandlePeriodic, EFFECT_0, SPELL_AURA_PERIODIC_DUMMY);
        }
    };
    
    AuraScript* GetAuraScript() const
    {
        return new spell_kilnara_wave_launch_AuraScript();
    }
};

void AddSC_boss_kilnara()
{
    new boss_kilnara();
    new mob_pride_of_bethekk();
    new spell_kilnara_tears_of_blood_trigger();
    new spell_kilnara_wave_selector();
    new spell_kilnara_wave_launch();
    new spell_kilnara_wave_damage();
    new spell_kilnara_wave_summon();
}
