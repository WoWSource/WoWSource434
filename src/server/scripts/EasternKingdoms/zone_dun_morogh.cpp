
#include "ScriptPCH.h"
#include "Unit.h"
#include "ScriptedEscortAI.h"
#include "Vehicle.h"

/*
    Gnome Starting Zone
*/

enum RandomQuests
{
    SAY_CONTAMINATION_START     = 0,
    SAY_CONTAMINATION_3         = 1,
    SAY_CONTAMINATION_OVERLOAD  = 2,
    SAY_MINDLESS_LEPER          = 0,
    SAY_HARD_TIME               = 0,
    SAY_MULTIBOT_0              = 0,
    SAY_MULTIBOT_1              = 1,
    SAY_MULTIBOT_2              = 2,
    SAY_MULTIBOT_3              = 3,

    SAY_UGH_NOT_THIS            = 0,
    SAY_OH_NO                   = 1,
  
    QUEST_PINNED_DOWN           = 27670,
    QUEST_STAGING_IN_BREWNALL   = 26339,
    QUEST_JOB_FOR_BOT           = 26205,
};

class DistanceSelector
{
    public:
        DistanceSelector(Unit* source, uint32 const distance) : _source(source), _distance(distance) {}

        bool operator()(Creature* creature)
        {
            return _source->GetDistance(creature) > _distance;
        }

    private:
        Unit* _source;
        uint32 const _distance;
};

/*######
## npc_sanitron500
######*/

/*
 *  @Npc   : Sanitron 500 (46185)
 *  @Quest : Decontamination (27635)
 *  @Descr : Board the Sanitron 500 to begin the decontamination process.
 */
enum eSanitron
{
    SPELL_CANNON_BURST          = 86080,
    SPELL_DECONTAMINATE_STAGE_1 = 86075,
    SPELL_DECONTAMINATE_STAGE_2 = 86086,
    SPELL_IRRADIATE             = 80653,

    SPELL_EXPLOSION             = 30934,	

    QUEST_DECONTAMINATION       = 27635,
	
    NPC_DECONTAMINATION_BUNNY   = 46165,
    NPC_CLEAN_CANNON            = 46208,
    NPC_SAFE_TECHNICAN          = 46230
};

class npc_sanitron500 : public CreatureScript
{
public:
    npc_sanitron500() : CreatureScript("npc_sanitron500") {}

    bool OnGossipHello(Player* pPlayer, Creature* pCreature)
    {
        QuestStatus status = pPlayer->GetQuestStatus(QUEST_DECONTAMINATION);
        if (status == QUEST_STATUS_INCOMPLETE)
        {
            pPlayer->HandleEmoteCommand(0);
            Vehicle *vehicle = pCreature->GetVehicleKit();
            pPlayer->EnterVehicle(pCreature->ToUnit(), 0);
            pCreature->MonsterSay("Commencing decontamination sequence...", LANG_UNIVERSAL, 0);
        }
        return true;
    }

    struct npc_sanitron500AI : public ScriptedAI
    {
        npc_sanitron500AI(Creature* pCreature) : ScriptedAI(pCreature), vehicle(pCreature->GetVehicleKit()) 
        {
            assert(vehicle);
        }
        
        Vehicle *vehicle;
        Unit* Technician;
        Creature::Unit* Bunny[4];
        Creature::Unit* Cannon[4];
        std::list<Unit*> targets;
        uint32 uiTimer;
        uint32 uiRespawnTimer;
        uint8 uiPhase;

        void Reset()
        {
            uiTimer = 0;
            uiRespawnTimer = 6000;
            uiPhase = 0;
        }

        Unit* unit(uint32 entry, uint32 range, bool alive)
        {
            if (Unit* unit = me->FindNearestCreature(entry, float(range), alive))
                if (Unit* unit2 = Unit::GetCreature(*me, unit->GetGUID()))
                    return unit2;
        }

