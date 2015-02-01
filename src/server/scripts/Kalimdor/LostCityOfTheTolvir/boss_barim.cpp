/*
 * Copyright (C) 2008-2013 TrinityCore <http://www.trinitycore.org/>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA
 */

#include "ScriptPCH.h"
#include "the_lost_city_of_tol_vir.h"


enum Texts
{
    SAY_AGGRO        = 0,
    SAY_REPETANCE    = 1,
    SAY_REPETANCE_2  = 2,
    SAY_DEATH        = 3,
    SAY_KILL         = 4
};

enum Spells
{
    // High Prophet Barim
    SPELL_FIFTY_LASHINGS                          = 82506,
    SPELL_FIFTY_LASHINGS_PROC                     = 82522,
    SPELL_PLAGUE_OF_AGES                          = 82622,
    SPELL_REPENTANCE                              = 82320,
    SPELL_REPENTANCE_PULL                         = 82430,
    SPELL_REPENTANCE_KNEEL                        = 81947,
    SPELL_REPENTANCE_KNOCKBACK                    = 82012,
    SPELL_REPENTANCE_PHASE_EFFECT                 = 82139,
    SPELL_HEAVENS_FURY                            = 81939,
    SPELL_HARBINGER_SUMMON                        = 82203,
    SPELL_BLAZE_SUMMON                            = 91180,
    // Harbinger of Darkness
    SPELL_SOUL_SEVER                              = 82255,
    SPELL_WAIL_OF_DARKNESS_AOE                    = 82425,
    SPELL_WAIL_OF_DARKNESS_RANDOM                 = 82533,
    // Heaven's Fury
    SPELL_HEAVENS_FURY_VISUAL                     = 81940,
    SPELL_HEAVENS_FURY_AURA                       = 81941,
    // Blaze of Heavens
    SPELL_BLAZE_OF_THE_HEAVENS_AURA               = 95248,
    SPELL_SUMMON_BURNING_SOUL                     = 91189,

    SPELL_BLAZE_OF_THE_HEAVENS_SUMMON_FIRE_AURA   = 91185,
    SPELL_BLAZE_OF_THE_HEAVENS_SUMMON_EGG         = 91181,
    SPELL_BLAZE_OF_THE_HEAVENS_EGG_VISUAL         = 91179,
    SPELL_SURGE                                   = 91212,
    SPELL_BURNING_SOUL                            = 91195,
    // Soul Fragment
    SPELL_MERGED_SOULS                            = 82263,
    SPELL_SOUL_SEVER_CLONE                        = 82219,
    SPELL_REPENTANCE_FRAGMENT                     = 69893,
    SPELL_SOUL_FRAGMENT                           = 82224,
    SPELL_BURNING_SOUL_AURA                       = 91277,
    // Wail of darkness
    SPELL_WAIL_OF_DARKNESS                        = 82195,
    SPELL_WAIL_OF_DARKNESS_AURA                   = 82197,
    SPELL_WAIL_OF_DARKNESS_SUMMON                 = 82203
};

enum Phases
{
    PHASE_NORMAL     = 1,
    PHASE_50_PRECENT = 2
};

enum Events
{
    EVENT_PLAGUE_OF_AGES       = 1,
    EVENT_FIFTY_LASHINGS       = 2,
    EVENT_HEAVENS_FURY         = 3,
    EVENT_REPENTANCE           = 4,
    EVENT_REPENTANCE_PULL      = 5,
    EVENT_REPENTANCE_KNEEL     = 6,
    EVENT_REPENTANCE_KNOCKBACK = 7,
    EVENT_WAIL_OF_DARKNESS     = 8,
    EVENT_HEAVENS_EGG_REG      = 9,
    EVENT_HEAVENS_EGG_CHECK    = 10,
    EVENT_FENIX_SUMMON         = 11,
    EVENT_BURNING_SOULS,
    EVENT_UNEGG,
};

enum Actions
{
    ACTION_BURNING_SOUL,
    ACTION_UNEGG,
};

