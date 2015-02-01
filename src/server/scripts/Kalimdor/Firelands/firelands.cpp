/*
 * Copyright (C) 2013 WoW Source  <http://wowsource.info/>
 *
 * Copyright (C) 2013 WoWSource [WS] <http://wowsource.info/>
 *
 * Dont Share The SourceCode
 * and read our WoWSource Terms
 *
 */

#include "ScriptPCH.h"
#include "firelands.h"

enum Adds
{
    NPC_MAGMAKIN = 54144, NPC_MAGMA_CONDUIT = 54145, // 97699, 98250, 100746
};

enum Spells
{
    // Ancient Core Hound
    SPELL_DINNER_TIME = 99693,
    SPELL_DINNER_TIME_VEHICLE = 99694,
    SPELL_FLAME_BREATH = 99736,
    SPELL_TERRIFYING_ROAR = 99692,

    // Ancient Lava Dweller
    SPELL_LAVA_SHOWER = 97549,
    SPELL_LAVA_SHOWER_MISSILE = 97551,
    SPELL_LAVA_SPIT = 97306,

    // Fire Scorpion
    SPELL_FIERY_BLOOD = 99993,
    SPELL_SLIGHTLY_WARM_PINCERS = 99984,

    // Fire Turtle Hatchling 
    SPELL_SHELL_SPIN = 100263,

    // Fire Archon
    SPELL_FLAME_TORRENT = 100795,
    SPELL_FIERY_TORMENT = 100797,
    SPELL_FIERY_TORMENT_DMG = 100802,

    // Molten Lord
    SPELL_FLAME_STOMP = 99530,
    SPELL_MELT_ARMOR = 99532,
    SPELL_SUMMON_LAVA_JETS = 99555,
    SPELL_SUMMON_LAVA_JET = 99538,

    // Legendary Questline
    SPELL_FIEROBLAST = 100094,
    SPELL_FIEROCLAST_BARRAGE = 100095,
    SPELL_FIRE_CHANNELING = 100109,

    SPELL_BR_NORD_BOSS_COS = 100255,

    SPELL_BLAZING_STOMP = 100156,
    SPELL_BLESSED_DEFENDER = 100274,
    SPELL_SMOULDERING_ROOTS = 100146
};

enum Events
{
    // Ancient Core Hound
    EVENT_DINNER_TIME = 1,
    EVENT_FLAME_BREATH = 2,
    EVENT_TERRIFYING_ROAR = 3,

    // Ancient Lava Dweller
    EVENT_LAVA_SHOWER = 4,

    // Fire Turtle Hatchling
    EVENT_SHELL_SPIN = 5,

    // Fire Archon
    EVENT_FIERY_TORMENT = 6,
    EVENT_FLAME_TORRENT = 7,

    // Molten Lord
    EVENT_MELT_ARMOR = 8,
    EVENT_FLAME_STOMP = 9,
    EVENT_SUMMON_LAVA_JETS = 10,

    // Legendary Questline
    EVENT_FIEROBLAST = 11,
    EVENT_FIEROCLAST_BARRAGE = 12,
    EVENT_BLAZING_STOMP = 13,
    EVENT_BLESSED_DEFENDER = 14,
    EVENT_SMOULDERING_ROOTS = 15,
    EVENT_SUMMON_TREANTS = 16,
    EVENT_ACTION_PHASE2_0 = 17,
    EVENT_ACTION_PHASE2_1 = 18,
    EVENT_ACTION_PHASE2_2 = 19,
    EVENT_ACTION_PHASE2_3 = 20,
    EVENT_ACTION_PHASE2_4 = 21,
    EVENT_ACTION_PHASE2_5 = 22,
    EVENT_ACTION_PHASE2_6 = 23
};

enum Talk
{
    SAY_STOP = 0, // Phase 1 End
    SAY_HISTORY_1 = 1,
    SAY_HISTORY_2 = 2,
    SAY_HISTORY_3 = 3,
    SAY_BIN_DA_WER_NOCH = 4
};

class npc_firelands_ancient_core_hound: public CreatureScript
{
    public:
        npc_firelands_ancient_core_hound() :
                CreatureScript("npc_firelands_ancient_core_hound")
        {
        }

        CreatureAI* GetAI(Creature* pCreature) const
        {
            return new npc_firelands_ancient_core_houndAI(pCreature);
        }

