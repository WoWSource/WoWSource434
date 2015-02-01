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

#include "ScriptMgr.h"
#include "ScriptedCreature.h"
#include "SpellScript.h"
#include "zulgurub.h"

enum eSpells
{
    SPELL_HAKKARS_CHAINS = 97022,
    SPELL_HAKKAR_BREAKS_FREE = 97209,
    SPELL_DRAIN_SPIRIT_ESSENCE = 97321,
    SPELL_DEADZONE = 97170,
    SPELL_SHADOWS_OF_HAKKAR = 97172,
    SPELL_ADD_PLAYERS_THREAD = 100401,
    SPELL_SPIRIT_WORLD = 98861,
    SPELL_VANISH = 97002,
    SPELL_TRANSFORM = 96716,
    SPELL_SHADOW_SPIKE_R = 97160,
    SPELL_SHADOW_SPIKE_L = 97208,
    SPELL_SUMMON_SPIRIT = 97123,
    SPELL_CALL_SPIRIT = 97152,
    SPELL_SHADOW_SPIKE = 97158,
    SPELL_BODY_SLAM = 97252,
    SPELL_BRITTLE_BARRIER = 97417,
    SPELL_SPIRIT_WARRIOR_GAZE = 97597,
    SPELL_FRENZY = 97088,
    SPELL_HAKKARS_CHAINS_VISUAL = 97091
};

enum eEvents
{
    EVENT_DRAIN_SPIRIT_ESSENCE_COSMETIC_INTRO       = 1,
    EVENT_DEADZONE                                  = 2,
    EVENT_SHADOWS_OF_HAKKAR                         = 3,
    EVENT_SET_FACING                                = 4,
    EVENT_SPIRIT_WORLD                              = 5,
    EVENT_SPIRIT_WORLD_SUMMON                       = 6,
    EVENT_HAKKAR_SET_FACING                         = 7,
    EVENT_HAKKAR_KILL_JINDO                         = 8,
    EVENT_HAKKAR_YELL_BYE                           = 9,
    EVENT_JINDO_KILL_SELF                           = 10,
    // Spirit world
    EVENT_SHADOW_SPIKE_AND_CALL_SPIRIT              = 1,
    // Gurubashi troll
    EVENT_BODY_SLAM                                 = 1,
    EVENT_FRENZY                                    = 2,
};

enum eTexts
{
    JINDO_YELL_START                                = 0,
    JINDO_EMOTE_SHADOWS_OF_HAKKAR                   = 1,
    JINDO_YELL_SPIRIT_REALM                         = 2,
    JINDO_KILL_PLAYER                               = 3,
    JINDO_SPIRIT_YELL_KILL_PLAYER                   = 0,
    JINDO_SPIRIT_YELL_DIED                          = 1,
    HAKKAR_YELL_SPIT                                = 0,
    HAKKAR_YELL_OVERSTEPPED                         = 1,
    HAKKAR_SPIRIT_YELL_INSECTS                      = 0,
};


enum eDefault
{
    POINT_SPIRIT_WORLD                              = 1,

    ACTION_HAKKAR_FREE                              = 1,
    ACTION_DESPAWN                                  = 2,
    ACTION_CHAIN_DIED                               = 3,

    TYPE_SPAWN_POSITION_ID                          = 1,
    TYPE_KILLED_TROLL_ID                            = 1,
};

const Position HakkarSP = {-11786.5f, -1690.92f, 53.01943f, 1.605703f};

const Position JindoSpiritSP = {-11786.32f, -1715.629f, 43.72891f, 1.466077f};

const Position HakkarChainSP[2][3]=
{
    {
        {-11814.56f, -1652.120f, 52.96632f, 0.0f},
        {-11778.60f, -1659.418f, 53.06332f, 0.0f},
        {-11801.36f, -1678.391f, 53.04712f, 0.0f},
    },
    {
        {-11761.55f, -1649.866f, 52.96572f, 0.0f},
        {-11772.65f, -1676.898f, 53.04771f, 0.0f},
        {-11796.30f, -1659.990f, 53.09364f, 0.0f},
    }
};

