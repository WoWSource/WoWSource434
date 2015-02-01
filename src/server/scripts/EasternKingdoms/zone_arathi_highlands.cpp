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

/* ScriptData
SDName: Arathi Highlands
SD%Complete: 100
SDComment: Quest support: 665
SDCategory: Arathi Highlands
EndScriptData */

/* ContentData
npc_professor_phizzlethorpe
EndContentData */

#include "ScriptMgr.h"
#include "ScriptedCreature.h"
#include "ScriptedEscortAI.h"
#include "Player.h"

/*######
## npc_professor_phizzlethorpe
######*/

enum eEnums
{
    SAY_PROGRESS_1          = 0,
    SAY_PROGRESS_2          = 1,
    SAY_PROGRESS_3          = 2,
    EMOTE_PROGRESS_4        = 3,
    SAY_AGGRO               = 4,
    SAY_PROGRESS_5          = 5,
    SAY_PROGRESS_6          = 6,
    SAY_PROGRESS_7          = 7,
    EMOTE_PROGRESS_8        = 8,
    SAY_PROGRESS_9          = 9,

    QUEST_SUNKEN_TREASURE   = 665,
    QUEST_GOOGLE_BOGGLE     = 26050,
    MOB_VENGEFUL_SURGE      = 2776
};

class npc_professor_phizzlethorpe : public CreatureScript
{
    public:

        npc_professor_phizzlethorpe() : CreatureScript("npc_professor_phizzlethorpe") {}

        struct npc_professor_phizzlethorpeAI : public npc_escortAI
        {
            npc_professor_phizzlethorpeAI(Creature* creature) : npc_escortAI(creature) {}

            void WaypointReached(uint32 PointId)
            {
                Player* player = GetPlayerForEscort();

                if (!player)
                    return;

                switch (PointId)
                {
                case 4:Talk(SAY_PROGRESS_2);break;
                case 5:Talk(SAY_PROGRESS_3);break;
                case 8:Talk(EMOTE_PROGRESS_4);break;
                case 9:
                    {
                    me->SummonCreature(MOB_VENGEFUL_SURGE, -2052.96f, -2142.49f, 20.15f, 1.0f, TEMPSUMMON_CORPSE_DESPAWN, 0);
                    me->SummonCreature(MOB_VENGEFUL_SURGE, -2052.96f, -2142.49f, 20.15f, 1.0f, TEMPSUMMON_CORPSE_DESPAWN, 0);
                    break;
                    }
                case 10:Talk(SAY_PROGRESS_5);break;
                case 11:
                    Talk(SAY_PROGRESS_6);
                    SetRun();
                    break;
                case 19:Talk(SAY_PROGRESS_7); break;
                case 20:
                    Talk(EMOTE_PROGRESS_8);
                    Talk(SAY_PROGRESS_9);
                    if (player)
                        CAST_PLR(player)->GroupEventHappens(QUEST_GOOGLE_BOGGLE, me);
                    break;
                }
            }

            void JustSummoned(Creature* summoned)
            {
                summoned->AI()->AttackStart(me);
            }

            void EnterCombat(Unit* /*who*/)
            {
                Talk(SAY_AGGRO);
            }

            void UpdateAI(const uint32 diff)
            {
                npc_escortAI::UpdateAI(diff);
            }
        };

        CreatureAI* GetAI(Creature* creature) const
        {
            return new npc_professor_phizzlethorpeAI(creature);
        }

        bool OnQuestAccept(Player* player, Creature* creature, Quest const* quest)
        {
            if (quest->GetQuestId() == QUEST_GOOGLE_BOGGLE)
            {
                creature->AI()->Talk(SAY_PROGRESS_1);
                if (npc_escortAI* escortAI = CAST_AI(npc_professor_phizzlethorpeAI, (creature->AI())))
                    escortAI->Start(false, false, player->GetGUID(), quest);

                creature->setFaction(113);
            }
            return true;
        }
};