        struct npc_firelands_ancient_core_houndAI: public ScriptedAI
        {
                npc_firelands_ancient_core_houndAI(Creature* pCreature) :
                        ScriptedAI(pCreature)
                {
                    me->ApplySpellImmune(0, IMMUNITY_EFFECT, SPELL_EFFECT_KNOCK_BACK, true);
                    me->ApplySpellImmune(0, IMMUNITY_MECHANIC, MECHANIC_GRIP, true);
                    me->ApplySpellImmune(0, IMMUNITY_MECHANIC, MECHANIC_STUN, true);
                    me->ApplySpellImmune(0, IMMUNITY_MECHANIC, MECHANIC_FEAR, true);
                    me->ApplySpellImmune(0, IMMUNITY_MECHANIC, MECHANIC_ROOT, true);
                    me->ApplySpellImmune(0, IMMUNITY_MECHANIC, MECHANIC_FREEZE, true);
                    me->ApplySpellImmune(0, IMMUNITY_MECHANIC, MECHANIC_POLYMORPH, true);
                    me->ApplySpellImmune(0, IMMUNITY_MECHANIC, MECHANIC_HORROR, true);
                    me->ApplySpellImmune(0, IMMUNITY_MECHANIC, MECHANIC_SAPPED, true);
                    me->ApplySpellImmune(0, IMMUNITY_MECHANIC, MECHANIC_CHARM, true);
                    me->ApplySpellImmune(0, IMMUNITY_MECHANIC, MECHANIC_DISORIENTED, true);
                }

                EventMap events;

                void Reset()
                {
                    events.Reset();
                }

                void EnterCombat(Unit* /*who*/)
                {
                    events.ScheduleEvent(EVENT_DINNER_TIME, urand(15000, 20000));
                    events.ScheduleEvent(EVENT_TERRIFYING_ROAR, urand(8000, 20000));
                    events.ScheduleEvent(EVENT_FLAME_BREATH, urand(5000, 10000));
                }

                void UpdateAI(uint32 const diff)
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
                            case EVENT_DINNER_TIME:
                                DoCastVictim(SPELL_DINNER_TIME);
                                events.ScheduleEvent(EVENT_DINNER_TIME, urand(30000, 40000));
                                break;
                            case EVENT_FLAME_BREATH:
                                DoCastVictim(SPELL_FLAME_BREATH);
                                events.ScheduleEvent(EVENT_FLAME_BREATH, urand(15000, 20000));
                                break;
                            case EVENT_TERRIFYING_ROAR:
                                DoCast(me, SPELL_TERRIFYING_ROAR);
                                events.ScheduleEvent(EVENT_TERRIFYING_ROAR, urand(30000, 35000));
                                break;
                        }
                    }

                    DoMeleeAttackIfReady();
                }
        };
};

class npc_firelands_ancient_lava_dweller: public CreatureScript
{
    public:
        npc_firelands_ancient_lava_dweller() :
                CreatureScript("npc_firelands_ancient_lava_dweller")
        {
        }

        CreatureAI* GetAI(Creature* pCreature) const
        {
            return new npc_firelands_ancient_lava_dwellerAI(pCreature);
        }

        struct npc_firelands_ancient_lava_dwellerAI: public Scripted_NoMovementAI
        {
                npc_firelands_ancient_lava_dwellerAI(Creature* pCreature) :
                        Scripted_NoMovementAI(pCreature)
                {
                }

                EventMap events;

                void Reset()
                {
                    events.Reset();
                }

                void EnterCombat(Unit* /*who*/)
                {
                    events.ScheduleEvent(EVENT_LAVA_SHOWER, urand(15000, 20000));
                }

                void UpdateAI(uint32 const diff)
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
                            case EVENT_LAVA_SHOWER:
                                DoCast(me, SPELL_LAVA_SHOWER);
                                events.ScheduleEvent(EVENT_LAVA_SHOWER, urand(45000, 55000));
                                break;
                        }
                    }

                    DoSpellAttackIfReady(SPELL_LAVA_SPIT);
                }
        };
};

class npc_firelands_fire_scorpion: public CreatureScript
{
    public:
        npc_firelands_fire_scorpion() :
                CreatureScript("npc_firelands_fire_scorpion")
        {
        }

        CreatureAI* GetAI(Creature* pCreature) const
        {
            return new npc_firelands_fire_scorpionAI(pCreature);
        }

        struct npc_firelands_fire_scorpionAI: public ScriptedAI
        {
                npc_firelands_fire_scorpionAI(Creature* pCreature) :
                        ScriptedAI(pCreature)
                {
                    me->ApplySpellImmune(0, IMMUNITY_MECHANIC, MECHANIC_SNARE, true);
                }

                void JustDied(Unit* killer)
                {
                    DoCast(me, SPELL_FIERY_BLOOD);
                }
        };
};

class npc_firelands_fire_turtle_hatchling: public CreatureScript
{
    public:
        npc_firelands_fire_turtle_hatchling() :
                CreatureScript("npc_firelands_fire_turtle_hatchling")
        {
        }

        CreatureAI* GetAI(Creature* pCreature) const
        {
            return new npc_firelands_fire_turtle_hatchlingAI(pCreature);
        }