        void GetTargets()
        {
            Trinity::AnyUnitInObjectRangeCheck u_check(me, 100.0f);
            Trinity::UnitListSearcher<Trinity::AnyUnitInObjectRangeCheck> searcher(me, targets, u_check);
            me->VisitNearbyObject(100.0f, searcher);
            if (!targets.empty())
                for (std::list<Unit*>::const_iterator iter = targets.begin(); iter != targets.end(); ++iter)
                {
                    if ((*iter)->GetTypeId() != TYPEID_PLAYER)
                    {
                        switch ((*iter)->GetEntry())
                        {
                        case 46230:
                            if ((*iter)->GetDistance2d(-5165.209961f, 713.809021f) <= 1)
                                Technician = (*iter);
                            break;
                        case 46165:
                            if ((*iter)->GetDistance2d(-5164.919922f, 723.890991f) <= 1)
                                Bunny[0] = (*iter);
                            if ((*iter)->GetDistance2d(-5182.560059f, 726.656982f) <= 1)
                                Bunny[1] = (*iter);
                            if ((*iter)->GetDistance2d(-5166.350098f, 706.336975f) <= 1)
                                Bunny[2] = (*iter);
                            if ((*iter)->GetDistance2d(-5184.040039f, 708.405029f) <= 1)
                                Bunny[3] = (*iter);
                            break;
                        case 46208:
                            if ((*iter)->GetDistance2d(-5164.209961f, 719.267029f) <= 1)
                                Cannon[0] = (*iter);
                            if ((*iter)->GetDistance2d(-5165.000000f, 709.453979f) <= 1)
                                Cannon[1] = (*iter);
                            if ((*iter)->GetDistance2d(-5183.830078f, 722.093994f) <= 1)
                                Cannon[2] = (*iter);
                            if ((*iter)->GetDistance2d(-5184.470215f, 712.554993f) <= 1)
                                Cannon[3] = (*iter);
                            break;
                        }
                    }
                }
        }

        void UpdateAI(const uint32 diff)
        {
            if (!vehicle->HasEmptySeat(0))
                if (uiTimer <= diff)
                {
                    switch(uiPhase)
                    {
                    case 0: me->GetMotionMaster()->MovePoint(1, -5173.34f, 730.11f, 294.25f);
                        GetTargets();
                        ++uiPhase;
                        uiTimer = 5500;
                        break;
                    case 1:
                        if (Bunny[0] && Bunny[1])
                        {
                            Bunny[0]->CastSpell(me, SPELL_DECONTAMINATE_STAGE_1, true);
                            Bunny[1]->CastSpell(me, SPELL_DECONTAMINATE_STAGE_1, true);
                        }
                        ++uiPhase;
                        uiTimer = 5000;
                        break;
                    case 2:
                        if (Cannon[0] && Cannon[1] && Cannon[2] && Cannon[3])
                        {
                            me->GetMotionMaster()->MovePoint(2, -5173.72f, 725.7f, 294.03f);
                            Cannon[0]->CastSpell(me, SPELL_CANNON_BURST, true);
                            Cannon[1]->CastSpell(me, SPELL_CANNON_BURST, true);
                            Cannon[2]->CastSpell(me, SPELL_CANNON_BURST, true);
                            Cannon[3]->CastSpell(me, SPELL_CANNON_BURST, true);
                        }
                        ++uiPhase;
                        uiTimer = 2000;
                        break;
                    case 3:
                        if (Technician)
                            me->GetMotionMaster()->MovePoint(3, -5174.57f, 716.45f, 289.53f);
                            Technician->MonsterSay("Ugh! Not this again! I'm asking for a new station next expedition...", LANG_UNIVERSAL, 0);
                        ++uiPhase;
                        uiTimer = 8000;
                        break;
                    case 4:
                        if (Bunny[2] && Bunny[3])
                        {
                            me->GetMotionMaster()->MovePoint(4, -5175.04f, 707.2f, 294.4f);
                            Bunny[2]->CastSpell(me, SPELL_DECONTAMINATE_STAGE_2, true);
                            Bunny[3]->CastSpell(me, SPELL_DECONTAMINATE_STAGE_2, true);
                        }
                        ++uiPhase;
                        uiTimer = 1000;
                        break;
                    case 5:
                        if (vehicle->GetPassenger(0))
                            if (Player* player = vehicle->GetPassenger(0)->ToPlayer())
                                player->CompleteQuest(QUEST_DECONTAMINATION);
                        me->MonsterSay("Decontamination complete. Standby for delivery.", LANG_UNIVERSAL, 0);
                        me->GetMotionMaster()->MovePoint(5, -5175.61f, 700.38f, 290.89f);
                        ++uiPhase;
                        uiTimer = 3000;
                        break;
                    case 6:
                        me->MonsterSay("Warning, system overload. Malfunction imminent!", LANG_UNIVERSAL, 0);
                        me->CastSpell(me, SPELL_EXPLOSION);
                        ++uiPhase;
                        uiTimer = 1000;
                        break;
                    case 7:
                        me->SetCanFly(false);
                        vehicle->GetPassenger(0)->RemoveAurasDueToSpell(SPELL_IRRADIATE);
                        vehicle->RemoveAllPassengers();
                        me->setDeathState(JUST_DIED);
                        ++uiPhase;
                        uiTimer = 0;
                        break;
                    }
                }
                else uiTimer -= diff;
        }
    };