#define ACHIEVEMENT_KILL_IT_WITH_FIRE 5290

class EggEvent : public BasicEvent
{
public:
    EggEvent(Unit* caster) : _caster(caster) {}

    bool Execute(uint64 execTime, uint32 /*diff*/)
    {
        _caster->CastSpell(_caster, SPELL_BLAZE_SUMMON);
        _caster->ToCreature()->DespawnOrUnsummon();
        return false;
    }

private:
    Unit* _caster;
};

class WailofDarknessEvent : public BasicEvent
{
public:
    WailofDarknessEvent(Unit* caster) : _caster(caster) {}

    bool Execute(uint64 execTime, uint32 /*diff*/)
    {
        _caster->CastSpell(_caster, SPELL_WAIL_OF_DARKNESS_SUMMON);
        _caster->ToCreature()->DespawnOrUnsummon();
        return false;
    }

private:
    Unit* _caster;
};

class fiftyLashingsProcDelay : public BasicEvent
{
public:
    fiftyLashingsProcDelay(Unit* caster) : _caster(caster) {}

    bool Execute(uint64 execTime, uint32 /*diff*/)
    {
        if (_caster->GetVictim())
            _caster->CastSpell(_caster->GetVictim(), SPELL_FIFTY_LASHINGS_PROC, true);
        return false;
    }

private:
    Unit* _caster;
};

class boss_high_prophet_barim : public CreatureScript
{
public:
    boss_high_prophet_barim() : CreatureScript("boss_high_prophet_barim") { }

    struct boss_high_prophet_barimAI : public ScriptedAI
    {
        boss_high_prophet_barimAI(Creature* creature) : ScriptedAI(creature), summons(creature)
        {
            instance = me->GetInstanceScript();
        }

        void Reset()
        {
            me->SetReactState(REACT_AGGRESSIVE);
            instance->SendEncounterUnit(ENCOUNTER_FRAME_DISENGAGE, me);
            instance->SetData(DATA_HIGH_PROPHET_BARIM_EVENT, NOT_STARTED);
            instance->DoRemoveAurasDueToSpellOnPlayers(SPELL_REPENTANCE_PHASE_EFFECT);
            me->RemoveAurasDueToSpell(SPELL_REPENTANCE_PHASE_EFFECT);
            summons.DespawnAll();
            events.Reset();
            soulCounter = 0;
            DespawnCreatures(48904);
            DespawnCreatures(48907);
            DespawnCreatures(NPC_BLAZE_OF_HEAVENS);
            DespawnCreatures(NPC_HARBINGER_OF_DARKNESS);
        }

        void EnterCombat(Unit* /*who*/)
        {
            instance->SetData(DATA_HIGH_PROPHET_BARIM_EVENT, IN_PROGRESS);
            first = true;
            events.SetPhase(PHASE_NORMAL);
            events.ScheduleEvent(EVENT_PLAGUE_OF_AGES, 7000, 0, PHASE_NORMAL);
            events.ScheduleEvent(EVENT_FIFTY_LASHINGS, 10000, 0, PHASE_NORMAL);
            events.ScheduleEvent(EVENT_HEAVENS_FURY, 12000, 0, PHASE_NORMAL);
            if (IsHeroic())
                events.ScheduleEvent(EVENT_FENIX_SUMMON, 9000, 0, PHASE_NORMAL);

            Talk(SAY_AGGRO);
            instance->SendEncounterUnit(ENCOUNTER_FRAME_ENGAGE, me);
        }

        void DespawnCreatures(uint32 entry)
        {
            std::list<Creature*> creatures;
            GetCreatureListWithEntryInGrid(creatures, me, entry, 1000.0f);
            if (creatures.empty())
                return;
            for (std::list<Creature*>::iterator iter = creatures.begin(); iter != creatures.end(); ++iter)
                (*iter)->DespawnOrUnsummon();
        }

        void KilledUnit(Unit* who)
        {
            if (who->GetTypeId() == TYPEID_PLAYER)
                Talk(SAY_KILL);
        }