float VictimsWaves[3][5]=
{
    {2775, -2144.64f, -1985.12f, 11.74f, 5.54f},
    {2775, -2148.25f, -1990.27f, 13.60f, 5.76f},
    {2775, -2140.80f, -1980.00f, 11.46f, 5.51f}
};

enum eShakes
{
    QUEST_DEATH_FROM_BELOW = 26628
};
class npc_shakes : public CreatureScript
{
public:
    npc_shakes() : CreatureScript("npc_shakes") { }

    bool OnQuestAccept(Player* pPlayer, Creature* pCreature, Quest const *pQuest)
    {
        if (pQuest->GetQuestId() == QUEST_DEATH_FROM_BELOW)
        {
            if(!(CAST_AI(npc_shakes::npc_shakesAI, pCreature->AI())->EventStarted))
            {
                    CAST_AI(npc_shakes::npc_shakesAI, pCreature->AI())->EventStarted = true;
                    CAST_AI(npc_shakes::npc_shakesAI, pCreature->AI())->PlayerGUID = pPlayer->GetGUID(); 
            }

		}return true;
	}

        CreatureAI* GetAI(Creature* pCreature) const
        {
            return new npc_shakesAI (pCreature);
        }

        struct npc_shakesAI : public ScriptedAI
        {
            npc_shakesAI(Creature* creature) : ScriptedAI(creature) {}
		
			uint8 Phase;
			uint32 EventTimer;
			uint64 PlayerGUID;
			bool EventStarted;
			bool Say;

			void Reset()
			{
				EventStarted = false;
				Say=false;
				EventTimer = 1000;
				Phase = 0;
				PlayerGUID = 0;
				me->GetMotionMaster()->MoveTargetedHome();
			}
			void DamageTaken(Unit * pWho, uint32 &uiDamage)
			{
				if (pWho->GetEntry()==2775)
				{
					me->AI()->AttackStart(pWho);
				}
			}

			void SummonInvaders()
			{ 
				for (int i = 0; i < 3; i++)
                {
                    me->SummonCreature(VictimsWaves[i][1], VictimsWaves[i][2], VictimsWaves[i][3], VictimsWaves[i][4], VictimsWaves[i][5], TEMPSUMMON_CORPSE_DESPAWN, 0);
                }
            }

            void UpdateAI(const uint32 diff)
            {
                if(EventStarted)
                {
                    if(EventTimer < diff)
                    {
                        if(Player* pPlayer = me->GetPlayer(*me, PlayerGUID))
                        {
                            switch(Phase)
                            {
                                case 0: me->MonsterYell("All hands to battle stations! Naga incoming!",0,0); EventTimer = 15000; Phase++; break;
                                case 1: SummonInvaders(); EventTimer = 1000; Phase++; break;
                                case 2:
                                {
                                    if (Creature* Naga = me->FindNearestCreature(2775,100.0f,true))
                                    {
                                        if(!Say)
                                        {
                                            Naga->MonsterYell("You've plundered our treasures too long. Prepare to meet your watery grave!",0,0);
                                            Say=true;
                                        }
                                    }
                                    EventTimer=15000;
									Phase++;
                                }break;
                                case 3: me->MonsterSay("If we can just hold them now, I am sure we will be in the clear.",0,0); EventTimer = 20000; Phase++; break;
                                case 4: SummonInvaders(); EventTimer = 10000; Phase++; break;
                                case 5:
                                {
                                    if(Creature* Naga = me->FindNearestCreature(2775,100.0f,true))
                                        Naga->MonsterYell("Nothing will stop us! You will die!",0,0);											

                                    Phase++;
									EventTimer=5000;
                                }break;
                                case 6:
                                {
                                    std::list<Creature*> NagaList;
                                    me->GetCreatureListWithEntryInGrid(NagaList, 2775, 60.0f);
                                    if (NagaList.empty())
                                    {
                                        Phase++;
                                        EventTimer=2000;
                                    }
                                }break;	
                                case 7: 
                                {
                                    if(pPlayer)
                                        pPlayer->AreaExploredOrEventHappens(26628);
                                    else
                                    {
                                        std::list<Player*> players;

                                        Trinity::AnyPlayerInObjectRangeCheck checker(me, 35.0f);
                                        Trinity::PlayerListSearcher<Trinity::AnyPlayerInObjectRangeCheck> searcher(me, players, checker);
                                        me->VisitNearbyWorldObject(35.0f, searcher);

                                        for (std::list<Player*>::const_iterator itr = players.begin(); itr != players.end(); ++itr)
                                            (*itr)->AreaExploredOrEventHappens(26628);
                                    }
                                    Reset();
                                }break;
                                default: break;
                            }
                        }
                    }else EventTimer -= diff;
                }
                DoMeleeAttackIfReady();
            }
        };
};
                            
