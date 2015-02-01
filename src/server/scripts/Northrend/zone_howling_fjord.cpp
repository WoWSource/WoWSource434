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

/* ScriptData
SDName: Sholazar_Basin
SD%Complete: 100
SDComment: Quest support: 11253, 11241.
SDCategory: howling_fjord
EndScriptData */

/* ContentData
npc_plaguehound_tracker
npc_apothecary_hanes
EndContentData */

#include "ScriptMgr.h"
#include "ScriptedCreature.h"
#include "ScriptedGossip.h"
#include "ScriptedEscortAI.h"
#include "Player.h"

/*######
## npc_apothecary_hanes
######*/
enum Entries
{
    NPC_APOTHECARY_HANES         = 23784,
    FACTION_ESCORTEE_A           = 774,
    FACTION_ESCORTEE_H           = 775,
    NPC_HANES_FIRE_TRIGGER       = 23968,
    QUEST_TRAIL_OF_FIRE          = 11241,
    SPELL_COSMETIC_LOW_POLY_FIRE = 56274
};

class npc_apothecary_hanes : public CreatureScript
{
public:
    npc_apothecary_hanes() : CreatureScript("npc_apothecary_hanes") { }

    bool OnQuestAccept(Player* player, Creature* creature, Quest const* quest)
    {
        if (quest->GetQuestId() == QUEST_TRAIL_OF_FIRE)
        {
            switch (player->GetTeam())
            {
                case ALLIANCE:
                    creature->setFaction(FACTION_ESCORTEE_A);
                    break;
                case HORDE:
                    creature->setFaction(FACTION_ESCORTEE_H);
                    break;
            }
            CAST_AI(npc_escortAI, (creature->AI()))->Start(true, false, player->GetGUID());
        }
        return true;
    }

    struct npc_Apothecary_HanesAI : public npc_escortAI
    {
        npc_Apothecary_HanesAI(Creature* creature) : npc_escortAI(creature){}
        uint32 PotTimer;

        void Reset()
        {
            SetDespawnAtFar(false);
            PotTimer = 10000; //10 sec cooldown on potion
        }

        void JustDied(Unit* /*killer*/)
        {
            if (Player* player = GetPlayerForEscort())
                player->FailQuest(QUEST_TRAIL_OF_FIRE);
        }

        void UpdateEscortAI(const uint32 diff)
        {
            if (HealthBelowPct(75))
            {
                if (PotTimer <= diff)
                {
                    DoCast(me, 17534, true);
                    PotTimer = 10000;
                } else PotTimer -= diff;
            }
            if (GetAttack() && UpdateVictim())
                DoMeleeAttackIfReady();
        }

        void WaypointReached(uint32 waypointId)
        {
            Player* player = GetPlayerForEscort();
            if (!player)
                return;

            switch (waypointId)
            {
                case 1:
                    me->SetReactState(REACT_AGGRESSIVE);
                    SetRun(true);
                    break;
                case 23:
                    player->GroupEventHappens(QUEST_TRAIL_OF_FIRE, me);
                    me->DespawnOrUnsummon();
                    break;
                case 5:
                    if (Unit* Trigger = me->FindNearestCreature(NPC_HANES_FIRE_TRIGGER, 10.0f))
                        Trigger->CastSpell(Trigger, SPELL_COSMETIC_LOW_POLY_FIRE, false);
                    SetRun(false);
                    break;
                case 6:
                    if (Unit* Trigger = me->FindNearestCreature(NPC_HANES_FIRE_TRIGGER, 10.0f))
                        Trigger->CastSpell(Trigger, SPELL_COSMETIC_LOW_POLY_FIRE, false);
                    SetRun(true);
                    break;
                case 8:
                    if (Unit* Trigger = me->FindNearestCreature(NPC_HANES_FIRE_TRIGGER, 10.0f))
                        Trigger->CastSpell(Trigger, SPELL_COSMETIC_LOW_POLY_FIRE, false);
                    SetRun(false);
                    break;
                case 9:
                    if (Unit* Trigger = me->FindNearestCreature(NPC_HANES_FIRE_TRIGGER, 10.0f))
                        Trigger->CastSpell(Trigger, SPELL_COSMETIC_LOW_POLY_FIRE, false);
                    break;
                case 10:
                    SetRun(true);
                    break;
                case 13:
                    SetRun(false);
                    break;
                case 14:
                    if (Unit* Trigger = me->FindNearestCreature(NPC_HANES_FIRE_TRIGGER, 10.0f))
                        Trigger->CastSpell(Trigger, SPELL_COSMETIC_LOW_POLY_FIRE, false);
                    SetRun(true);
                    break;
            }
        }
    };