        void JustDied(Unit* /*Killer*/)
        {
            Talk(SAY_DEATH);
            instance->SendEncounterUnit(ENCOUNTER_FRAME_DISENGAGE, me);
            instance->SetData(DATA_HIGH_PROPHET_BARIM_EVENT, DONE);
            summons.DespawnAll();

            if (soulCounter >= 3 && IsHeroic())
                instance->DoCompleteAchievement(ACHIEVEMENT_KILL_IT_WITH_FIRE);
            DespawnCreatures(48904);
            DespawnCreatures(48907);
            DespawnCreatures(NPC_BLAZE_OF_HEAVENS);
            DespawnCreatures(NPC_HARBINGER_OF_DARKNESS);
        }

        void JustSummoned(Creature* summon)
        {
            summons.Summon(summon);
            switch (summon->GetEntry())
            {
                case 48904: // blaze egg
                {
                    summon->CastSpell(summon, SPELL_BLAZE_OF_THE_HEAVENS_EGG_VISUAL);
                    summon->m_Events.AddEvent(new EggEvent(summon), summon->m_Events.CalculateTime(4000));
                    break;
                }
                case NPC_BLAZE_OF_HEAVENS:
                case NPC_HARBINGER_OF_DARKNESS:
                    if (Unit* target = SelectTarget(SELECT_TARGET_RANDOM,0))
                        summon->AI()->AttackStart(target);
                    break;
                case NPC_HEAVENS_FURY:
                    summon->SetFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_NOT_SELECTABLE | UNIT_FLAG_NON_ATTACKABLE | UNIT_FLAG_DISABLE_MOVE);
                    summon->CastSpell(summon, SPELL_HEAVENS_FURY_VISUAL, true);
                    summon->CastSpell(summon, SPELL_HEAVENS_FURY_AURA, true);
                    break;
                case NPC_VEIL_OF_TWILIGHT:
                    summon->CastSpell(summon, SPELL_WAIL_OF_DARKNESS_AURA);
                    summon->m_Events.AddEvent(new WailofDarknessEvent(summon), summon->m_Events.CalculateTime(2000));
                    break;
            }
        }

        void DoAction(int32 const action)
        {
            switch (action)
            {
                case PHASE_NORMAL:
                    me->SetReactState(REACT_AGGRESSIVE);
                    me->RemoveAurasDueToSpell(SPELL_REPENTANCE);
                    instance->SetData(DATA_HIGH_PROPHET_BARIM_EVENT, IN_PROGRESS);
                    instance->DoRemoveAurasDueToSpellOnPlayers(SPELL_REPENTANCE_PHASE_EFFECT);
                    DoCastAOE(SPELL_REPENTANCE_PULL);
                    events.SetPhase(PHASE_NORMAL);
                    events.ScheduleEvent(EVENT_PLAGUE_OF_AGES, 7000, 0, PHASE_NORMAL);
                    events.ScheduleEvent(EVENT_FIFTY_LASHINGS, 10000, 0, PHASE_NORMAL);
                    events.ScheduleEvent(EVENT_HEAVENS_FURY, 12000, 0, PHASE_NORMAL);
                    break;
                case ACTION_BURNING_SOUL:
                    soulCounter++;
                    break;
                default:
                    break;
            }
        }

        void DamageTaken(Unit* caster, uint32& damage)
        {
            if (!HealthAbovePct(50) && first)
            {
                first = false;
                instance->SetData(DATA_HIGH_PROPHET_BARIM_EVENT, SPECIAL);
                events.SetPhase(PHASE_50_PRECENT);
                events.ScheduleEvent(EVENT_REPENTANCE, 100, 0, PHASE_50_PRECENT);
                me->SetReactState(REACT_PASSIVE);
                me->AttackStop();
            }
        }

