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
#include "ScriptedCreature.h"
#include "the_lost_city_of_tol_vir.h"

enum Texts
{
    SAY_AGGRO       = 0,
    SAY_SHOCKWAVE   = 1,
    SAY_DETONATE    = 2,
    SAY_DEATH       = 3
};

enum Spells
{
    SPELL_BAD_INTENTIONS             = 83113,
    SPELL_HAMMER_FIST                = 83655,

    SPELL_MYSTIC_TRAP_PLANT_TARGET   = 83646,
    SPELL_MYSTIC_TRAP_PLANT          = 83122,
    SPELL_MYSTIC_TRAP_VISUAL         = 83110,
    SPELL_MYSTIC_TRAP_SEARCHER       = 83111,
    SPELL_MYSTIC_TRAP_EXPLODE        = 83171,
    SPELL_DETONATE_TIMED             = 85523,
    SPELL_DETONATE_TRAPS             = 91263,

    SPELL_SHOCKWAVE_VISUAL_TRIGGER   = 83127,
    SPELL_SHOCKWAVE_SUMMON_EFFECT    = 83128,
    SPELL_SHOCKWAVE_VISUAL_PERIODIC  = 83129,
    SPELL_SHOWKWAVE_VISUAL_DAMAGE    = 83130,
    SPELL_SHOCKWAVE_TARGET_N         = 83131,
    SPELL_SHOCKWAVE_TARGET_S         = 83132,
    SPELL_SHOCKWAVE_TARGET_E         = 83133,
    SPELL_SHOCKWAVE_TARGET_W         = 83134,
    SPELL_SHOCKWAVE                  = 83445,
    SPELL_SHOCKWAVE_DAMAGE           = 83454
};

enum Events
{
    EVENT_BAD_INTENTIONS    = 1,
    EVENT_SHOCKWAVE         = 2,
    EVENT_HAMMER_FIST       = 3,
    EVENT_PLANT_MINE        = 4,
    EVENT_DETONATE_MINES    = 5,
    EVENT_MINE_ACTIVATION   = 6,
    EVENT_MINE_EXPLOSION    = 7
};

enum Misc
{
    POINT_DROP_PLAYER       = 0
};

class SummonShockwaveStalker : public BasicEvent
{
public:
    SummonShockwaveStalker(Unit* caster) : _caster(caster) {}

    bool Execute(uint64 execTime, uint32 /*diff*/)
    {
        _caster->CastSpell(_caster, SPELL_SHOCKWAVE_SUMMON_EFFECT);
        _caster->m_Events.AddEvent(new SummonShockwaveStalker(_caster), _caster->m_Events.CalculateTime(1000));
        return false;
    }

private:
    Unit* _caster;
};

class boss_general_husam : public CreatureScript
{
public:
    boss_general_husam() : CreatureScript("boss_general_husam") { }

    struct boss_general_husamAI : public ScriptedAI
    {
        boss_general_husamAI(Creature* creature) : ScriptedAI(creature), summons(creature)
        {
            instance = creature->GetInstanceScript();
        }

        void Reset()
        {
            events.Reset();
            summons.DespawnAll();
            instance->SetData(DATA_GENERAL_HUSAM_EVENT, NOT_STARTED);
        }

        void EnterCombat(Unit* /*who*/)
        {
            instance->SetData(DATA_GENERAL_HUSAM_EVENT, IN_PROGRESS);
            Talk(SAY_AGGRO);
            events.ScheduleEvent(EVENT_BAD_INTENTIONS, 13000);
            events.ScheduleEvent(EVENT_SHOCKWAVE, 18000);
            events.ScheduleEvent(EVENT_HAMMER_FIST, 5000);
            events.ScheduleEvent(EVENT_PLANT_MINE, 9000);

            if (IsHeroic())
                events.ScheduleEvent(EVENT_DETONATE_MINES, 25000);
        }

        void JustDied(Unit* /*Killer*/)
        {
            Talk(SAY_DEATH);
            summons.DespawnAll();
            instance->SetData(DATA_GENERAL_HUSAM_EVENT, DONE);
        }

