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
SDName: Duskwood
SD%Complete: 100
SDComment: Quest Support:8735
SDCategory: Duskwood
EndScriptData */

#include "ScriptMgr.h"
#include "ScriptedCreature.h"
#include "Player.h"

/*######
# at_twilight_grove
######*/

class at_twilight_grove : public AreaTriggerScript
{
public:
    at_twilight_grove() : AreaTriggerScript("at_twilight_grove") { }

    bool OnTrigger(Player* player, const AreaTriggerEntry * /*at*/)
    {
        if (player->HasQuestForItem(21149))
        {
            if (Unit* TCorrupter = player->SummonCreature(15625,-10328.16f,-489.57f,49.95f,0,TEMPSUMMON_MANUAL_DESPAWN,60000))
            {
                TCorrupter->setFaction(14);
                TCorrupter->SetMaxHealth(832750);
			}
            
            if (Unit* CorrupterSpeaker = player->SummonCreature(1,player->GetPositionX(),player->GetPositionY(),player->GetPositionZ()-1,0,TEMPSUMMON_TIMED_DESPAWN,15000))
            {
                CorrupterSpeaker->SetName("Twilight Corrupter");
                CorrupterSpeaker->SetVisible(true);
                CorrupterSpeaker->MonsterYell("Come, $N. See what the Nightmare brings...",0,player->GetGUID());
            }
        }
        return false;
    };

};

/*######
# boss_twilight_corrupter
######*/

#define SPELL_SOUL_CORRUPTION       25805
#define SPELL_CREATURE_OF_NIGHTMARE 25806
#define SPELL_LEVEL_UP              24312

class boss_twilight_corrupter : public CreatureScript
{
public:
    boss_twilight_corrupter() : CreatureScript("boss_twilight_corrupter") { }

    CreatureAI* GetAI(Creature* creature) const
    {
        return new boss_twilight_corrupterAI (creature);
    }

    struct boss_twilight_corrupterAI : public ScriptedAI
    {
        boss_twilight_corrupterAI(Creature *creature) : ScriptedAI(creature) {}

        uint32 SoulCorruption_Timer;
        uint32 CreatureOfNightmare_Timer;
        uint8 KillCount;

        void Reset()
        {
            SoulCorruption_Timer      = 15000;
            CreatureOfNightmare_Timer = 30000;
            KillCount                 = 0;
        }
        void EnterCombat(Unit* /*who*/)
        {
            me->MonsterYell("The Nightmare cannot be stopped!", 0, me->GetGUID());
        }

        void KilledUnit(Unit* victim)
        {
            if (victim->GetTypeId() == TYPEID_PLAYER)
            {
                ++KillCount;
                me->MonsterTextEmote("Twilight Corrupter squeezes the last bit of life out of $N and swallows their soul.", victim->GetGUID(), true);

                if (KillCount == 3)
                {
                    DoCast(me, SPELL_LEVEL_UP, true);
                    KillCount = 0;
                }
            }
        }

        void UpdateAI(const uint32 diff)
        {
            if (!UpdateVictim())
                return;
            if (SoulCorruption_Timer <= diff)
            {
                DoCast(me->GetVictim(), SPELL_SOUL_CORRUPTION);
                SoulCorruption_Timer = rand()%4000+15000; //gotta confirm Timers
            } else SoulCorruption_Timer-=diff;
            if (CreatureOfNightmare_Timer <= diff)
            {
                DoCast(me->GetVictim(), SPELL_CREATURE_OF_NIGHTMARE);
                CreatureOfNightmare_Timer = 45000; //gotta confirm Timers
            } else CreatureOfNightmare_Timer-=diff;
            DoMeleeAttackIfReady();
        };
    };

};

/*##########
##npc_blaze_darkshire###
############*/
enum eFireSeas
{
  NPC_BLAZE = 43918
};
class npc_blaze_darkshire : public CreatureScript
{
    public:
        npc_blaze_darkshire() : CreatureScript("npc_blaze_darkshire") { }

        CreatureAI* GetAI(Creature* pCreature) const
        {
            return new npc_blaze_darkshireAI (pCreature);
        }

        struct npc_blaze_darkshireAI : public ScriptedAI
        {
            npc_blaze_darkshireAI(Creature* creature) : ScriptedAI(creature) {}