        struct npc_firelands_fire_turtle_hatchlingAI: public ScriptedAI
        {
                npc_firelands_fire_turtle_hatchlingAI(Creature* pCreature) :
                        ScriptedAI(pCreature)
                {
                    me->ApplySpellImmune(0, IMMUNITY_EFFECT, SPELL_EFFECT_KNOCK_BACK, true);
                    me->ApplySpellImmune(0, IMMUNITY_MECHANIC, MECHANIC_GRIP, true);
                    me->ApplySpellImmune(0, IMMUNITY_MECHANIC, MECHANIC_STUN, true);
                    me->ApplySpellImmune(0, IMMUNITY_MECHANIC, MECHANIC_FEAR, true);
                    me->ApplySpellImmune(0, IMMUNITY_MECHANIC, MECHANIC_ROOT, true);
                    me->ApplySpellImmune(0, IMMUNITY_MECHANIC, MECHANIC_FREEZE, true);
                    me->ApplySpellImmune(0, IMMUNITY_MECHANIC, MECHANIC_POLYMORPH, true);
                    me->ApplySpellImmune(0, IMMUNITY_MECHANIC, MECHANIC_HORROR, true);
                    me->ApplySpellImmune(0, IMMUNITY_MECHANIC, MECHANIC_SAPPED, true);
                    me->ApplySpellImmune(0, IMMUNITY_MECHANIC, MECHANIC_CHARM, true);
                    me->ApplySpellImmune(0, IMMUNITY_MECHANIC, MECHANIC_DISORIENTED, true);
                }

                EventMap events;

                void Reset()
                {
                    events.Reset();
                }

                void EnterCombat(Unit* /*who*/)
                {
                    events.ScheduleEvent(EVENT_SHELL_SPIN, urand(10000, 20000));
                }

                void UpdateAI(uint32 const diff)
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
                            case EVENT_SHELL_SPIN:
                                DoCast(me, SPELL_SHELL_SPIN);
                                events.ScheduleEvent(EVENT_SHELL_SPIN, urand(35000, 50000));
                                break;
                        }
                    }

                    DoMeleeAttackIfReady();
                }
        };
};

class npc_firelands_flame_archon: public CreatureScript
{
    public:
        npc_firelands_flame_archon() :
                CreatureScript("npc_firelands_flame_archon")
        {
        }

        CreatureAI* GetAI(Creature* pCreature) const
        {
            return new npc_firelands_flame_archonAI(pCreature);
        }

        struct npc_firelands_flame_archonAI: public ScriptedAI
        {
                npc_firelands_flame_archonAI(Creature* pCreature) :
                        ScriptedAI(pCreature)
                {
                    me->ApplySpellImmune(0, IMMUNITY_EFFECT, SPELL_EFFECT_KNOCK_BACK, true);
                    me->ApplySpellImmune(0, IMMUNITY_MECHANIC, MECHANIC_GRIP, true);
                    me->ApplySpellImmune(0, IMMUNITY_MECHANIC, MECHANIC_STUN, true);
                    me->ApplySpellImmune(0, IMMUNITY_MECHANIC, MECHANIC_FEAR, true);
                    me->ApplySpellImmune(0, IMMUNITY_MECHANIC, MECHANIC_ROOT, true);
                    me->ApplySpellImmune(0, IMMUNITY_MECHANIC, MECHANIC_FREEZE, true);
                    me->ApplySpellImmune(0, IMMUNITY_MECHANIC, MECHANIC_POLYMORPH, true);
                    me->ApplySpellImmune(0, IMMUNITY_MECHANIC, MECHANIC_HORROR, true);
                    me->ApplySpellImmune(0, IMMUNITY_MECHANIC, MECHANIC_SAPPED, true);
                    me->ApplySpellImmune(0, IMMUNITY_MECHANIC, MECHANIC_CHARM, true);
                    me->ApplySpellImmune(0, IMMUNITY_MECHANIC, MECHANIC_DISORIENTED, true);
                }

                EventMap events;

                void Reset()
                {
                    events.Reset();
                }

                void EnterCombat(Unit* /*who*/)
                {
                    events.ScheduleEvent(EVENT_FLAME_TORRENT, 10000);
                    events.ScheduleEvent(EVENT_FIERY_TORMENT, 20000);
                }

                void UpdateAI(uint32 const diff)
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
                            case EVENT_FLAME_TORRENT:
                                DoCast(me, SPELL_FLAME_TORRENT);
                                events.ScheduleEvent(EVENT_FLAME_TORRENT, 40000);
                                break;
                            case EVENT_FIERY_TORMENT:
                                DoCast(me, SPELL_FIERY_TORMENT);
                                events.ScheduleEvent(EVENT_FIERY_TORMENT, 40000);
                                break;
                        }
                    }

                    DoMeleeAttackIfReady();
                }
        };
};

class npc_firelands_molten_lord: public CreatureScript
{
    public:
        npc_firelands_molten_lord() :
                CreatureScript("npc_firelands_molten_lord")
        {
        }

        CreatureAI* GetAI(Creature* pCreature) const
        {
            return new npc_firelands_molten_lordAI(pCreature);
        }