        void JustSummoned(Creature* summon)
        {
            summons.Summon(summon);
            switch (summon->GetEntry())
            {
                case NPC_LAND_MINE_TARGET:
                    summon->SetFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_NOT_ATTACKABLE_1 | UNIT_FLAG_NOT_SELECTABLE | UNIT_FLAG_PACIFIED | UNIT_FLAG_DISABLE_MOVE);
                    DoCast(summon, SPELL_MYSTIC_TRAP_PLANT);
                    break;
                default:
                    break;
            }
        }

        void UpdateAI(const uint32 diff)
        {
            if (!UpdateVictim())
                return;

            events.Update(diff);

            if (me->HasUnitState(UNIT_STATE_CASTING))
                return;

            while(uint32 eventId = events.ExecuteEvent())
            {
                switch(eventId)
                {
                    case EVENT_BAD_INTENTIONS:
                        DoCastRandom(SPELL_BAD_INTENTIONS, 100.0f);
                        events.ScheduleEvent(EVENT_BAD_INTENTIONS, 26000);
                        break;
                    case EVENT_SHOCKWAVE:
                    {
                        Talk(SAY_SHOCKWAVE);
                        DoCast(SPELL_SHOCKWAVE_TARGET_N);
                        DoCast(SPELL_SHOCKWAVE_TARGET_S);
                        DoCast(SPELL_SHOCKWAVE_TARGET_E);
                        DoCast(SPELL_SHOCKWAVE_TARGET_W);
                        DoCastAOE(SPELL_SHOCKWAVE);

                        std::list<Creature*> creatures;
                        GetCreatureListWithEntryInGrid(creatures, me, NPC_SHOCKWAVE_VISUAL, 200.0f);
                        if (!creatures.empty())
                        {
                            for (std::list<Creature*>::iterator iter = creatures.begin(); iter != creatures.end(); ++iter)
                            {
                                Position pos;
                                (*iter)->GetPosition(&pos);
                                (*iter)->Relocate(me->GetPositionX(), me->GetPositionY(), me->GetPositionZ(), 0.0f);
                                if (Creature* creature = (*iter)->ToCreature())
                                    creature->NearTeleportTo(me->GetPositionX(), me->GetPositionY(), me->GetPositionZ(), 0);
                                (*iter)->AddAura(SPELL_SHOCKWAVE_VISUAL_PERIODIC, (*iter));
                                (*iter)->GetMotionMaster()->MovePoint(0, pos);
                                (*iter)->m_Events.AddEvent(new SummonShockwaveStalker((*iter)), (*iter)->m_Events.CalculateTime(1000));
                            }
                        }
                        events.ScheduleEvent(EVENT_SHOCKWAVE, 40000);
                    }
                        break;
                    case EVENT_HAMMER_FIST:
                        DoCastVictim(SPELL_HAMMER_FIST);
                        events.ScheduleEvent(EVENT_HAMMER_FIST, 12000);
                        break;
                    case EVENT_PLANT_MINE:
                        for(uint8 i = 0; i < 3; i++)
                            DoCastRandom(SPELL_MYSTIC_TRAP_PLANT_TARGET, 100.0f, true);
                        events.ScheduleEvent(EVENT_PLANT_MINE, (IsHeroic() ? 10000 : 17000));
                        break;
                    case EVENT_DETONATE_MINES:
                        Talk(SAY_DETONATE);
                        DoCastAOE(SPELL_DETONATE_TRAPS);
                        events.ScheduleEvent(EVENT_DETONATE_MINES, 25000);
                        break;
                }
            }
            DoMeleeAttackIfReady();
        }
    private:
        InstanceScript* instance;
        EventMap events;
        SummonList summons;
    };

    CreatureAI* GetAI(Creature* creature) const
    {
        return new boss_general_husamAI (creature);
    }
};

class npc_husam_mine : public CreatureScript
{
public:
    npc_husam_mine() : CreatureScript("npc_husam_mine") { }

    struct npc_husam_mineAI : public ScriptedAI
    {
        npc_husam_mineAI(Creature *creature) : ScriptedAI(creature)
        {
            me->SetFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_NOT_ATTACKABLE_1 | UNIT_FLAG_NOT_SELECTABLE | UNIT_FLAG_PACIFIED | UNIT_FLAG_DISABLE_MOVE);
        }

