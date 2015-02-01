/*
 * Copyright (C) 2008-2011 TrinityCore <http://www.trinitycore.org/>
 * Copyright (C) 2005-2009 MaNGOS <http://getmangos.com/>
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
SDName: Boss_Ahune
SD%Complete: 100
SDComment: 
SDCategory: Slave Pens / Midsummer fest event
EndScriptData */

#include "ScriptPCH.h"

enum Spells
{
    SPELL_SUM_VISUAL        = 45937,
    SPELL_AHUNES_SHIELD     = 45954,
    SPELL_COLD_SLAP         = 46145,
    SPELL_ICE_SPEAR         = 46359,
    SPELL_FROST_AURA        = 45937,
    SPELL_BONFIRE           = 45930,

    SPELL_CHILLING_AURA     = 46542, // Used by Ahunite Hailstone

    //SPELL_SUMM_CHEST        = 45939, this makes the chest spawn with infinite loot
};

enum Creatures
{
    NPC_AHUNITE_HAILSTONE   = 25755,
    NPC_AHUNITE_FROSTWIND   = 25757,
    NPC_AHUNTIE_COLD_WAVE   = 25756,
    NPC_FROZEN_CORE         = 25865,
};

enum Phases
{
    PHASE_1         = 1,
    PHASE_2         = 2,
};

enum Timers
{
    TIMER_COLD_SLAP         = 1000,
    TIMER_ICE_SPEAR         = 6000,
    TIMER_SUMM_ADDS         = 7000,
    TIMER_SUBMERGE          = 90000,
    TIMER_EMERGE            = 30000,
};

enum Encounters
{
    BOSS_AHUNE              = 1,
};

enum GO
{
    GO_ICE_CHEST        = 187892,
};

const Position Pos[3] =
{
    {-98.27f, -215.39f, -1.27f, 1.46f},
    {-90.66f, -208.18f, -1.14f, 1.74f},
    {-103.97f, -208.33f, -1.28f, 1.59f},
};

class boss_ahune : public CreatureScript
{
    public:
        boss_ahune() : CreatureScript("boss_ahune") { }
                
    struct boss_ahuneAI : public BossAI
    {
        boss_ahuneAI(Creature *pCreature) : BossAI(pCreature, BOSS_AHUNE), summons(me)
        {
            me->ApplySpellImmune(0, IMMUNITY_EFFECT, SPELL_EFFECT_KNOCK_BACK, true);
            me->ApplySpellImmune(0, IMMUNITY_MECHANIC, MECHANIC_GRIP, true);
            me->SetFlag(UNIT_FIELD_FLAGS,UNIT_FLAG_DISABLE_MOVE);
            frozenCore = NULL;
        }

        Phases phase;
        Unit* frozenCore;

        uint32 uiColdSlapTimer;
        uint32 uiIceSpearTimer;
        uint32 uiSummAddsTimer;
        uint32 uiSubmergeTimer;
        uint32 uiMergeTimer;
        
        uint8 uiCyclesCount;

        SummonList summons;

        void Reset()
        {
            _Reset();
            summons.DespawnAll();
            phase = PHASE_1;
            if (frozenCore && frozenCore->ToCreature())
                frozenCore->ToCreature()->DespawnOrUnsummon();
            frozenCore = NULL;

            me->RemoveFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_NOT_SELECTABLE);
            me->SetUInt32Value(UNIT_NPC_EMOTESTATE, 0);

            DoCast(me, SPELL_AHUNES_SHIELD, true);
        };

        void JustDied(Unit * /*victim*/) 
        {
            _JustDied();
            //DoCast(me,SPELL_SUMM_CHEST,true);
            me->setFaction(35);
            me->SummonGameObject(GO_ICE_CHEST, -92.1535f, -227.2263f, -1.22206f, 1.668f, 0, 0, 1, 1, 0);
        };