        void UpdateAI(const uint32 diff)
        {
            if (!UpdateVictim())
                return;

            events.Update(diff);

            while (uint32 eventId = events.ExecuteEvent())
            {
                switch (eventId)
                {
                    case EVENT_FENIX_SUMMON:
                        DoCastRandom(SPELL_BLAZE_OF_THE_HEAVENS_SUMMON_EGG, 100.0f, true);
                        break;
                    case EVENT_PLAGUE_OF_AGES:
                        if (Unit* target = SelectTarget(SELECT_TARGET_RANDOM, 0))
                            DoCast(target, SPELL_PLAGUE_OF_AGES, true);
                        events.ScheduleEvent(EVENT_PLAGUE_OF_AGES, urand(12000, 16000), 0, PHASE_NORMAL);
                        break;
                    case EVENT_FIFTY_LASHINGS:
                        DoCast(me, SPELL_FIFTY_LASHINGS);
                        events.ScheduleEvent(EVENT_FIFTY_LASHINGS, 45000, 0, PHASE_NORMAL);
                        break;
                    case EVENT_HEAVENS_FURY:
                        if (Unit* target = SelectTarget(SELECT_TARGET_RANDOM, 0))
                            DoCast(target, SPELL_HEAVENS_FURY, true);
                        events.ScheduleEvent(EVENT_HEAVENS_FURY, urand(22000, 35000), 0, PHASE_NORMAL);
                        break;
                    case EVENT_REPENTANCE:
                        Talk(SAY_REPETANCE);
                        me->StopMoving();
                        DoCastAOE(SPELL_REPENTANCE);
                        if (Creature *c = me->FindNearestCreature(NPC_BLAZE_OF_HEAVENS, 500.0f))
                            c->AI()->DoAction(ACTION_UNEGG);
                        events.ScheduleEvent(EVENT_REPENTANCE_PULL, 1500, 0, PHASE_50_PRECENT);
                        break;
                    case EVENT_REPENTANCE_PULL:
                        DoCastAOE(SPELL_REPENTANCE_PULL);
                        events.ScheduleEvent(EVENT_REPENTANCE_KNEEL, 1500, 0, PHASE_50_PRECENT);
                        break;
                    case EVENT_REPENTANCE_KNEEL:
                        Talk(SAY_REPETANCE_2);
                        DoCastAOE(SPELL_REPENTANCE_KNEEL);
                        events.ScheduleEvent(EVENT_REPENTANCE_KNOCKBACK, 5100, 0, PHASE_50_PRECENT);
                        break;
                    case EVENT_REPENTANCE_KNOCKBACK:
                        DoCastAOE(SPELL_REPENTANCE_KNOCKBACK);
                        DoCastAOE(SPELL_REPENTANCE_PHASE_EFFECT);
                        events.ScheduleEvent(EVENT_WAIL_OF_DARKNESS, 1000, 0, PHASE_50_PRECENT);
                        break;
                    case EVENT_WAIL_OF_DARKNESS:
                        DoCast(SPELL_WAIL_OF_DARKNESS);
                        break;
                }
            }
            DoMeleeAttackIfReady();
        }
        private:
            InstanceScript* instance;
            SummonList summons;
            EventMap events;
            bool first;
            uint8 soulCounter;
    };

    CreatureAI* GetAI(Creature* creature) const
    {
        return new boss_high_prophet_barimAI (creature);
    }
};

class npc_harbinger_of_darkness : public CreatureScript
{
    public:
        npc_harbinger_of_darkness() : CreatureScript("npc_harbinger_of_darkness") { }

        struct npc_harbinger_of_darknessAI : public ScriptedAI
        {
            npc_harbinger_of_darknessAI(Creature* creature) : ScriptedAI(creature), summons(creature)
            {
                instance = me->GetInstanceScript();
            }

            void Reset()
            {
                summons.DespawnAll();
                me->SetSpeed(MOVE_RUN, 0.35f, true);
                me->SetSpeed(MOVE_WALK, 0.35f, true);
                SoulSeverTimer = 7000;
                WailOfDarknessTimer = 1000;
            }

            void JustSummoned(Creature* summon)
            {
                summons.Summon(summon);
                switch (summon->GetEntry())
                {
                    case NPC_VEIL_OF_TWILIGHT:
                        summon->SetReactState(REACT_PASSIVE);
                        break;
                }
            }