const Position SpiritPortalSP[15]=
{
    {-11811.90f, -1637.995f, 52.99569f, 0.0f},
    {-11793.88f, -1646.389f, 54.62890f, 0.0f},
    {-11782.06f, -1646.380f, 54.56368f, 0.0f},
    {-11769.54f, -1663.590f, 53.06438f, 0.0f},
    {-11788.62f, -1628.262f, 54.86021f, 0.0f},
    {-11751.88f, -1662.833f, 53.08055f, 0.0f},
    {-11764.74f, -1636.479f, 52.98969f, 0.0f},
    {-11752.42f, -1645.153f, 52.87562f, 0.0f},
    {-11756.94f, -1680.793f, 53.07720f, 0.0f},
    {-11749.95f, -1631.240f, 52.89489f, 0.0f},
    {-11823.27f, -1665.200f, 53.08056f, 0.0f},
    {-11823.57f, -1647.462f, 52.87725f, 0.0f},
    {-11816.96f, -1682.873f, 53.07767f, 0.0f},
    {-11805.72f, -1664.719f, 53.06411f, 0.0f},
    {-11827.62f, -1634.120f, 52.93477f, 0.0f},
};

const Position SpiritWarriorSP[4]=
{
    {-11837.85f, -1698.944f, 40.88818f, 0.0f},
    {-11735.52f, -1695.576f, 40.88869f, 0.0f},
    {-11737.58f, -1592.231f, 40.88970f, 0.0f},
    {-11842.00f, -1595.804f, 40.88970f, 0.0f},
};

class boss_jindo_the_godbreaker : public CreatureScript
{
    public:
        boss_jindo_the_godbreaker() : CreatureScript("boss_jindo_the_godbreaker") { }

    private:
        CreatureAI* GetAI(Creature* creature) const
        {
            return GetZulGurubAI<boss_jindo_the_godbreakerAI>(creature);
        }

        struct boss_jindo_the_godbreakerAI : public BossAI
        {
            boss_jindo_the_godbreakerAI(Creature* creature) : BossAI(creature, DATA_JINDO) { }

            uint64 HakkarGUID;
            uint64 JindoSpiritGUID;
            uint64 HakkarSpiritGUID;
            uint8 ChainCount;
            uint8 SpiritTrollSpawMask;
            bool SpiritWorld;

            void InitializeAI()
            {
                for (int i = 0; i < 4; ++i)
                    if (Creature* warrior = me->SummonCreature(52167, SpiritWarriorSP[i]))
                        warrior->AI()->SetData(TYPE_SPAWN_POSITION_ID, i);

                if (me->isAlive())
                    me->SummonCreature(52650, HakkarSP);

                SpiritTrollSpawMask = 0;
                Reset();
            }

            void Reset()
            {
                EntryCheckPredicate pred(52430);
                summons.DoAction(ACTION_DESPAWN, pred);
                _Reset();
                me->SetReactState(REACT_AGGRESSIVE);
                events.ScheduleEvent(EVENT_DRAIN_SPIRIT_ESSENCE_COSMETIC_INTRO, urand(5000, 15000));
                HakkarGUID = 0;
                JindoSpiritGUID = 0;
                HakkarSpiritGUID = 0;
                ChainCount = 0;
                SpiritWorld = false;
                instance->SendEncounterUnit(ENCOUNTER_FRAME_DISENGAGE, me);

                uint8 count = 0;

                for (int data = DATA_VENOXIS; data <= DATA_ZANZIL; ++data)
                    if (instance->GetBossState(data) == DONE)
                        ++count;

                if (count < 2)
                {
                    me->SetFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_NON_ATTACKABLE);
                    me->SetReactState(REACT_PASSIVE);
                }
            }

            void SetData(uint32 type, uint32 data)
            {
                if (type == TYPE_KILLED_TROLL_ID)
                    SpiritTrollSpawMask |= 1 << data;
            }

            void JustSummoned(Creature* summoned)
            {
                switch (summoned->GetEntry())
                {
                    case 52167:
                        summoned->SetPhaseMask(1, true);
                        return;
                    case 52650:
                        HakkarSpiritGUID = summoned->GetGUID();
                        summoned->SetPhaseMask(1, true);
                        return;
                    case 52730:
                    case 52732:
                        summoned->SetPhaseMask(2, true);
                        summons.Summon(summoned);
                        return;
                    case 52222:
                        HakkarGUID = summoned->GetGUID();
                        summoned->SetReactState(REACT_PASSIVE);
                        summoned->SetPhaseMask(2, true);
                        break;
                    case 52150:
                        JindoSpiritGUID = summoned->GetGUID();
                        summoned->SetPhaseMask(2, true);
                        break;
                    case 52430:
                        summoned->SetPhaseMask(2, true);
                        summoned->CastSpell((Unit*)NULL, SPELL_HAKKARS_CHAINS, false);
                        break;
                }

                summons.Summon(summoned);

                if (me->isInCombat())
                    summoned->SetInCombatWithZone();
            }