            void Reset()
            {
                me->CastSpell(me,82182,true);
            }
        };
};

enum eStitches
{
    QUEST_26727 = 26727,
};

#define guard 10038

class npc_stitches : public CreatureScript
{
public:
    npc_stitches() : CreatureScript("npc_stitches") { }

    CreatureAI* GetAI(Creature* pCreature) const
    {
        return new npc_stitchesAI(pCreature);
    }

    struct npc_stitchesAI : public ScriptedAI
    {
        npc_stitchesAI(Creature *c) : ScriptedAI(c) {}

        uint32 Timer;

        void Reset()
        {
            me->SetRespawnTime(10);
            Timer = 5000;
        }
        void DamageTaken(Unit * pWho, uint32 &uiDamage)
        {
            if (pWho->GetTypeId() == TYPEID_PLAYER)
            {
                me->getThreatManager().resetAllAggro();
                pWho->AddThreat(me, 100000.0f);
                me->AddThreat(pWho, 100000.0f);
                me->AI()->AttackStart(pWho);
            }
            else if (pWho->isPet() || pWho->isGuardian())
            {
                me->getThreatManager().resetAllAggro();
                me->AddThreat(pWho, 100000.0f);
                me->AI()->AttackStart(pWho);
            }
            else uiDamage = 0;
        }
        void DamageDealt(Unit* target, uint32& damage, DamageEffectType damageType)
        {
            if (target->GetEntry() == guard)
                damage = 0;
        }
        void JustDied(Unit* /*pKiller*/)
        {
            me->DespawnOrUnsummon(4000);
            me->SetRespawnTime(10);
        }
        void UpdateAI(const uint32 diff)
        {
            if (me->isAlive() && !me->isInCombat() && (me->GetDistance2d(me->GetHomePosition().GetPositionX(), me->GetHomePosition().GetPositionY()) <= 1.0f))
                if (Creature* enemy = me->FindNearestCreature(10038, 16.0f, true))
                    me->AI()->AttackStart(enemy);

            if (!UpdateVictim())
                return;

            DoMeleeAttackIfReady();

            if (me->HasReactState(REACT_AGGRESSIVE) && Timer <= diff)
            {
                me->CastSpell(me->GetVictim(),3106,true);
                Timer = 5000;
            }
            else Timer -= diff;

        }
    };
};

enum eStalvan
{
    NPC_STALVAN = 315,
    SPELL_FETID_BREATH = 85234
};
class npc_stalvan : public CreatureScript
{
    public:
        npc_stalvan() : CreatureScript("npc_stalvan") { }

        CreatureAI* GetAI(Creature* pCreature) const
        {
            return new npc_stalvanAI (pCreature);
        }

        struct npc_stalvanAI : public ScriptedAI
        {
            npc_stalvanAI(Creature* creature) : ScriptedAI(creature) {}

            uint32 StepTimer;
            uint32 FetidBreathTimer;
            uint8 Step;

            void Reset()
            {
                me->RestoreFaction();
                me->GetMotionMaster()->MoveTargetedHome();
                me->SetFlag(UNIT_FIELD_FLAGS,UNIT_FLAG_NON_ATTACKABLE);
                me->SetReactState(REACT_PASSIVE);
                Step = 0;
                StepTimer = 3000;
                FetidBreathTimer = 8000;
            }
            void JustDied(Unit*)
            {
                me->MonsterSay("You see, brother... we're not so different...",0,0);
            }

            void UpdateAI(const uint32 diff)
            {
                if(StepTimer <= diff)
                {
                    switch(Step)
                    {
                        case 0: me->MonsterSay("My ring... Who holds my family ring... Tilloa, is that you?",0,0); StepTimer = 6000; Step++; break;
                        case 1: me->MonsterSay("Tobias...",0,0); StepTimer = 6000; Step++; break;
                        case 2: me->MonsterSay("It's all true, brother. Every word. You doubted it?",0,0); StepTimer = 7000; Step++; break;
                        case 3: me->MonsterSay("You know why!",0,0); StepTimer = 4000; Step++; break;
                        case 4: me->MonsterSay("Surely you've felt anger. Anger so foul and vicious that it makes you want to tear someone to shreds...",0,0); StepTimer = 6000; Step++; break;
                        case 5: me->MonsterSay("Aren't you feeling it right now?",0,0); StepTimer = 2000; Step++; break;
                        case 6: 
                        {
                            me->SetReactState(REACT_AGGRESSIVE);
                            me->RemoveFlag(UNIT_FIELD_FLAGS,UNIT_FLAG_NON_ATTACKABLE);
                            Step++;
                        } break;
                        default: break;
                    }
                } else StepTimer -= diff;

                if(Step == 7)
                {
                    if(FetidBreathTimer <= diff)
                    {
                        me->CastSpell(me, SPELL_FETID_BREATH, false);
                        FetidBreathTimer = urand(8000, 25000);
                    } else FetidBreathTimer -= diff;
                }
                DoMeleeAttackIfReady();
            }
        };
};
enum eTobias
{
    NPC_TOBIAS = 43453
};
class npc_tobias : public CreatureScript
{
    public:
        npc_tobias() : CreatureScript("npc_tobias") { }

