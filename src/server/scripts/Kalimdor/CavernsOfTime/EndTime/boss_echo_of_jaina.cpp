#include "ScriptPCH.h"
#include "end_time.h"
#include "Vehicle.h"
#include "Unit.h"
#include "ScriptMgr.h"
#include "ScriptedCreature.h"
#include "ScriptedGossip.h"
#include "ScriptedEscortAI.h"
#include "Cell.h"
#include "CellImpl.h"
#include "GridNotifiers.h"
#include "GridNotifiersImpl.h"
#include "CombatAI.h"
#include "PassiveAI.h"
#include "ObjectMgr.h"
#include "SpellInfo.h"
#include "SpellScript.h"
#include "SpellAuraEffects.h"
#include "SpellAuras.h"
#include "CreatureTextMgr.h"
#include "Vehicle.h"
#include "VehicleDefines.h"
#include "Spell.h"
#include "Player.h"
#include "Map.h"
#include "InstanceScript.h"

enum Yells
{
    SAY_AGGRO_1 = 1,
    SAY_AGGRO_2,
    SAY_FROST_BLADES, // Spell 1
    SAY_FROSTBOLT_VOLLEY, // Spell 2
    SAY_BLINK, // Spell 3
    SAY_SLAY_1,
    SAY_SLAY_2,
    SAY_SLAY_3,
    SAY_DEATH
};

enum Spells
{
    SPELL_BLINK = 101398,

    SPELL_FLARECORE_MISSILE = 101927,
    SPELL_UNSTABLE_FLARE = 101980, // When touched. Needs radius 5y.
    SPELL_TIME_EXPIRE_FLARE = 101587, // If not exploded in 10s. Needs radius 200y.
    SPELL_CHECK_PLAYER_DIST = 101588, // Periodic dummy on npc, check 500 ms.

    SPELL_FROSTBOLT_VOLLEY = 101810, // Needs radius 65y.
    SPELL_PYROBLAST = 101809,

    SPELL_FROST_BLADES = 101339, // Summon Frost Blades spell. Use with JustSummoned to send them away in a straight line, otherwise npc works ok.
    SPELL_FROST_BLADES_STUN = 101338,
};

enum Events
{
    EVENT_FLARECORE = 1, // 20 sec cd
    EVENT_BLINK, // Every 30 sec.
    EVENT_AFTER_BLINK,
    EVENT_FROSTBOLT_VOLLEY, // 3 times in succession after blink.
    EVENT_PYROBLAST, // Constantly, no melee. Every 5 to 6 sec.
    EVENT_FROST_BLADES, // 25 sec cd, just after blink.

    // FlareCore
    EVENT_CHECK_PLAYER,
    EVENT_EXPLODE

};

enum Creatures
{
    NPC_FLARECORE   = 54446,
    NPC_FROSTBLADES = 54494,
    NPC_BLINK_TARGT = 54542,
    NPC_CIRCLE_VIS  = 54639,
};

#define SAY_HELLO_CHAT "I don't know who you are, but i'll defend this shrine with my life. Leave, now, before we come to blows."

uint32 FragmentsCount = 1;
#define MAX_FRAGMENTS_COUNT 16

static const Position JainaSummonPos = {3044.697f, 515.669f, 21.460f, 2.96f};

class boss_echo_of_jaina : public CreatureScript
{
    public:
        boss_echo_of_jaina() : CreatureScript("boss_echo_of_jaina") { }

        struct boss_echo_of_jainaAI : public BossAI
        {
            boss_echo_of_jainaAI(Creature* creature) : BossAI(creature, BOSS_ECHO_OF_JAINA), summons(me)
            {
                instance = me->GetInstanceScript();
				DespawnGameobjects(209318, 1000.0f);
            }

		    void DespawnGameobjects(uint32 entry, float distance)
		    {
		    	std::list<GameObject*> gameobjects;
		    	GetGameObjectListWithEntryInGrid(gameobjects, me, entry, distance);

		    	if (gameobjects.empty())
			    	return;

		    	for (std::list<GameObject*>::iterator iter = gameobjects.begin(); iter != gameobjects.end(); ++iter)
			    	(*iter)->RemoveFromWorld();
	    	}
			
            InstanceScript* instance;
            uint32 VolleyCount;
            SummonList summons;
            EventMap events;

            void Reset()
            {
                events.Reset();
                if (instance)
                    instance->SetBossState(BOSS_ECHO_OF_JAINA, NOT_STARTED);

                VolleyCount = 0;
            }

            void EnterEvadeMode()
            {
                me->GetMotionMaster()->MoveTargetedHome();
                Reset();

                me->SetHealth(me->GetMaxHealth());

                if (instance)
                {
                    instance->SetBossState(BOSS_ECHO_OF_JAINA, FAIL);
                    instance->SendEncounterUnit(ENCOUNTER_FRAME_DISENGAGE, me); // Remove
                }

                _EnterEvadeMode();
            }

            void IsSummonedBy(Unit* summoner)
            {
                me->setFaction(35);
                me->MonsterYell(SAY_HELLO_CHAT,0,0);
                me->setFaction(16);
            }