    CreatureAI *GetAI(Creature *pCreature) const
    {
        return new npc_sanitron500AI(pCreature);
    }
};

float CrushcogAddPlace[4][4] =
{
    //X          Y          Z        O
    {-5250.250f, 130.0f, 394.269f, 4.50f}, //2nd from left
    {-5239.800f, 128.200f, 394.500f, 4.26f}, //1st from left
    {-5245.937f, 105.853f, 392.336f, 1.75f},
    {-5255.550f, 106.500f, 392.402f, 1.60f}
};

uint32 RayType[] = {80098, 80110, 80148};
#define GOSSIP_ITEM_START "I'm ready to start the assault."

class npc_MekkaTorque : public CreatureScript
{
public:
    npc_MekkaTorque() : CreatureScript("npc_MekkaTorque") { }

    bool OnGossipHello(Player* pPlayer, Creature* pCreature)
    {
        if(!CAST_AI(npc_MekkaTorque::npc_MekkaTorqueAI, pCreature->AI())->EventStartedPart1)
        {
            if (pPlayer->GetQuestStatus(26364) == QUEST_STATUS_INCOMPLETE)
                if(Creature* pHelper = pCreature->FindNearestCreature(42852, 20.0f, true))
                    pPlayer->ADD_GOSSIP_ITEM(GOSSIP_ICON_CHAT, GOSSIP_ITEM_START, GOSSIP_SENDER_MAIN, GOSSIP_ACTION_INFO_DEF);

            pPlayer->SEND_GOSSIP_MENU(pPlayer->GetGossipTextId(pCreature), pCreature->GetGUID());

            return true;
        }
        return false;
    }

    bool OnGossipSelect(Player* pPlayer, Creature* pCreature, uint32 /*uiSender*/, uint32 uiAction)
    {
        pPlayer->PlayerTalkClass->ClearMenus();
        if (uiAction == GOSSIP_ACTION_INFO_DEF)
        {
            pPlayer->CLOSE_GOSSIP_MENU();
            CAST_AI(npc_MekkaTorque::npc_MekkaTorqueAI, pCreature->AI())->EventStartedPart1 = true;
            CAST_AI(npc_MekkaTorque::npc_MekkaTorqueAI, pCreature->AI())->PlayerGuid = pPlayer->GetGUID();
        }
        return true;
    }

    CreatureAI* GetAI(Creature* pCreature) const
    {
        return new npc_MekkaTorqueAI(pCreature);
    }

struct npc_MekkaTorqueAI : public ScriptedAI
{
    npc_MekkaTorqueAI(Creature *c) : ScriptedAI(c) {}