            void DoAction(int32 const action)
            {
                if (action == ACTION_CHAIN_DIED)
                {
                    ++ChainCount;

                    if (ChainCount == 3)
                    {
                        summons.DespawnEntry(52624);
                        summons.DespawnEntry(52532);

                        if (Creature* jindo = ObjectAccessor::GetCreature(*me, JindoSpiritGUID))
                            jindo->AI()->DoAction(ACTION_HAKKAR_FREE);

                        if (Creature* hakkar = ObjectAccessor::GetCreature(*me, HakkarGUID))
                        {
                            hakkar->AI()->Talk(HAKKAR_YELL_OVERSTEPPED);
                            hakkar->CastSpell(hakkar, SPELL_HAKKAR_BREAKS_FREE, false);
                            events.ScheduleEvent(EVENT_HAKKAR_SET_FACING, 1000);
                        }
                    }
                }
            }

            void DamageTaken(Unit* /*done_by*/, uint32 & /*damage*/)
            {
                if (!SpiritWorld && me->GetHealthPct() <= 70)
                {
                    SpiritWorld = true;
                    Talk(JINDO_YELL_SPIRIT_REALM);
                    me->SetReactState(REACT_PASSIVE);
                    me->AttackStop();
                    me->CastStop();
                    me->GetMotionMaster()->MovePoint(0, -11787.0f, -1695.73f, 52.98806f);
                    events.Reset();
                    events.ScheduleEvent(EVENT_SET_FACING, 5000);
                }
            }

            void EnterCombat(Unit* /*who*/)
            {
                _EnterCombat();
                Talk(JINDO_YELL_START);
                me->CastStop();
                events.Reset();
                events.ScheduleEvent(EVENT_DEADZONE, urand(5000, 15000));
                instance->SendEncounterUnit(ENCOUNTER_FRAME_ENGAGE, me);
            }

            void KilledUnit(Unit* victim)
            {
                if (victim->GetTypeId() == TYPEID_PLAYER)
                    Talk(JINDO_KILL_PLAYER);
            }

            void JustDied(Unit* /*killer*/)
            {
                EntryCheckPredicate pred(52430);
                summons.DoAction(ACTION_DESPAWN, pred);
                instance->SendEncounterUnit(ENCOUNTER_FRAME_DISENGAGE, me);
                _JustDied();
            }