    CreatureAI* GetAI(Creature* creature) const
    {
        return new npc_Apothecary_HanesAI(creature);
    }
};

/*######
## npc_plaguehound_tracker
######*/

enum ePlaguehound
{
    QUEST_SNIFF_OUT_ENEMY        = 11253
};

class npc_plaguehound_tracker : public CreatureScript
{
public:
    npc_plaguehound_tracker() : CreatureScript("npc_plaguehound_tracker") { }

    struct npc_plaguehound_trackerAI : public npc_escortAI
    {
        npc_plaguehound_trackerAI(Creature* creature) : npc_escortAI(creature) { }

        void Reset()
        {
            uint64 summonerGUID = 0;

            if (me->isSummon())
                if (Unit* summoner = me->ToTempSummon()->GetSummoner())
                    if (summoner->GetTypeId() == TYPEID_PLAYER)
                        summonerGUID = summoner->GetGUID();

            if (!summonerGUID)
                return;

            me->SetUnitMovementFlags(MOVEMENTFLAG_WALKING);
            Start(false, false, summonerGUID);
        }

        void WaypointReached(uint32 waypointId)
        {
            if (waypointId != 26)
                return;

            me->DespawnOrUnsummon();
        }
    };

    CreatureAI* GetAI(Creature* creature) const
    {
        return new npc_plaguehound_trackerAI(creature);
    }
};

/*######
## npc_razael_and_lyana
######*/

#define GOSSIP_RAZAEL_REPORT "High Executor Anselm wants a report on the situation."
#define GOSSIP_LYANA_REPORT "High Executor Anselm requests your report."

enum eRazael
{
    QUEST_REPORTS_FROM_THE_FIELD = 11221,
    NPC_RAZAEL = 23998,
    NPC_LYANA = 23778,
    GOSSIP_TEXTID_RAZAEL1 = 11562,
    GOSSIP_TEXTID_RAZAEL2 = 11564,
    GOSSIP_TEXTID_LYANA1 = 11586,
    GOSSIP_TEXTID_LYANA2 = 11588
};

class npc_razael_and_lyana : public CreatureScript
{
public:
    npc_razael_and_lyana() : CreatureScript("npc_razael_and_lyana") { }

    bool OnGossipHello(Player* player, Creature* creature)
    {
        if (creature->IsQuestGiver())
            player->PrepareQuestMenu(creature->GetGUID());

        if (player->GetQuestStatus(QUEST_REPORTS_FROM_THE_FIELD) == QUEST_STATUS_INCOMPLETE)
            switch (creature->GetEntry())
            {
                case NPC_RAZAEL:
                    if (!player->GetReqKillOrCastCurrentCount(QUEST_REPORTS_FROM_THE_FIELD, NPC_RAZAEL))
                    {
                        player->ADD_GOSSIP_ITEM(GOSSIP_ICON_CHAT, GOSSIP_RAZAEL_REPORT, GOSSIP_SENDER_MAIN, GOSSIP_ACTION_INFO_DEF + 1);
                        player->SEND_GOSSIP_MENU(GOSSIP_TEXTID_RAZAEL1, creature->GetGUID());
                        return true;
                    }
                break;
                case NPC_LYANA:
                    if (!player->GetReqKillOrCastCurrentCount(QUEST_REPORTS_FROM_THE_FIELD, NPC_LYANA))
                    {
                        player->ADD_GOSSIP_ITEM(GOSSIP_ICON_CHAT, GOSSIP_LYANA_REPORT, GOSSIP_SENDER_MAIN, GOSSIP_ACTION_INFO_DEF + 2);
                        player->SEND_GOSSIP_MENU(GOSSIP_TEXTID_LYANA1, creature->GetGUID());
                        return true;
                    }
                break;
            }
        player->SEND_GOSSIP_MENU(player->GetGossipTextId(creature), creature->GetGUID());
        return true;
    }

