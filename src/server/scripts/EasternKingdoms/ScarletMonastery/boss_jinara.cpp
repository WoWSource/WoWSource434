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

#include "ScriptMgr.h"
#include "ScriptedCreature.h"
#include "scarlet_monastery.h"
#include "InstanceScript.h"
#include "ObjectAccessor.h"

enum Texts
{
    SAY_AGGRO       = 0,
    SAY_EARTHQUAKE  = 1,
    SAY_OVERRUN     = 2,
    SAY_SLAY        = 3,
    SAY_DEATH       = 4
};

enum Spells
{
    // Jinara
    SPELL_OVERRUN           = 32636,
    SPELL_BOOM              = 59084,
    SPELL_STORM_EFFECT      = 10092,
    SPELL_STORM_VISUAL      = 68802,
    SPELL_LIGHTNING         = 64785,
    SPELL_SHADOW_STORM      = 14297,
    SPELL_CIRCLE_BLIZZARD   = 29952,
    SPELL_ICE               =  3355,
    SPELL_WIND              = 51877,
    SPELL_BEAM              = 68341,
    SPELL_CHAIN             = 95918,
    SPELL_BOLT              = 94982,
    SPELL_ANIM_CHANNEL      = 89176,
    SPELL_WIND_KNOCK        = 93263,
    SPELL_SHADOW_CHARGE     = 108551,
    SPELL_BRAIN_SPIKE       = 91497,
    SPELL_CASTER_KNOCK      = 77355,
    SPELL_HOT_BOLT          = 89668,

    // Asmodina
    SPELL_FAN_OF_KNIVES     = 107105,
    SPELL_VANISH            = 96639,
    SPELL_SHADOWSTEP        = 72326,
    SPELL_SUMMON_POISON     = 56225,

    // Kelestres
    SPELL_STARFALL          = 26540,
    SPELL_COSMIC_SMASH      = 62311,
    SPELL_ARCANE_ORB        = 34172,
    SPELL_BALL_LIGHTNING    = 28299,

    // Trash

    // Mage
    SPELL_POLYMORPH         = 61305,
    SPELL_DRAGONS_BREATH    = 31661,
    SPELL_LIVING_BOMB       = 82886,
    SPELL_ENERGY_BOLT       = 44342,

    // Priest
    SPELL_SHADOWFIEND       = 34433,
    SPELL_DARK_ARCHANGEL    = 87153,
    SPELL_PENANCE           = 54518,
    SPELL_DISPERSION        = 47585,
    SPELL_FEAR              = 5782,

    // Shaman
    SPELL_CHAIN_LIGHT       = 83455,
    SPELL_HEX               = 51514,
    SPELL_FERAL_SPIRIT      = 51533,
    SPELL_THUNDERSTORM      = 51490,

    // Paladin
    SPELL_CONSECRATION      = 41541,
    SPELL_DIVINE_SHIELD     = 66010,
    SPELL_DIVINE_STORM      = 66006,

    // Warrior
    SPELL_BLADESTORM        = 78828,
    SPELL_CHARGE            = 56107,
    SPELL_LAST_STAND        = 12975,

    // Warlock
    SPELL_RAIN_OF_FIRE      = 43440,
    SPELL_DEMONIC_SHIELD    = 31901,
    SPELL_CURSE_OF_FATIGUE  = 52592,

    // Voidbest
    SPELL_VOID_BLAST        = 59128,
    SPELL_VOID_WAVE         = 63703,

    // Blob
    SPELL_VENOM_SPLASH      = 79607,
    SPELL_VENOM_SPIT        = 96415,

};

enum Events
{
    // Jinara
    EVENT_ACTION_LAND   = 0,
    EVENT_SUMMON        = 1,
    EVENT_FOLLOW_PLAYER = 2,
    EVENT_DESPAWN       = 3,
    EVENT_SHADOW_STORM  = 4,
    EVENT_BLIZZARD      = 5,
    EVENT_ICE           = 6,
    EVENT_WIND          = 7,
    EVENT_SUMMON_WALL   = 8,
    EVENT_WIND_KNOCK    = 9,
    EVENT_SHADOW_CHARGE = 10,
    EVENT_BRAIN_SPIKE   = 11,
    EVENT_CASTER_KNOCK  = 12,
    EVENT_HOT_BOLT      = 13,