    bool EventStartedPart1;
    bool EventStartedPart2;
    bool AttackPhase;
    uint8 Phase;
    uint32 Timer;
    uint64 PlayerGuid;

    void Reset()
    {
        Phase = 0;
        PlayerGuid = 0;
        EventStartedPart1 = false;
        EventStartedPart2 = false;
        AttackPhase = false;
        Timer = 5000;

        me->SetFlag(UNIT_NPC_FLAGS, UNIT_NPC_FLAG_GOSSIP);
    }

    void SummonCrushcogAndHisAdds()
    {
        if(Creature* pC = me->SummonCreature(42839, -5246.240f, 119.70f, 394.33f, 3.02f, TEMPSUMMON_MANUAL_DESPAWN, 30000))
        {
            pC->SetFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_NON_ATTACKABLE);
            pC->SetReactState(REACT_PASSIVE);

            for(int i = 0; i < 4; i++)
                pC->SummonCreature(42294, CrushcogAddPlace[i][0], CrushcogAddPlace[i][1], CrushcogAddPlace[i][2], CrushcogAddPlace[i][3], TEMPSUMMON_CORPSE_TIMED_DESPAWN, 15000);
        }
    };

    void AddRemoveFlagsFromAdds()
    {
        std::list<Creature*> CrushcogAdds;
        GetCreatureListWithEntryInGrid(CrushcogAdds, me, 42294, 150.0f);

        if(AttackPhase)
        {
            for (std::list<Creature*>::iterator itr = CrushcogAdds.begin(); itr != CrushcogAdds.end(); ++itr)
            {
                (*itr)->RemoveFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_NON_ATTACKABLE);
                (*itr)->SetReactState(REACT_AGGRESSIVE);
                (*itr)->CombatStart(me, true);
                (*itr)->Attack(me, true);
            }
        }
        else
        {
            for (std::list<Creature*>::iterator itr = CrushcogAdds.begin(); itr != CrushcogAdds.end(); ++itr)
            {
                (*itr)->SetReactState(REACT_PASSIVE);
                (*itr)->SetFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_NON_ATTACKABLE);
            }
        }
    }

    void RemoveCrushcogAdds()
    {
        std::list<Creature*> CrushcogAddsRemove;
        GetCreatureListWithEntryInGrid(CrushcogAddsRemove, me, 42294, 60.0f);
        for (std::list<Creature*>::iterator itr = CrushcogAddsRemove.begin(); itr != CrushcogAddsRemove.end(); ++itr)
            (*itr)->DespawnOrUnsummon();
    }

    void AddQuestComplete()
    {
        std::list<Player*> players;
        Trinity::AnyPlayerInObjectRangeCheck checker(me, 35.0f);
        Trinity::PlayerListSearcher<Trinity::AnyPlayerInObjectRangeCheck> searcher(me, players, checker);
        me->VisitNearbyWorldObject(35.0f, searcher);

        for (std::list<Player*>::const_iterator itr = players.begin(); itr != players.end(); ++itr)
            (*itr)->CastSpell((*itr), 79931, true);
    }

    void UpdateAI(const uint32 diff)
    {
        DoMeleeAttackIfReady();

        if(Timer < diff)
        {
            if(EventStartedPart1)
            {
                me->RemoveFlag(UNIT_NPC_FLAGS, UNIT_NPC_FLAG_GOSSIP);
                if(Player* pPlayer = me->GetPlayer(*me, PlayerGuid))
                if(Creature* pCreature = me->FindNearestCreature(42852, 8.0f, true))
                {
                    switch(Phase)
                    {
                        case 0: me->MonsterSay("Mekgineer Thermaplugg refuses to acknowledge that his defeat is imminent! He has sent Razlo Crushcog to prevent us from rebuilding our beloved Gnomeregan!", 0, 0); Timer = 5000; Phase++; break;
                        case 1: me->MonsterSay("But $N has thwarted his plans at every turn, and the dwarves of Ironforge stand with us!", 0, PlayerGuid); Timer = 5000; Phase++; break;
                        case 2: me->MonsterSay("Let's send him crawling back to his master in defeat!", 0, 0); Timer = 3000; Phase++; break;
                        case 3:
                        {
                            me->AddUnitMovementFlag(MOVEMENTFLAG_WALKING);
                            pCreature->AddUnitMovementFlag(MOVEMENTFLAG_WALKING);
                            pCreature->MonsterSay("Aye, let's teach this addle-brained gnome a lesson!", 0, 0);
                            Timer = 500; Phase++;
                        } break;
                        case 4:
                        {
                            me->GetMotionMaster()->MovePoint(1, -5294.91f, 134.433f, 386.11f);
                            pCreature->GetMotionMaster()->MovePoint(2, -5291.88f, 138.85f, 386.11f);
                            SummonCrushcogAndHisAdds();
                            Timer = 10000;
                            Phase++;
                        } break;
                        case 5: AddRemoveFlagsFromAdds(); Phase++; Timer = 250; break;
                        case 6:
                        {
                            me->GetMotionMaster()->MovePoint(1, -5261.2f, 119.33f, 393.79f);
                            pCreature->GetMotionMaster()->MovePoint(2, -5260.88f, 123.25f, 393.88f);
                            Timer = 16000;
                            Phase++;
                        } break;
                        case 7: EventStartedPart1 = false; EventStartedPart2 = true; Phase = 8; break;
                        default: break;
                    }
                }
            }
            if(EventStartedPart2)
            {
                if(Player* pPlayer = me->GetPlayer(*me, PlayerGuid))
                if(Creature* pCreature = me->FindNearestCreature(42852, 40.0f, true))
                if(Creature* pCrushcog = me->FindNearestCreature(42839, 40.0f, true))
                {
                    switch(Phase)
                    {
                        case 8: pCrushcog->MonsterSay("You! How did you escape detection by my sentry-bots?", 0, 0); Timer = 3000; Phase++; break;
                        case 9: pCrushcog->MonsterSay("No matter! My guardians and I will make short work of you. To arms, men!", 0, 0); Timer = 2000; Phase++; break;
                        case 10: AttackPhase = true; Phase++; Timer = 250; break;
                        case 11:
                        {
                            AddRemoveFlagsFromAdds();
                            pCrushcog->MonsterSay("You will never defeat the true sons of Gnomergan", 0, 0);
                            pCrushcog->SetReactState(REACT_AGGRESSIVE);
                            pCrushcog->RemoveFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_NON_ATTACKABLE);
                            pCrushcog->CombatStart(me, true);
                            me->Attack(pCrushcog, true);
                            Phase = 12;
                        } break;
                        default: break;
                    }
                    if(pCrushcog)
                        Timer = 7000;
                }
                if(Creature* pCreature = me->FindNearestCreature(42852, 40.0f, true))
                if(Creature* pCrushcog = me->FindNearestCreature(42839, 40.0f, false))
                {
                    switch(Phase)
                    {
                        case 12:
                        {
                            RemoveCrushcogAdds();
                            pCreature->MonsterSay("That'll teach you to mess with the might of Ironforge and Gnomeregan!", 0, 0);
                            Timer = 1500; Phase++;
                        } break;
                        case 13: me->MonsterSay("We've done it! We're victorious!", 0, 0); Timer = 4500; Phase++; break;
                        case 14: me->MonsterSay("With Crushcog defeated. Thermaplugg is sure to be quaking in his mechano-tank, and rightly so. You're next Thermaplugg. You're next!", 0, 0); Phase++; Timer = 4000; break;
                        case 15:
                        {
                            AddQuestComplete();
                            me->GetMotionMaster()->MovePoint(1, -5261.2f, 119.33f, 393.79f);
                            pCreature->GetMotionMaster()->MovePoint(2, -5260.88f, 123.25f, 393.88f);
                            Phase++;
                            Timer = 16000;
                        }
                        case 16:
                        {
                            me->GetMotionMaster()->MoveTargetedHome();
                            pCreature->GetMotionMaster()->MoveTargetedHome();
                            me->AI()->Reset();
                            pCreature->AI()->Reset();
                        } break;
                        default: break;
                    }
                }
                if(Creature* pCrushcog = me->FindNearestCreature(42839, 10.0f, true))
                if(Creature* pRayTarget = me->FindNearestCreature(42929, 50.0f, true))
                {
                    //Niestety Spelle jeszcze niedzia?aj? wiec work-around
                    //me->CastSpell(pRayTarget, RayType[urand(0,3)], true);
                    me->MonsterYell("Mekkatorque-Ray!", 0, 0);

                    std::list<Unit*> BeamTarget;
                    Trinity::AnyUnitInObjectRangeCheck checker(me, 15);
                    Trinity::UnitListSearcher<Trinity::AnyUnitInObjectRangeCheck> searcher(me, BeamTarget, checker);
                    me->VisitNearbyWorldObject(15.0f, searcher);

                    for (std::list<Unit*>::iterator itr = BeamTarget.begin(); itr != BeamTarget.end(); ++itr)
                    {
                        if((*itr)->GetTypeId() == TYPEID_PLAYER)
                            return;

                        me->CastSpell((*itr), RayType[urand(0,3)], true);
                        me->DealDamage((*itr), ((*itr)->GetHealth()*0.15), NULL, DIRECT_DAMAGE, SPELL_SCHOOL_MASK_NORMAL, NULL, false);
                    }
                    me->CastSpell(pCrushcog, RayType[urand(0,3)], true);
                    me->DealDamage(pCrushcog, (pCrushcog->GetHealth()*0.15), NULL, DIRECT_DAMAGE, SPELL_SCHOOL_MASK_NORMAL, NULL, false);
                    pRayTarget->DespawnOrUnsummon();
                }
            //I know it's imposible but...
                if(Creature* pCreature = me->FindNearestCreature(42852, 40.0f, false))
                    me->AI()->Reset();

                if(me->isDead())
                {
                    Creature* pCreature = me->FindNearestCreature(42852, 40.0f, false);
                    if(pCreature)
                        pCreature->AI()->Reset();
                }
            }
        } else Timer -= diff;
    }
    };
};