    bool OnGossipSelect(Player* player, Creature* creature, uint32 /*sender*/, uint32 action)
    {
        player->PlayerTalkClass->ClearMenus();
        switch (action)
        {
            case GOSSIP_ACTION_INFO_DEF + 1:
                player->SEND_GOSSIP_MENU(GOSSIP_TEXTID_RAZAEL2, creature->GetGUID());
                player->TalkedToCreature(NPC_RAZAEL, creature->GetGUID());
                break;
            case GOSSIP_ACTION_INFO_DEF + 2:
                player->SEND_GOSSIP_MENU(GOSSIP_TEXTID_LYANA2, creature->GetGUID());
                player->TalkedToCreature(NPC_LYANA, creature->GetGUID());
                break;
        }
        return true;
    }
};

/*######
## npc_mcgoyver
######*/

#define GOSSIP_ITEM_MG_I  "Walt sent me to pick up some dark iron ingots."
#define GOSSIP_ITEM_MG_II "Yarp."

enum eMcGoyver
{
    QUEST_WE_CAN_REBUILD_IT             = 11483,

    SPELL_CREATURE_DARK_IRON_INGOTS     = 44512,
    SPELL_TAXI_EXPLORERS_LEAGUE         = 44280,

    GOSSIP_TEXTID_MCGOYVER              = 12193
};

class npc_mcgoyver : public CreatureScript
{
public:
    npc_mcgoyver() : CreatureScript("npc_mcgoyver") { }

    bool OnGossipHello(Player* player, Creature* creature)
    {
        if (player->GetQuestStatus(QUEST_WE_CAN_REBUILD_IT) == QUEST_STATUS_INCOMPLETE)
            player->ADD_GOSSIP_ITEM(GOSSIP_ICON_CHAT, GOSSIP_ITEM_MG_I, GOSSIP_SENDER_MAIN, GOSSIP_ACTION_INFO_DEF+1);

        player->SEND_GOSSIP_MENU(player->GetGossipTextId(creature), creature->GetGUID());
        return true;
    }

    bool OnGossipSelect(Player* player, Creature* creature, uint32 /*sender*/, uint32 action)
    {
        player->PlayerTalkClass->ClearMenus();
        switch (action)
        {
            case GOSSIP_ACTION_INFO_DEF+1:
                player->ADD_GOSSIP_ITEM(GOSSIP_ICON_CHAT, GOSSIP_ITEM_MG_II, GOSSIP_SENDER_MAIN, GOSSIP_ACTION_INFO_DEF+2);
                player->SEND_GOSSIP_MENU(GOSSIP_TEXTID_MCGOYVER, creature->GetGUID());
                player->CastSpell(player, SPELL_CREATURE_DARK_IRON_INGOTS, true);
                break;
            case GOSSIP_ACTION_INFO_DEF+2:
                player->CastSpell(player, SPELL_TAXI_EXPLORERS_LEAGUE, true);
                player->CLOSE_GOSSIP_MENU();
                break;
        }
        return true;
    }
};

/*######
## npc_daegarn
######*/

enum eDaegarnn
{
    QUEST_DEFEAT_AT_RING            = 11300,

    NPC_FIRJUS                      = 24213,
    NPC_JLARBORN                    = 24215,
    NPC_YOROS                       = 24214,
    NPC_OLUF                        = 23931,