    // Trash
    EVENT_LIVING_BOMB,
    EVENT_ENERGY_BOLT,
    EVENT_DRAGONS_BREATH,
    EVENT_POLYMORPH,
    EVENT_SHADOWFIEND,
    EVENT_DARK_ARCHANGEL,
    EVENT_PENANCE,
    EVENT_CHAIN_LIGHT,
    EVENT_HEX,
    EVENT_FERAL_SPIRIT,
    EVENT_THUNDERSTORM,
    EVENT_CONSECRATION,
    EVENT_DIVINE_STORM,
    EVENT_FEAR,
    EVENT_FAN_OF_KNIVES,
    EVENT_SHADOWSTEP,
    EVENT_SUMMON_POISON,
    EVENT_VANISH,
    EVENT_VENOM_SPLASH,
    EVENT_VENOM_SPIT,
    EVENT_STARFALL,
    EVENT_COSMIC_SMASH,
    EVENT_ARCANE_ORB,
    EVENT_BALL_LIGHTNING,
    EVENT_BLADESTORM,
    EVENT_WARR_CHARGE,
    EVENT_LAST_STAND,
    EVENT_RAIN_OF_FIRE,
    EVENT_DEMONIC_SHIELD,
    EVENT_CURSE_OF_FATIGUE,
    EVENT_VOID_BLAST
};

class boss_jinara : public CreatureScript
{
    public:
        boss_jinara() : CreatureScript("boss_jinara") { }

        struct boss_jinaraAI : public BossAI
        {
            boss_jinaraAI(Creature* creature) : BossAI(creature, NPC_JINARA)
            {
                me->ApplySpellImmune(0, IMMUNITY_ID, 77355, true); // dragon transform
            }

            void Reset()
            {
                events.Reset();
                RespawnBeams();
                start = false;
                rage = false;
                me->AddAura(SPELL_CHAIN, me);

                if (GameObject* wall = me->FindNearestGameObject(184320, 100.0f))
                    me->RemoveGameObject(wall, true);
            }

            void MoveInLineOfSight(Unit* who)
            {
                if (who && who->GetTypeId() == TYPEID_PLAYER && me->IsValidAttackTarget(who))

                if (!start && me->IsWithinDistInMap(who, 5.5f))
                {
                    start = true;
                    DoCast(me, SPELL_BOOM);
                    me->RemoveAura(SPELL_CHAIN);
                    DespawnBeams(NPC_JINARA_BEAM);
                    me->SetReactState(REACT_AGGRESSIVE);
                    ScriptedAI::MoveInLineOfSight(who);
                }
            }

            void JustDied(Unit* /*killer*/)
            {
                Talk(SAY_DEATH);

                if (GameObject* wall = me->FindNearestGameObject(184320, 100.0f))
                    me->RemoveGameObject(wall, true);
            }

            void DespawnBeams(uint32 entry)
            {
                std::list<Creature*> creatures;
                GetCreatureListWithEntryInGrid(creatures, me, entry, 1000.0f);
                if (creatures.empty())
                    return;
                for (std::list<Creature*>::iterator iter = creatures.begin(); iter != creatures.end(); ++iter)
                    (*iter)->DespawnOrUnsummon();
            }

            void RespawnBeams()
            {
                std::list<Creature*> creatures;
                GetCreatureListWithEntryInGrid(creatures, me, NPC_JINARA_BEAM, 100.0f);
                if (creatures.empty())
                    return;
                for (std::list<Creature*>::iterator iter = creatures.begin(); iter != creatures.end(); ++iter)
                {
                    if ((*iter)->isDead())
                        (*iter)->Respawn();
                }
            }

            void DamageTaken(Unit* /*attacker*/, uint32& /*damage*/)
            {
                if(!rage && !me->IsNonMeleeSpellCasted(false) && HealthBelowPct(50))
                {
                    rage = true;
                    events.RescheduleEvent(EVENT_SHADOW_CHARGE, 1500);
                    events.RescheduleEvent(EVENT_CASTER_KNOCK, 8000);
                }
                if(!me->IsNonMeleeSpellCasted(false) && HealthBelowPct(25))
                {
                    events.ScheduleEvent(EVENT_BRAIN_SPIKE, 3000);
                }
                if(!me->IsNonMeleeSpellCasted(false) && HealthBelowPct(10))
                {
                    events.ScheduleEvent(EVENT_HOT_BOLT, 500);
                    events.CancelEvent(EVENT_CASTER_KNOCK);
                    events.CancelEvent(EVENT_WIND_KNOCK);
                    events.CancelEvent(EVENT_BRAIN_SPIKE);
                }
            }