            void MoveInLineOfSight(Unit* who)
            {
                if (who->GetEntry() == NPC_SOUL_FRAGMENT && me->IsWithinMeleeRange(who))
                {
                    who->ToCreature()->DespawnOrUnsummon();
                    DoCast(me, SPELL_MERGED_SOULS);
                }
            }

            void IsSummonedBy(Unit* /*who*/)
            {
                if (Creature* barim = Creature::GetCreature(*me, instance->GetData64(DATA_HIGH_PROPHET_BARIM)))
                    barim->AI()->JustSummoned(me);
            }

            void JustReachedHome()
            {
                if (Creature* barim = Creature::GetCreature(*me, instance->GetData64(DATA_HIGH_PROPHET_BARIM)))
                    barim->AI()->EnterEvadeMode();
            }

            void JustDied(Unit* /*killer*/)
            {
                DoCastAOE(SPELL_WAIL_OF_DARKNESS_AOE);

                if (Creature* barim = Creature::GetCreature(*me, instance->GetData64(DATA_HIGH_PROPHET_BARIM)))
                    barim->AI()->DoAction(PHASE_NORMAL);

                me->DespawnOrUnsummon();
            }

            void UpdateAI(const uint32 diff)
            {
                if (!UpdateVictim())
                    return;

                if (SoulSeverTimer <= diff)
                {
                    if (Unit* target = SelectTarget(SELECT_TARGET_RANDOM, 0, 100.0f, true))
                        DoCast(target, SPELL_SOUL_SEVER);
                    SoulSeverTimer = urand(12000,22000);
                } else SoulSeverTimer -= diff;

                if (WailOfDarknessTimer <= diff)
                {
                    if (Unit* target = SelectTarget(SELECT_TARGET_RANDOM, 0))
                        DoCast(target, SPELL_WAIL_OF_DARKNESS_RANDOM, true);
                    WailOfDarknessTimer = 2000;
                } else WailOfDarknessTimer -= diff;

                DoMeleeAttackIfReady();
            }
        private:
            InstanceScript* instance;
            SummonList summons;
            uint32 SoulSeverTimer;
            uint32 WailOfDarknessTimer;
        };

        CreatureAI* GetAI(Creature* creature) const
        {
            return new npc_harbinger_of_darknessAI(creature);
        }
};

class npc_blaze_of_heavens : public CreatureScript
{
    public:
        npc_blaze_of_heavens() : CreatureScript("npc_blaze_of_heavens") { }

        struct npc_blaze_of_heavensAI : public ScriptedAI
        {
            npc_blaze_of_heavensAI(Creature* creature) : ScriptedAI(creature)
            {
                instance = me->GetInstanceScript();
            }

            void Reset()
            {
                me->SetSpeed(MOVE_RUN, 0.35f, true);
                me->SetSpeed(MOVE_WALK, 0.35f, true);
                DoCast(me, SPELL_BLAZE_OF_THE_HEAVENS_AURA);
                //                DoCast(me, SPELL_BLAZE_OF_THE_HEAVENS_SUMMON_FIRE_AURA);
                events.ScheduleEvent(EVENT_BURNING_SOULS, 2000);
                eggPhase = false;
            }

            void IsSummonedBy(Unit* summoner)
            {
                me->SetInCombatWithZone();
            }

            void DamageTaken(Unit* caster, uint32& damage)
            {
                if (me->GetHealth() < damage || eggPhase)
                {
                    damage = 0;
                    if (!eggPhase)
                        DoAction(ACTION_UNEGG);
                }
            }

            void DoAction(const int32 act)
            {
                if (act == ACTION_UNEGG)
                {
                    eggPhase = true;
                    me->CastSpell(me, SPELL_SUMMON_BURNING_SOUL, true);
                    me->RemoveAurasDueToSpell(SPELL_BLAZE_OF_THE_HEAVENS_AURA);
                    me->SetHealth(me->GetMaxHealth());
                    me->SetDisplayId(20245);
                    me->CastSpell(me, 99430, true);
                    events.ScheduleEvent(EVENT_UNEGG, 10000);
                }
            }