    NPC_PRISONER_1                  = 24253,  // looks the same but has different abilities
    NPC_PRISONER_2                  = 24254,
    NPC_PRISONER_3                  = 24255,
};

static float afSummon[] = {838.81f, -4678.06f, -94.182f};
static float afCenter[] = {801.88f, -4721.87f, -96.143f};

class npc_daegarn : public CreatureScript
{
public:
    npc_daegarn() : CreatureScript("npc_daegarn") { }

    bool OnQuestAccept(Player* player, Creature* creature, const Quest* quest)
    {
        if (quest->GetQuestId() == QUEST_DEFEAT_AT_RING)
        {
            if (npc_daegarnAI* pDaegarnAI = CAST_AI(npc_daegarn::npc_daegarnAI, creature->AI()))
                pDaegarnAI->StartEvent(player->GetGUID());
        }

        return true;
    }

    // TODO: make prisoners help (unclear if summoned or using npc's from surrounding cages (summon inside small cages?))
    struct npc_daegarnAI : public ScriptedAI
    {
        npc_daegarnAI(Creature* creature) : ScriptedAI(creature) { }

        bool bEventInProgress;
        uint64 uiPlayerGUID;

        void Reset()
        {
            bEventInProgress = false;
            uiPlayerGUID = 0;
        }

        void StartEvent(uint64 uiGUID)
        {
            if (bEventInProgress)
                return;

            uiPlayerGUID = uiGUID;

            SummonGladiator(NPC_FIRJUS);
        }

        void JustSummoned(Creature* summon)
        {
            if (Player* player = me->GetPlayer(*me, uiPlayerGUID))
            {
                if (player->isAlive())
                {
                    summon->SetWalk(false);
                    summon->GetMotionMaster()->MovePoint(0, afCenter[0], afCenter[1], afCenter[2]);
                    summon->AI()->AttackStart(player);
                    return;
                }
            }

            Reset();
        }

        void SummonGladiator(uint32 uiEntry)
        {
            me->SummonCreature(uiEntry, afSummon[0], afSummon[1], afSummon[2], 0.0f, TEMPSUMMON_TIMED_DESPAWN_OUT_OF_COMBAT, 30*IN_MILLISECONDS);
        }

        void SummonedCreatureDies(Creature* summoned, Unit* /*killer*/)
        {
            uint32 uiEntry = 0;

            // will eventually reset the event if something goes wrong
            switch (summoned->GetEntry())
            {
                case NPC_FIRJUS:    uiEntry = NPC_JLARBORN; break;
                case NPC_JLARBORN:  uiEntry = NPC_YOROS;    break;
                case NPC_YOROS:     uiEntry = NPC_OLUF;     break;
                case NPC_OLUF:      Reset();                return;
            }

            SummonGladiator(uiEntry);
        }
    };

    CreatureAI* GetAI(Creature* creature) const
    {
        return new npc_daegarnAI(creature);
    }
};

enum riven_widow_cocoon_npcs
{
    Gjalerbron_Sleep_Watcher      = 23989,
    Gjalerbron_Warrior            = 23991,
    Gjalerbron_Rune_Caster        = 23990,
    Freed_Winterhoof_Longrunner   = 24211
};

class npc_riven_widow_cocoon : public CreatureScript
{
public:
    npc_riven_widow_cocoon() : CreatureScript("riven_widow_cocoon") { }

    struct npc_riven_widow_cocoonAI : public ScriptedAI
    {
        npc_riven_widow_cocoonAI(Creature *c) : ScriptedAI(c) {}

