/*
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
#include <algorithm>

enum Texts
{
    SAY_LOCKMAW_DEATH   = 0,
    SAY_CROCOLISK       = 1,
};

enum Spells
{
    // Lockmaw
    SPELL_DUST_FLAIL          = 81642,
    SPELL_SCENT_OF_BLOOD      = 81690,
    SPELL_VENOMOUS_RAGE       = 81706,
    SPELL_VISCOUS_POISON      = 81630,
    // Augh
    SPELL_FRENCY              = 23537,
    SPELL_FIRE_DRAGON         = 29964,
    SPELL_CLOUD               = 84768,
    SPELL_WHIRLWIND           = 84784,
    SPELL_PARALYTIC_BLOW_DART = 84799,
    SPELL_RANDOM_AGGRO        = 50230,
    SPELL_STEALTHED           = 84244,
    // misc
    SPELL_VICIOUS             = 81677,
    SPELL_SUMMON_CROCOLISK    = 84242,
    SPELL_SUMMON_AUGH         = 84808,
    SPELL_SUMMON_AUGH_2       = 84809
};

enum Events
{
    //Lockmaw
    EVENT_DUST_FLAIL            = 1,
    EVENT_SCENT_OF_BLOOD        = 2,
    EVENT_VISCOUS_POISON        = 3,
    //Augh
    EVENT_PARALYTIC_BLOW_DART   = 4,
    EVENT_CLOUD                 = 5,
    EVENT_FIRE_DRAGON           = 6,
    EVENT_WHIRLWIND             = 7,
    EVENT_SUMMON_AUGH           = 8,
    EVENT_STEALTH               = 9
};

#define ACHIEVEMENT_EVENT_ACROCALYPSE    43658

class SummonAughEvent : public BasicEvent
{
public:
    SummonAughEvent(Unit* lockmaw) : _lockmaw(lockmaw)
    {
    }

    bool Execute(uint64 execTime, uint32 /*diff*/)
    {
        std::list<Creature*> stalker;
        _lockmaw->GetCreatureListWithEntryInGrid(stalker, NPC_ADD_STALKER, 200.0f);
        if (Unit* trigger = Trinity::Containers::SelectRandomContainerElement(stalker))
              trigger->CastSpell(trigger, roll_chance_i(50) ? SPELL_SUMMON_AUGH : SPELL_SUMMON_AUGH_2);
        return false;
    }

private:
    Unit* _lockmaw;
};

class boss_lockmaw : public CreatureScript
{
public:
    boss_lockmaw() : CreatureScript("boss_lockmaw") {}

    struct boss_lockmawAI : public ScriptedAI
    {
        boss_lockmawAI(Creature* creature) : ScriptedAI(creature), summons(creature)
        {
            instance = creature->GetInstanceScript();
        }

        void Reset ()
        {
            instance->SetData(DATA_LOCKMAW_EVENT, NOT_STARTED);
            instance->SendEncounterUnit(ENCOUNTER_FRAME_DISENGAGE, me);
            events.Reset();
            summons.DespawnAll();
            Rage = false;
        }

        void JustSummoned(Creature* summon)
        {
            switch (summon->GetEntry())
            {
                case 43650: // dust trigger
                    summon->SetReactState(REACT_PASSIVE);
                    summon->AddAura(81646, summon);
                    break;
                default:
                    summon->SetInCombatWithZone();
                    break;
            }
            summons.Summon(summon);
        }

        void JustDied(Unit* /*killer*/)
        {
            summons.DespawnAll();
            instance->SetData(DATA_LOCKMAW_EVENT, DONE);
            instance->SendEncounterUnit(ENCOUNTER_FRAME_DISENGAGE, me);

            if (!IsHeroic())
                instance->SetData(DATA_AUGH_EVENT, DONE);
        }

        void EnterCombat(Unit* /*Who*/)
        {
            DoZoneInCombat();
            events.ScheduleEvent(EVENT_DUST_FLAIL, urand(15000, 22000));
            events.ScheduleEvent(EVENT_VISCOUS_POISON, 12000);
            events.ScheduleEvent(EVENT_SCENT_OF_BLOOD, 30000);
            events.ScheduleEvent(EVENT_SUMMON_AUGH, 16000);
            instance->SendEncounterUnit(ENCOUNTER_FRAME_ENGAGE, me);
            instance->SetData(DATA_LOCKMAW_EVENT, IN_PROGRESS);
        }

        void DamageTaken(Unit* /*target*/, uint32& damage)
        {
            if (HealthBelowPct(30) && Rage == false)
            {
                Rage = true;
                DoCast(me, SPELL_VENOMOUS_RAGE);
            }
        }