            void EnterCombat(Unit* /*who*/)
            {
                me->SetSpeed(MOVE_RUN, 2.1f);

                events.ScheduleEvent(EVENT_SHADOW_STORM, urand(30*IN_MILLISECONDS, 70*IN_MILLISECONDS));
                events.ScheduleEvent(EVENT_SUMMON, urand(30*IN_MILLISECONDS, 35*IN_MILLISECONDS));
                events.ScheduleEvent(EVENT_BLIZZARD, 1*MINUTE*IN_MILLISECONDS);
                events.ScheduleEvent(EVENT_ICE, 25*IN_MILLISECONDS);
                events.ScheduleEvent(EVENT_WIND, 20*IN_MILLISECONDS);
                events.ScheduleEvent(EVENT_SUMMON_WALL, 1*IN_MILLISECONDS);
                events.ScheduleEvent(EVENT_WIND_KNOCK, 12*IN_MILLISECONDS);
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
                        case EVENT_SUMMON:
                            me->SummonCreature(NPC_STORM, me->GetPositionX()+15, me->GetPositionY(), me->GetPositionZ(), float(rand()%5), TEMPSUMMON_TIMED_DESPAWN, 11000);
                            me->SummonCreature(NPC_STORM, me->GetPositionX()-15, me->GetPositionY(), me->GetPositionZ(), float(rand()%5), TEMPSUMMON_TIMED_DESPAWN, 11000);
                            me->SummonCreature(NPC_STORM, me->GetPositionX(), me->GetPositionY()+15, me->GetPositionZ(), float(rand()%5), TEMPSUMMON_TIMED_DESPAWN, 11000);
                            me->SummonCreature(NPC_STORM, me->GetPositionX(), me->GetPositionY()-15, me->GetPositionZ(), float(rand()%5), TEMPSUMMON_TIMED_DESPAWN, 11000);
                            events.ScheduleEvent(EVENT_SUMMON, urand(30*IN_MILLISECONDS, 35*IN_MILLISECONDS));
                            break;
                        case EVENT_SHADOW_STORM:
                            DoCastVictim(SPELL_SHADOW_STORM);
                            events.ScheduleEvent(EVENT_SHADOW_STORM, urand(10*IN_MILLISECONDS, 15*IN_MILLISECONDS));
                            break;
                        case EVENT_BLIZZARD:
                            DoCast(SPELL_CIRCLE_BLIZZARD);
                            events.ScheduleEvent(EVENT_BLIZZARD, 1*MINUTE*IN_MILLISECONDS);
                            break;
                        case EVENT_ICE:
                            DoCastRandom(SPELL_ICE, 10.0f);
                            events.ScheduleEvent(EVENT_ICE, urand(40*IN_MILLISECONDS, 45*IN_MILLISECONDS));
                            break;
                        case EVENT_WIND:
                            DoCastRandom(SPELL_WIND, 30.0f);
                            events.ScheduleEvent(EVENT_ICE, urand(20*IN_MILLISECONDS, 30*IN_MILLISECONDS));
                            break;
                        case EVENT_WIND_KNOCK:
                            DoCastVictim(SPELL_WIND_KNOCK);
                            events.ScheduleEvent(EVENT_WIND_KNOCK, urand(30*IN_MILLISECONDS, 35*IN_MILLISECONDS));
                            break;
                        case EVENT_SUMMON_WALL:
                            me->SummonGameObject(184320, 170.131f, -428.911f, 18.533f, 1.566390f, 0, 0, 0, 0, 0);
                            break;
                        case EVENT_SHADOW_CHARGE:
                            if (Unit* target = SelectTarget(SELECT_TARGET_FARTHEST, 0))
                                DoCast(target, SPELL_SHADOW_CHARGE);
                            events.ScheduleEvent(EVENT_SHADOW_CHARGE, urand(15*IN_MILLISECONDS, 20*IN_MILLISECONDS));
                            break;
                        case EVENT_BRAIN_SPIKE:
                            DoCast(SPELL_BRAIN_SPIKE);
                            events.ScheduleEvent(EVENT_BRAIN_SPIKE, 15000);
                            break;
                        case EVENT_CASTER_KNOCK:
                            DoCast(me, SPELL_CASTER_KNOCK);
                            events.ScheduleEvent(EVENT_CASTER_KNOCK, urand(20*IN_MILLISECONDS, 25*IN_MILLISECONDS));
                            break;
                        case EVENT_HOT_BOLT:
                            if (Unit* target = SelectTarget(SELECT_TARGET_BOTTOMAGGRO, 0)) // lets stress Healer and Lowies
                                me->AddAura(SPELL_HOT_BOLT, target);
                            break;
                        default:
                            break;
                    }
                }
                DoMeleeAttackIfReady();
            }

            private:
                EventMap events;
                bool rage;
                bool start;
        };

        CreatureAI* GetAI(Creature* creature) const
        {
            return new boss_jinaraAI (creature);
        }
};