        void AttackStart(Unit* /*target*/) {}

        void IsSummonedBy(Unit* /*summoner*/)
        {
            me->SetInCombatWithZone();
            events.ScheduleEvent(EVENT_MINE_ACTIVATION, (IsHeroic() ? 1000 : 2000));
            events.ScheduleEvent(EVENT_MINE_EXPLOSION, 20000);
        }

        void SpellHit(Unit* /*caster*/, SpellInfo const* spellInfo)
        {
            if (spellInfo->Id == SPELL_DETONATE_TRAPS)
            {
                events.CancelEvent(EVENT_MINE_EXPLOSION);
                DoCast(me, SPELL_DETONATE_TIMED);
                me->DespawnOrUnsummon(5400);
            }
        }

        void UpdateAI(uint32 const diff)
        {
            events.Update(diff);

            while(uint32 eventId = events.ExecuteEvent())
            {
                switch(eventId)
                {
                    case EVENT_MINE_ACTIVATION:
                        me->AddAura(SPELL_MYSTIC_TRAP_VISUAL, me);
                        DoCast(SPELL_MYSTIC_TRAP_SEARCHER);
                        break;
                    case EVENT_MINE_EXPLOSION:
                        DoCast(me, SPELL_DETONATE_TIMED);
                        me->DespawnOrUnsummon(5400);
                        break;
                    default:
                        break;
                }
            }
        }

    private:
        EventMap events;
    };

    CreatureAI* GetAI(Creature* creature) const
    {
        return new npc_husam_mineAI(creature);
    }
};

class npc_bad_intentions_vehicle : public CreatureScript
{
public:
    npc_bad_intentions_vehicle() : CreatureScript("npc_bad_intentions_vehicle") { }

    struct npc_bad_intentions_vehicleAI : public PassiveAI
    {
        npc_bad_intentions_vehicleAI(Creature* creature) : PassiveAI(creature)
        {
            me->SetSpeed(MOVE_FLIGHT, 0.35f);
        }

        void PassengerBoarded(Unit* who, int8 seatId, bool apply)
        {
            if (!apply)
                return;

            me->GetMotionMaster()->MovePoint(0, me->GetPositionX(), me->GetPositionY(), me->GetPositionZ() - 12.0f);
        }

        void MovementInform(uint32 type, uint32 id)
        {
            if (type != POINT_MOTION_TYPE)
                return;

            if (id == POINT_DROP_PLAYER)
            {
                if(me->GetVehicleKit())
                    me->GetVehicleKit()->RemoveAllPassengers();

                me->GetMotionMaster()->MoveTargetedHome();
            }
        }
    };

    CreatureAI* GetAI(Creature* creature) const
    {
        return new npc_bad_intentions_vehicleAI(creature);
    }
};

class spell_mine_player_search_effect : public SpellScriptLoader
{
public:
    spell_mine_player_search_effect() : SpellScriptLoader("spell_mine_player_search_effect") { }

    class spell_mine_player_search_effect_SpellScript : public SpellScript
    {
        PrepareSpellScript(spell_mine_player_search_effect_SpellScript);

        void HandleScript(SpellEffIndex /*effIndex*/)
        {
            GetCaster()->RemoveAurasDueToSpell(SPELL_MYSTIC_TRAP_SEARCHER);
            GetCaster()->CastSpell((Unit*)NULL, SPELL_MYSTIC_TRAP_EXPLODE);

            if (Unit* veh = GetCaster()->GetVehicleBase())
                veh->ToCreature()->DespawnOrUnsummon();
        }

        void Register()
        {
            OnEffectHitTarget += SpellEffectFn(spell_mine_player_search_effect_SpellScript::HandleScript, EFFECT_0, SPELL_EFFECT_DUMMY);
        }
    };

    SpellScript* GetSpellScript() const
    {
        return new spell_mine_player_search_effect_SpellScript();
    }
};

class spell_bad_intentions_effect : public SpellScriptLoader
{
public:
    spell_bad_intentions_effect() : SpellScriptLoader("spell_bad_intentions_effect") { }

