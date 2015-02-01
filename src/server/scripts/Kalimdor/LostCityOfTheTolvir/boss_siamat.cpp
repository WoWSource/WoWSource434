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
#include "WorldPacket.h"
#include "the_lost_city_of_tol_vir.h"
#include "ScriptMgr.h"
#include "ScriptedCreature.h"
#include "SpellScript.h"
#include "SpellAuraEffects.h"

// Minion Of Siamat 44704
// Servant Of Siamat 45269
// Cloud Burst 44541

enum Texts
{
    SAY_INTRO   = 0,
    SAY_AGGRO   = 1,
    SAY_EVENT   = 2,
    SAY_KILL    = 3,
    SAY_DEATH   = 4
};

enum Spells
{
    // Siamat
    SPELL_DEFLECTING_WINDS      = 84589, // Initial shield
    SPELL_STATIC_SHOCK          = 84546,
    SPELL_CALL_OF_SKY           = 84956,
    SPELL_STORM_BOLT            = 73564,
    SPELL_STORM_BOLT_RANDOM     = 91853,
    SPELL_ABSORB_STORMS         = 83151,
    SPELL_CLOUD_BURST_SIAMAT    = 83048, // unused
    SPELL_CLOUD_BURST_SUMMON    = 83790,
    SPELL_WAILING_WINDS         = 83066,
    SPELL_WAILING_WINDS_TRIGGER = 83089,
    SPELL_TEMPEST_STORM_ROOT    = 84616,

    // Minion Of Siamat
    SPELL_TEMPEST_STORM_VISUAL  = 84512,
    SPELL_TEMPEST_STORM_SUMMON  = 83414,

    SPELL_DEPLETION             = 84550, // Aura
    SPELL_CHAIN_LIGHTNING       = 83455,

    // tempest storm
    SPELL_TEMPEST_STORM_FORM    = 83170,
    SPELL_TEMPEST_STORM_AURA    = 83406,

    // Servant Of Siamat
    SPELL_THUNDER_CRASH         = 84521,
    SPELL_LIGHTNING_NOVA        = 84544,
    SPELL_LIGHTNING_CHARGE      = 91872,
    SPELL_LIGHTNING_CHARGE_AURA = 93959,

    // Cloud
    SPELL_CLOUD_BURST_VISUAL    = 83048, // Aura Cloud
    SPELL_CLOUD_BURST           = 83051,

    // Wind Tunnel
    SPELL_SLIPSTREAM           = 85016,
    SPELL_EJECT_ALL_PASSENGERS = 79737
};

enum Events
{
    // Siamat Event
    EVENT_DEFLECTING_WINDS                  = 1,
    EVENT_STORM_BOLT_RANDOM                 = 2,
    EVENT_SUMMON_NPC_SERVANT_OF_SIAMAT      = 3,
    EVENT_SUMMON_NPC_MINION_OF_SIAMAT       = 4,
    EVENT_ABSORB_STORMS                     = 5,
    EVENT_CLOUD_BURST_SUMMON                = 6,

    // Servant Of Siamat Event
    EVENT_THUNDER_CRASH                     = 7,
    EVENT_LIGHTNING_NOVA                    = 8,

    // Cloud Event
    EVENT_CLOUD_BURST                       = 9,
    EVENT_DESPAWN_CLOUD_BURST               = 10,

    // tempest storm
    EVENT_UPDATE_SCALE                      = 11,

    // wind tunnel
    EVENT_EJECT_PASSANGERS                  = 12
};

enum Phases
{
    PHASE_ALL           = 0,
    PHASE_ONE           = 1,
    PHASE_TWO           = 2
};

#define ACHIEVEMENT_HEADED_SOUTH    5292

class boss_siamat : public CreatureScript
{
    public:
        boss_siamat() : CreatureScript("boss_siamat") {}

        struct boss_siamatAI : public ScriptedAI
        {
            boss_siamatAI(Creature* creature) : ScriptedAI(creature), summons(creature)
            {
                instance = creature->GetInstanceScript();
            }