class npc_tornado_jinara : public CreatureScript
{
    public:
        npc_tornado_jinara() : CreatureScript("npc_tornado_jinara") { }

        struct npc_tornadoAI : public ScriptedAI
        {
            npc_tornadoAI(Creature* creature) : ScriptedAI(creature)
            {
                me->SetFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_NOT_SELECTABLE | UNIT_FLAG_NON_ATTACKABLE);
            }

            void Reset()
            {
                events.Reset();
                me->AddAura(SPELL_STORM_VISUAL, me);
                me->AddAura(SPELL_LIGHTNING, me);
                DoCast(me, SPELL_STORM_EFFECT);
                me->SetSpeed(MOVE_RUN, 0.5f);
            }

            void EnterCombat(Unit* /*who*/)
            {
                events.ScheduleEvent(EVENT_FOLLOW_PLAYER, urand(10000, 15000));
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
                        case EVENT_FOLLOW_PLAYER:
                            if(Player * player = me->FindNearestPlayer(200.0f))
                               me->GetMotionMaster()->MoveFollow(player, 5, 0);
                            events.ScheduleEvent(EVENT_FOLLOW_PLAYER, urand(5000, 10000));
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
            return new npc_tornadoAI (creature);
        }
};

class mob_jinara_beam : public CreatureScript
{
public:
    mob_jinara_beam() : CreatureScript("mob_jinara_beam") { }

    struct mob_jinara_beamAI : public ScriptedAI
    {
        mob_jinara_beamAI(Creature* creature) : ScriptedAI(creature) { }

        uint32 SpellTimer;

        void Reset()
        {
            SpellTimer = 1000;
        }

        void EnterCombat(Unit* /*who*/) { }

        void UpdateAI(const uint32 diff)
        {
            if (SpellTimer <= diff)
            {
                if (Creature* target = me->FindNearestCreature(NPC_JINARA, 55.0f))
                {
                    DoCast(target, SPELL_BEAM);
                    SpellTimer = 500;
                }

             } else SpellTimer -= diff;

            if (!UpdateVictim())
                return;
        }
    };

    CreatureAI* GetAI(Creature* creature) const
    {
        return new mob_jinara_beamAI (creature);
    }
};

class npc_sm_arcane_mage : public CreatureScript
{
public:
    npc_sm_arcane_mage() : CreatureScript("npc_sm_arcane_mage") { }

    struct npc_sm_arcane_mageAI : public ScriptedAI
    {
        npc_sm_arcane_mageAI(Creature* creature) : ScriptedAI(creature) { }

        void Reset()
        {
            events.Reset();
        }

