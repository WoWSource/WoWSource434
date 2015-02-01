/*
 * Copyright (C) 2013 WoW Source  <http://wowsource.info/>
 *
 * Copyright (C) 2013 WoWSource [WS] <http://wowsource.info/>
 *
 * Dont Share The SourceCode
 * and read our WoWSource Terms
 *
 */

#include "ScriptMgr.h"
#include "ScriptedCreature.h"
#include "ScriptedGossip.h"
#include "Player.h"
#include "nexus.h"

enum Spells
{
    SPELL_TRANSFORM_VISUAL = 74620,
    SPELL_INVISIBLE        = 99277,
    SPELL_ARCANE_MISSILE   = 99283,
    SPELL_FROST_BOLT       = 79858,
    SPELL_ICE_BLOCK        = 99247,

    // Trash
    SPELL_ERUPTING_ICE     = 99407,
    SPELL_RETURN_FIRE      = 99813,
};

enum Yells
{
    SAY_FOLLOW     = 0,
    SAY_LETS_GO    = 1,
    SAY_GUARD      = 2,
    SAY_GOAHEAD    = 3,
    SAY_PRISON     = 4,

    TEXT_ID_BEGINN = 53439
};

enum Events
{
    EVENT_START              = 1,
    EVENT_TRANSFORM_BACK     = 2,
    EVENT_START_WALK         = 3,
    EVENT_CAST_AOE           = 4,
    EVENT_STEP_TWO           = 5,
    EVENT_ARCANE_MISSILE     = 6,
    EVENT_FROSTBOLT          = 7,
    EVENT_ERUPTING_ICE       = 8,
    EVENT_RETURN_FIRE        = 9,
    EVENT_STEP_THREE         = 10,
    EVENT_ICE_BEAM_1         = 11,
    EVENT_ICE_BEAM_2         = 12,
    EVENT_ICE_BEAM_3         = 13,
    EVENT_FROST_FLOOR        = 14,
    EVENT_FROST_FLOOR_1      = 15,
    EVENT_CHECK_NEXT_TARGETS = 16,
    EVENT_RESET              = 17,
};

enum Actions
{
    ACTION_START_EVENT = 0,
    ACTION_GO_AHEAD    = 1
};

enum Misc
{
    DISPLAYID_TAREC_ELF = 38397
};

#define GOSSIP_ACTIVATE_EVENT "Welchen Weg sollen wir nehmen?"

const Position TarecgosaNexusRoutes[] =
{
    {235.639f, 29.610f, -16.636f, 0.771114f},  // Start Point
    {291.915f, 86.266f, -17.083f, 0.783231f},
    {180.484f, -24.843f, -16.636f, 3.920141f}, // Warden
    {302.448f, 124.416f, -16.636f, 1.560895f}, // go ahead event
    {303.494f, 140.082f, -17.942f, 1.144634f},
    {315.800f, 157.279f, -23.209f, 0.304258f},
    {337.125f, 158.685f, -28.677f, 6.230087f},
    {377.770f, 155.041f, -35.019f, 0.667356f},
    {413.472f, 185.902f, -35.019f, 6.231903f},
};

class npc_tarecgosa_nexus : public CreatureScript
{
    public:
        npc_tarecgosa_nexus() : CreatureScript("npc_tarecgosa_nexus") { }

    bool OnGossipSelect(Player* player, Creature* creature, uint32 /*sender*/, uint32 action)
    {
        player->PlayerTalkClass->ClearMenus();
        if (action == GOSSIP_ACTION_INFO_DEF+1 || action == GOSSIP_ACTION_INFO_DEF+2)
        {
            player->CLOSE_GOSSIP_MENU();
            CAST_AI(npc_tarecgosa_nexus::npc_tarecgosa_nexusAI, creature->AI())->StartEvent();
        }

        return true;
    }

    bool OnGossipHello(Player* player, Creature* creature)
    {
        if (creature->IsQuestGiver())
            player->PrepareQuestMenu(creature->GetGUID());

        player->ADD_GOSSIP_ITEM(GOSSIP_ICON_CHAT, GOSSIP_ACTIVATE_EVENT, GOSSIP_SENDER_MAIN, GOSSIP_ACTION_INFO_DEF+1);
        player->SEND_GOSSIP_MENU(TEXT_ID_BEGINN, creature->GetGUID());

        return true;
    }

    struct npc_tarecgosa_nexusAI : public ScriptedAI
    {
        npc_tarecgosa_nexusAI(Creature* creature) : ScriptedAI(creature), _EventInProgress(false) { }