            void Reset ()
            {
                killCount = 0;
                events.Reset();
                summons.DespawnAll();

                instance->SendEncounterUnit(ENCOUNTER_FRAME_DISENGAGE, me);
                instance->SetData(DATA_SIAMAT_EVENT, NOT_STARTED);
                me->SetFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_DISABLE_MOVE); // idk why siamat is moving idle WITHOUT movement id and movetype... 
            }

            void EnterCombat(Unit* /*victim*/)
            {
                Talk(SAY_AGGRO);
                DoCast(me, SPELL_TEMPEST_STORM_ROOT);
                DoZoneInCombat();

                events.SetPhase(PHASE_ONE);
                events.ScheduleEvent(EVENT_DEFLECTING_WINDS, 1500, 0, PHASE_ONE);
                events.ScheduleEvent(EVENT_STORM_BOLT_RANDOM, urand(3000, 8000), 0, PHASE_ONE);
                events.ScheduleEvent(EVENT_SUMMON_NPC_SERVANT_OF_SIAMAT, urand(2000, 4000), 0, PHASE_ONE);
                events.ScheduleEvent(EVENT_CLOUD_BURST_SUMMON, urand(10000, 15000), 0, PHASE_ALL);
                events.ScheduleEvent(EVENT_SUMMON_NPC_MINION_OF_SIAMAT, 15000, 0, PHASE_ALL);
                instance->SetData(DATA_SIAMAT_EVENT, IN_PROGRESS);
            }

            void JustDied(Unit* /*killer*/)
            {
                Talk(SAY_DEATH);
                summons.DespawnAll();

                instance->SendEncounterUnit(ENCOUNTER_FRAME_DISENGAGE, me);
                instance->SetData(DATA_SIAMAT_EVENT, DONE);

                if (IsHeroic())
                {
                    AchievementEntry const* headedSouth   = sAchievementStore.LookupEntry(ACHIEVEMENT_HEADED_SOUTH);
                    Map::PlayerList const& players = me->GetMap()->GetPlayers();
                    if (!players.isEmpty())
                    {
                        for (Map::PlayerList::const_iterator itr = players.begin(); itr != players.end(); ++itr)
                        {
                            if (Player* player = itr->getSource())
                                if (player->GetAuraCount(SPELL_LIGHTNING_CHARGE_AURA) == 3)
                                    player->CompletedAchievement(headedSouth);
                        }
                    }
                }
            }

            void KilledUnit(Unit* victim)
            {
                if (victim->GetTypeId() == TYPEID_PLAYER)
                    Talk(SAY_KILL);
            }

            void JustSummoned(Creature* summon)
            {
                summons.Summon(summon);

                summon->AI()->AttackStart(SelectTarget(SELECT_TARGET_RANDOM));
            }

            void SummonedCreatureDespawn(Creature* summon)
            {
                if (summon->GetEntry() == NPC_SERVANT_OF_SIAMAT)
                    instance->SendEncounterUnit(ENCOUNTER_FRAME_DISENGAGE, summon);
            }