        CreatureAI* GetAI(Creature* pCreature) const
        {
            return new npc_tobiasAI (pCreature);
        }

        struct npc_tobiasAI : public ScriptedAI
        {
            npc_tobiasAI(Creature* creature) : ScriptedAI(creature) {}
    
            uint8 Phase;
            uint32 PhaseTimer;

            void Reset()
            {
                PhaseTimer = 4000;
                Phase = 0;
                me->RestoreDisplayId();    
            }
            
            void UpdateAI(const uint32 diff)
            {
                if(PhaseTimer<=diff)
                {
                    if(Creature* pStalvan = me->FindNearestCreature(NPC_STALVAN, 30.0f, true))
                    {
                        switch(Phase)
                        {
                            case 0: me->MonsterSay("Brother!",0,0); PhaseTimer = 6000; me->RemoveFlag(UNIT_NPC_FLAGS,UNIT_NPC_FLAG_QUESTGIVER); Phase++; break;
                            case 1: me->MonsterSay("Tell me it's not true, brother. Tell me you didn't die a murderer!",0,0); PhaseTimer = 7000; Phase++; break;
                            case 2: me->MonsterSay("But why?! How could you?",0,0); PhaseTimer = 15000; Phase++; break;
                            case 3:
                            {
                                me->MonsterYell("No. NO! STOP IT!",0,0);
                                me->CastSpell(me,84798,true);
                                me->CombatStart(pStalvan,true);
                                me->Attack(pStalvan,true);
                                me->AddThreat(pStalvan,1000.0f);
                                PhaseTimer = 2000;
                                Phase++;
                            }break;
                            case 4: 
                            {
                                if(pStalvan->isDead())
                                    me->MonsterSay("No...",0,0); PhaseTimer = 2000; Phase++;
                            }break;
                            case 5:
                            {
                                me->GetMotionMaster()->MoveCharge(-10362.98f, -1220.066f, 39.45f, 15.0f);
                                me->SetFlag(UNIT_NPC_FLAGS,UNIT_NPC_FLAG_QUESTGIVER);
                                me->DespawnOrUnsummon(6000);
                            }break;
                            default: break;
                        }
                    }
                }else PhaseTimer -= diff;

                DoMeleeAttackIfReady();
            }
        };
};

enum eGuard
{
    QUEST_REVENGE = 26727,
    NPC_STITCHES = 43862,
};

#define stitches 43862

class npc_nightwatch : public CreatureScript
{
public:
    npc_nightwatch() : CreatureScript("npc_nightwatch") { }

    CreatureAI* GetAI(Creature* pCreature) const
    {
        return new npc_nightwatchAI(pCreature);
    }

    struct npc_nightwatchAI : public ScriptedAI
    {
        npc_nightwatchAI(Creature *c) : ScriptedAI(c) {}