        void DoAction(int32 const act)
        {
            switch (act)
            {
                case ACTION_START_EVENT:
                {
                    //todoo fix lackmaw heroic start event
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

            if (me->HasUnitState(UNIT_STATE_CASTING))
                return;

            while(uint32 eventId = events.ExecuteEvent())
            {
                switch(eventId)
                {
                    case EVENT_DUST_FLAIL:
                        me->CastSpell(me, SPELL_DUST_FLAIL);
                        events.ScheduleEvent(EVENT_DUST_FLAIL, urand(15000, 22000));
                        break;
                    case EVENT_VISCOUS_POISON:
                        DoCastVictim(SPELL_VISCOUS_POISON);
                        events.ScheduleEvent(EVENT_VISCOUS_POISON, 12000);
                        break;
                    case EVENT_SCENT_OF_BLOOD:
                    {
                        if (Unit* target = SelectTarget(SELECT_TARGET_RANDOM, 0, 0.0f, true))
                            me->CastCustomSpell(SPELL_SCENT_OF_BLOOD, SPELLVALUE_MAX_TARGETS, 1, target, false);

                        std::list<Creature*> stalker;
                        me->GetCreatureListWithEntryInGrid(stalker, NPC_ADD_STALKER, 200.0f);
                        std::vector<Creature*> random_shuffle_stacker_container;
                        for (std::list<Creature*>::iterator itr = stalker.begin(); itr != stalker.end(); ++itr)
                            random_shuffle_stacker_container.push_back(*itr);
                        std::random_shuffle(random_shuffle_stacker_container.begin(), random_shuffle_stacker_container.end());
                        for (uint32 cnt = 0; cnt < 4 && cnt < random_shuffle_stacker_container.size(); cnt++)
                        random_shuffle_stacker_container[cnt]->CastSpell(random_shuffle_stacker_container[cnt], SPELL_SUMMON_CROCOLISK);
                        events.ScheduleEvent(EVENT_SCENT_OF_BLOOD, 30000);
                        break;
                    }
                    case EVENT_SUMMON_AUGH:
                    {
                        std::list<Creature*> stalker;
                        me->GetCreatureListWithEntryInGrid(stalker, NPC_ADD_STALKER, 200.0f);
                        if (!stalker.empty())
                            if (Unit* trigger = Trinity::Containers::SelectRandomContainerElement(stalker))
                                trigger->CastSpell(trigger, roll_chance_i(50) ? SPELL_SUMMON_AUGH : SPELL_SUMMON_AUGH_2);
                        break;
                    }
                    default:
                        break;
                }
            }

            DoMeleeAttackIfReady();
        }
    private:
        InstanceScript* instance;
        EventMap events;
        SummonList summons;
        bool Rage;
    };

    CreatureAI* GetAI(Creature* creature) const
    {
        return new boss_lockmawAI(creature);
    }
};

class npc_augh : public CreatureScript
{
public:
    npc_augh() : CreatureScript("npc_augh") {}

    struct npc_aughAI : public ScriptedAI
    {
        npc_aughAI(Creature *creature) : ScriptedAI(creature)
        {
            instance = creature->GetInstanceScript();
            isBoss = (creature->GetEntry() == BOSS_AUGH ? true : false);
            if (!isBoss)
                me->SetFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_NON_ATTACKABLE|UNIT_FLAG_NOT_SELECTABLE);
        }

        void Reset ()
        {
            if (isBoss)
                instance->SendEncounterUnit(ENCOUNTER_FRAME_DISENGAGE, me);
        }

        void JustReachedHome()
        {
            if (!isBoss)
            {
                DoCast(me, SPELL_STEALTHED);
                if (Unit* lockmaw = me->GetUnit(*me, instance->GetData64(DATA_LOCKMAW)))
                    lockmaw->m_Events.AddEvent(new SummonAughEvent(lockmaw), lockmaw->m_Events.CalculateTime(5000));
                me->DespawnOrUnsummon(1000);
            }
        }

        void IsSummonedBy(Unit* /*who*/)
        {
            if (Creature* lockmaw = Creature::GetCreature(*me, instance->GetData64(DATA_LOCKMAW)))
                lockmaw->AI()->JustSummoned(me);
            if (!isBoss)
                _cureEvent = RAND(EVENT_CLOUD, EVENT_WHIRLWIND);
        }

        void EnterCombat(Unit* /*who*/)
        {
            if (isBoss)
            {
                DoCast(me, SPELL_FRENCY);
                events.ScheduleEvent(EVENT_FIRE_DRAGON, 35000);
                instance->SendEncounterUnit(ENCOUNTER_FRAME_ENGAGE, me);
                instance->SetData(DATA_AUGH_EVENT, IN_PROGRESS);
                events.ScheduleEvent(EVENT_CLOUD, 18000);
                events.ScheduleEvent(EVENT_WHIRLWIND, 25000);
           }
            events.ScheduleEvent(EVENT_PARALYTIC_BLOW_DART, 6000);
        }