            void SummonedCreatureDies(Creature* summon, Unit* /*killer*/)
            {
                switch (summon->GetEntry())
                {
                    case NPC_SERVANT_OF_SIAMAT:
                        instance->SendEncounterUnit(ENCOUNTER_FRAME_DISENGAGE, summon);
                        killCount++;
                        if (killCount > 2 && me->HasAura(SPELL_DEFLECTING_WINDS))
                        {
                            me->RemoveFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_DISABLE_MOVE);
                            instance->SendEncounterUnit(ENCOUNTER_FRAME_ENGAGE, me);
                            me->RemoveAurasDueToSpell(SPELL_TEMPEST_STORM_ROOT);
                            me->RemoveAurasDueToSpell(SPELL_DEFLECTING_WINDS);
                            events.SetPhase(PHASE_TWO);
                            DoCast(SPELL_WAILING_WINDS);
                            events.ScheduleEvent(EVENT_ABSORB_STORMS, urand(0, 10 * IN_MILLISECONDS), 0, PHASE_TWO);
                        }
                        break;
                    default:
                        break;
                }
            }

            void DoAction(int32 const action)
            {
                if (action == ACTION_INTRO)
                    Talk(SAY_INTRO);
            }

            void UpdateAI(const uint32 diff)
            {
                if (!UpdateVictim())
                    return;

                events.Update(diff);
                
                while(uint32 eventId = events.ExecuteEvent())
                {
                    switch(eventId)
                    {
                        case EVENT_DEFLECTING_WINDS:
                            DoCast(me, SPELL_DEFLECTING_WINDS, true);
                            break;
                        case EVENT_STORM_BOLT_RANDOM:
                            DoCastRandom(SPELL_STORM_BOLT_RANDOM, 100.0f, true);
                            events.ScheduleEvent(EVENT_STORM_BOLT_RANDOM, urand(20000, 25000), 0, PHASE_ONE);
                            break;
                        case EVENT_SUMMON_NPC_SERVANT_OF_SIAMAT:
                            DoCast(SPELL_STATIC_SHOCK);
                            events.ScheduleEvent(EVENT_SUMMON_NPC_SERVANT_OF_SIAMAT, 20000, 0, PHASE_ONE);
                            break;
                        case EVENT_SUMMON_NPC_MINION_OF_SIAMAT:
                            Talk(SAY_EVENT);
                            DoCast(SPELL_CALL_OF_SKY);
                            events.ScheduleEvent(EVENT_SUMMON_NPC_MINION_OF_SIAMAT, 20000, 0, PHASE_ALL);
                            break;
                        case EVENT_ABSORB_STORMS:
                            DoCastAOE(SPELL_ABSORB_STORMS);
                            events.ScheduleEvent(EVENT_ABSORB_STORMS, 25 * IN_MILLISECONDS, 0, PHASE_TWO);
                            break;
                        case EVENT_CLOUD_BURST_SUMMON:
                            DoCastRandom(SPELL_CLOUD_BURST_SUMMON, 100.0f, true);
                            events.ScheduleEvent(EVENT_CLOUD_BURST_SUMMON, urand(15000, 20000), 0, PHASE_ALL);
                            break;
                        default:
                            break;
                    }
                }
                if (events.IsInPhase(PHASE_ONE))
                    DoSpellAttackIfReady(SPELL_STORM_BOLT);
                else
                    DoMeleeAttackIfReady();
            }
            private:
                InstanceScript* instance;
                EventMap events;
                SummonList summons;
                uint8 killCount;
        };

        CreatureAI* GetAI(Creature* creature) const
        {
            return new boss_siamatAI(creature);
        }
};

class npc_minion_of_siamat : public CreatureScript
{
    public:
        npc_minion_of_siamat() : CreatureScript("npc_minion_of_siamat") {}

        struct npc_minion_of_siamatAI : public Scripted_NoMovementAI
        {
            npc_minion_of_siamatAI(Creature* creature) : Scripted_NoMovementAI(creature) 
            {
                instance = me->GetInstanceScript();
            }

            void EnterCombat(Unit* /*who*/)
            {
                DoCast(me, SPELL_DEPLETION);
            }

            void JustSummoned(Creature* summon)
            {
                if (Creature* siamat = Creature::GetCreature(*me, instance->GetData64(DATA_SIAMAT)))
                    siamat->AI()->JustSummoned(summon);
            }

            void JustDied(Unit* /*killer*/)
            {
                DoCast(SPELL_TEMPEST_STORM_SUMMON);
            }

            void UpdateAI(const uint32 diff)
            {
                if (!UpdateVictim())
                    return;

                DoSpellAttackIfReady(SPELL_CHAIN_LIGHTNING);
            }
        private:
            InstanceScript* instance;

        };

        CreatureAI* GetAI(Creature* creature) const
        {
            return new npc_minion_of_siamatAI(creature);
        }
};

class npc_servant_of_siamat : public CreatureScript
{
    public:
        npc_servant_of_siamat() : CreatureScript("npc_servant_of_siamat") {}

        struct npc_servant_of_siamatAI : public ScriptedAI
        {
            npc_servant_of_siamatAI(Creature* creature) : ScriptedAI(creature)
            {
                instance = creature->GetInstanceScript();
            }

            void EnterCombat(Unit* /*attacker*/)
            {
                instance->SendEncounterUnit(ENCOUNTER_FRAME_ENGAGE, me);
                events.ScheduleEvent(EVENT_LIGHTNING_NOVA, urand(10000, 15000));
                events.ScheduleEvent(EVENT_THUNDER_CRASH, urand(15000, 20000));
            }