            void UpdateAI(uint32 const diff)
            {
                events.Update(diff);

                if (!UpdateVictim())
                {
                    if (events.ExecuteEvent() == EVENT_DRAIN_SPIRIT_ESSENCE_COSMETIC_INTRO)
                    {
                        me->CastSpell(me, SPELL_DRAIN_SPIRIT_ESSENCE, false);
                        events.ScheduleEvent(EVENT_DRAIN_SPIRIT_ESSENCE_COSMETIC_INTRO, urand(10000, 20000));
                    }

                    return;
                }

                if (me->HasUnitState(UNIT_STATE_CASTING))
                    return;

                if (uint32 eventId = events.ExecuteEvent())
                {
                    switch (eventId)
                    {
                        case EVENT_DEADZONE:
                            me->CastSpell(me, SPELL_DEADZONE, false);
                            events.ScheduleEvent(EVENT_DEADZONE, 21000);
                            events.ScheduleEvent(EVENT_SHADOWS_OF_HAKKAR, urand(5000, 10000));
                            break;
                        case EVENT_SHADOWS_OF_HAKKAR:
                            Talk(JINDO_EMOTE_SHADOWS_OF_HAKKAR);
                            me->CastSpell(me, SPELL_SHADOWS_OF_HAKKAR, false);
                            break;
                        case EVENT_SET_FACING:
                            me->SetFacingTo(1.570796f);
                            events.ScheduleEvent(EVENT_SPIRIT_WORLD, 500);
                            break;
                        case EVENT_SPIRIT_WORLD:
                            me->CastSpell(me, SPELL_ADD_PLAYERS_THREAD, false);
                            me->CastSpell(me, SPELL_SPIRIT_WORLD, false);
                            me->CastSpell(me, SPELL_VANISH, false);
                            events.ScheduleEvent(EVENT_SPIRIT_WORLD_SUMMON, 3000);
                            instance->SendEncounterUnit(ENCOUNTER_FRAME_DISENGAGE, me);
                            break;
                        case EVENT_SPIRIT_WORLD_SUMMON:
                            {
                                me->SummonCreature(52150, JindoSpiritSP);
                                uint8 id = urand(0, 1);

                                for (int i = 0; i < 3; ++i)
                                    me->SummonCreature(52430, HakkarChainSP[id][i]);

                                for (int i = 0; i < 15; ++i)
                                    me->SummonCreature(52532, SpiritPortalSP[i]);

                                for (int i = 0; i < 4; ++i)
                                    me->SummonCreature(SpiritTrollSpawMask & 1 << i ? 52730 : 52732, SpiritWarriorSP[i]);

                                if (Creature* hakkar = me->SummonCreature(52222, HakkarSP))
                                    hakkar->AI()->Talk(HAKKAR_YELL_SPIT);
                            }
                            break;
                        case EVENT_HAKKAR_SET_FACING:
                            {
                                if (Creature* hakkar = ObjectAccessor::GetCreature(*me, HakkarGUID))
                                    hakkar->SetFacingTo(4.712389f);

                                events.ScheduleEvent(EVENT_HAKKAR_KILL_JINDO, 17000);
                            }
                            break;
                        case EVENT_HAKKAR_KILL_JINDO:
                            {
                                if (Creature* hakkar = ObjectAccessor::GetCreature(*me, HakkarGUID))
                                    hakkar->HandleEmoteCommand(EMOTE_ONESHOT_ATTACK1H);

                                me->RemoveAura(SPELL_SPIRIT_WORLD);
                                events.ScheduleEvent(EVENT_HAKKAR_YELL_BYE, 2500);
                            }
                            break;
                        case EVENT_HAKKAR_YELL_BYE:
                            {
                                if (Creature* hakkar = ObjectAccessor::GetCreature(*me, HakkarSpiritGUID))
                                {
                                    hakkar->AI()->Talk(HAKKAR_SPIRIT_YELL_INSECTS);
                                    hakkar->DespawnOrUnsummon(5000);
                                }

                                events.ScheduleEvent(EVENT_JINDO_KILL_SELF, 5000);
                            }
                            break;
                        case EVENT_JINDO_KILL_SELF:
                            {
                                me->RemoveAllAuras();
                                me->CastSpell(me, SPELL_TRANSFORM, false);

                                if (!me->getThreatManager().getThreatList().empty())
                                    if (Unit* killer = ObjectAccessor::GetUnit(*me, (*me->getThreatManager().getThreatList().begin())->getUnitGuid()))
                                        killer->DealDamage(me, me->GetHealth(), NULL, DIRECT_DAMAGE, SPELL_SCHOOL_MASK_NORMAL, NULL, false);
                            }
                            break;
                    }
                }

                DoMeleeAttackIfReady();
                EnterEvadeIfOutOfCombatArea(diff);
            }
        };
};

class boss_jindo_the_godbreaker_spirit_world : public CreatureScript
{
    public:
        boss_jindo_the_godbreaker_spirit_world() : CreatureScript("boss_jindo_the_godbreaker_spirit_world") { }

    private:
        CreatureAI* GetAI(Creature* creature) const
        {
            return new boss_jindo_the_godbreaker_spirit_worldAI(creature);
        }

        struct boss_jindo_the_godbreaker_spirit_worldAI : public ScriptedAI
        {
            boss_jindo_the_godbreaker_spirit_worldAI(Creature* creature) : ScriptedAI(creature) { }

            EventMap events;
            bool RightArm;

            void InitializeAI()
            {
                RightArm = true;
                me->SetReactState(REACT_PASSIVE);
            }

            void KilledUnit(Unit* victim)
            {
                if (victim->GetTypeId() == TYPEID_PLAYER)
                    Talk(JINDO_SPIRIT_YELL_KILL_PLAYER);
            }

            void DoAction(int32 const action)
            {
                if (action == ACTION_HAKKAR_FREE)
                {
                    Talk(JINDO_SPIRIT_YELL_DIED);
                    events.Reset();
                }
            }