class npc_daggerspine : public CreatureScript
{
public:
    npc_daggerspine() : CreatureScript("npc_daggerspine") { }

    CreatureAI* GetAI(Creature* pCreature) const
    {
          return new npc_daggerspineAI (pCreature);
    }
 
    struct npc_daggerspineAI  : public ScriptedAI
    {
        npc_daggerspineAI(Creature *c) : ScriptedAI(c) {}

        void Reset()
        {
            if (Creature* Shakes = me->FindNearestCreature(2610,100.0f,true))
            {
                me->Attack(Shakes,true);
                me->SetReactState(REACT_AGGRESSIVE);
                me->GetMotionMaster()->MoveCharge(Shakes->GetPositionX(),Shakes->GetPositionY(),Shakes->GetPositionZ(),5);
            }
        }	

        void UpdateAI(const uint32 diff)
        {
            DoMeleeAttackIfReady();
        }
    };
};

/*######
## npc_kinelory
######*/

enum eKinelory
{
    SAY_START		= 0,
    SAY_POINT_1     = 1,
    SAY_ON_AGGRO    = 2,
    SAY_PROFESOR    = 3,
    SAY_HOUSE_1     = 4,
    SAY_HOUSE_2     = 5,
    SAY_HOUSE_3		= 6,
    SAY_END			= 7,

    QUEST_KINELORY_STRIKES     = 26116,
    NPC_HAMMERFALL_GRUNT       = 2619
};

class npc_kinelory : public CreatureScript
{
    public:

        npc_kinelory(): CreatureScript("npc_kinelory") {}

        struct npc_kineloryAI : public npc_escortAI
        {
            npc_kineloryAI(Creature *c) : npc_escortAI(c) {}

            void Reset()
            {
                me->SetRespawnTime(10);
            }
            void WaypointReached(uint32 uiPointId)
            {
                Player* pPlayer = GetPlayerForEscort();

                if (!pPlayer)
                    return;

                switch(uiPointId)
                {
                case 9:Talk(SAY_POINT_1);break;
                case 10:
                {
                    me->SummonCreature(NPC_HAMMERFALL_GRUNT, -1459.62f, -3019.78f, 11.8f, 5.08f, TEMPSUMMON_CORPSE_DESPAWN, 0);
                    me->SummonCreature(NPC_HAMMERFALL_GRUNT, -1471.77f, -3032.95f, 12.39f, 6.03f, TEMPSUMMON_CORPSE_DESPAWN, 0);
                    break;
                }
                case 11:
                {
                    me->SummonCreature(NPC_HAMMERFALL_GRUNT, -1517.95f, -3023.16f, 12.93f, 4.79f, TEMPSUMMON_CORPSE_DESPAWN, 0);
                    me->SummonCreature(NPC_HAMMERFALL_GRUNT, -1522.69f, -3023.90f, 12.45f, 5.12f, TEMPSUMMON_CORPSE_DESPAWN, 0);
                    break;
                }
                case 13:
                {
                    if(Creature* Jorell = me->FindNearestCreature(2733,8.0f,true))
                    {
                        Jorell->MonsterSay("You will never stop the Forsaken, Kinelory. The Dark Lady shall make you suffer.",0,0);
                    }
                    break;
                }              
                case 14:Talk(SAY_HOUSE_1);break;
                case 15:Talk(SAY_HOUSE_2); break;
                case 16:Talk(SAY_HOUSE_3); break;
                case 17:SetRun(); break;
                case 27:
                    Talk(SAY_END);
                    if (pPlayer)
                        CAST_PLR(pPlayer)->GroupEventHappens(QUEST_KINELORY_STRIKES, me);
                    Reset();
                    break;
                }
            }