        void EnterCombat(Unit* /*who*/)
        {
            events.ScheduleEvent(EVENT_POLYMORPH, 10000);
            events.ScheduleEvent(EVENT_DRAGONS_BREATH, 15000);
            events.ScheduleEvent(EVENT_LIVING_BOMB, 8000);
            events.ScheduleEvent(EVENT_ENERGY_BOLT, 12000);
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
                case EVENT_POLYMORPH:
                    if (Unit* target = SelectTarget(SELECT_TARGET_TOPAGGRO, 0, 30.0f))
                        DoCast(target, SPELL_POLYMORPH);
                    events.ScheduleEvent(EVENT_POLYMORPH, urand(15*IN_MILLISECONDS, 20*IN_MILLISECONDS));
                    break;
                case EVENT_DRAGONS_BREATH:
                    DoCastVictim(SPELL_DRAGONS_BREATH);
                    events.ScheduleEvent(EVENT_DRAGONS_BREATH, urand(20*IN_MILLISECONDS, 25*IN_MILLISECONDS));
                    break;
                case EVENT_LIVING_BOMB:
                    DoCastRandom(SPELL_LIVING_BOMB, 35.0f);
                    events.ScheduleEvent(EVENT_LIVING_BOMB, urand(10*IN_MILLISECONDS, 15*IN_MILLISECONDS));
                    break;
                case EVENT_ENERGY_BOLT:
                    DoCast(SPELL_ENERGY_BOLT);
                    events.ScheduleEvent(EVENT_ENERGY_BOLT, urand(8*IN_MILLISECONDS, 15*IN_MILLISECONDS));
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
        return new npc_sm_arcane_mageAI (creature);
    }
};

class npc_sm_dark_priest : public CreatureScript
{
public:
    npc_sm_dark_priest() : CreatureScript("npc_sm_dark_priest") { }

    struct npc_sm_dark_priestAI : public ScriptedAI
    {
        npc_sm_dark_priestAI(Creature* creature) : ScriptedAI(creature) { }

        void Reset()
        {
            events.Reset();
            dispersion = false;
        }

        void EnterCombat(Unit* /*who*/)
        {
            events.ScheduleEvent(EVENT_SHADOWFIEND, 15000);
            events.ScheduleEvent(EVENT_DARK_ARCHANGEL, 5000);
            events.ScheduleEvent(EVENT_PENANCE, 12000);
            events.ScheduleEvent(EVENT_FEAR, 20000);
        }

        void DamageTaken(Unit* /*attacker*/, uint32& /*damage*/)
        {
            if(!dispersion && !me->IsNonMeleeSpellCasted(false) && HealthBelowPct(20))
            {
                dispersion = true;
                DoCast(me, SPELL_DISPERSION);
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
                case EVENT_FEAR:
                    DoCast(SPELL_FEAR);
                    break;
                case EVENT_DARK_ARCHANGEL:
                    DoCast(me, SPELL_DARK_ARCHANGEL);
                    break;
                case EVENT_SHADOWFIEND:
                    DoCast(SPELL_SHADOWFIEND);
                    break;
                case EVENT_PENANCE:
                    DoCastRandom(SPELL_PENANCE, 30.0f);
                    events.ScheduleEvent(EVENT_PENANCE, urand(10*IN_MILLISECONDS, 15*IN_MILLISECONDS));
                    break;
                }
            }
            DoMeleeAttackIfReady();
        }
    private:
        EventMap events;
        bool dispersion;
    };

    CreatureAI* GetAI(Creature* creature) const
    {
        return new npc_sm_dark_priestAI (creature);
    }
};

class npc_sm_paladin : public CreatureScript
{
public:
    npc_sm_paladin() : CreatureScript("npc_sm_paladin") { }

    struct npc_sm_paladinAI : public ScriptedAI
    {
        npc_sm_paladinAI(Creature* creature) : ScriptedAI(creature) { }

        void Reset()
        {
            events.Reset();
            shield = false;
        }

        void EnterCombat(Unit* /*who*/)
        {
            events.ScheduleEvent(EVENT_CONSECRATION, 10000);
            events.ScheduleEvent(EVENT_DIVINE_STORM, 5000);
        }

        void DamageTaken(Unit* /*attacker*/, uint32& /*damage*/)
        {
            if(!shield && !me->IsNonMeleeSpellCasted(false) && HealthBelowPct(15))
            {
                shield = true;
                DoCast(me, SPELL_DIVINE_SHIELD);
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
                case EVENT_CONSECRATION:
                    DoCast(me, SPELL_CONSECRATION);
                    events.ScheduleEvent(EVENT_CONSECRATION, urand(8*IN_MILLISECONDS, 12*IN_MILLISECONDS));
                    break;
                case EVENT_DIVINE_STORM:
                    DoCast(SPELL_DIVINE_STORM);
                    events.ScheduleEvent(EVENT_DIVINE_STORM, urand(10*IN_MILLISECONDS, 15*IN_MILLISECONDS));
                    break;
                }
            }
            DoMeleeAttackIfReady();
        }
    private:
        EventMap events;
        bool shield;
    };

    CreatureAI* GetAI(Creature* creature) const
    {
        return new npc_sm_paladinAI (creature);
    }
};