            void SpellHitTarget(Unit* target, const SpellInfo* spell)
            {
                switch (spell->Id)
                {
                    case SPELL_SHADOW_SPIKE:
                        me->CastSpell(target, RightArm ? SPELL_SHADOW_SPIKE_R : SPELL_SHADOW_SPIKE_L, true);
                        RightArm = !RightArm;
                        break;
                    case SPELL_CALL_SPIRIT:
                        target->CastSpell(target, SPELL_SUMMON_SPIRIT, false, NULL, NULL, me->ToTempSummon()->GetSummonerGUID());
                        break;
                }
            }

            void EnterCombat(Unit* /*who*/)
            {
                events.ScheduleEvent(EVENT_SHADOW_SPIKE_AND_CALL_SPIRIT, 1000);
            }

            void UpdateAI(uint32 const diff)
            {
                if (!UpdateVictim())
                    return;

                events.Update(diff);

                if (events.ExecuteEvent() == EVENT_SHADOW_SPIKE_AND_CALL_SPIRIT)
                {
                    me->CastSpell((Unit*)NULL, SPELL_CALL_SPIRIT, false);
                    me->CastSpell((Unit*)NULL, SPELL_SHADOW_SPIKE, false);
                    events.ScheduleEvent(EVENT_SHADOW_SPIKE_AND_CALL_SPIRIT, 3100);
                }

                DoMeleeAttackIfReady();
            }
        };
};

struct npc_gurubashi_heavy_trollAI : public ScriptedAI
{
    npc_gurubashi_heavy_trollAI(Creature* creature) : ScriptedAI(creature) { }

    EventMap events;

    void InitializeAI()
    {
        JustReachedHome();
    }

    void JustReachedHome()
    {
        me->GetMotionMaster()->MoveRandom(3.0f);
    }

    void JustSummoned(Creature* summoned)
    {
        switch (summoned->GetEntry())
        {
            case 52400:
            case 52407:
                summoned->DespawnOrUnsummon(20000);
                break;
        }
    }

    void SpellHitTarget(Unit* victim, const SpellInfo* spell)
    {
        if (spell->Id == SPELL_BODY_SLAM)
            victim->RemoveAura(SPELL_BRITTLE_BARRIER);
    }

    void EnterCombat(Unit* /*who*/)
    {
        events.ScheduleEvent(EVENT_BODY_SLAM, urand(7000, 14000));
        events.ScheduleEvent(EVENT_FRENZY, urand(7000, 14000));
    }

    void UpdateAI(uint32 const diff)
    {
        if (!UpdateVictim())
            return;

        events.Update(diff);

        if (me->HasUnitState(UNIT_STATE_CASTING))
            return;

        if (uint32 eventId = events.ExecuteEvent())
        {
            switch (eventId)
            {
                case EVENT_BODY_SLAM:
                    me->CastSpell((Unit*)NULL, SPELL_SPIRIT_WARRIOR_GAZE, false);
                    events.ScheduleEvent(EVENT_BODY_SLAM, urand(10000, 20000));
                    break;
                case EVENT_FRENZY:
                    me->CastSpell(me, SPELL_FRENZY, false);
                    events.ScheduleEvent(EVENT_FRENZY, urand(7000, 14000));
                    break;
            }
        }

        DoMeleeAttackIfReady();
    }
};

class npc_gurubashi_spirit : public CreatureScript
{
    public:
        npc_gurubashi_spirit() : CreatureScript("npc_gurubashi_spirit") { }

    private:
        CreatureAI* GetAI(Creature* creature) const
        {
            return new npc_gurubashi_heavy_trollAI(creature);
        }
};

class npc_gurubashi_spirit_warrior : public CreatureScript
{
    public:
        npc_gurubashi_spirit_warrior() : CreatureScript("npc_gurubashi_spirit_warrior") { }

    private:
        struct npc_gurubashi_spirit_warriorAI : public npc_gurubashi_heavy_trollAI
        {
            npc_gurubashi_spirit_warriorAI(Creature* creature) : npc_gurubashi_heavy_trollAI(creature) { }

            uint8 SpawnPositionId;

            void InitializeAI()
            {
                SpawnPositionId = 0;
                JustReachedHome();
            }

            void SetData(uint32 type, uint32 data)
            {
                if (type == TYPE_SPAWN_POSITION_ID)
                    SpawnPositionId = data;
            }