        void JustDied(Unit *pKiller)
        {
            if (!pKiller->ToPlayer())
                return;

            TempSummon *Spawn = NULL;
            switch (urand(0,4))
            {
            case 0:
                Spawn = me->SummonCreature(Gjalerbron_Sleep_Watcher,me->GetPositionX(),me->GetPositionY(),me->GetPositionZ(),me->GetOrientation(),TEMPSUMMON_TIMED_OR_CORPSE_DESPAWN,120*IN_MILLISECONDS);
                break;
            case 1:
                Spawn = me->SummonCreature(Gjalerbron_Warrior,me->GetPositionX(),me->GetPositionY(),me->GetPositionZ(),me->GetOrientation(),TEMPSUMMON_TIMED_OR_CORPSE_DESPAWN,120*IN_MILLISECONDS);
                break;
            case 2:
                Spawn = me->SummonCreature(Gjalerbron_Rune_Caster,me->GetPositionX(),me->GetPositionY(),me->GetPositionZ(),me->GetOrientation(),TEMPSUMMON_TIMED_OR_CORPSE_DESPAWN,120*IN_MILLISECONDS);
                break;
            default:
                me->SummonCreature(Freed_Winterhoof_Longrunner,me->GetPositionX(),me->GetPositionY(),me->GetPositionZ(),me->GetOrientation(),TEMPSUMMON_TIMED_OR_CORPSE_DESPAWN,20*IN_MILLISECONDS);
                pKiller->ToPlayer()->KilledMonsterCredit(Freed_Winterhoof_Longrunner, pKiller->ToPlayer()->GetGUID());
                break;
            }
            if (Spawn)
                Spawn->AI()->AttackStart(pKiller);
        }
    };

    CreatureAI* GetAI(Creature* creature) const
    {
        return new npc_riven_widow_cocoonAI (creature);
    }
};

/*######
## npc_feknut_bunny
######*/

enum FeknutBunnyData
{
    NPC_DARKCLAW_BAT        = 23959,
    SPELL_SUMMON_GUANO      = 43307

};

class npc_feknut_bunny : public CreatureScript
{
public:
    npc_feknut_bunny() : CreatureScript("npc_feknut_bunny") {}

    struct npc_feknut_bunnyAI : public ScriptedAI
    {
        npc_feknut_bunnyAI (Creature* creature) : ScriptedAI(creature) {}

        uint32 CheckTimer;
        bool Checked;

        void Reset()
        {
            CheckTimer = 3000;
            Checked = false;
        }

        void UpdateAI(const uint32 diff)
        {
            if(!Checked)
            {
                if (CheckTimer <= diff)
                {
                    if(Creature* bat = GetClosestCreatureWithEntry(me, NPC_DARKCLAW_BAT, 45.0f))
                    {
                        if(Player* player = me->GetOwner()->ToPlayer())
                        {
                            bat->Attack(player, true);
                            bat->GetMotionMaster()->MoveChase(player);
                        }

                        if(bat->isAlive())
                        {
                            me->CastSpell(me, SPELL_SUMMON_GUANO, false);
                            Checked = true;
                            me->DespawnOrUnsummon();
                        }

                    }
                } else CheckTimer -= diff;
            }
        }
    };
    CreatureAI* GetAI(Creature* creature) const
    {
        return new npc_feknut_bunnyAI(creature);
    }
};

enum ironRuneConstructData
{
    QUEST_IRON_RUNE_CONST_JUMP  = 11485,
    QUEST_IRON_RUNE_CONST_DATA  = 11489,
    QUEST_IRON_RUNE_CONST_BLUFF = 11491,

    NPC_IRON_RUNE_CONST_JUMP    = 24823,
    NPC_IRON_RUNE_CONST_DATA    = 24821,
    NPC_IRON_RUNE_CONST_BLUFF   = 24825,

    SPELL_BLUFF                 = 44562,
};

class npc_iron_rune_construct : public CreatureScript
{
public:
    npc_iron_rune_construct() : CreatureScript("npc_iron_rune_construct") { }

    struct npc_iron_rune_constructAI : public ScriptedAI
    {
        npc_iron_rune_constructAI(Creature* creature) : ScriptedAI(creature) { }

        bool ocuppied;
        uint8 seatID;