        struct npc_firelands_molten_lordAI: public ScriptedAI
        {
                npc_firelands_molten_lordAI(Creature* pCreature) :
                        ScriptedAI(pCreature)
                {
                    me->ApplySpellImmune(0, IMMUNITY_EFFECT, SPELL_EFFECT_KNOCK_BACK, true);
                    me->ApplySpellImmune(0, IMMUNITY_MECHANIC, MECHANIC_GRIP, true);
                    me->ApplySpellImmune(0, IMMUNITY_MECHANIC, MECHANIC_STUN, true);
                    me->ApplySpellImmune(0, IMMUNITY_MECHANIC, MECHANIC_FEAR, true);
                    me->ApplySpellImmune(0, IMMUNITY_MECHANIC, MECHANIC_ROOT, true);
                    me->ApplySpellImmune(0, IMMUNITY_MECHANIC, MECHANIC_FREEZE, true);
                    me->ApplySpellImmune(0, IMMUNITY_MECHANIC, MECHANIC_POLYMORPH, true);
                    me->ApplySpellImmune(0, IMMUNITY_MECHANIC, MECHANIC_HORROR, true);
                    me->ApplySpellImmune(0, IMMUNITY_MECHANIC, MECHANIC_SAPPED, true);
                    me->ApplySpellImmune(0, IMMUNITY_MECHANIC, MECHANIC_CHARM, true);
                    me->ApplySpellImmune(0, IMMUNITY_MECHANIC, MECHANIC_DISORIENTED, true);
                    pInstance = pCreature->GetInstanceScript();
                }

                InstanceScript* pInstance;
                EventMap events;

                void Reset()
                {
                    events.Reset();
                }

                void JustDied(Unit* killer)
                {
                    if (pInstance)
                    {
                        Map::PlayerList const& PlayerList = pInstance->instance->GetPlayers();
                        if (!PlayerList.isEmpty())
                        {
                            for (Map::PlayerList::const_iterator itr = PlayerList.begin(); itr != PlayerList.end();
                                    ++itr)
                            {
                                if (Player* pPlayer = itr->getSource())
                                {
                                    if (!pPlayer->IsAtGroupRewardDistance(me))
                                        continue;

                                    // Only mages, warlocks priests, shamans and druids
                                    // in caster specialization can accept quest
                                    if (!(pPlayer->getClass() == CLASS_MAGE || pPlayer->getClass() == CLASS_WARLOCK
                                            || pPlayer->getClass() == CLASS_PRIEST
                                            || (pPlayer->getClass() == CLASS_SHAMAN
                                                    && (pPlayer->GetPrimaryTalentTree(pPlayer->GetActiveSpec())
                                                            == TALENT_TREE_SHAMAN_ELEMENTAL
                                                            || pPlayer->GetPrimaryTalentTree(pPlayer->GetActiveSpec())
                                                                    == TALENT_TREE_SHAMAN_RESTORATION))
                                            || (pPlayer->getClass() == CLASS_DRUID
                                                    && (pPlayer->GetPrimaryTalentTree(pPlayer->GetActiveSpec())
                                                            == TALENT_TREE_DRUID_BALANCE
                                                            || pPlayer->GetPrimaryTalentTree(pPlayer->GetActiveSpec())
                                                                    == TALENT_TREE_DRUID_RESTORATION))))
                                        continue;

                                    if (pPlayer->GetTeam() == ALLIANCE
                                            && pPlayer->GetQuestStatus(29453) == QUEST_STATUS_NONE)
                                        pPlayer->AddQuest(sObjectMgr->GetQuestTemplate(29453), NULL);
                                        else if (pPlayer->GetTeam() == HORDE && pPlayer->GetQuestStatus(29452) ==QUEST_STATUS_NONE)
                                    pPlayer->AddQuest(sObjectMgr->GetQuestTemplate(29452), NULL);
                                    }
                                }
                            }
                        }
                    }

                    void EnterCombat(Unit* /*who*/)
                    {
                        events.ScheduleEvent(EVENT_FLAME_STOMP, 5000);
                        events.ScheduleEvent(EVENT_MELT_ARMOR, urand(3000, 7000));
                        events.ScheduleEvent(EVENT_SUMMON_LAVA_JETS, 10000);
                    }

                    void UpdateAI(uint32 const diff)
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
                                case EVENT_FLAME_STOMP:
                                DoCast(me, SPELL_FLAME_STOMP);
                                events.ScheduleEvent(EVENT_FLAME_STOMP, urand(10000, 18000));
                                break;
                                case EVENT_MELT_ARMOR:
                                DoCastVictim(SPELL_MELT_ARMOR);
                                events.ScheduleEvent(EVENT_MELT_ARMOR, urand(7000, 14000));
                                break;
                                case EVENT_SUMMON_LAVA_JETS:
                                DoCast(me, SPELL_SUMMON_LAVA_JETS);
                                events.ScheduleEvent(EVENT_SUMMON_LAVA_JETS, urand(20000, 25000));
                                break;
                            }
                        }

                        DoMeleeAttackIfReady();
                    }
                };
            };

const Position SpawnPosition[] =
{
{ 455.199f, 515.552f, 244.823837f, 5.64499f }, // Volc(anus) Spawn Pos
        };