        void Reset(){ }
        void DamageTaken(Unit * pWho, uint32 &uiDamage)
        {
            if (pWho->GetTypeId() == TYPEID_PLAYER)
            {
                me->getThreatManager().resetAllAggro();
                pWho->AddThreat(me, 100000.0f);
                me->AddThreat(pWho, 100000.0f);
                me->AI()->AttackStart(pWho);
            }
            else if (pWho->isPet())
            {
                me->getThreatManager().resetAllAggro();
                me->AddThreat(pWho, 100000.0f);
                me->AI()->AttackStart(pWho);
            }
            else uiDamage = 0;
        }
        void DamageDealt(Unit* target, uint32& damage, DamageEffectType damageType)
        {
            if (target->GetEntry() == stitches)
                damage = 0;
        }
        void JustDied(Unit* /*pKiller*/)
        {
            me->SetRespawnTime(10);
        }
        void UpdateAI(const uint32 diff)
        {
            //This make CPU usage increase, need to set every something (time, event)
            if (me->isAlive() && !me->isInCombat() && (me->GetDistance2d(me->GetHomePosition().GetPositionX(), me->GetHomePosition().GetPositionY()) <= 1.0f))
                if (Creature* enemy = me->FindNearestCreature(NPC_STITCHES, 16.0f, true))
                    me->AI()->AttackStart(enemy);

            DoMeleeAttackIfReady();
        }
    };
};

enum eMoundofLooseDirt
{
    SPELL_STUNNING_POUNCE       = 81949,
    SPELL_STUNNING_POUNCE_A     = 81957
};

class npc_lurking_worgen : public CreatureScript
{
public:
    npc_lurking_worgen() : CreatureScript("npc_lurking_worgen") { }

    CreatureAI* GetAI(Creature* pCreature) const
    {
        return new npc_lurking_worgenAI(pCreature);
    }

    struct npc_lurking_worgenAI : public ScriptedAI
    {
        npc_lurking_worgenAI(Creature *c) : ScriptedAI(c) {}

        uint8 Phase;
        uint32 StunningPounceTimer;

        void Reset()
        {
            Phase = 0;
            StunningPounceTimer = 0; 
        }
        void UpdateAI(const uint32 uiDiff)
        {
            ScriptedAI::UpdateAI(uiDiff);

            if (Player* pPlayer = me->FindNearestPlayer(20.0f,true))		
            {
                if(pPlayer->GetQuestStatus(26717) == QUEST_STATUS_INCOMPLETE)
                {
                    switch(Phase)
                    {
                        case 0:
                        {
                            me->CastSpell(pPlayer,SPELL_STUNNING_POUNCE_A,true); 
                            Phase++;
                            StunningPounceTimer=2500;
                        } break;
                        //This can work?
                        if(StunningPounceTimer <= uiDiff)
                        {
                            case 1:
                            {
                                pPlayer->CastSpell(pPlayer,SPELL_STUNNING_POUNCE,true);
                                pPlayer->AreaExploredOrEventHappens(26717);
                                me->GetMotionMaster()->MoveCharge(-11124.71f, -499.84f, 34.95f, 8.0f);
                                me->DespawnOrUnsummon(5000);
                            } break;
                        }else StunningPounceTimer -= uiDiff;
                        default: break;
                    }
                }
            }
        }
    };
};

class go_mound_dirt : public GameObjectScript
{
public:
    go_mound_dirt() : GameObjectScript("go_mound_dirt") { }

    bool OnGossipHello(Player *pPlayer, GameObject *pGO)
    {
       if(pPlayer->GetQuestStatus(26717) == QUEST_STATUS_INCOMPLETE)
            pPlayer->SummonCreature(43799,-11127.5f, -518.42f, 35.25f, 0.43f, TEMPSUMMON_CORPSE_DESPAWN, 0);
        return true;
    }
};

enum PotWorgen
{
    SPELL_HARRIS_AMPULE = 82058
};

class npc_lurking_potion : public CreatureScript
{
    public:
        npc_lurking_potion() : CreatureScript("npc_lurking_potion") { }

        CreatureAI* GetAI(Creature* pCreature) const
        {
            return new npc_lurking_potionAI (pCreature);
        }

        struct npc_lurking_potionAI : public ScriptedAI
        {
            npc_lurking_potionAI(Creature* creature) : ScriptedAI(creature) {}

            bool bCast;
            bool bSay;

            void Reset()
            {
                bSay=false;
                bCast=false;
                me->SetRespawnTime(10);
                me->RemoveFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_DISABLE_MOVE);
            }
            