        void UpdateAI(const uint32 /*diff*/)
        {
            Unit* player = me->GetVehicleKit()->GetPassenger(seatID);

            if (!player)
                me->DisappearAndDie();
        }

        void PassengerBoarded(Unit* passenger, int8 seatId, bool /*apply*/)
        {
            seatID = seatId;
        }
    };

    CreatureAI* GetAI(Creature* creature) const
    {
        return new npc_iron_rune_constructAI(creature);
    }
};

class npc_lebronski : public CreatureScript
{
public:
    npc_lebronski() : CreatureScript("npc_lebronski") { }

    struct npc_lebronskiAI : public ScriptedAI
    {
        npc_lebronskiAI(Creature* creature) : ScriptedAI(creature) { }

        void SpellHit(Unit* caster, SpellInfo const* spell)
        {
            if(spell->Id == SPELL_BLUFF)
                caster->GetVehicleKit()->GetPassenger(0)->ToPlayer()->GroupEventHappens(QUEST_IRON_RUNE_CONST_BLUFF, me);
        }

    };

    CreatureAI* GetAI(Creature* creature) const
    {
        return new npc_lebronskiAI(creature);
    }
};

class go_iron_rune_construct_workbank : public GameObjectScript
{
public:
    go_iron_rune_construct_workbank() : GameObjectScript("go_iron_rune_construct_workbank") { }

    bool OnGossipHello(Player* player, GameObject* go)
    {
        if (player->GetQuestStatus(QUEST_IRON_RUNE_CONST_JUMP) == QUEST_STATUS_INCOMPLETE)
        {
            if(Creature* pConstruct = player->SummonCreature(NPC_IRON_RUNE_CONST_JUMP, player->GetPositionX(), player->GetPositionY(), player->GetPositionZ(), player->GetOrientation()))
            {
                pConstruct->SetLevel(player->getLevel());
                player->EnterVehicle(pConstruct);
                player->GroupEventHappens(QUEST_IRON_RUNE_CONST_JUMP, pConstruct);
            }
            return true;
        }

        if (player->GetQuestStatus(QUEST_IRON_RUNE_CONST_DATA) == QUEST_STATUS_INCOMPLETE)
        {
            if(Creature* pConstruct = player->SummonCreature(NPC_IRON_RUNE_CONST_DATA, player->GetPositionX(), player->GetPositionY(), player->GetPositionZ(), player->GetOrientation()))
            {
                pConstruct->SetLevel(player->getLevel());
                player->EnterVehicle(pConstruct);
            }
            return true;
        }

        if (player->GetQuestStatus(QUEST_IRON_RUNE_CONST_BLUFF) == QUEST_STATUS_INCOMPLETE)
        {
            if(Creature* pConstruct = player->SummonCreature(NPC_IRON_RUNE_CONST_BLUFF, player->GetPositionX(), player->GetPositionY(), player->GetPositionZ(), player->GetOrientation()))
            {
                pConstruct->SetLevel(player->getLevel());
                player->EnterVehicle(pConstruct);
            }
            return true;
        }

        return true;
    }
};

enum enum_the_echo_of_ymiron
{
    spell_grasp_of_the_lich_king = 43489,
    spell_magnetic_pull = 29661,
    spell_wrath_of_the_lich_king = 50156,
    spell_wrath_of_the_lich_king_quest = 43488,

    npc_the_lich_king = 24248,
    npc_ancient_male_vrykul = 24314,
    npc_ancient_female_vrykul = 24315,
    npc_valkyr_soulclaimer = 24327,

    quest_id_lich_king_speach = 12485,
    quest_id_the_echo_of_ymiron = 11343,
};

const Position valkyr_position[] =
{
    {1072.0f, -5032.0f, 9.0f, 0.0f},
};

class quest_the_echo_of_ymiron : public CreatureScript
{
public:
    quest_the_echo_of_ymiron() : CreatureScript("quest_the_echo_of_ymiron") { }