class npc_harbinger_legendary: public CreatureScript
{
        struct npc_harbinger_legendaryAI: public ScriptedAI
        {
                npc_harbinger_legendaryAI(Creature* creature) :
                        ScriptedAI(creature)
                {
                    SetCombatMovement(false);
                }

                void Reset()
                {
                    if (Creature* staffnordrassil = me->FindNearestCreature(NPC_BRANCH_OF_NORDRASSIL, 50.0f))
                    {
                        DoCast(staffnordrassil, SPELL_FIRE_CHANNELING);
                    }
                }

                void EnterCombat(Unit* /*who*/)
                {
                    _events.ScheduleEvent(EVENT_FIEROBLAST, 1000);
                    _events.ScheduleEvent(EVENT_FIEROCLAST_BARRAGE, 5000);
                }

                void UpdateAI(uint32 const diff)
                {
                    if (!UpdateVictim())
                        return;

                    _events.Update(diff);

                    if (me->HasUnitState(UNIT_STATE_CASTING))
                        return;

                    if (uint32 eventId = _events.ExecuteEvent())
                    {
                        switch (eventId)
                        {
                            case EVENT_FIEROBLAST:
                                DoCast(SPELL_FIEROBLAST);
                                _events.ScheduleEvent(EVENT_FIEROBLAST, urand(2000, 5000));
                                break;
                            case EVENT_FIEROCLAST_BARRAGE:
                                DoCast(SPELL_FIEROCLAST_BARRAGE);
                                _events.ScheduleEvent(EVENT_FIEROCLAST_BARRAGE, urand(9000, 12000));
                                break;
                        }
                    }
                }
            private:
                EventMap _events;
        };
    public:
        npc_harbinger_legendary() :
                CreatureScript("npc_harbinger_legendary")
        {
        }

        CreatureAI* GetAI(Creature* creature) const
        {
            return new npc_harbinger_legendaryAI(creature);
        }
}
;

class npc_tormented_protector: public CreatureScript
{
        struct npc_tormented_protectorAI: public ScriptedAI
        {
                npc_tormented_protectorAI(Creature* creature) :
                        ScriptedAI(creature)
                {
                    defaultsize = creature->GetFloatValue(OBJECT_FIELD_SCALE_X);
                }

                float defaultsize;

                void Reset()
                {
                }

                void EnterCombat(Unit* /*who*/)
                {
                    _events.ScheduleEvent(EVENT_BLAZING_STOMP, 15000);
                    _events.ScheduleEvent(EVENT_SMOULDERING_ROOTS, 10000);
                    _events.ScheduleEvent(EVENT_SUMMON_TREANTS, 40000);
                }

                void MoveInLineOfSight(Unit* who)
                {
                    if (who && who->GetTypeId() == TYPEID_PLAYER && me->IsValidAttackTarget(who))

                        if (me->IsWithinDistInMap(who, 120.0f))
                        {
                            AttackStart(who);
                            ScriptedAI::MoveInLineOfSight(who);
                        }
                }

                void UpdateAI(uint32 const diff)
                {
                    if (!UpdateVictim())
                        return;

                    _events.Update(diff);

                    if (me->HasUnitState(UNIT_STATE_CASTING))
                        return;

                    if (uint32 eventId = _events.ExecuteEvent())
                    {
                        switch (eventId)
                        {
                            case EVENT_BLAZING_STOMP:
                                DoCast(SPELL_BLAZING_STOMP);
                                _events.ScheduleEvent(EVENT_BLAZING_STOMP, 20000);
                                break;
                            case EVENT_SMOULDERING_ROOTS:
                                DoCast(SPELL_SMOULDERING_ROOTS);
                                _events.ScheduleEvent(EVENT_SMOULDERING_ROOTS, 30000);
                                break;
                            case EVENT_SUMMON_TREANTS:
                                me->SummonCreature(NPC_BURNING_TREANTS, me->GetPositionX() + 30, me->GetPositionY(),
                                        me->GetPositionZ() + 1, float(rand() % 5),
                                        TEMPSUMMON_TIMED_DESPAWN_OUT_OF_COMBAT, 5000);
                                me->SummonCreature(NPC_BURNING_TREANTS, me->GetPositionX() - 30, me->GetPositionY(),
                                        me->GetPositionZ() + 1, float(rand() % 5),
                                        TEMPSUMMON_TIMED_DESPAWN_OUT_OF_COMBAT, 5000);
                                me->SummonCreature(NPC_BURNING_TREANTS, me->GetPositionX(), me->GetPositionY() + 30,
                                        me->GetPositionZ() + 1, float(rand() % 5),
                                        TEMPSUMMON_TIMED_DESPAWN_OUT_OF_COMBAT, 5000);
                                me->SummonCreature(NPC_BURNING_TREANTS, me->GetPositionX(), me->GetPositionY() - 30,
                                        me->GetPositionZ() + 1, float(rand() % 5),
                                        TEMPSUMMON_TIMED_DESPAWN_OUT_OF_COMBAT, 5000);
                                _events.ScheduleEvent(EVENT_SUMMON_TREANTS, 40000);
                                break;
                        }
                    }

                    if (me->HealthBelowPct(20))
                    {
                        me->SetFloatValue(OBJECT_FIELD_SCALE_X, defaultsize * 0.5f);
                        _events.CancelEvent(EVENT_BLAZING_STOMP); // Cancel Events, seems he need the combat active for the Event
                        _events.CancelEvent(EVENT_SMOULDERING_ROOTS);
                        _events.CancelEvent(EVENT_SUMMON_TREANTS);

                        me->setFaction(35);
                        me->CombatStop(true);
                        me->SetReactState(REACT_PASSIVE);
                        me->SetFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_IN_COMBAT); // no regen health here
                        me->SetFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_NON_ATTACKABLE);
                        me->RemoveAllAuras();
                    }