enum Misc
{
    NPC_COLDRIDGE_DEFENDER      = 37177,
    NPC_ROCKJAW_INVADER         = 37070
};

class npc_coldridge_defender : public CreatureScript
{
public:
    npc_coldridge_defender() : CreatureScript("npc_coldridge_defender") { }

    struct npc_coldridge_defenderAI : public ScriptedAI
    {
        npc_coldridge_defenderAI(Creature* creature) : ScriptedAI(creature)
        {
            me->HandleEmoteCommand(EMOTE_STATE_READY1H);
        }

        EventMap events;

        void EnterCombat(Unit * who)
        {
            me->AddUnitState(UNIT_STATE_ROOT);
            me->HandleEmoteCommand(EMOTE_STATE_READY1H);
        }

        void Reset()
        {
            me->HandleEmoteCommand(EMOTE_STATE_READY1H);
        }

        void DamageTaken(Unit* attacker, uint32& damage)
        {
            if (attacker->GetEntry() == NPC_ROCKJAW_INVADER && ((me->GetHealth() - damage) <= me->GetHealth() / 2))
                return;
        }

        void UpdateAI(const uint32 diff)
        {
            DoMeleeAttackIfReady();
        }
    };
    CreatureAI* GetAI(Creature* creature) const
    {
        return new npc_coldridge_defenderAI (creature);
    }
};