            void SpellHit(Unit* caster, const SpellInfo* pSpell)
            {
                if (pSpell->Id == SPELL_HARRIS_AMPULE && caster->GetTypeId() == TYPEID_PLAYER && HealthBelowPct(25))
                {
                    caster->ToPlayer()->KilledMonsterCredit(43860,0);
                    me->DespawnOrUnsummon(2500);
                }
            }
            void UpdateAI(const uint32 uiDiff)
            {
                if (Player* pPlayer = me->FindNearestPlayer(10.0f,true))
                {			
                    if(pPlayer->GetQuestStatus(26720) == QUEST_STATUS_INCOMPLETE)
                    {
                        if(!bCast)
                        {
                            me->CastSpell(pPlayer,81957,true);
                            me->Attack(pPlayer,true);
                            bCast=true;
                        }

                    
                        if(HealthBelowPct(25))
                        {
                            if(!bSay)
                            {
                                me->MonsterTextEmote("The worgen stares and hesitates!",0,true);
                                me->AttackStop();
                                me->SetReactState(REACT_PASSIVE);
                                me->SetFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_DISABLE_MOVE);
								pPlayer->KilledMonsterCredit(43860,0);// Hack since the spell hit not working
                                me->DespawnOrUnsummon(2500);
                                bSay=true;
                            }
                        }				
                    }
                }
                DoMeleeAttackIfReady();
            }
        };
};

enum eCryMoon
{
    QUEST_CRY_FOR_THE_MOON	= 26760,
    NPC_OLIVER_HARRIS	    = 43858, //This is the summend one
    NPC_JITTERS		        = 43859,
    NPC_LUR_WORGEN		    = 43950,
    SPELL_INVISIBILITY_7	= 82288,
    SPELL_INVISIBILITY_8	= 82289,
    SPELL_CHOKING_JITTERS	= 82262,
    SPELL_CHOKED_BY_SVEN	= 82266,
    SPELL_EJECT_PASSENGERS	= 65785,
    SPELL_KILL_CREDIT	    = 82286
};
class npc_oliver_harris : public CreatureScript
{
    public:
        npc_oliver_harris() : CreatureScript("npc_oliver_harris") { }

        CreatureAI* GetAI(Creature* pCreature) const
        {
            return new npc_oliver_harrisAI (pCreature);
        }

    bool OnQuestAccept(Player* pPlayer, Creature* pCreature, Quest const *pQuest)
    {
        if (pQuest->GetQuestId() == QUEST_CRY_FOR_THE_MOON)
        {
            pPlayer->RemoveAurasDueToSpell(SPELL_INVISIBILITY_7);
            pPlayer->RemoveAurasDueToSpell(SPELL_INVISIBILITY_8);
            CAST_AI(npc_oliver_harris::npc_oliver_harrisAI, pCreature->AI())->PlayerGUID = pPlayer->GetGUID();

            if (!pPlayer->FindNearestCreature(NPC_OLIVER_HARRIS, 20.0f, true))
            {
                pPlayer->SummonCreature(NPC_JITTERS,-10748.52f,333.62f,37.46f,5.37f,TEMPSUMMON_TIMED_OR_DEAD_DESPAWN,90000);
                pPlayer->SummonCreature(NPC_OLIVER_HARRIS,-10752.87f,338.19f,37.294f,5.48f,TEMPSUMMON_TIMED_OR_DEAD_DESPAWN,90000);//This is the summend one
                pPlayer->SummonCreature(NPC_LUR_WORGEN,-10747.40f,332.28f,37.74f,4.48f,TEMPSUMMON_TIMED_OR_DEAD_DESPAWN,90000);
            }
        }
        return true;
    }		
		
        struct npc_oliver_harrisAI : public ScriptedAI
        {
            npc_oliver_harrisAI(Creature* creature) : ScriptedAI(creature) {}
            
            uint8 Phase;
            uint32 TalkEventTimer;
            uint64 PlayerGUID;
            uint64 Harris;
			
            void Reset()
            {
                Phase = 0;
                TalkEventTimer = 2000;
                //PlayerGUID = pPlayer->GetGUID();
            }