        void JustDied(Unit* /*killer*/)
        {
            if (isBoss)
            {
                instance->SendEncounterUnit(ENCOUNTER_FRAME_DISENGAGE, me);
                instance->SetData(DATA_AUGH_EVENT, DONE);
            }
        }

        bool CanAIAttack(const Unit* /*who*/) const
        {
            if (instance->GetData(DATA_LOCKMAW_EVENT) != DONE && isBoss)
                return false;

            return true;
        }

        void UpdateAI(const uint32 diff)
        {
            if (!UpdateVictim())
                return;

            events.Update(diff);

            if (me->HasUnitState(UNIT_STATE_CASTING))
                return;

            while (uint32 eventId = events.ExecuteEvent())
            {
                switch (eventId)
                {
                    case EVENT_PARALYTIC_BLOW_DART:
                        DoCastAOE(SPELL_RANDOM_AGGRO);
                        DoCastVictim(SPELL_PARALYTIC_BLOW_DART);
                        if (!isBoss)
                        {
                            if (rand() % 2 == 0)
                                events.ScheduleEvent(EVENT_PARALYTIC_BLOW_DART, 7000);
                            else
                                events.ScheduleEvent(_cureEvent, 1000);
                        }
                        else
                            events.ScheduleEvent(EVENT_PARALYTIC_BLOW_DART, 25000);
                        break;
                    case EVENT_CLOUD:
                        DoCastVictim(SPELL_CLOUD);
                        if (!isBoss)
                            events.ScheduleEvent(EVENT_STEALTH, 1000);
                        else
                            events.ScheduleEvent(EVENT_CLOUD, 10000);
                        break;
                    case EVENT_FIRE_DRAGON:
                        DoCastVictim(SPELL_FIRE_DRAGON);
                        events.ScheduleEvent(EVENT_FIRE_DRAGON, 40000);
                        break;
                    case EVENT_WHIRLWIND:
                        DoCastAOE(SPELL_RANDOM_AGGRO);
                        me->AddAura(SPELL_WHIRLWIND, me);
                        if (!isBoss)
                            events.ScheduleEvent(EVENT_STEALTH, 25000);
                        else
                            events.ScheduleEvent(EVENT_WHIRLWIND, 45000);
                        break;
                    case EVENT_STEALTH:
                        me->SetReactState(REACT_PASSIVE);
                        EnterEvadeMode();
                        break;
                    default:
                        break;
                }
            }
            if (isBoss)
                DoMeleeAttackIfReady();
        }
        private:
            EventMap events;
            InstanceScript* instance;
            bool isBoss;
            Events _cureEvent;
    };

    CreatureAI* GetAI(Creature* creature) const
    {
        return new npc_aughAI(creature);
    }
};

class npc_crosilik: public CreatureScript
{
public:
 npc_crosilik() : CreatureScript("npc_crosilik") { }

    struct npc_crosilikAI : public ScriptedAI
    {
        npc_crosilikAI(Creature *creature) : ScriptedAI(creature)
        {
            instance = me->GetInstanceScript();
        }

        void Reset()
        {
           vicious = 10000;
           searchTimer = 1000;
        }

        void JustDied(Unit* /*killer*/)
        {
            instance->DoStartTimedAchievement(ACHIEVEMENT_TIMED_TYPE_EVENT, ACHIEVEMENT_EVENT_ACROCALYPSE);
        }

        void IsSummonedBy(Unit* /*who*/)
        {
            if (Creature* lockmaw = Creature::GetCreature(*me, instance->GetData64(DATA_LOCKMAW)))
                lockmaw->AI()->JustSummoned(me);
        }

        void UpdateAI(const uint32 diff)
        {
            if (!UpdateVictim())
                return;

            if (searchTimer <= diff)
            {
                if (Unit* target = SelectTarget(SELECT_TARGET_NEAREST, 0, 0.0f, true, SPELL_SCENT_OF_BLOOD))
                {
                    DoResetThreat();
                    me->AddThreat(target, 1000000.0f);
                    me->AI()->AttackStart(target);
                }
                searchTimer = 1000;
            } else searchTimer -= diff;

            if (vicious <= diff)
            {
                if (Unit* target = SelectTarget(SELECT_TARGET_RANDOM, 0, 0.0f, true))
                    DoCast(target, SPELL_VICIOUS);
                vicious = 10000;
            } else vicious -= diff;

            DoMeleeAttackIfReady();
        }
    private:
        InstanceScript* instance;
        uint32 vicious;
        uint32 searchTimer;
    };