            void JustDied(Unit* killer)
            {
                Talk(SAY_DEATH);

                if (instance)
                {
                    instance->SetData(DATA_JAINA, DONE);
                    
                    instance->SetBossState(BOSS_ECHO_OF_JAINA, DONE);
                    instance->SetData(DATA_JAINA_PICKED_STATE, DONE);
                    instance->SendEncounterUnit(ENCOUNTER_FRAME_DISENGAGE, me); // Remove
                }
            }

            void KilledUnit(Unit * /*victim*/)
            {
                Talk(RAND(SAY_SLAY_1, SAY_SLAY_2, SAY_SLAY_3));
            }

            void JustSummoned(Creature* summon)
            {
                summons.Summon(summon);
                summon->setActive(true);

                if(me->isInCombat())
                summon->AI()->DoZoneInCombat();

                switch(summon->GetEntry())
                {
                    case NPC_FROSTBLADES:
                        summon->SetReactState(REACT_PASSIVE);
                        float x, y, z;
                        summon->GetClosePoint(x, y, z, me->GetObjectSize() / 3, 100.0f);
                        summon->GetMotionMaster()->MovePoint(1, x, y, z);
                        summon->DespawnOrUnsummon(10000);
                        break;
                    case NPC_BLINK_TARGT:
                        summon->SetReactState(REACT_PASSIVE);
                        summon->GetMotionMaster()->MoveFollow(me, 15.0f, 15.0f);
                        break;
                    default:
                        break;
                }
            }

            void EnterCombat(Unit* /*who*/)
            {
                Talk(RAND(SAY_AGGRO_1, SAY_AGGRO_2));

                if (Creature* blink = me->FindNearestCreature(NPC_BLINK_TARGT, 200.0f, true))
                    blink->GetMotionMaster()->MoveFollow(me, 15.0f, 15.0f);
                else
                    me->SummonCreature(NPC_BLINK_TARGT,me->GetPositionX(),me->GetPositionY(),me->GetPositionZ());

                if (instance)
                {
                    instance->SetBossState(BOSS_ECHO_OF_JAINA, IN_PROGRESS);
                    instance->SendEncounterUnit(ENCOUNTER_FRAME_ENGAGE, me); // Add
                }

                events.ScheduleEvent(EVENT_PYROBLAST, 500);
                events.ScheduleEvent(EVENT_BLINK, urand(20000, 25000));
                events.ScheduleEvent(EVENT_FLARECORE, urand(14000, 17000));
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
                        case EVENT_PYROBLAST:
                            DoCast(me->GetVictim(), SPELL_PYROBLAST);
                            events.ScheduleEvent(EVENT_PYROBLAST, 3500);
                            break;

                        case EVENT_FLARECORE:
                            Position pos;
                            me->GetRandomNearPosition(pos, 12.0f);
                            me->CastSpell(pos.GetPositionX(),pos.GetPositionY(),pos.GetPositionZ(), SPELL_FLARECORE_MISSILE, true);
                            events.ScheduleEvent(EVENT_FLARECORE, urand(19000, 21000));
                            break;

                        case EVENT_BLINK:
                            Talk(SAY_BLINK);
                            events.CancelEvent(EVENT_PYROBLAST);
                            events.CancelEvent(EVENT_FLARECORE);
                            DoCast(me, SPELL_BLINK);
                            events.ScheduleEvent(EVENT_AFTER_BLINK, 2000);
                            break;

                        case EVENT_FROSTBOLT_VOLLEY:
                            if(VolleyCount < 3)
                            {
                                DoCast(me, SPELL_FROSTBOLT_VOLLEY);
                                events.ScheduleEvent(EVENT_FROSTBOLT_VOLLEY, 2200);
                                VolleyCount++;
                            }
                            else
                            {
                                Talk(SAY_FROSTBOLT_VOLLEY);
                                DoCast(me, SPELL_FROSTBOLT_VOLLEY);
                                events.CancelEvent(EVENT_FROSTBOLT_VOLLEY);
                                VolleyCount = 0;
                                events.ScheduleEvent(EVENT_PYROBLAST, 3500);
                                events.ScheduleEvent(EVENT_FLARECORE, urand (7500, 8500));
                            }
                            break;

                        case EVENT_FROST_BLADES:
                            Talk(SAY_FROST_BLADES);
                            me->SummonCreature(NPC_FROSTBLADES, me->GetPositionX(), me->GetPositionY(), me->GetPositionZ(), me->GetOrientation() - 0.7f, TEMPSUMMON_TIMED_DESPAWN, 7000);
                            me->SummonCreature(NPC_FROSTBLADES, me->GetPositionX(), me->GetPositionY(), me->GetPositionZ(), me->GetOrientation(), TEMPSUMMON_TIMED_DESPAWN, 7000);
                            me->SummonCreature(NPC_FROSTBLADES, me->GetPositionX(), me->GetPositionY(), me->GetPositionZ(), me->GetOrientation() + 0.7f, TEMPSUMMON_TIMED_DESPAWN, 7000);
                            break;

                        case EVENT_AFTER_BLINK:
                            if (Creature* blink = me->FindNearestCreature(NPC_BLINK_TARGT, 200.0f, true))
                            {
                                me->NearTeleportTo(blink->GetPositionX(), blink->GetPositionY(), blink->GetPositionZ(), blink->GetOrientation());
                                me->UpdatePosition(blink->GetPositionX(), blink->GetPositionY(), blink->GetPositionZ(), blink->GetOrientation());
                                Position pos;
                                me->GetRandomNearPosition(pos, urand(10,20));
                                while (blink->GetDistance(pos) < 10.0f)
                                    me->GetRandomNearPosition(pos, urand(10,20));
                                blink->GetMotionMaster()->MovePoint(0,pos);
                            }
                            events.ScheduleEvent(EVENT_FROST_BLADES, 1000);
                            events.ScheduleEvent(EVENT_FROSTBOLT_VOLLEY, 2000);
                            events.ScheduleEvent(EVENT_BLINK, 25000);
                            break;
                    }
                }
            }
        };

        CreatureAI* GetAI(Creature* creature) const
        {
            return new boss_echo_of_jainaAI(creature);
        }
};