            void UpdateAI(const uint32 diff)
            {
                if(TalkEventTimer <= diff)
                {
                    if (Player* pPlayer = me->GetPlayer(*me,PlayerGUID)) 
                    if (Creature* Jitters = me->FindNearestCreature(NPC_JITTERS,20.0f,true))
                    if (Creature* Worgen = me->FindNearestCreature(NPC_LUR_WORGEN,20.0f,true))
					if (Creature* Harris = me->FindNearestCreature(NPC_OLIVER_HARRIS,20.0f,true))
                    {
                        switch(Phase)
                        {
                            case 0: Harris->GetMotionMaster()->MovePoint(0,-10745.14f,331.53f,37.86f); Jitters->HandleEmoteCommand(69); TalkEventTimer=4000; Phase++; break;
                            case 1: Harris->MonsterSay("Here we go...",0,0); Harris->SetFacingToObject(Worgen); Harris->HandleEmoteCommand(396); TalkEventTimer=3500; Phase++; break;
                            case 2: Harris->MonsterSay("It's working. Hold him still ,Jitters.",0,0);Harris->HandleEmoteCommand(390); TalkEventTimer=2500; Phase++; break;
                            case 3: Jitters->MonsterSay("I...I can't",0,0); TalkEventTimer=2000; Phase++; break;
                            case 4: Worgen->MonsterSay("Jitters...",0,0); TalkEventTimer=1000; Phase++; break;
                            case 5: Harris->MonsterSay("Damn it, Jitters. I said HOLD!",0,0); Harris->HandleEmoteCommand(5); TalkEventTimer=3000; Phase++; break;
                            case 6: Worgen->MonsterYell("JITTERS!",0,0); Worgen->HandleEmoteCommand(53); Worgen->SetOrientation(2.3997f); TalkEventTimer=4000; Phase++; break;
                            case 7: Worgen->MonsterSay("I remember now...it's all your fault!",0,0); Worgen->HandleEmoteCommand(384); Jitters->HandleEmoteCommand(473); TalkEventTimer=4500; Phase++; break;
                            case 8: Worgen->MonsterSay("You brought the worgen to Duskwood! You led the Dark Riders to my farm, and hid while they murdered my family!",0,0); TalkEventTimer=4500; Phase++; break;
                            case 9: Worgen->MonsterYell("Every speak of suffering in my life is YOUR PATHETIC FAULT! I SHOULD KILL YOU!",0,0); TalkEventTimer=9000; Phase++; break;
                            case 10: Harris->MonsterSay("Letting him go is the only thing that's going to separateyou from the beasts now, my friend.",0,0); TalkEventTimer=1500; Phase++; break;
                            case 11: Worgen->MonsterSay("You've got a lot to make up for, Jitters. I won't give you the easy way out.",0,0); TalkEventTimer=2000; Phase++; break;
                            case 12: Worgen->GetMotionMaster()->MovePoint(1,-10761.66f,338.77f,37.82f); TalkEventTimer=6000; Worgen->HandleEmoteCommand(0); Jitters->HandleEmoteCommand(0); Phase++; break;
                            case 13: Worgen->SetFacingToObject(Jitters); Harris->GetMotionMaster()->MoveTargetedHome(); TalkEventTimer=3000; Phase++; break;
                            case 14: Jitters->DespawnOrUnsummon(); Harris->DespawnOrUnsummon(); Worgen->DespawnOrUnsummon(); pPlayer->CastSpell(pPlayer,82288,true); pPlayer->CastSpell(pPlayer,82289,true); pPlayer->KilledMonsterCredit(43969,0); 
							break;
                            default: break;
                        }
                    }
                }else TalkEventTimer -= diff;
            }
        };
};

class npc_forlorn_spirit : public CreatureScript
{
    public:
        npc_forlorn_spirit() : CreatureScript("npc_forlorn_spirit") { }

        CreatureAI* GetAI(Creature* pCreature) const
        {
            return new npc_forlorn_spiritAI (pCreature);
        }

        struct npc_forlorn_spiritAI : public ScriptedAI
        {
        npc_forlorn_spiritAI(Creature *c) : ScriptedAI(c) {}

        void Reset()
        {
            me->CastSpell(me,82199,true);
            if(Player* pPlayer = me->FindNearestPlayer(10.0f,true))
                pPlayer->KilledMonsterCredit(43930,0);
        }		
    };
};

void AddSC_duskwood()
{
    new boss_twilight_corrupter();
    new at_twilight_grove();
    new npc_blaze_darkshire();
    new npc_stitches();
    new npc_stalvan();
    new npc_tobias();
    new npc_nightwatch();
    new go_mound_dirt();
    new npc_lurking_worgen();
    new npc_lurking_potion();
    //new npc_cry_for_the_moon();
    new npc_oliver_harris();
    new npc_forlorn_spirit();
}