        void EnterCombat(Unit* /*pWho*/)
        {
            _EnterCombat();
            phase = PHASE_1;
            DoCast(me,SPELL_SUM_VISUAL,true);
            me->SetReactState(REACT_PASSIVE);

            uiColdSlapTimer = TIMER_COLD_SLAP;
            uiIceSpearTimer = TIMER_ICE_SPEAR;
            uiSummAddsTimer = TIMER_SUMM_ADDS;
            uiSubmergeTimer = TIMER_SUBMERGE;
            uiMergeTimer = TIMER_EMERGE;
            uiCyclesCount = 0;

            me->SummonCreature(NPC_AHUNITE_HAILSTONE, Pos[2], TEMPSUMMON_CORPSE_TIMED_DESPAWN, 3000);
            me->LowerPlayerDamageReq(me->GetHealth());
        };

        void JustSummoned(Creature* summon)
        {
            summons.Summon(summon);
        }
        
        void UpdateAI(const uint32 diff)
        {
            if (!UpdateVictim())
                return;

            if (phase == PHASE_1)
            {
                if (uiColdSlapTimer <= diff)
                {
                    if (Unit *pTarget = SelectTarget(SELECT_TARGET_NEAREST, 0, 10, true))
                        pTarget->CastSpell(pTarget, SPELL_COLD_SLAP, true);
                    uiColdSlapTimer = TIMER_COLD_SLAP;
                }
                else
                    uiColdSlapTimer -= diff;

                if (uiIceSpearTimer <= diff)
                {
                    if (Unit *pTarget = SelectTarget(SELECT_TARGET_RANDOM, 0, 80, true))
                        me->CastSpell(pTarget, SPELL_ICE_SPEAR, true);
                    uiIceSpearTimer = TIMER_ICE_SPEAR;
                }
                else
                    uiIceSpearTimer -= diff;

                if (uiSummAddsTimer <= diff)
                {
                    me->SummonCreature(NPC_AHUNITE_FROSTWIND, Pos[1], TEMPSUMMON_CORPSE_TIMED_DESPAWN, 3000);
                    for (uint8 i = 0; i <= uiCyclesCount; ++i)
                        me->SummonCreature(NPC_AHUNTIE_COLD_WAVE, Pos[1], TEMPSUMMON_CORPSE_TIMED_DESPAWN, 3000);
                    uiSummAddsTimer = TIMER_SUMM_ADDS;
                }
                else
                    uiSummAddsTimer -= diff;

                if (uiSubmergeTimer <= diff)
                {
                    //set p2
                    phase = PHASE_2;
                    DoCast(me,SPELL_BONFIRE,true);

                    //set ahune untouchable
                    me->AttackStop();
                    me->InterruptNonMeleeSpells(false);
                    me->SetFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_NOT_SELECTABLE);
                    me->SetUInt32Value(UNIT_NPC_EMOTESTATE, EMOTE_STATE_SUBMERGED);
                    me->HandleEmoteCommand(EMOTE_ONESHOT_SUBMERGE);
                    me->RemoveAura(SPELL_AHUNES_SHIELD);

                    //expose core
                    Position pos;
                    me->GetPosition(&pos);
                    frozenCore = me->SummonCreature(NPC_FROZEN_CORE, pos, TEMPSUMMON_CORPSE_TIMED_DESPAWN, 3000);
                    if (frozenCore)
                    {
                        frozenCore->SetMaxHealth(me->GetMaxHealth());
                        frozenCore->SetHealth(me->GetHealth());
                    }
                    uiSubmergeTimer = TIMER_SUBMERGE;
                }
                else
                    uiSubmergeTimer -= diff;
            }
            else
            {
                if (uiMergeTimer <= diff)
                {
                    //set p1
                    phase = PHASE_1;

                    //set ahune touchable again
                    me->RemoveFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_NOT_SELECTABLE);
                    me->SetUInt32Value(UNIT_NPC_EMOTESTATE, 0);
                    me->HandleEmoteCommand(EMOTE_ONESHOT_EMERGE);
                    DoCast(me,SPELL_AHUNES_SHIELD,true);
                                    
                    //unexpose core
                    if (frozenCore && frozenCore->ToCreature())
                    {
                        frozenCore->ToCreature()->DespawnOrUnsummon();
                        frozenCore = NULL;
                    }
                    me->SummonCreature(NPC_AHUNITE_HAILSTONE, Pos[2], TEMPSUMMON_CORPSE_TIMED_DESPAWN, 3000);

                    ++uiCyclesCount;                    
                    uiMergeTimer = TIMER_EMERGE;
                }
                else
                    uiMergeTimer -= diff;
            }
        }
    };

    CreatureAI* GetAI(Creature* pCreature) const
    {
        return new boss_ahuneAI(pCreature);
    }
};