    CreatureAI* GetAI(Creature* creature) const
    {
        return new npc_crosilikAI (creature);
    }
};

class npc_augh_fake: public CreatureScript
{
public:
 npc_augh_fake() : CreatureScript("npc_augh_fake") { }

    struct npc_augh_fakeAI : public ScriptedAI
    {
        npc_augh_fakeAI(Creature *creature) : ScriptedAI(creature)
        {
            instance = me->GetInstanceScript();
        }

        void Reset() { }

        void EnterCombat(Unit* /*who*/)
        {
            events.ScheduleEvent(EVENT_WHIRLWIND, 1000);
        }

        void DamageTaken(Unit* /*target*/, uint32& damage)
        {
            if (me->HasReactState(REACT_PASSIVE))
                return;
            if (HealthBelowPct(50))
            {
                me->SetReactState(REACT_PASSIVE);
                me->SetFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_NON_ATTACKABLE|UNIT_FLAG_NOT_SELECTABLE);
                Talk(0);
                if (Creature *c = me->FindNearestCreature(43614, 1000))
                    me->GetMotionMaster()->MoveChase(c);
            }
            me->DespawnOrUnsummon(5000);
        }

        void IsSummonedBy(Unit* /*who*/)
        {
            if (Creature* lockmaw = Creature::GetCreature(*me, instance->GetData64(DATA_LOCKMAW)))
                lockmaw->AI()->JustSummoned(me);
        }

        void UpdateAI(const uint32 diff)
        {
            if (!UpdateVictim())
                return;

            events.Update(diff);

            if (me->HasUnitState(UNIT_STATE_CASTING))
                return;

            while (uint32 eventId = events.ExecuteEvent())
            {
                switch (eventId)
                {
                    case EVENT_WHIRLWIND:
                        DoCastAOE(SPELL_RANDOM_AGGRO);
                        me->AddAura(SPELL_WHIRLWIND, me);
                        break;
                    default:
                        break;
                }
            }

            DoMeleeAttackIfReady();
        }
    private:
        InstanceScript* instance;
        EventMap events;
    };

    CreatureAI* GetAI(Creature* creature) const
    {
        return new npc_augh_fakeAI (creature);
    }
};

class npc_augh_fake2: public CreatureScript
{
public:
 npc_augh_fake2() : CreatureScript("npc_augh_fake2") { }

    struct npc_augh_fake2AI : public ScriptedAI
    {
        npc_augh_fake2AI(Creature *creature) : ScriptedAI(creature)
        {
            instance = me->GetInstanceScript();
        }

        void Reset() { }

        void EnterCombat(Unit* /*who*/)
        {
            events.ScheduleEvent(EVENT_CLOUD, 1500);
            events.ScheduleEvent(EVENT_PARALYTIC_BLOW_DART, 1000);
        }

        void DamageTaken(Unit* /*target*/, uint32& damage)
        {
            if (me->HasReactState(REACT_PASSIVE))
                return;
            if (HealthBelowPct(50))
            {
                me->SetReactState(REACT_PASSIVE);
                me->SetFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_NON_ATTACKABLE|UNIT_FLAG_NOT_SELECTABLE);
                Talk(0);
                if (Creature *c = me->FindNearestCreature(43614, 1000))
                    me->GetMotionMaster()->MoveChase(c);
            }
            me->DespawnOrUnsummon(5000);
        }

        void IsSummonedBy(Unit* /*who*/)
        {
            if (Creature* lockmaw = Creature::GetCreature(*me, instance->GetData64(DATA_LOCKMAW)))
                lockmaw->AI()->JustSummoned(me);
        }

        void UpdateAI(const uint32 diff)
        {
            if (!UpdateVictim())
                return;

            events.Update(diff);

            if (me->HasUnitState(UNIT_STATE_CASTING))
                return;

            while (uint32 eventId = events.ExecuteEvent())
            {
                switch (eventId)
                {
                    case EVENT_CLOUD:
                        DoCastVictim(SPELL_CLOUD);
                        break;
                    case EVENT_PARALYTIC_BLOW_DART:
                        DoCastRandom(SPELL_PARALYTIC_BLOW_DART, 40.0f);
                        break;
                    default:
                        break;
                }
            }

            DoMeleeAttackIfReady();
        }
    private:
        InstanceScript* instance;
        EventMap events;
    };

    CreatureAI* GetAI(Creature* creature) const
    {
        return new npc_augh_fake2AI (creature);
    }
};

void AddSC_boss_lockmaw()
{
    new boss_lockmaw();
    new npc_augh();
    new npc_crosilik();
    new npc_augh_fake();
    new npc_augh_fake2();
}