            void JustDied(Unit* /*killer*/)
            {
                if (me->isSummon())
                    if (Creature* summoner = me->ToTempSummon()->GetSummonerCreatureBase())
                        summoner->AI()->SetData(TYPE_KILLED_TROLL_ID, SpawnPositionId);
            }
        };

        CreatureAI* GetAI(Creature* creature) const
        {
            return new npc_gurubashi_spirit_warriorAI(creature);
        }
};

class npc_hakkars_chains : public CreatureScript
{
    public:
        npc_hakkars_chains() : CreatureScript("npc_hakkars_chains") { }

    private:
        CreatureAI* GetAI(Creature* creature) const
        {
            return new npc_hakkars_chainsAI (creature);
        }

        struct npc_hakkars_chainsAI : public ScriptedAI
        {
            npc_hakkars_chainsAI(Creature* creature) : ScriptedAI(creature), instance(me->GetInstanceScript()) { }

            InstanceScript* instance;

            void InitializeAI()
            {
                ASSERT(instance);
                me->SetReactState(REACT_PASSIVE);
                me->AddAura(SPELL_HAKKARS_CHAINS_VISUAL, me);
                me->AddAura(SPELL_BRITTLE_BARRIER, me);
            }

            void EnterCombat(Unit* /*who*/)
            {
                instance->SendEncounterUnit(ENCOUNTER_FRAME_ENGAGE, me);
            }

            void DoAction(int32 const action)
            {
                if (action == ACTION_DESPAWN)
                    instance->SendEncounterUnit(ENCOUNTER_FRAME_DISENGAGE, me);
            }

            void JustDied(Unit* /*killer*/)
            {
                if (Creature* jindo = ObjectAccessor::GetCreature(*me, instance->GetData64(DATA_JINDO)))
                    jindo->AI()->DoAction(ACTION_CHAIN_DIED);

                instance->SendEncounterUnit(ENCOUNTER_FRAME_DISENGAGE, me);
            }
        };
};

class spell_sunder_rift_visual : public SpellScriptLoader
{
    public:
        spell_sunder_rift_visual() : SpellScriptLoader("spell_sunder_rift_visual") { }

    private:
        class spell_sunder_rift_visual_SpellScript : public SpellScript
        {
            PrepareSpellScript(spell_sunder_rift_visual_SpellScript);

            void ModDestHeight(SpellEffIndex /*effIndex*/)
            {
                Position offset = {0.0f, 0.0f, 10.0f, 0.0f};
                const_cast<WorldLocation*>(GetExplTargetDest())->RelocateOffset(offset);
                GetHitDest()->RelocateOffset(offset);
            }

            void Register()
            {
                OnEffectLaunch += SpellEffectFn(spell_sunder_rift_visual_SpellScript::ModDestHeight, EFFECT_0, SPELL_EFFECT_DUMMY);
            }
        };

        SpellScript* GetSpellScript() const
        {
            return new spell_sunder_rift_visual_SpellScript();
        }
};

class spell_spirit_warriors_gaze : public SpellScriptLoader
{
    public:
        spell_spirit_warriors_gaze() : SpellScriptLoader("spell_spirit_warriors_gaze") { }

    private:
        class spell_spirit_warriors_gazeAuraScript : public AuraScript
        {
            PrepareAuraScript(spell_spirit_warriors_gazeAuraScript)

            void EffectApply(AuraEffect const * /*aurEff*/, AuraEffectHandleModes /*mode*/)
            {
                GetCaster()->CastSpell(GetTarget(), GetSpellInfo()->Effects[EFFECT_0].BasePoints, false);
            }

            void Register()
            {
                OnEffectApply += AuraEffectRemoveFn(spell_spirit_warriors_gazeAuraScript::EffectApply, EFFECT_0, SPELL_AURA_DUMMY, AURA_EFFECT_HANDLE_REAL);
            }
        };

        AuraScript *GetAuraScript() const
        {
            return new spell_spirit_warriors_gazeAuraScript();
        }
};

void AddSC_boss_jindo_the_godbreaker()
{
    new boss_jindo_the_godbreaker();
    new boss_jindo_the_godbreaker_spirit_world();
    new npc_gurubashi_spirit();
    new npc_gurubashi_spirit_warrior();
    new npc_hakkars_chains();

    new spell_sunder_rift_visual();
    new spell_spirit_warriors_gaze();
}