                    DoMeleeAttackIfReady();
                }
            private:
                EventMap _events;
        };
    public:
        npc_tormented_protector() :
                CreatureScript("npc_tormented_protector")
        {
        }

        CreatureAI* GetAI(Creature* creature) const
        {
            return new npc_tormented_protectorAI(creature);
        }
};

class npc_burning_treant: public CreatureScript
{
        struct npc_burning_treantAI: public ScriptedAI
        {
                npc_burning_treantAI(Creature* creature) :
                        ScriptedAI(creature)
                {
                }

                void Reset()
                {
                }

                void MoveInLineOfSight(Unit* who)
                {
                    if (who && who->GetTypeId() == TYPEID_PLAYER && me->IsValidAttackTarget(who))

                        if (me->IsWithinDistInMap(who, 130.0f))
                        {
                            me->SetSpeed(MOVE_RUN, 0.2f, true);
                            me->GetMotionMaster()->MoveChase(who, 0.0f, 0.0f);
                            ScriptedAI::MoveInLineOfSight(who);
                        }
                }
        };
    public:
        npc_burning_treant() :
                CreatureScript("npc_burning_treant")
        {
        }

        CreatureAI* GetAI(Creature* creature) const
        {
            return new npc_burning_treantAI(creature);
        }
};

class spell_firelands_ancient_core_hound_dinner_time: public SpellScriptLoader
{
    public:
        spell_firelands_ancient_core_hound_dinner_time() :
                SpellScriptLoader("spell_firelands_ancient_core_hound_dinner_time")
        {
        }

        class spell_firelands_ancient_core_hound_dinner_time_AuraScript: public AuraScript
        {
                PrepareAuraScript(spell_firelands_ancient_core_hound_dinner_time_AuraScript)
                ;

                void OnApply(AuraEffect const* /*aurEff*/, AuraEffectHandleModes /*mode*/)
                {
                    if (!GetTarget())
                        return;

                    GetTarget()->SetControlled(true, UNIT_STATE_STUNNED);
                }

                void OnRemove(AuraEffect const* aurEff, AuraEffectHandleModes /*mode*/)
                {
                    if (!GetTarget())
                        return;

                    GetTarget()->SetControlled(false, UNIT_STATE_STUNNED);
                }

                void Register()
                {
                    OnEffectApply +=
                            AuraEffectApplyFn(spell_firelands_ancient_core_hound_dinner_time_AuraScript::OnApply, EFFECT_0, SPELL_AURA_PERIODIC_DAMAGE, AURA_EFFECT_HANDLE_REAL);
                    OnEffectRemove +=
                            AuraEffectRemoveFn(spell_firelands_ancient_core_hound_dinner_time_AuraScript::OnRemove, EFFECT_0, SPELL_AURA_PERIODIC_DAMAGE, AURA_EFFECT_HANDLE_REAL);
                }
        };

        AuraScript* GetAuraScript() const
        {
            return new spell_firelands_ancient_core_hound_dinner_time_AuraScript();
        }
};

class spell_firelands_ancient_core_hound_flame_breath: public SpellScriptLoader
{
    public:
        spell_firelands_ancient_core_hound_flame_breath() :
                SpellScriptLoader("spell_firelands_ancient_core_hound_flame_breath")
        {
        }

        class spell_firelands_ancient_core_hound_flame_breath_AuraScript: public AuraScript
        {
                PrepareAuraScript(spell_firelands_ancient_core_hound_flame_breath_AuraScript)
                ;

                void OnApply(AuraEffect const* /*aurEff*/, AuraEffectHandleModes /*mode*/)
                {
                    if (!GetTarget())
                        return;

                    GetTarget()->SetControlled(true, UNIT_STATE_STUNNED);
                }

                void OnRemove(AuraEffect const* aurEff, AuraEffectHandleModes /*mode*/)
                {
                    if (!GetTarget())
                        return;

                    GetTarget()->SetControlled(false, UNIT_STATE_STUNNED);
                }