class npc_sm_shaman : public CreatureScript
{
public:
    npc_sm_shaman() : CreatureScript("npc_sm_shaman") { }

    struct npc_sm_shamanAI : public ScriptedAI
    {
        npc_sm_shamanAI(Creature* creature) : ScriptedAI(creature) { }

        void Reset()
        {
            events.Reset();
        }

        void EnterCombat(Unit* /*who*/)
        {
            events.ScheduleEvent(EVENT_CHAIN_LIGHT, 10000);
            events.ScheduleEvent(EVENT_HEX, 8000);
            events.ScheduleEvent(EVENT_FERAL_SPIRIT, 5000);
            events.ScheduleEvent(EVENT_THUNDERSTORM, 12000);
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
                case EVENT_FERAL_SPIRIT:
                    DoCast(me, SPELL_FERAL_SPIRIT);
                    break;
                case EVENT_THUNDERSTORM:
                    DoCast(me, SPELL_THUNDERSTORM);
                    events.ScheduleEvent(EVENT_THUNDERSTORM, urand(50*IN_MILLISECONDS, 55*IN_MILLISECONDS));
                    break;
                case EVENT_HEX:
                    DoCastRandom(SPELL_HEX, 30.0f);
                    events.ScheduleEvent(EVENT_HEX, urand(20*IN_MILLISECONDS, 25*IN_MILLISECONDS));
                    break;
                case EVENT_CHAIN_LIGHT:
                    DoCastVictim(SPELL_CHAIN_LIGHT);
                    events.ScheduleEvent(EVENT_CHAIN_LIGHT, urand(15*IN_MILLISECONDS, 25*IN_MILLISECONDS));
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
        return new npc_sm_shamanAI (creature);
    }
};

class boss_asmodina : public CreatureScript
{
public:
    boss_asmodina() : CreatureScript("boss_asmodina") { }

    struct boss_asmodinaAI : public BossAI
    {
        boss_asmodinaAI(Creature* creature) : BossAI(creature, NPC_ASMODINA) { }

        void Reset()
        {
            events.Reset();
            DespawnPoison(NPC_POISON);
            instance->HandleGameObject(instance->GetData64(GOB_ASM_DOOR), false);
        }

        void EnterCombat(Unit* /*who*/)
        {
            events.ScheduleEvent(EVENT_FAN_OF_KNIVES, 10000);
            events.ScheduleEvent(EVENT_VANISH, 15000);
            events.ScheduleEvent(EVENT_SUMMON_POISON, 25000);
        }

        void JustDied(Unit* /*killer*/)
        {
            DespawnPoison(NPC_POISON);
            instance->HandleGameObject(instance->GetData64(GOB_ASM_DOOR), true);
        }

        void DespawnPoison(uint32 entry)
        {
            std::list<Creature*> creatures;
            GetCreatureListWithEntryInGrid(creatures, me, entry, 1000.0f);
            if (creatures.empty())
                return;
            for (std::list<Creature*>::iterator iter = creatures.begin(); iter != creatures.end(); ++iter)
                (*iter)->DespawnOrUnsummon();
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
                case EVENT_FAN_OF_KNIVES:
                    DoCast(SPELL_FAN_OF_KNIVES);
                    events.ScheduleEvent(EVENT_THUNDERSTORM, urand(20*IN_MILLISECONDS, 30*IN_MILLISECONDS));
                    break;
                case EVENT_SHADOWSTEP:
                    if (Unit* target = SelectTarget(SELECT_TARGET_FARTHEST, 0))
                        DoCast(target, SPELL_SHADOWSTEP);
                    me->SetReactState(REACT_AGGRESSIVE);
                    break;
                case EVENT_SUMMON_POISON:
                    DoCast(SPELL_SUMMON_POISON);
                    events.ScheduleEvent(EVENT_SUMMON_POISON, 40*IN_MILLISECONDS);
                    break;
                case EVENT_VANISH:
                    me->getThreatManager().resetAllAggro();
                    me->SetReactState(REACT_DEFENSIVE);
                    DoCast(me, SPELL_VANISH);
                    events.ScheduleEvent(EVENT_VANISH, urand(25*IN_MILLISECONDS, 30*IN_MILLISECONDS));
                    events.ScheduleEvent(EVENT_SHADOWSTEP, 1000);
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
        return new boss_asmodinaAI (creature);
    }
};

class npc_sm_poison : public CreatureScript
{
public:
    npc_sm_poison() : CreatureScript("npc_sm_poison") { }