    class spell_bad_intentions_effect_SpellScript : public SpellScript
    {
        PrepareSpellScript(spell_bad_intentions_effect_SpellScript);

        void HandleScript(SpellEffIndex effIndex)
        {
            PreventHitDefaultEffect(effIndex);

            std::list<Creature*> triggers;
            GetCreatureListWithEntryInGrid(triggers, GetHitUnit(), NPC_BAD_INTENTIONS_TARGET, 100.0f);
            if (!triggers.empty())
            {
                triggers.sort(Trinity::ObjectDistanceOrderPred(GetHitUnit(), true));
                Creature* trigger = triggers.front();
                GetHitUnit()->CastSpell(trigger, uint32(GetEffectValue()), true);
            }
        }

        void Register()
        {
            OnEffectHitTarget += SpellEffectFn(spell_bad_intentions_effect_SpellScript::HandleScript, EFFECT_1, SPELL_EFFECT_SCRIPT_EFFECT);
        }
    };

    SpellScript* GetSpellScript() const
    {
        return new spell_bad_intentions_effect_SpellScript();
    }
};

class spell_husam_shockwave : public SpellScriptLoader
{
public:
    spell_husam_shockwave() : SpellScriptLoader("spell_husam_shockwave") { }

    class spell_husam_shockwave_SpellScript : public SpellScript
    {
        PrepareSpellScript(spell_husam_shockwave_SpellScript);

        void HandleScript(SpellEffIndex /*effIndex*/)
        {
            GetCaster()->CastSpell(GetHitUnit(), SPELL_SHOWKWAVE_VISUAL_DAMAGE, true);

            std::list<Creature*> shockwave;
            GetCreatureListWithEntryInGrid(shockwave, GetCaster(), NPC_SHOCKWAVE, 200.0f);
            for (std::list<Creature*>::iterator iter = shockwave.begin(); iter != shockwave.end(); ++iter)
            {
                (*iter)->CastSpell((*iter), SPELL_SHOCKWAVE_DAMAGE);
                (*iter)->DespawnOrUnsummon(200); // ~ 200ms visual effect delay
            }

            std::list<Creature*> shockwaveVisual;
            GetCreatureListWithEntryInGrid(shockwaveVisual, GetCaster(), NPC_SHOCKWAVE_VISUAL, 200.0f);
            for (std::list<Creature*>::iterator iter = shockwaveVisual.begin(); iter != shockwaveVisual.end(); ++iter)
                (*iter)->DespawnOrUnsummon(200);
        }

        void Register()
        {
            OnEffectHitTarget += SpellEffectFn(spell_husam_shockwave_SpellScript::HandleScript, EFFECT_0, SPELL_EFFECT_SCRIPT_EFFECT);
        }
    };

    SpellScript* GetSpellScript() const
    {
        return new spell_husam_shockwave_SpellScript();
    }
};

class spell_husam_shockwave_summon_search : public SpellScriptLoader
{
public:
    spell_husam_shockwave_summon_search() : SpellScriptLoader("spell_husam_shockwave_summon_search") { }

    class spell_husam_shockwave_summon_search_SpellScript : public SpellScript
    {
        PrepareSpellScript(spell_husam_shockwave_summon_search_SpellScript);

        bool Validate(SpellInfo const* /*spellInfo*/)
        {
            return true;
        }

        void HandleEffectDummy(SpellEffIndex /*effIndex*/)
        {
            GetHitUnit()->CastSpell(GetHitUnit(), SPELL_SHOCKWAVE_VISUAL_TRIGGER);
        }

        void Register()
        {
            OnEffectHitTarget += SpellEffectFn(spell_husam_shockwave_summon_search_SpellScript::HandleEffectDummy, EFFECT_0, SPELL_EFFECT_DUMMY);
        }
    };

    SpellScript* GetSpellScript() const
    {
        return new spell_husam_shockwave_summon_search_SpellScript();
    }
};

void AddSC_boss_general_husam()
{
    new boss_general_husam();
    new npc_husam_mine();
    new npc_bad_intentions_vehicle();
    new spell_mine_player_search_effect();
    new spell_bad_intentions_effect();
    new spell_husam_shockwave();
    new spell_husam_shockwave_summon_search();
}