class npc_flarecore : public CreatureScript
{
    public:
        npc_flarecore() : CreatureScript("npc_flarecore") { }

        struct npc_flarecoreAI : public ScriptedAI
        {
            npc_flarecoreAI(Creature* creature) : ScriptedAI(creature) 
            {
                instance = me->GetInstanceScript();
            }

            InstanceScript* instance;
            EventMap events;

            void Reset()
            {
                if (!me->HasAura(SPELL_CHECK_PLAYER_DIST))
                    me->AddAura(SPELL_CHECK_PLAYER_DIST, me);
                events.ScheduleEvent(EVENT_CHECK_PLAYER, 500);
                events.ScheduleEvent(EVENT_EXPLODE, 10000);
            }

            void EnterCombat(Unit* /*who*/)
            {
                me->SetReactState(REACT_PASSIVE);
            }

            void UpdateAI(uint32 const diff)
            {
                if (!UpdateVictim() || me->HasUnitState(UNIT_STATE_CASTING))
                    return;

                events.Update(diff);

                while (uint32 eventId = events.ExecuteEvent())
                {
                    switch (eventId)
                    {
                        case EVENT_CHECK_PLAYER:
                            if(me->FindNearestPlayer(2.0f, true))
                            {
                                DoCast(me, SPELL_UNSTABLE_FLARE);
                                me->DespawnOrUnsummon();
                            }
                            events.ScheduleEvent(EVENT_CHECK_PLAYER, 500);
                            break;

                        case EVENT_EXPLODE:
                            DoCast(me, SPELL_TIME_EXPIRE_FLARE);
                            me->DespawnOrUnsummon(500);
                            break;
                    }
                }
            }
        };

        CreatureAI* GetAI(Creature* creature) const
        {
            return new npc_flarecoreAI(creature);
        }
};

class jaina_frost_blades : public CreatureScript
{
    public:
        jaina_frost_blades() : CreatureScript("jaina_frost_blades") { }

        struct jaina_frost_bladesAI : public ScriptedAI
        {
            jaina_frost_bladesAI(Creature* creature) : ScriptedAI(creature) 
            {
                instance = me->GetInstanceScript();
                me->SetFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_NON_ATTACKABLE | UNIT_FLAG_NOT_SELECTABLE);
                me->SetReactState(REACT_PASSIVE);
            }

            InstanceScript* instance;

            void Reset ()
            {
                DoCast(me, SPELL_FROST_BLADES_STUN);
            }

            void UpdateAI(uint32 const diff)
            {
                float x, y, z;
                me->GetClosePoint(x, y, z, me->GetObjectSize() / 3);
                me->GetMotionMaster()->MovePoint(0, x, y, z);
            }
        };

        CreatureAI* GetAI(Creature* creature) const
        {
            return new jaina_frost_bladesAI(creature);
        }
};

class go_jaina_staff_fragment : public GameObjectScript
{
    public:
        go_jaina_staff_fragment() : GameObjectScript("go_jaina_staff_fragment") { }

        bool OnGossipHello(Player* player, GameObject* go)
        {
            InstanceScript* instance = go->GetInstanceScript();

            if (FragmentsCount < MAX_FRAGMENTS_COUNT)
            {
                instance->DoUpdateWorldState(WORLDSTATE_FRAGMENTS_COLLECTED, FragmentsCount);
                FragmentsCount++;
            }
            else // Fragments reach 16.
            {
                instance->DoUpdateWorldState(WORLDSTATE_FRAGMENTS_COLLECTED, MAX_FRAGMENTS_COUNT);
                instance->instance->SummonCreature(NPC_ECHO_OF_JAINA, JainaSummonPos);
            }
            player->PlayerTalkClass->ClearMenus();
            go->SetFlag(GAMEOBJECT_FLAGS, GO_FLAG_IN_USE);
            go->Delete();
            return true;
        }
};

void AddSC_boss_echo_of_jaina()
{
    new boss_echo_of_jaina();
    new npc_flarecore();
    new go_jaina_staff_fragment();
    new jaina_frost_blades();
}