                void Register()
                {
                    OnEffectApply +=
                            AuraEffectApplyFn(spell_firelands_ancient_core_hound_flame_breath_AuraScript::OnApply, EFFECT_0, SPELL_AURA_PERIODIC_TRIGGER_SPELL, AURA_EFFECT_HANDLE_REAL);
                    OnEffectRemove +=
                            AuraEffectRemoveFn(spell_firelands_ancient_core_hound_flame_breath_AuraScript::OnRemove, EFFECT_0, SPELL_AURA_PERIODIC_TRIGGER_SPELL, AURA_EFFECT_HANDLE_REAL);
                }
        };

        AuraScript* GetAuraScript() const
        {
            return new spell_firelands_ancient_core_hound_flame_breath_AuraScript();
        }
};

class spell_firelands_ancient_lava_dweller_lava_shower: public SpellScriptLoader
{
    public:
        spell_firelands_ancient_lava_dweller_lava_shower() :
                SpellScriptLoader("spell_firelands_ancient_lava_dweller_lava_shower")
        {
        }

        class spell_firelands_ancient_lava_dweller_lava_shower_AuraScript: public AuraScript
        {
                PrepareAuraScript(spell_firelands_ancient_lava_dweller_lava_shower_AuraScript)
                ;

                void PeriodicTick(AuraEffect const* aurEff)
                {
                    if (!GetCaster())
                        return;

                    if (GetCaster()->GetAI())
                        if (Unit* pTarget = GetCaster()->GetAI()->SelectTarget(SELECT_TARGET_RANDOM, 0, 0.0f, true))
                            GetCaster()->CastSpell(pTarget, SPELL_LAVA_SHOWER_MISSILE, true);
                }

                void Register()
                {
                    OnEffectPeriodic +=
                            AuraEffectPeriodicFn(spell_firelands_ancient_lava_dweller_lava_shower_AuraScript::PeriodicTick, EFFECT_0, SPELL_AURA_PERIODIC_DUMMY);
                }
        };

        AuraScript* GetAuraScript() const
        {
            return new spell_firelands_ancient_lava_dweller_lava_shower_AuraScript();
        }
};

class spell_firelands_fire_turtle_hatchling_shell_spin: public SpellScriptLoader
{
    public:
        spell_firelands_fire_turtle_hatchling_shell_spin() :
                SpellScriptLoader("spell_firelands_fire_turtle_hatchling_shell_spin")
        {
        }

        class spell_firelands_fire_turtle_hatchling_shell_spin_AuraScript: public AuraScript
        {
                PrepareAuraScript(spell_firelands_fire_turtle_hatchling_shell_spin_AuraScript)
                ;

                void OnApply(AuraEffect const* /*aurEff*/, AuraEffectHandleModes /*mode*/)
                {
                    if (!GetTarget())
                        return;

                    GetTarget()->SetControlled(true, UNIT_STATE_ROOT);
                }

                void OnRemove(AuraEffect const* aurEff, AuraEffectHandleModes /*mode*/)
                {
                    if (!GetTarget())
                        return;

                    GetTarget()->SetControlled(false, UNIT_STATE_ROOT);
                }

                void Register()
                {
                    OnEffectApply +=
                            AuraEffectApplyFn(spell_firelands_fire_turtle_hatchling_shell_spin_AuraScript::OnApply, EFFECT_0, SPELL_AURA_PERIODIC_TRIGGER_SPELL, AURA_EFFECT_HANDLE_REAL);
                    OnEffectRemove +=
                            AuraEffectRemoveFn(spell_firelands_fire_turtle_hatchling_shell_spin_AuraScript::OnRemove, EFFECT_0, SPELL_AURA_PERIODIC_TRIGGER_SPELL, AURA_EFFECT_HANDLE_REAL);
                }
        };

        AuraScript* GetAuraScript() const
        {
            return new spell_firelands_fire_turtle_hatchling_shell_spin_AuraScript();
        }
};

class spell_firelands_flame_archon_fiery_torment: public SpellScriptLoader
{
    public:
        spell_firelands_flame_archon_fiery_torment() :
                SpellScriptLoader("spell_firelands_flame_archon_fiery_torment")
        {
        }

        class spell_firelands_flame_archon_fiery_torment_AuraScript: public AuraScript
        {
                PrepareAuraScript(spell_firelands_flame_archon_fiery_torment_AuraScript)
                ;

                void OnApply(AuraEffect const* /*aurEff*/, AuraEffectHandleModes /*mode*/)
                {
                    if (!GetTarget())
                        return;

                    GetTarget()->SetControlled(true, UNIT_STATE_STUNNED);
                }

                void OnRemove(AuraEffect const* aurEff, AuraEffectHandleModes /*mode*/)
                {
                    if (!GetTarget())
                        return;

                    GetTarget()->SetControlled(false, UNIT_STATE_STUNNED);
                }

                void PeriodicTick(AuraEffect const* aurEff)
                {
                    if (!GetCaster())
                        return;

                    if (GetCaster()->GetAI())
                        if (Unit* pTarget = GetCaster()->GetAI()->SelectTarget(SELECT_TARGET_NEAREST, 0, 100.0f, true))
                            GetCaster()->CastSpell(pTarget, SPELL_FIERY_TORMENT_DMG, true);
                }