            void DamageTaken(Unit* damageDealer, uint32& damage)
            {
                if (HealthBelowPct(5) && IsHeroic())
                {
                    DoCast(me, SPELL_LIGHTNING_CHARGE); 
                }
            }

            void SpellHitTarget(Unit* /*target*/, SpellInfo const* spell)
            {
                if (spell->Id == SPELL_LIGHTNING_CHARGE)
                    me->DespawnOrUnsummon();
            }

            void UpdateAI(const uint32 diff)
            {
                if (!UpdateVictim())
                    return;

                events.Update(diff);

                 while(uint32 eventId = events.ExecuteEvent())
                {
                    switch(eventId)
                    {
                        case EVENT_THUNDER_CRASH:
                            DoCastRandom(SPELL_THUNDER_CRASH, 100.0f);
                            events.ScheduleEvent(EVENT_THUNDER_CRASH, urand(15000, 20000));
                            break;
                        case EVENT_LIGHTNING_NOVA:
                            DoCastRandom(SPELL_LIGHTNING_NOVA, 100.0f);
                            events.ScheduleEvent(EVENT_LIGHTNING_NOVA, urand(10000, 15000));
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
            return new npc_servant_of_siamatAI(creature);
        }
};

class npc_cloud_burst : public CreatureScript
{
    public:
        npc_cloud_burst() : CreatureScript("npc_cloud_burst") {}

        struct npc_cloud_burstAI : public Scripted_NoMovementAI
        {
            npc_cloud_burstAI(Creature* creature) : Scripted_NoMovementAI(creature) {}

            void IsSummonedBy(Unit* /*summoner*/)
            {
                DoCast(me, SPELL_CLOUD_BURST_VISUAL);
                events.ScheduleEvent(EVENT_CLOUD_BURST, 3 * IN_MILLISECONDS);
            }

            void UpdateAI(const uint32 diff)
            {   
                events.Update(diff);

                while(uint32 eventId = events.ExecuteEvent())
                {
                    switch(eventId)
                    {
                        case EVENT_CLOUD_BURST:
                            DoCast(SPELL_CLOUD_BURST);
                            events.ScheduleEvent(EVENT_DESPAWN_CLOUD_BURST, 2 * IN_MILLISECONDS);
                            break;
                        case EVENT_DESPAWN_CLOUD_BURST:
                            me->DespawnOrUnsummon();
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
            return new npc_cloud_burstAI(creature);
        }
};

class jumpDest : public BasicEvent
{
public:
    jumpDest(Unit* caster) : _caster(caster) {}

    bool Execute(uint64 execTime, uint32 /*diff*/)
    {
        if (_caster)
        {
            _caster->CastSpell(_caster, SPELL_SLIPSTREAM);
            Creature *dest = _caster->FindNearestCreature(48097, 200.0f);

            if (dest)
                _caster->CastSpell(dest, 46598);

            return true;
        }

        return false;
    }

private:
    Unit* _caster;
};

class npc_wind_tunnel : public CreatureScript
{
public:
    npc_wind_tunnel() : CreatureScript("npc_wind_tunnel") { }

    struct npc_wind_tunnelAI : public PassiveAI
    {
        npc_wind_tunnelAI(Creature* creature) : PassiveAI(creature) 
        {
            instance = me->GetInstanceScript();
            me->SetFlag(UNIT_NPC_FLAGS, UNIT_NPC_FLAG_SPELLCLICK);
        }

        void PassengerBoarded(Unit* who, int8 seatId, bool apply)
        {
            if (!apply)
                return;

            who->m_Events.AddEvent(new jumpDest(who), who->m_Events.CalculateTime(2000));
        }

        void UpdateAI(const uint32 diff)
        {
            if (instance->GetData(DATA_GENERAL_HUSAM_EVENT) == DONE && instance->GetData(DATA_HIGH_PROPHET_BARIM_EVENT) == DONE && instance->GetData(DATA_AUGH_EVENT) == DONE && instance->GetData(DATA_LOCKMAW_EVENT) == DONE)
                me->SetVisible(true);
            else me->SetVisible(false);
        }
    private:
        InstanceScript* instance;
    };

    CreatureAI* GetAI(Creature* creature) const
    {
        return new npc_wind_tunnelAI (creature);
    }
};

class npc_wind_tunnel_landing_zone : public CreatureScript
{
public:
    npc_wind_tunnel_landing_zone() : CreatureScript("npc_wind_tunnel_landing_zone") { }

    struct npc_wind_tunnel_landing_zoneAI : public PassiveAI
    {
        npc_wind_tunnel_landing_zoneAI(Creature* creature) : PassiveAI(creature) {}

        void PassengerBoarded(Unit* who, int8 seatId, bool apply)
        {
            if (!apply)
                return;

            events.ScheduleEvent(EVENT_EJECT_PASSANGERS, 1000);
        }

        void UpdateAI(const uint32 diff)
        {   
            events.Update(diff);

            while(uint32 eventId = events.ExecuteEvent())
            {
                if (eventId == EVENT_EJECT_PASSANGERS)
                    if (me->GetVehicleKit())
                        me->GetVehicleKit()->RemoveAllPassengers();
            }
        }
    private:
        EventMap events;
    };

    CreatureAI* GetAI(Creature* creature) const
    {
        return new npc_wind_tunnel_landing_zoneAI (creature);
    }
};

class npc_tempest_storm : public CreatureScript
{
public:
    npc_tempest_storm() : CreatureScript("npc_tempest_storm") { }

    struct npc_tempest_stormAI : public ScriptedAI
    {
        npc_tempest_stormAI(Creature* creature) : ScriptedAI(creature) {}

        void AttackStart(Unit* /*who*/) {} // just for disable attacks/aggro
        void EnterEvadeMode() {}           // just for blocking evade mode...

        void IsSummonedBy(Unit* summoner)
        {
            summoner->ToCreature()->DespawnOrUnsummon();
            me->AddAura(SPELL_TEMPEST_STORM_FORM, me);
            me->AddAura(SPELL_TEMPEST_STORM_AURA, me);
            me->GetMotionMaster()->MoveRandom(10.0f);
        }

        void SpellHit(Unit* /*who*/, SpellInfo const* spellInfo)
        {
            if (spellInfo->Id == SPELL_ABSORB_STORMS)
            {
                me->DespawnOrUnsummon(3000);
                events.ScheduleEvent(EVENT_UPDATE_SCALE, 2000);
            }
        }

        void UpdateAI(uint32 const diff)
        {
            events.Update(diff);

            while(uint32 eventId = events.ExecuteEvent())
            {
                switch(eventId)
                {
                    case EVENT_UPDATE_SCALE:
                        me->SetObjectScale(0.1f);
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
        return new npc_tempest_stormAI (creature);
    }
};

class spell_wailing_winds : public SpellScriptLoader
{
public:
    spell_wailing_winds() : SpellScriptLoader("spell_wailing_winds") { }

    class spell_wailing_winds_AuraScript : public AuraScript
    {
        PrepareAuraScript(spell_wailing_winds_AuraScript);

        bool Load()
        {
            _instance = GetCaster()->GetInstanceScript();
            return _instance != NULL;
        }

        void HandleEffectPeriodic(AuraEffect const* /*aurEff*/)
        {
            if (!GetCaster())
                return;

            GetCaster()->CastSpell((Unit*)NULL, SPELL_WAILING_WINDS_TRIGGER, true);
        }

        void Register()
        {
            OnEffectPeriodic += AuraEffectPeriodicFn(spell_wailing_winds_AuraScript::HandleEffectPeriodic, EFFECT_0, SPELL_AURA_PERIODIC_DUMMY);
        }

        InstanceScript* _instance;
    };

    AuraScript* GetAuraScript() const
    {
        return new spell_wailing_winds_AuraScript();
    }
};

void AddSC_boss_siamat()
{
    new boss_siamat();
    new npc_minion_of_siamat();
    new npc_servant_of_siamat();
    new npc_cloud_burst();
    new npc_wind_tunnel();
    new npc_wind_tunnel_landing_zone();
    new npc_tempest_storm();
    new spell_wailing_winds();
}