            void JustSummoned(Creature* summon)
            {
                switch (summon->GetEntry())
                {
                    case 48907: // burning soul
                    {
                        summon->CastSpell(summon, SPELL_BURNING_SOUL);
                        break;
                    }
                    default:
                        break;
                }
            }

            void UpdateAI(const uint32 diff)
            {
                if (!UpdateVictim())
                    return;

                events.Update(diff);

                while (uint32 eventId = events.ExecuteEvent())
                {
                    switch (eventId)
                    {
                        case EVENT_BURNING_SOULS:
                        {
                            if (!eggPhase)
                                me->CastSpell(me, SPELL_SUMMON_BURNING_SOUL, true);
                            events.ScheduleEvent(EVENT_BURNING_SOULS, 5000);
                            break;
                        }
                        case EVENT_UNEGG:
                        {
                            if (Creature *c = me->FindNearestCreature(43612, 500.0f))
                            {
                                if (!c->HasAura(SPELL_REPENTANCE))
                                {
                                    eggPhase = false;
                                    me->SetDisplayId(me->GetNativeDisplayId());
                                    me->RemoveAurasDueToSpell(99430);
                                    DoCast(me, SPELL_BLAZE_OF_THE_HEAVENS_AURA);
                                    me->SetInCombatWithZone();
                                }
                                else
                                    events.ScheduleEvent(EVENT_UNEGG, 2000);
                            }
                            break;
                        }
                    }
                }

                DoMeleeAttackIfReady();
            }

        private:
            InstanceScript* instance;
            EventMap events;
            bool eggPhase;
        };

        CreatureAI* GetAI(Creature* creature) const
        {
            return new npc_blaze_of_heavensAI(creature);
        }
};

class npc_blaze_of_heavens_egg : public CreatureScript
{
    public:
        npc_blaze_of_heavens_egg() : CreatureScript("npc_blaze_of_heavens_egg") { }

        struct npc_blaze_of_heavens_eggAI : public ScriptedAI
        {
            npc_blaze_of_heavens_eggAI(Creature *creature) : ScriptedAI(creature) {}

            void IsSummonedBy(Unit* /*owner*/)
            {
                me->SetHealth(me->GetMaxHealth() / 100 * 10);
                //                me->AddAura(SPELL_BLAZE_OF_THE_HEAVENS_EGG_VISUAL, me);
                events.ScheduleEvent(EVENT_HEAVENS_EGG_REG, 5000);
                events.ScheduleEvent(EVENT_HEAVENS_EGG_CHECK, 5100);
            }

            void UpdateAI(const uint32 diff)
            {
                events.Update(diff);

                while (uint32 eventId = events.ExecuteEvent())
                {
                    switch (eventId)
                    {
                        case EVENT_BURNING_SOULS:
                        {
                            me->CastSpell(me, SPELL_SUMMON_BURNING_SOUL, true);
                            events.ScheduleEvent(EVENT_BURNING_SOULS, 20000);
                            break;
                        }
                        case EVENT_HEAVENS_EGG_REG:
                            me->SetHealth(me->GetHealth() + me->GetMaxHealth() / 100 * 10);
                            events.ScheduleEvent(EVENT_HEAVENS_EGG_REG, 5000);
                            break;
                        case EVENT_HEAVENS_EGG_CHECK:
                            if (HealthAbovePct(99))
                            {
                                DoCast(me, SPELL_BLAZE_SUMMON);
                                me->DespawnOrUnsummon(100);
                            } else events.ScheduleEvent(EVENT_HEAVENS_EGG_CHECK, 5000);
                            break;
                    }
                }
            }
        private:
            EventMap events;
        };

        CreatureAI* GetAI(Creature* creature) const
        {
            return new npc_blaze_of_heavens_eggAI(creature);
        }
};

class npc_soul_fragment : public CreatureScript
{
    public:
        npc_soul_fragment() : CreatureScript("npc_soul_fragment") { }