        void Reset()  { }

        void JustDied(Unit* /*Killer*/)  { }

        void StartEvent()
        {
             _EventInProgress = true;
            DoAction(ACTION_START_EVENT);
            me->RemoveFlag(UNIT_NPC_FLAGS, UNIT_NPC_FLAG_GOSSIP|UNIT_NPC_FLAG_QUESTGIVER);
        }

        void DoAction(int32 const action)
        {
            switch (action)
            {
                case ACTION_START_EVENT:
                    _EventInProgress = true;
                    me->setActive(true);
                    events.ScheduleEvent(EVENT_START, 2000);
                    break;
                case ACTION_GO_AHEAD:
                    _EventInProgress = true;
                    me->setActive(true);
                    events.CancelEvent(EVENT_RESET);
                    events.ScheduleEvent(EVENT_STEP_THREE, 1000);
                    break;
                default:
                    break;
            }
        }

        void UpdateAI(const uint32 diff)
        {
            if(!UpdateVictim() && !_EventInProgress)
                return;

            events.Update(diff);

            if(me->HasUnitState(UNIT_STATE_CASTING))
                return;

            if(uint32 eventId = events.ExecuteEvent())
            {
                switch(eventId)
                {
                case EVENT_START:
                    Talk(SAY_FOLLOW);
                    events.ScheduleEvent(EVENT_TRANSFORM_BACK, 5000);
                    break;
                case EVENT_TRANSFORM_BACK: // This Event here is a little bit Hackhish, because there is no transformation Spell for her
                    DoCast(SPELL_TRANSFORM_VISUAL);
                    me->SetDisplayId(DISPLAYID_TAREC_ELF);
                    events.ScheduleEvent(EVENT_START_WALK, 2000);
                    break;
                case EVENT_START_WALK:
                    Talk(SAY_LETS_GO);
                    me->GetMotionMaster()->MovePoint(0, TarecgosaNexusRoutes[0]);
                    events.ScheduleEvent(EVENT_CAST_AOE, 6000);
                    break;
                case EVENT_CAST_AOE:
                    Talk(SAY_GUARD);
                    DoCast(SPELL_INVISIBLE);
                    me->SummonCreature(NPC_NEXUS_WARDEN, 322.338f, 115.080f, -16.636f, 3.920141f, TEMPSUMMON_TIMED_DESPAWN_OUT_OF_COMBAT, 20000);
                    events.ScheduleEvent(EVENT_STEP_TWO, 6200);
                    break;
                case EVENT_STEP_TWO:
                    me->GetMotionMaster()->MovePoint(0, TarecgosaNexusRoutes[1]);
                    me->SetSpeed(MOVE_RUN, 0.5f);
                    events.ScheduleEvent(EVENT_ARCANE_MISSILE, 22800);
                    break;
               case EVENT_ARCANE_MISSILE:
                   if (Creature* sentinel = me->FindNearestCreature(NPC_ICEBOUND_SENTINEL, 60.5f))
                   {
                      me->SetInCombatWith(sentinel);
                      DoCast(sentinel, SPELL_ARCANE_MISSILE, false);
                   }
                   events.ScheduleEvent(EVENT_FROSTBOLT, 3200);
                   break;
               case EVENT_FROSTBOLT:
                   if (Creature* sentinel = me->FindNearestCreature(NPC_ICEBOUND_SENTINEL, 60.5f))
                       DoCast(sentinel, SPELL_FROST_BOLT, false);
                   events.ScheduleEvent(EVENT_RESET, 320000);
                   break;
               case EVENT_STEP_THREE:
                   Talk(SAY_GOAHEAD);
                   me->GetMotionMaster()->MovePoint(0, TarecgosaNexusRoutes[3]);
                   me->SetSpeed(MOVE_RUN, 1.0f);
                   events.ScheduleEvent(EVENT_ICE_BEAM_1, 5000);
                   break;
               case EVENT_ICE_BEAM_1:
                   me->GetMotionMaster()->MovePoint(0, TarecgosaNexusRoutes[4]);
                   events.ScheduleEvent(EVENT_ICE_BEAM_2, 5000);
                   break;
               case EVENT_ICE_BEAM_2:
                   me->GetMotionMaster()->MovePoint(0, TarecgosaNexusRoutes[5]);
                   events.ScheduleEvent(EVENT_ICE_BEAM_3, 5000);
                   break;
               case EVENT_ICE_BEAM_3:
                   me->GetMotionMaster()->MovePoint(0, TarecgosaNexusRoutes[6]);
                   events.ScheduleEvent(EVENT_FROST_FLOOR, 5000);
                   break;
               case EVENT_FROST_FLOOR:
                   me->GetMotionMaster()->MovePoint(0, TarecgosaNexusRoutes[7]);
                   events.ScheduleEvent(EVENT_FROST_FLOOR_1, 5000);
                   break;
               case EVENT_FROST_FLOOR_1:
                   me->GetMotionMaster()->MovePoint(0, TarecgosaNexusRoutes[8]);
                   events.ScheduleEvent(EVENT_CHECK_NEXT_TARGETS, 5000);
                   break;
               case EVENT_CHECK_NEXT_TARGETS:
                   Talk(SAY_PRISON);
                   if(Unit* prisonplayer = me->FindNearestPlayer(500.0f))
                       DoCast(prisonplayer, SPELL_ICE_BLOCK);
                   events.ScheduleEvent(EVENT_RESET, 320000);
                   break;
               case EVENT_RESET:
                   _EventInProgress = false;
                   me->DespawnOrUnsummon(2000);
                   break;
                }
            }
            DoMeleeAttackIfReady();
        }
    private:
        EventMap events;
        bool _EventInProgress;
    };