    CreatureAI* GetAI(Creature* creature) const
    {
        return new quest_the_echo_of_ymironAI (creature);
    }

    struct quest_the_echo_of_ymironAI : public ScriptedAI
    {
        quest_the_echo_of_ymironAI(Creature* creature) : ScriptedAI(creature) {}

        uint32 uiTimer;
        uint32 uiPhase;
        uint32 uiPhaseFemale;
        uint32 uiPhaseMale;

        void Reset()
        {
            uiTimer = 0;
            uiPhase = 0;
            uiPhaseFemale = 0;
            uiPhaseMale = 0;
        }

        void KilledUnit(Unit* victim)
        {
        }

        void UpdateAI(const uint32 uiDiff)
        {
            if (Player* target = me->SelectNearestPlayer(30)) {
                if (target->isAlive()){
                    if (uiTimer <= uiDiff){
                        switch (me->GetEntry()) {
    case npc_the_lich_king:
        if (uiPhase == 0 && target->GetQuestRewardStatus(quest_id_lich_king_speach)) uiPhase = 11;
        switch (uiPhase) {
    case 0:
        me->AI()->Talk(0);
        me->AI()->DoCast(target, spell_magnetic_pull, true);
        uiTimer = 500;
        uiPhase ++;
        break;
    case 1:
        me->AI()->Talk(1);
        me->AI()->DoCast(target, spell_grasp_of_the_lich_king, true);
        uiTimer = 7000;
        uiPhase ++;
        break;
    case 2:
        me->AI()->Talk(2);
        uiTimer = 500;
        uiPhase ++;
        break;
    case 3:
        me->AI()->Talk(3);
        uiTimer = 5000;
        uiPhase ++;
        break;
    case 4:
        if (Creature* minion = me->FindNearestCreature(npc_valkyr_soulclaimer, 50, true))
            minion->AI()->Talk(0);
        uiTimer = 5000;
        uiPhase ++;
        break;
    case 5:
        me->AI()->Talk(4);
        uiTimer = 5000;
        uiPhase ++;
        break;
    case 6:
        me->AI()->Talk(5);
        uiTimer = 10000;
        uiPhase ++;
        break;
    case 7:
        me->AI()->Talk(6);
        uiTimer = 10000;
        uiPhase ++;
        break;
    case 8:
        me->AI()->Talk(7);
        uiTimer = 10000;
        uiPhase ++;
        break;
    case 9:
        me->AI()->DoCast(target, spell_wrath_of_the_lich_king_quest, true);
        uiTimer = 1000;
        uiPhase ++;
        break;
    case 10:
        target->setDeathState(JUST_DIED);
        me->AI()->Reset();
        break;
    case 11:
        me->AI()->Talk(8);
        me->AI()->DoCast(target, spell_magnetic_pull, true);
        uiTimer = 1000;
        uiPhase ++;
        break;
    case 12:
        me->AI()->DoCast(target, spell_grasp_of_the_lich_king, true);
        uiTimer = 6000;
        uiPhase = 9;
        break;
        }
        break;
    case npc_ancient_male_vrykul:
        if (target->GetDistance(valkyr_position[0])<7 && target->GetQuestStatus(quest_id_the_echo_of_ymiron) == QUEST_STATUS_INCOMPLETE){
            if (uiPhase == 0 || uiPhase == 1 || uiPhase == 3 || uiPhase == 5) {
                me->AI()->Talk(uiPhaseMale);
                uiPhaseMale ++;
            } else {
                if (Creature* minion = me->FindNearestCreature(npc_ancient_female_vrykul, 50, true))
                    minion->AI()->Talk(uiPhaseFemale);
                uiPhaseFemale ++;
            }
            if (uiPhase == 6) {
                target->CompleteQuest(quest_id_the_echo_of_ymiron);
                me->AI()->Reset();
            }
            uiTimer = 4000;
            uiPhase ++;
        }
        break;
                        }
                    } else uiTimer -= uiDiff;
                }} else me->AI()->Reset();
        };
    };
};