class npc_rockjaw_defender : public CreatureScript
{
public:
    npc_rockjaw_defender() : CreatureScript("npc_rockjaw_defender") { }

    CreatureAI* GetAI(Creature* creature) const
    {
        return new npc_rockjaw_defenderAI (creature);
    }

    struct npc_rockjaw_defenderAI : public ScriptedAI
    {
        npc_rockjaw_defenderAI(Creature* creature) : ScriptedAI(creature) { }

        void DamageTaken(Unit* who, uint32& damage)
        {
            if (who->GetEntry() == NPC_COLDRIDGE_DEFENDER && ((me->GetHealth() - damage) <= me->GetHealth() / 2))
                return;

            if (who->GetTypeId() == TYPEID_PLAYER || who->isPet())
            {
                if (Creature* guard = me->FindNearestCreature(NPC_COLDRIDGE_DEFENDER, 6.0f, true))
                {
                    guard->getThreatManager().resetAllAggro();
                    guard->CombatStop(true);
                }

                me->getThreatManager().resetAllAggro();
                me->GetMotionMaster()->MoveChase(who);
                me->AI()->AttackStart(who);
            }
        }

        void UpdateAI(const uint32 diff)
        {
            if (!UpdateVictim())
                if (Creature* guard = me->FindNearestCreature(NPC_COLDRIDGE_DEFENDER, 6.0f, true))
                    me->AI()->AttackStart(guard);

                DoMeleeAttackIfReady();
        }
    };
};