        struct npc_soul_fragmentAI : public ScriptedAI
        {
            npc_soul_fragmentAI(Creature *creature) : ScriptedAI(creature)
            {
                instance = me->GetInstanceScript();
            }

            void JustDied(Unit* /*killer*/)
            {
                if (IsHeroic() && me->HasAura(SPELL_BURNING_SOUL_AURA))
                    if (Creature* barim = Creature::GetCreature(*me, instance->GetData64(DATA_HIGH_PROPHET_BARIM)))
                        barim->AI()->DoAction(ACTION_BURNING_SOUL);
            }

            void IsSummonedBy(Unit* owner)
            {
                me->SetReactState(REACT_PASSIVE);
                owner->CastSpell(me, SPELL_SOUL_SEVER_CLONE);

                if (Creature* harbinger = me->FindNearestCreature(NPC_HARBINGER_OF_DARKNESS, 100.0f, true))
                {
                    DoCast(harbinger, SPELL_SOUL_FRAGMENT);
                    me->ClearUnitState(UNIT_STATE_CASTING);
                    me->GetMotionMaster()->Clear();
                    me->GetMotionMaster()->MoveChase(harbinger, 0.0f, 0.0f);
                } else me->DespawnOrUnsummon();
            }

            void UpdateAI(uint32 const diff)
            {
                if (instance->GetData(DATA_HIGH_PROPHET_BARIM_EVENT) != SPECIAL)
                    me->DespawnOrUnsummon();
            }

        private:
            InstanceScript* instance;
        };

        CreatureAI* GetAI(Creature* creature) const
        {
            return new npc_soul_fragmentAI(creature);
        }
};

class spell_soul_fragment_effects : public SpellScriptLoader
{
public:
    spell_soul_fragment_effects() : SpellScriptLoader("spell_soul_fragment_effects") { }

    class spell_soul_fragment_effects_SpellScript : public SpellScript
    {
        PrepareSpellScript(spell_soul_fragment_effects_SpellScript);

        void HandleScript(SpellEffIndex effIndex)
        {
            PreventHitDefaultEffect(effIndex);

            GetCaster()->CastSpell(GetHitUnit(), uint32(GetEffectValue()), true);
        }

        void Register()
        {
            OnEffectHitTarget += SpellEffectFn(spell_soul_fragment_effects_SpellScript::HandleScript, EFFECT_0, SPELL_EFFECT_SCRIPT_EFFECT);
        }
    };

    SpellScript* GetSpellScript() const
    {
        return new spell_soul_fragment_effects_SpellScript();
    }
};

class spell_fifty_lashings : public SpellScriptLoader
{
public:
    spell_fifty_lashings() : SpellScriptLoader("spell_fifty_lashings") { }

    class spell_fifty_lashings_AuraScript : public AuraScript
    {
        PrepareAuraScript(spell_fifty_lashings_AuraScript);

        bool Validate(SpellInfo const* /*spellInfo*/)
        {
            if (!sSpellMgr->GetSpellInfo(SPELL_FIFTY_LASHINGS))
                return false;
            return true;
        }

        void OnProc(AuraEffect const* aurEff, ProcEventInfo& eventInfo)
        {
            PreventDefaultAction();
            GetCaster()->m_Events.AddEvent(new fiftyLashingsProcDelay(GetCaster()), GetCaster()->m_Events.CalculateTime(1000));
        }

        void Register()
        {
            OnEffectProc += AuraEffectProcFn(spell_fifty_lashings_AuraScript::OnProc, EFFECT_0, SPELL_AURA_PROC_TRIGGER_SPELL);
        }
    };

    AuraScript* GetAuraScript() const
    {
        return new spell_fifty_lashings_AuraScript();
    }
};

void AddSC_boss_high_prophet_barim()
{
    new boss_high_prophet_barim();
    new npc_harbinger_of_darkness();
    new npc_blaze_of_heavens();
    new npc_soul_fragment();
    new npc_blaze_of_heavens_egg();
    new spell_soul_fragment_effects();
    new spell_fifty_lashings();
}