                void Register()
                {
                    OnEffectApply +=
                            AuraEffectApplyFn(spell_firelands_flame_archon_fiery_torment_AuraScript::OnApply, EFFECT_0, SPELL_AURA_PERIODIC_TRIGGER_SPELL, AURA_EFFECT_HANDLE_REAL);
                    OnEffectRemove +=
                            AuraEffectRemoveFn(spell_firelands_flame_archon_fiery_torment_AuraScript::OnRemove, EFFECT_0, SPELL_AURA_PERIODIC_TRIGGER_SPELL, AURA_EFFECT_HANDLE_REAL);
                    OnEffectPeriodic +=
                            AuraEffectPeriodicFn(spell_firelands_flame_archon_fiery_torment_AuraScript::PeriodicTick, EFFECT_0, SPELL_AURA_PERIODIC_TRIGGER_SPELL);
                }
        };

        AuraScript* GetAuraScript() const
        {
            return new spell_firelands_flame_archon_fiery_torment_AuraScript();
        }
};

class spell_firelands_molten_lord_summon_lava_jets: public SpellScriptLoader
{
    public:
        spell_firelands_molten_lord_summon_lava_jets() :
                SpellScriptLoader("spell_firelands_molten_lord_summon_lava_jets")
        {
        }

        class spell_firelands_molten_lord_summon_lava_jets_SpellScript: public SpellScript
        {
                PrepareSpellScript(spell_firelands_molten_lord_summon_lava_jets_SpellScript)
                ;

                void HandleScript(SpellEffIndex /*effIndex*/)
                {
                    if (!GetCaster() || !GetHitUnit())
                        return;

                    GetCaster()->CastSpell(GetHitUnit(), 99538, true);
                }

                void Register()
                {
                    OnEffectHitTarget +=
                            SpellEffectFn(spell_firelands_molten_lord_summon_lava_jets_SpellScript::HandleScript, EFFECT_0, SPELL_EFFECT_DUMMY);
                }
        };

        SpellScript* GetSpellScript() const
        {
            return new spell_firelands_molten_lord_summon_lava_jets_SpellScript();
        }
};

class spell_branch_of_nordrassil_summon: public SpellScriptLoader
{
    public:
        spell_branch_of_nordrassil_summon() :
                SpellScriptLoader("spell_branch_of_nordrassil_summon")
        {
        }

        class spell_branch_of_nordrassil_summon_SpellScript: public SpellScript
        {
                PrepareSpellScript(spell_branch_of_nordrassil_summon_SpellScript)
                ;

                bool Load()
                {
                    target = NULL;
                    return true;
                }

                void AfterCast(SpellEffIndex /*effIndex*/)
                {
                    Unit* caster = GetCaster();
                    caster->SummonCreature(NPC_TORMENTED_PROTECTOR, caster->m_positionX, caster->m_positionY,
                            caster->m_positionZ, 0);
                }

                void Register()
                {
                    OnEffectHitTarget +=
                            SpellEffectFn(spell_branch_of_nordrassil_summon_SpellScript::AfterCast, EFFECT_0, SPELL_EFFECT_DUMMY);
                }

                WorldObject* target;
        };

        SpellScript* GetSpellScript() const
        {
            return new spell_branch_of_nordrassil_summon_SpellScript();
        }
};

class mob_fl_teleport : public CreatureScript
{
public:
    mob_fl_teleport() : CreatureScript("mob_fl_teleport") { }

    CreatureAI* GetAI(Creature* creature) const
    {
        return new mob_fl_teleportAI (creature);
    }

    struct mob_fl_teleportAI : public ScriptedAI
    {
        mob_fl_teleportAI(Creature* creature) : ScriptedAI(creature) { }

        void MoveInLineOfSight(Unit* who)
        {
            if (!who)
                return;

            if (!who->ToPlayer())
                return;
            
            if (me->GetExactDist(who) <= 17.0f) // we use here not GetExactDist2d because player fly also into the FL portal
                who->ToPlayer()->TeleportTo(720, -542.885f, 316.925f, 115.493f, 5.947003f);
        }
    };
};

void AddSC_firelands()
{
    new npc_firelands_ancient_core_hound();
    new npc_firelands_ancient_lava_dweller();
    new npc_firelands_fire_scorpion();
    new npc_firelands_fire_turtle_hatchling();
    new npc_firelands_flame_archon();
    new npc_firelands_molten_lord();
    new npc_harbinger_legendary();
    new npc_burning_treant();
    new npc_tormented_protector();
    new spell_firelands_ancient_core_hound_dinner_time();
    new spell_firelands_ancient_core_hound_flame_breath();
    new spell_firelands_ancient_lava_dweller_lava_shower();
    new spell_firelands_fire_turtle_hatchling_shell_spin();
    new spell_firelands_flame_archon_fiery_torment();
    new spell_firelands_molten_lord_summon_lava_jets();
    new spell_branch_of_nordrassil_summon();
    new mob_fl_teleport();
}