/*######
## npc_engineer_grindspark
######*/

class npc_engineer_grindspark : public CreatureScript
{
public:
    npc_engineer_grindspark() : CreatureScript("npc_engineer_grindspark") { }

     bool OnQuestAccept(Player* player, Creature* creature, Quest const* quest)
     {
        if (quest->GetQuestId() == QUEST_JOB_FOR_BOT)
        {
        
          creature->AI()->Talk(SAY_MULTIBOT_0, 3000);
          creature->AI()->Talk(SAY_MULTIBOT_1, 3000);
          creature->AI()->Talk(SAY_MULTIBOT_2, 3000);
          creature->AI()->Talk(SAY_MULTIBOT_3, 3000);
          player->CastSpell(player, 79419, true);

        }
          return true;
     }        

};

/*######
## npc_gs_9x_multi_bot
######*/

enum eMultiBotData
{
    OBJECT_TOXIC_POOL = 203975,
    SPELL_CLEAN_UP_TOXIC_POOL = 79424,
    SPELL_TOXIC_POOL_CREDIT_TO_MASTER = 79422,
    SAY_MULTI_BOT = 0
};

class npc_gs_9x_multi_bot : public CreatureScript
{
public:
    npc_gs_9x_multi_bot() : CreatureScript("npc_gs_9x_multi_bot") { }

    CreatureAI* GetAI(Creature* creature) const
    {
        return new npc_gs_9x_multi_botAI (creature);
    }

    struct npc_gs_9x_multi_botAI : public ScriptedAI
    {
        npc_gs_9x_multi_botAI(Creature* c): ScriptedAI(c) { }

        void UpdateAI(const uint32 /*diff*/)
        {
            GameObject* pool = me->FindNearestGameObject(OBJECT_TOXIC_POOL, 2.0f);

            if (pool)
            {
                if (Player* player = me->GetCharmerOrOwnerPlayerOrPlayerItself())
                {
                    Talk(SAY_MULTI_BOT);
                    me->CastSpell(me, SPELL_CLEAN_UP_TOXIC_POOL, true);
                    me->CastSpell(player, SPELL_TOXIC_POOL_CREDIT_TO_MASTER, true);
                    pool->Delete();
                }
            }
        }
    };
};

/*######
## npc_nevin_twistwrench
######*/

class npc_nevin_twistwrench : public CreatureScript
{
public:
    npc_nevin_twistwrench() : CreatureScript("npc_nevin_twistwrench") { }

    bool OnQuestAccept(Player* player, Creature* creature, Quest const* quest)
    {
        if (quest->GetQuestId() == QUEST_PINNED_DOWN)
        {
          creature->AI()->Talk(SAY_MINDLESS_LEPER, player->GetGUID());
        }
        return true;
    }

};