    CreatureAI* GetAI(Creature* creature) const 
    {
        return new npc_tarecgosa_nexusAI(creature);
    }
};

class npc_nexus_warden : public CreatureScript
{
public:
    npc_nexus_warden() : CreatureScript("npc_nexus_warden") { }

    struct npc_nexus_wardenAI : public ScriptedAI
    {
        npc_nexus_wardenAI(Creature* creature) : ScriptedAI(creature) { }

        void Reset()
        {
            me->SetReactState(REACT_DEFENSIVE);
        }

        void IsSummonedBy(Unit* summoner)
        {
            me->GetMotionMaster()->MovePoint(0, TarecgosaNexusRoutes[2]);
        }
    };

    CreatureAI* GetAI(Creature* creature) const
    {
        return new npc_nexus_wardenAI (creature);
    }
};

class npc_icebound_sentinel : public CreatureScript
{
public:
    npc_icebound_sentinel() : CreatureScript("npc_icebound_sentinel") { }

    struct npc_icebound_sentinelAI : public ScriptedAI
    {
        npc_icebound_sentinelAI(Creature* creature) : ScriptedAI(creature) { }

        void Reset()
        {
            me->SetReactState(REACT_AGGRESSIVE);
        }

        void EnterCombat(Unit* /*who*/)
        {
            events.ScheduleEvent(EVENT_ERUPTING_ICE, 10000);
            events.ScheduleEvent(EVENT_RETURN_FIRE, 3000);

            if (Creature* tarecgosa = me->FindNearestCreature(NPC_TARECGOSA, 60.0f, true))
            {
                AttackStart(tarecgosa);
                me->getThreatManager().addThreat(tarecgosa, 100.0f);
                me->getThreatManager().modifyThreatPercent(tarecgosa, -100);
                me->SetInCombatWith(tarecgosa);
            }
        }

        void JustDied(Unit* /*Killer*/)
        {
            if (Creature* tarecgosa = me->FindNearestCreature(NPC_TARECGOSA, 60.0f, true))
                tarecgosa->AI()->DoAction(ACTION_GO_AHEAD);
        }

        void UpdateAI(const uint32 diff)
        {
            if(!UpdateVictim())
                return;

            events.Update(diff);

            if(me->HasUnitState(UNIT_STATE_CASTING))
                return;

            if (Creature* tarecgosa = me->FindNearestCreature(NPC_TARECGOSA, 34))
                AttackStart(tarecgosa);

            if(uint32 eventId = events.ExecuteEvent())
            {
                switch(eventId)
                {
                case EVENT_ERUPTING_ICE:
                    DoCastRandom(SPELL_ERUPTING_ICE, 30.0f);
                    events.ScheduleEvent(EVENT_ERUPTING_ICE, urand(10*IN_MILLISECONDS, 15*IN_MILLISECONDS));
                    break;
                case EVENT_RETURN_FIRE:
                    DoCast(me, SPELL_RETURN_FIRE);
                    events.ScheduleEvent(EVENT_RETURN_FIRE, urand(25*IN_MILLISECONDS, 30*IN_MILLISECONDS));
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
        return new npc_icebound_sentinelAI (creature);
    }
};

void AddSC_nexus()
{
    new npc_tarecgosa_nexus();
    new npc_nexus_warden();
    new npc_icebound_sentinel();
}