            void JustSummoned(Creature* pSummoned)
            {
                pSummoned->AI()->AttackStart(me);
            }

            void EnterCombat(Unit* /*pWho*/)
            {
                Talk(SAY_ON_AGGRO);
            }

            void UpdateAI(const uint32 diff)
            {
                npc_escortAI::UpdateAI(diff);
            }
        };

        CreatureAI* GetAI(Creature* creature) const
        {
            return new npc_kineloryAI(creature);
        }

        bool OnQuestAccept(Player* player, Creature* creature, Quest const* quest)
        {
            if (quest->GetQuestId() == QUEST_KINELORY_STRIKES)
            {
                creature->AI()->Talk(SAY_START);
                if (npc_escortAI* pEscortAI = CAST_AI(npc_kineloryAI, (creature->AI())))
                    pEscortAI->Start(false, false, player->GetGUID(), quest);
            }
            return true;
        }
};

enum eMyzrael
{
    SPELL_EARTHQUAKE			=	4938,
    SPELL_PRISMATIC_EXILE		=   10388
};

class npc_myzrael : public CreatureScript
{
public:
    npc_myzrael() : CreatureScript("npc_myzrael") { }

    CreatureAI* GetAI(Creature* pCreature) const
    {
          return new npc_myzraelAI (pCreature);
    }
 
    struct npc_myzraelAI  : public ScriptedAI
    {
        npc_myzraelAI(Creature *c) : ScriptedAI(c) {}

        uint8 Phase;
        uint32 StateTimer;
        uint32 ElementalTimer;
        uint32 QuakeTimer;
        uint64 PlayerGUID;

        bool MakeAggressive;

        void Reset()
        {
            Phase = 0;
            StateTimer = 1000;
            ElementalTimer = 10000;
            QuakeTimer = 6000;
            PlayerGUID = 0;
            me->SetReactState(REACT_PASSIVE);
        }

        void UpdateAI(const uint32 diff)
        {
                if(StateTimer <= diff)
                {
                    if(Player* pPlayer = me->FindNearestPlayer(25.0f,true))
                    {
                        switch(Phase)
                        {
                            case 0: me->MonsterSay("What? %N, you served me well, but ...",0,0); StateTimer=4000; Phase++; break;
                            case 1: me->MonsterSay("Why have you summoned me so soon? I haven't yet reached my full power!",0,0); StateTimer=5000; Phase++; break;
                            case 2:
                            {
                                me->MonsterSay("No matter. You were a fool to help me, and now you will pay!",0,0);
                                me->SetReactState(REACT_AGGRESSIVE); 
                                me->Attack(pPlayer,true);
                                Phase++;
                            }break;
                            default: break;
                        }

                        if(QuakeTimer <= diff)
                        {
                            me->CastSpell(me->GetVictim(),4938,true);
                            QuakeTimer = 10000;
                        }else QuakeTimer -= diff;

                        if(ElementalTimer <= diff)
                        {
                            me->CastSpell(me->GetVictim(),10388,true);
                            ElementalTimer = 15000;
                        }else ElementalTimer -= diff;
                    }
                }else StateTimer -= diff;

            DoMeleeAttackIfReady();
        }
    };
};


void AddSC_arathi_highlands()
{
    new npc_professor_phizzlethorpe();
    new npc_shakes();
    new npc_daggerspine();
    new npc_kinelory();
    new npc_myzrael();
}