/*######
## npc_kelsey_steelspark
######*/

class npc_kelsey_steelspark : public CreatureScript
{
public:
    npc_kelsey_steelspark() : CreatureScript("npc_kelsey_steelspark") { }

    bool OnQuestAccept(Player* player, Creature* creature, Quest const* quest)
    {
        if (quest->GetQuestId() == QUEST_STAGING_IN_BREWNALL)
        {
          creature->AI()->Talk(SAY_HARD_TIME, player->GetGUID());
        }
        return true;
    }

};

/*######
## npc_ultrasafe_personnel_launcher
######*/
 
enum eultasafeplData 
{
    USPLQUEST        = 25839,
    SPELL_USPL       = 77393,
    SPELL_USPLBUFF   = 80381,
};
class npc_ultrasafe_personnel_launcher: public CreatureScript {
public:
    npc_ultrasafe_personnel_launcher() :
            CreatureScript("npc_ultrasafe_personnel_launcher") {
    }
 
    CreatureAI* GetAI(Creature* creature) const {
        return new npc_ultrasafe_personnel_launcherAI(creature);
    }
 
    struct npc_ultrasafe_personnel_launcherAI: public ScriptedAI {
        npc_ultrasafe_personnel_launcherAI(Creature* c) :
                ScriptedAI(c) {
        }
 
    void Reset()
    {
        me->CastSpell(me, SPELL_USPLBUFF, true);
    }
   
    void UpdateAI(const uint32 /*diff*/)
    {
        if (me->IsVehicle() && me->GetVehicleKit())
        {
            Unit* unit = me->GetVehicleKit()->GetPassenger(0);
            if (unit)
            {   
                unit->ToPlayer()->ExitVehicle();
                unit->ToPlayer()->SetOrientation(3.95f);
                unit->ToPlayer()->CastSpell(unit,SPELL_USPL,true);
            }
        }
 
    }
 
    };
};

/*######
## npc_stolen_ram
######*/

enum
{
    CREDIT_RAMS_ON_THE_LAM       = 43064,
    QUEST_RAMS_ON_THE_LAM        = 25905
};

class npc_stolen_ram : public CreatureScript
{
public:
    npc_stolen_ram() : CreatureScript("npc_stolen_ram") { }

    struct npc_stolen_ramAI : public ScriptedAI
    {
        npc_stolen_ramAI(Creature *c) : ScriptedAI(c) 
        
        { 
            whistle = false;
        }

        bool whistle;

        void Reset()
        {
            whistle = false;
        }

        void ReceiveEmote(Player* player, uint32 emote)
        {
            if (emote==TEXT_EMOTE_WHISTLE && whistle == false && player->IsWithinDistInMap(me,20.00f))
            {
                if (player->GetQuestStatus(QUEST_RAMS_ON_THE_LAM) == QUEST_STATUS_INCOMPLETE)
                {
                    player->KilledMonsterCredit(CREDIT_RAMS_ON_THE_LAM,0);
                    me->RemoveUnitMovementFlag(MOVEMENTFLAG_WALKING);
                    me->SetSpeed(MOVE_RUN, 1.0f, true);
                    me->GetMotionMaster()->MoveChase(player);
                    me->DespawnOrUnsummon(3000);
                    whistle = true;
                }
            }
        }
    };

    CreatureAI *GetAI(Creature *creature) const
    {
        return new npc_stolen_ramAI(creature);
    }
};

void AddSC_dun_morogh()
{
    new npc_sanitron500();
	new npc_MekkaTorque();
    new npc_coldridge_defender();
    new npc_rockjaw_defender();
    new npc_gs_9x_multi_bot();
    new npc_nevin_twistwrench();
    new npc_kelsey_steelspark();
    new npc_engineer_grindspark();
	new npc_ultrasafe_personnel_launcher();
	new npc_stolen_ram();
}