    struct npc_sm_poisonAI : public ScriptedAI
    {
        npc_sm_poisonAI(Creature* creature) : ScriptedAI(creature) { }

        void Reset()
        {
            events.Reset();
        }

        void EnterCombat(Unit* /*who*/)
        {
            events.ScheduleEvent(EVENT_VENOM_SPLASH, 3000);
            events.ScheduleEvent(EVENT_VENOM_SPIT, 10000);
        }

        void JustDied(Unit* /*killer*/)
        {
            me->DespawnOrUnsummon(3000);
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
                case EVENT_VENOM_SPLASH:
                    DoCastRandom(SPELL_VENOM_SPLASH, 40.0f);
                    events.ScheduleEvent(EVENT_VENOM_SPLASH, urand(20*IN_MILLISECONDS, 25*IN_MILLISECONDS));
                    break;
                case EVENT_VENOM_SPIT:
                    DoCastRandom(SPELL_VENOM_SPIT, 30.0f);
                    events.ScheduleEvent(EVENT_VENOM_SPIT, urand(15*IN_MILLISECONDS, 30*IN_MILLISECONDS));
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
        return new npc_sm_poisonAI (creature);
    }
};

class boss_kelestres : public CreatureScript
{
public:
    boss_kelestres() : CreatureScript("boss_kelestres") { }

    struct boss_kelestresAI : public BossAI
    {
        boss_kelestresAI(Creature* creature) : BossAI(creature, NPC_KELESTRES) { }

        void Reset()
        {
            events.Reset();
            instance->HandleGameObject(instance->GetData64(GOB_KEL_DOOR), false);

            if (GameObject* wallkel = me->FindNearestGameObject(184320, 100.0f))
                me->RemoveGameObject(wallkel, true);
        }

        void EnterCombat(Unit* /*who*/)
        {
            events.ScheduleEvent(EVENT_STARFALL, 20000);
            events.ScheduleEvent(EVENT_COSMIC_SMASH, 5000);
            events.ScheduleEvent(EVENT_ARCANE_ORB, 25000);
            events.ScheduleEvent(EVENT_ARCANE_ORB, 12000);

            me->SummonGameObject(184320, 264.983f, -414.829f, 20.077f, 3.12472f, 0, 0, 0, 0, 0);
        }

        void JustDied(Unit* /*killer*/)
        {
            instance->HandleGameObject(instance->GetData64(GOB_KEL_DOOR), true);
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
                case EVENT_STARFALL:
                    DoCast(me, SPELL_STARFALL);
                    events.ScheduleEvent(EVENT_STARFALL, urand(30*IN_MILLISECONDS, 35*IN_MILLISECONDS));
                    break;
                case EVENT_COSMIC_SMASH:
                    DoCastRandom(SPELL_COSMIC_SMASH, 40.0f);
                    events.ScheduleEvent(EVENT_COSMIC_SMASH, urand(20*IN_MILLISECONDS, 25*IN_MILLISECONDS));
                    break;
                case EVENT_ARCANE_ORB:
                    DoCastRandom(SPELL_ARCANE_ORB, 45.0f);
                    events.ScheduleEvent(EVENT_ARCANE_ORB, urand(25*IN_MILLISECONDS, 35*IN_MILLISECONDS));
                    break;
                case EVENT_BALL_LIGHTNING:
                    DoCastRandom(SPELL_BALL_LIGHTNING, 45.0f);
                    events.ScheduleEvent(EVENT_BALL_LIGHTNING, urand(10*IN_MILLISECONDS, 25*IN_MILLISECONDS));
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
        return new boss_kelestresAI (creature);
    }
};

class npc_sm_warrior : public CreatureScript
{
public:
    npc_sm_warrior() : CreatureScript("npc_sm_warrior") { }

    struct npc_sm_warriorAI : public ScriptedAI
    {
        npc_sm_warriorAI(Creature* creature) : ScriptedAI(creature) { }