enum questDropItThenRockIt
{
    QUEST_DROP_IT_THEN_ROCK_IT    = 11429,
    NPC_WINTERSKORN_DEFENDER = 24015,
};

class npc_banner_q11429 : public CreatureScript
{
public:
    npc_banner_q11429() : CreatureScript("npc_banner_q11429") { }

    CreatureAI* GetAI(Creature* creature) const
    {
        return new npc_banner_q11429AI(creature);
    }

    struct npc_banner_q11429AI : public ScriptedAI
    {
        npc_banner_q11429AI(Creature* c) : ScriptedAI(c) { }

        uint32 uiWaveTimer;
        uint8 killCounter;

        void Reset()
        {
            killCounter = 0;
            uiWaveTimer = 2000;
            me->SetReactState(REACT_PASSIVE);
            me->GetMotionMaster()->MoveIdle();
        }

        void JustDied(Unit* /*killer*/)
        {
            if (Player* player = me->GetOwner()->ToPlayer())
                player->FailQuest(QUEST_DROP_IT_THEN_ROCK_IT);
        }

        void UpdateAI(const uint32 diff)
        {
            if (uiWaveTimer <= diff)
            {
                if (Creature* pVrykul = me->SummonCreature(NPC_WINTERSKORN_DEFENDER, (1476.85f + rand()%20), (-5327.56f + rand()%20), (194.8f  + rand()%2), 0.0f, TEMPSUMMON_CORPSE_DESPAWN))
                {
                    pVrykul->AI()->AttackStart(me);
                    pVrykul->GetMotionMaster()->Clear();
                    pVrykul->GetMotionMaster()->MoveChase(me);
                }
                uiWaveTimer = urand(8000, 16000);
            }
            else
                uiWaveTimer -=diff;
        }

        void SummonedCreatureDespawn(Creature* summon)
        {
            if (summon->GetEntry() == NPC_WINTERSKORN_DEFENDER)
                killCounter++;

            if (killCounter >= 3)
            {
                if (Player* player = me->GetOwner()->ToPlayer())
                    player->GroupEventHappens(QUEST_DROP_IT_THEN_ROCK_IT, me);

                me->DespawnOrUnsummon(2000);
            }
        }
    };
};

enum ProtoDrake_Q11188
{
    SPELL_EMBER_FLAME_STRAFE_SPAWN = 49209
};
class npc_Proto_Drake_q11188 : public CreatureScript
{
public:
    npc_Proto_Drake_q11188() : CreatureScript("npc_Proto_Drake_q11188") { }

    CreatureAI* GetAI(Creature* creature) const
    {
        return new npc_Proto_Drake_q11188AI(creature);
    }

    struct npc_Proto_Drake_q11188AI : public ScriptedAI
    {
        npc_Proto_Drake_q11188AI(Creature* c) : ScriptedAI(c) { }

        uint32 resetTimer;

        void Reset()
        {
            resetTimer = 60000;
            DoCast(SPELL_EMBER_FLAME_STRAFE_SPAWN);
        }

        void UpdateAI(const uint32 diff)
        {
            if (resetTimer <= diff)
            {
                EnterEvadeMode();
                resetTimer = 60000;
            }
            else
            {
                resetTimer -=diff;

                DoMeleeAttackIfReady();
            }
        }
    };
};

void AddSC_howling_fjord()
{
    new npc_apothecary_hanes;
    new npc_plaguehound_tracker;
    new npc_razael_and_lyana;
    new npc_mcgoyver;
    new npc_daegarn;
    new npc_riven_widow_cocoon();
    new npc_feknut_bunny();
    new npc_iron_rune_construct();
    new npc_lebronski();
    new go_iron_rune_construct_workbank();
    new quest_the_echo_of_ymiron;
    new npc_banner_q11429();
    new npc_Proto_Drake_q11188();
 }