class npc_frozen_core : public CreatureScript
{
    public:
        npc_frozen_core() : CreatureScript("npc_frozen_core") { }
                
    struct npc_frozen_coreAI : public ScriptedAI
    {
        npc_frozen_coreAI(Creature *pCreature) : ScriptedAI(pCreature)
        {
            me->ApplySpellImmune(0, IMMUNITY_EFFECT, SPELL_EFFECT_KNOCK_BACK, true);
            me->ApplySpellImmune(0, IMMUNITY_MECHANIC, MECHANIC_GRIP, true);
            me->SetFlag(UNIT_FIELD_FLAGS,UNIT_FLAG_DISABLE_MOVE);
            me->SetReactState(REACT_PASSIVE);
        }

        void DamageTaken(Unit* pDone, uint32& damage)
        {
            if (Unit *pAhune = me->ToTempSummon()->GetSummoner())
            {
                if (damage > me->GetHealth())
                    damage = me->GetHealth();
                
                if (pDone)
                    pDone->DealDamage(pAhune, damage);
            }
        }
    };

    CreatureAI* GetAI(Creature* pCreature) const
    {
        return new npc_frozen_coreAI(pCreature);
    }
};

enum IceSpear
{
    SPELL_SUMMON_ICE_SPEAR_OBJECT       = 46369,
    SPELL_KNOCKBACK                     = 46360,
    GO_ICE_SPEAR_ENTRY                  = 188077,
};

class npc_ice_spear : public CreatureScript
{
public:
    npc_ice_spear() : CreatureScript("npc_ice_spear") { }

    CreatureAI* GetAI(Creature* pCreature) const
    {
        return new npc_ice_spearAI (pCreature);
    }

    struct npc_ice_spearAI : public ScriptedAI
    {
        npc_ice_spearAI(Creature *pCreature) : ScriptedAI(pCreature)
        {
            me->SetFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_NOT_SELECTABLE | UNIT_FLAG_NON_ATTACKABLE | UNIT_FLAG_PACIFIED);
            me->SetReactState(REACT_PASSIVE);
            me->SetFlag(UNIT_FIELD_FLAGS,UNIT_FLAG_DISABLE_MOVE);
            DoCast(me, SPELL_SUMMON_ICE_SPEAR_OBJECT, true);
        }

        uint32 iceSpearTimer;
        bool despawn;

        void Reset()
        {
            iceSpearTimer = 3000;
            despawn = false;
        }

        void UpdateAI(const uint32 diff)
        {
            if (iceSpearTimer <= diff)
            {
                if (!despawn)
                {
                    if (GameObject* pSpear = me->FindNearestGameObject(GO_ICE_SPEAR_ENTRY, 5))
                        pSpear->UseDoorOrButton(0);
                    if (Unit* pTarget = SelectTarget(SELECT_TARGET_NEAREST, 0, 4, true))
                        if (pTarget->GetTypeId() == TYPEID_PLAYER)
                            pTarget->CastSpell(pTarget, SPELL_KNOCKBACK, true);
                    iceSpearTimer = 3000;
                    despawn = true;
                }
                else
                {
                    if (GameObject* pSpear = me->FindNearestGameObject(GO_ICE_SPEAR_ENTRY, 5))
                        me->RemoveGameObject(pSpear, true);
                    me->DisappearAndDie();
                    iceSpearTimer = 10000;
                }
            }
            else iceSpearTimer -= diff;
        }
    };
};


void AddSC_boss_ahune()
{
    new boss_ahune();
    new npc_frozen_core();
    new npc_ice_spear();;
}