        void Reset()
        {
            events.Reset();
            laststand = false;
        }

        void EnterCombat(Unit* /*who*/)
        {
            events.ScheduleEvent(EVENT_BLADESTORM, 10000);
            events.ScheduleEvent(EVENT_WARR_CHARGE, 8000);
        }

        void DamageTaken(Unit* /*attacker*/, uint32& /*damage*/)
        {
            if(!laststand && !me->IsNonMeleeSpellCasted(false) && HealthBelowPct(15))
            {
                laststand = true;
                DoCast(me, SPELL_LAST_STAND);
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
                case EVENT_BLADESTORM:
                    DoCast(SPELL_BLADESTORM);
                    events.ScheduleEvent(EVENT_BLADESTORM, urand(20*IN_MILLISECONDS, 35*IN_MILLISECONDS));
                    break;
                case EVENT_WARR_CHARGE:
                    DoCastRandom(SPELL_CHARGE, 30.0f);
                    events.ScheduleEvent(EVENT_WARR_CHARGE, urand(15*IN_MILLISECONDS, 25*IN_MILLISECONDS));
                    break;
                }
            }
            DoMeleeAttackIfReady();
        }
    private:
        EventMap events;
        bool laststand;
    };

    CreatureAI* GetAI(Creature* creature) const
    {
        return new npc_sm_warriorAI (creature);
    }
};

class npc_sm_warlock : public CreatureScript
{
public:
    npc_sm_warlock() : CreatureScript("npc_sm_warlock") { }

    struct npc_sm_warlockAI : public ScriptedAI
    {
        npc_sm_warlockAI(Creature* creature) : ScriptedAI(creature) { }

        void Reset()
        {
            events.Reset();
        }

        void EnterCombat(Unit* /*who*/)
        {
            events.ScheduleEvent(EVENT_RAIN_OF_FIRE, 15000);
            events.ScheduleEvent(EVENT_DEMONIC_SHIELD, 8000);
            events.ScheduleEvent(EVENT_CURSE_OF_FATIGUE, 8000);
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
                case EVENT_RAIN_OF_FIRE:
                    DoCast(SPELL_RAIN_OF_FIRE);
                    events.ScheduleEvent(EVENT_RAIN_OF_FIRE, urand(25*IN_MILLISECONDS, 30*IN_MILLISECONDS));
                    break;
                case EVENT_DEMONIC_SHIELD:
                    DoCast(me, SPELL_DEMONIC_SHIELD);
                    events.ScheduleEvent(EVENT_DEMONIC_SHIELD, 25*IN_MILLISECONDS);
                    break;
                case EVENT_CURSE_OF_FATIGUE:
                    DoCastVictim(SPELL_CURSE_OF_FATIGUE);
                    events.ScheduleEvent(EVENT_CURSE_OF_FATIGUE, 20*IN_MILLISECONDS);
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
        return new npc_sm_warlockAI (creature);
    }
};

class npc_sm_voidbeast : public CreatureScript
{
public:
    npc_sm_voidbeast() : CreatureScript("npc_sm_voidbeast") { }

    struct npc_sm_voidbeastAI : public ScriptedAI
    {
        npc_sm_voidbeastAI(Creature* creature) : ScriptedAI(creature) { }

        void Reset()
        {
            events.Reset();
        }

        void EnterCombat(Unit* /*who*/)
        {
            DoCast(me, SPELL_VOID_WAVE);
            events.ScheduleEvent(EVENT_VOID_BLAST, 5000);
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
                case EVENT_VOID_BLAST:
                    DoCast(me, SPELL_VOID_BLAST);
                    events.ScheduleEvent(EVENT_VOID_BLAST, urand(10*IN_MILLISECONDS, 15*IN_MILLISECONDS));
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
        return new npc_sm_voidbeastAI (creature);
    }
};

void AddSC_boss_jinara()
{
    new boss_jinara;
    new npc_tornado_jinara;
    new mob_jinara_beam;
    new npc_sm_arcane_mage;
    new npc_sm_dark_priest;
    new npc_sm_paladin;
    new npc_sm_shaman;
    new boss_asmodina;
    new npc_sm_poison;
    new boss_kelestres;
    new npc_sm_warrior;
    new npc_sm_warlock;
    new npc_sm_voidbeast;
}
