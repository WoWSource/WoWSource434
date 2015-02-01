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
#include "SpellAuraEffects.h"
#include "CreatureTextMgr.h"
#include "zulgurub.h"

enum Yells
{
    SAY_AGGRO                   = 0,
    SAY_PLAYER_KILL             = 1,
    SAY_DISMOUNT_OHGAN          = 2,
    EMOTE_DEVASTATING_SLAM      = 3,
    SAY_REANIMATE_OHGAN         = 4,
    EMOTE_FRENZY                = 5,
    SAY_FRENZY                  = 6,
    SAY_DEATH                   = 7,
    SAY_LEVEL_UP                = 8,

    JINDO_SAY_GRATS             = 4,
};

enum Spells
{
    // Bloodlord Mandokir
    SPELL_BLOODLORD_AURA            = 96480,
    SPELL_SUMMON_OHGAN              = 96717,
    SPELL_REANIMATE_OHGAN           = 96724,
    SPELL_DECAPITATE                = 96682,
    SPELL_BLOODLETTING              = 96776,
    SPELL_BLOODLETTING_DAMAGE       = 96777,
    SPELL_BLOODLETTING_HEAL         = 96778,
    SPELL_FRENZY                    = 96800,
    SPELL_LEVEL_UP                  = 96662,
    SPELL_DEVASTATING_SLAM          = 96740,
    SPELL_DEVASTATING_SLAM_TRIGGER  = 96761,
    SPELL_DEVASTATING_SLAM_DAMAGE   = 97385,
    SPELL_SPIRIT_VENGEANCE_CANCEL   = 96821,

    // Chained Spirit
    SPELL_REVIVE                    = 96484,

    // Ohgan
    SPELL_OHGAN_HEART_VISUAL        = 96727,
    SPELL_PERMANENT_FEIGN_DEATH     = 96733,
    SPELL_CLEAR_ALL                 = 28471,
    SPELL_OHGAN_ORDERS              = 96721,
    SPELL_OHGAN_ORDERS_TRIGGER      = 96722
};

enum Events
{
    // Bloodlord Mandokir
    EVENT_SUMMON_OHGAN              = 1,
    EVENT_DECAPITATE                = 2,
    EVENT_BLOODLETTING              = 3,
    EVENT_REANIMATE_OHGAN           = 4,
    EVENT_REANIMATE_OHGAN_COOLDOWN  = 5,
    EVENT_DEVASTATING_SLAM          = 6,
    EVENT_BLOODLORD_AURA            = 7,
    EVENT_RETURN_TO_FIGHT           = 8,
    EVENT_CHECK_VALID_PLAYERS       = 9,
};

enum Action
{
    // Bloodlord Mandokir
    ACTION_OHGAN_IS_DEATH       = 1,
    ACTION_START_REVIVE         = 2,
    ACTION_EVADE                = 3,

    // Chained Spirit
    ACTION_REVIVE               = 1,
    ACTION_CHANGE_FACTION       = 2,
    ACTION_REANIMATE_OHGAN      = 3,
    ACTION_CLEAR_ALL            = 4,
};

enum Misc
{
    POINT_START_REVIVE          = 1,
    TYPE_PLAYER_REVIVE          = 1,
    FACTION_NONE                = 1665,
    DATA_OHGANOT_SO_FAST        = 5762,
};

Position const ChainedSpiritsSpawnPos[8] =
{
    { -12330.34f, -1878.406f, 127.3196f, 3.892084f   },
    { -12351.94f, -1861.51f,  127.4807f, 4.677482f   },
    { -12326.71f, -1904.328f, 127.4111f, 2.75762f    },
    { -12347.41f, -1917.535f, 127.3196f, 1.553343f   },
    { -12378.57f, -1861.222f, 127.5416f, 5.340707f   },
    { -12397.79f, -1887.731f, 127.5453f, 0.03490658f },
    { -12372.36f, -1918.844f, 127.343f,  1.151917f   },
    { -12391.23f, -1905.273f, 127.3196f, 0.6108652f  },
};

struct DiedPlayer
{
    uint64 playerGUID;
    uint32 resetTimer;
};

class boss_mandokir : public CreatureScript
{
    public:
        boss_mandokir() : CreatureScript("boss_mandokir") { }

    private:
        CreatureAI* GetAI(Creature* creature) const
        {
            return GetZulGurubAI<boss_mandokirAI>(creature);
        }

        struct boss_mandokirAI : public BossAI
        {
            boss_mandokirAI(Creature* creature) : BossAI(creature, DATA_MANDOKIR), SummonedSpirit(creature) { }

            SummonList SummonedSpirit;
            std::list<DiedPlayer> players;
            uint32 checkTimer;
            bool OhganotSoFast;
            bool Frenzy;
            bool CanEvade;

            void InitializeAI()
            {
                Reset();

                if (me->isAlive())
                    JustReachedHome();
            }

            void Reset()
            {
                _Reset();
                me->CastSpell((Unit*)NULL, SPELL_SPIRIT_VENGEANCE_CANCEL, false);
                OhganotSoFast = true;
                Frenzy = false;
                CanEvade = true;
                checkTimer = 1000;
                SummonedSpirit.DespawnAll();
                players.clear();
                me->SetReactState(REACT_AGGRESSIVE);
                instance->SendEncounterUnit(ENCOUNTER_FRAME_DISENGAGE, me);
            }

            void JustReachedHome()
            {
                for (uint8 i = 0; i < 8; ++i)
                    me->SummonCreature(NPC_CHAINED_SPIRIT, ChainedSpiritsSpawnPos[i], TEMPSUMMON_CORPSE_TIMED_DESPAWN, 1000);
            }

            void EnterEvadeMode()
            {
                if (CanEvade)
                    BossAI::EnterEvadeMode();
            }

            void SummonedCreatureDies(Creature* summoned, Unit * /*killer*/)
            {
                if (summoned->GetEntry() == NPC_CHAINED_SPIRIT)
                    SummonedSpirit.Despawn(summoned);
            }

            void JustSummoned(Creature* summoned)
            {
                switch (summoned->GetEntry())
                {
                    case NPC_CHAINED_SPIRIT:
                        SummonedSpirit.Summon(summoned);
                        return;
                    case 52324:
                        {
                            me->SetReactState(REACT_PASSIVE);
                            me->AttackStop();
                            me->CastStop();
                            me->StopMoving();
                            me->SetFacingTo(me->GetAngle(summoned));

                            for (uint8 i = 0; i <= 50; ++i)
                            {
                                float x, y;
                                me->GetNearPoint2D(x, y, frand(-2.5f, 50.0f), me->GetOrientation() + frand(-M_PI / 6, M_PI / 6));
                                me->CastSpell(x, y, me->GetPositionZ(), SPELL_DEVASTATING_SLAM_DAMAGE, true);
                            }

                            me->CastSpell(me, SPELL_DEVASTATING_SLAM, false);
                            events.ScheduleEvent(EVENT_RETURN_TO_FIGHT, 4000);
                        }
                        break;
                }

                summons.Summon(summoned);

                if (me->isInCombat())
                    summoned->SetInCombatWithZone();
            }

            void EnterCombat(Unit* /*who*/)
            {
                _EnterCombat();
                Talk(SAY_AGGRO);
                CanEvade = false;
                EntryCheckPredicate pred(NPC_CHAINED_SPIRIT);
                SummonedSpirit.DoAction(ACTION_CHANGE_FACTION, pred);
                events.ScheduleEvent(EVENT_DECAPITATE, 10000);
                events.ScheduleEvent(EVENT_BLOODLETTING, 15000);
                events.ScheduleEvent(EVENT_SUMMON_OHGAN, 20000);
                events.ScheduleEvent(EVENT_DEVASTATING_SLAM, 25000);
                events.ScheduleEvent(EVENT_BLOODLORD_AURA, 1000);
                instance->SendEncounterUnit(ENCOUNTER_FRAME_ENGAGE, me);
            }

            void JustDied(Unit* /*killer*/)
            {
                _JustDied();
                Talk(SAY_DEATH);
                players.clear();
                SummonedSpirit.DespawnAll();
                me->CastSpell((Unit*)NULL, SPELL_SPIRIT_VENGEANCE_CANCEL, false);
                instance->SendEncounterUnit(ENCOUNTER_FRAME_DISENGAGE, me);
            }

            void KilledUnit(Unit* victim)
            {
                if (victim->GetTypeId() == TYPEID_PLAYER && !CanEvade)
                {
                    Talk(SAY_PLAYER_KILL);
                    RevivePlayer(victim->ToPlayer());
                    me->CastSpell(me, SPELL_LEVEL_UP, true);

                    if (!urand(0, 9))
                    {
                        Talk(SAY_LEVEL_UP);

                        if (Creature* jindo = ObjectAccessor::GetCreature(*me, instance->GetData64(DATA_JINDO)))
                            sCreatureTextMgr->SendChat(jindo, JINDO_SAY_GRATS, 0, CHAT_MSG_ADDON, LANG_ADDON, TEXT_RANGE_ZONE);
                    }
                }
            }

            void DamageTaken(Unit* /*attacker*/, uint32& damage)
            {
                if (me->HealthBelowPctDamaged(20, damage) && !Frenzy)
                {
                    Frenzy = true;
                    Talk(SAY_FRENZY);
                    Talk(EMOTE_FRENZY);
                    me->CastSpell(me, SPELL_FRENZY, true);
                }
            }

            void DoAction(int32 const action)
            {
                switch (action)
                {
                    case ACTION_OHGAN_IS_DEATH:
                        {
                            OhganotSoFast = false;
                            uint32 LeftTime = 0;

                            if (events.GetNextEventTime(EVENT_REANIMATE_OHGAN_COOLDOWN))
                            {
                                LeftTime = events.GetNextEventTime(EVENT_REANIMATE_OHGAN_COOLDOWN) - events.GetTimer();
                                events.CancelEvent(EVENT_REANIMATE_OHGAN_COOLDOWN);
                            }

                            events.ScheduleEvent(EVENT_REANIMATE_OHGAN, LeftTime + urand(3000, 7000));
                        }
                        break;
                    case ACTION_EVADE:
                        CanEvade = true;
                        EnterEvadeMode();
                        break;
                }
            }

            uint32 GetData(uint32 type) const
            {
                if (type == DATA_OHGANOT_SO_FAST)
                    return uint32(OhganotSoFast);

                return 0;
            }

            void RevivePlayer(Player* player)
            {
                if (!SummonedSpirit.empty())
                {
                    if (Creature* chainedSpirit = ObjectAccessor::GetCreature(*me, Trinity::Containers::SelectRandomContainerElement(SummonedSpirit)))
                    {
                        DiedPlayer _player;
                        _player.playerGUID = player->GetGUID();
                        _player.resetTimer = 15000;
                        players.push_back(_player);
                        SummonedSpirit.Despawn(chainedSpirit);
                        me->CombatStart(chainedSpirit);
                        me->AddThreat(chainedSpirit, 100.0f);
                        chainedSpirit->AI()->SetGUID(player->GetGUID(), TYPE_PLAYER_REVIVE);
                    }
                }

                if (SummonedSpirit.empty())
                    CanEvade = true;
            }

            void UpdateAI(uint32 const diff)
            {
                if (!CanEvade)
                {
                    if (checkTimer <= diff)
                    {
                        bool evade = true;
                        std::list<HostileReference*> const& tList = me->getThreatManager().getThreatList();

                        for (std::list<HostileReference*>::const_iterator itr = tList.begin(); itr != tList.end(); ++itr)
                            if (Unit* target = Unit::GetUnit((*me), (*itr)->getUnitGuid()))
                                if (target->GetTypeId() == TYPEID_PLAYER && (target->isAlive()))
                                {
                                    evade = false;
                                    break;
                                }

                        if (evade)
                            for (std::list<DiedPlayer>::iterator itr = players.begin(); itr != players.end(); )
                            {
                                (*itr).resetTimer -= 1000;

                                if (!(*itr).resetTimer)
                                    itr = players.erase(itr);
                                else
                                    ++itr;
                            }

                        if (!players.empty())
                            evade = false;

                        if (evade)
                        {
                            CanEvade = true;
                            EnterEvadeMode();
                        }

                        checkTimer = 1000;
                    }
                    else
                        checkTimer -= diff;
                }

                if (!UpdateVictim())
                    return;

                if (me->HasUnitState(UNIT_STATE_CASTING))
                    return;

                events.Update(diff);

                if (uint32 eventId = events.ExecuteEvent())
                {
                    switch (eventId)
                    {
                        case EVENT_BLOODLORD_AURA:
                            me->CastSpell((Unit*)NULL, SPELL_BLOODLORD_AURA, false);
                            events.ScheduleEvent(EVENT_BLOODLORD_AURA, 1000);
                            break;
                        case EVENT_SUMMON_OHGAN:
                            Talk(SAY_DISMOUNT_OHGAN);
                            me->Dismount();
                            me->CastSpell(me, SPELL_SUMMON_OHGAN, true);
                            break;
                        case EVENT_DECAPITATE:
                            me->CastSpell((Unit*)NULL, SPELL_DECAPITATE, false);
                            events.ScheduleEvent(EVENT_DECAPITATE, Frenzy ? 17500 : 35000);
                            break;
                        case EVENT_BLOODLETTING:
                            {
                                if (Unit* target = SelectTarget(SELECT_TARGET_RANDOM, 0, 0.0f, true))
                                {
                                    me->CastSpell(target, SPELL_BLOODLETTING, true);
                                    me->ClearUnitState(UNIT_STATE_CASTING);
                                }

                                events.ScheduleEvent(EVENT_BLOODLETTING, 25000);
                            }
                            break;
                        case EVENT_REANIMATE_OHGAN:
                            Talk(SAY_REANIMATE_OHGAN);
                            me->CastSpell((Unit*)NULL, SPELL_REANIMATE_OHGAN, false);
                            events.ScheduleEvent(EVENT_REANIMATE_OHGAN_COOLDOWN, 25000);
                            break;
                        case EVENT_DEVASTATING_SLAM:
                            me->CastSpell(me, 96739, false);
                            me->CastSpell((Unit*)NULL, SPELL_DEVASTATING_SLAM_TRIGGER, false);
                            events.ScheduleEvent(EVENT_DEVASTATING_SLAM, 30000);
                            break;
                        case EVENT_RETURN_TO_FIGHT:
                            me->SetReactState(REACT_AGGRESSIVE);
                            break;
                    }
                }

                DoMeleeAttackIfReady();
                EnterEvadeIfOutOfCombatArea(diff);
            }
        };
};

class npc_ohgan : public CreatureScript
{
    public:
        npc_ohgan() : CreatureScript("npc_ohgan") { }

    private:
        CreatureAI* GetAI(Creature* creature) const
        {
            return GetZulGurubAI<npc_ohganAI>(creature);
        }

        struct npc_ohganAI : public ScriptedAI
        {
            npc_ohganAI(Creature* creature) : ScriptedAI(creature), instance(me->GetInstanceScript()) { }

            InstanceScript* instance;

            void InitializeAI()
            {
                ASSERT(instance);
                me->CastSpell(me, SPELL_OHGAN_HEART_VISUAL, true);
                me->SetReactState(REACT_PASSIVE);
            }

            void SpellHitTarget(Unit* victim, const SpellInfo * spell)
            {
                if (spell->Id == SPELL_OHGAN_ORDERS)
                    AttackStart(victim);
            }

            void EnterCombat(Unit* /*who*/)
            {
                me->CastSpell((Unit*)NULL, SPELL_OHGAN_ORDERS, true);
                instance->SendEncounterUnit(ENCOUNTER_FRAME_ENGAGE, me);
            }

            void DoAction(int32 const action)
            {
                switch (action)
                {
                    case ACTION_REANIMATE_OHGAN:
                        me->RemoveAura(SPELL_PERMANENT_FEIGN_DEATH);
                        me->CastSpell(me, SPELL_OHGAN_HEART_VISUAL, true);
                        me->CastSpell((Unit*)NULL, SPELL_OHGAN_ORDERS, true);
                        instance->SendEncounterUnit(ENCOUNTER_FRAME_ENGAGE, me);
                        break;
                    case ACTION_CLEAR_ALL:
                        me->AttackStop();
                        me->StopMoving();
                        me->SetTarget(0);
                        me->ClearInCombat();
                        me->RemoveAllAuras();
                        me->getThreatManager().resetAllAggro();
                        break;
                }
            }

            void DamageTaken(Unit* /*attacker*/, uint32& damage)
            {
                if (damage >= me->GetHealth())
                {
                    damage = 0;
                    me->CastSpell(me, SPELL_CLEAR_ALL, true);
                    me->CastSpell(me, SPELL_PERMANENT_FEIGN_DEATH);
                    instance->SendEncounterUnit(ENCOUNTER_FRAME_DISENGAGE, me);

                    if (Creature* summoner = me->ToTempSummon()->GetSummonerCreatureBase())
                        summoner->AI()->DoAction(ACTION_OHGAN_IS_DEATH);
                }
            }

            void KilledUnit(Unit* victim)
            {
                if (victim->GetEntry() == NPC_CHAINED_SPIRIT)
                    me->CastSpell((Unit*)NULL, SPELL_OHGAN_ORDERS, true);
            }

            void UpdateAI(uint32 const /*diff*/)
            {
                if (!UpdateVictim())
                    return;

                DoMeleeAttackIfReady();
            }
        };
};

class npc_chained_spirit : public CreatureScript
{
    public:
        npc_chained_spirit() : CreatureScript("npc_chained_spirit") { }

    private:
        CreatureAI* GetAI(Creature* creature) const
        {
            return GetZulGurubAI<npc_chained_spiritAI>(creature);
        }

        struct npc_chained_spiritAI : public ScriptedAI
        {
            npc_chained_spiritAI(Creature* creature) : ScriptedAI(creature) { }

            uint64 _revivePlayerGUID;
            EventMap events;

            void InitializeAI()
            {
                Reset();
            }

            void Reset()
            {
                _revivePlayerGUID = 0;
            }

            void DamageTaken(Unit* /*killer*/, uint32 &damage)
            {
                if (_revivePlayerGUID && damage > me->GetHealth())
                {
                    me->SetHealth(1);
                    damage = 0;
                }
            }

            void SetGUID(uint64 guid, int32 type)
            {
                if (type == TYPE_PLAYER_REVIVE)
                {
                    _revivePlayerGUID = guid;

                    if (Player* player = ObjectAccessor::GetPlayer(*me, _revivePlayerGUID))
                    {
                        float x, y;
                        player->GetNearPoint2D(x, y, 5.0f, player->GetAngle(me));
                        me->GetMotionMaster()->MovePoint(POINT_START_REVIVE, x, y, player->m_positionZ);
                    }
                }
            }

            void DoAction(int32 const action)
            {
                if (action == ACTION_CHANGE_FACTION)
                {
                    me->setFaction(FACTION_NONE);
                    me->SetInCombatState(false, me->ToTempSummon()->GetSummoner());
                    me->getThreatManager().addThreat(me->ToTempSummon()->GetSummoner(), std::numeric_limits<float>::max());
                    me->SetReactState(REACT_PASSIVE);
                }
            }

            void MovementInform(uint32 type, uint32 point)
            {
                if (type != POINT_MOTION_TYPE)
                    return;

                if (point == POINT_START_REVIVE)
                {
                    if (Player* player = ObjectAccessor::GetPlayer(*me, _revivePlayerGUID))
                    {
                        Talk(0, _revivePlayerGUID);
                        player->RemoveAura(SPELL_BLOODLORD_AURA);
                        me->CastSpell(player, SPELL_REVIVE, false, NULL, NULL, player->GetGUID());
                    }

                    if (Aura* aura = me->GetAura(SPELL_OHGAN_ORDERS_TRIGGER))
                        if (Unit* caster = aura->GetCaster())
                            caster->CastSpell((Unit*)NULL, SPELL_OHGAN_ORDERS, true);

                    me->DespawnOrUnsummon(3000);
                }
            }
        };
};

class spell_mandokir_decapitate : public SpellScriptLoader
{
    public:
        spell_mandokir_decapitate() : SpellScriptLoader("spell_mandokir_decapitate") { }

    private:
        class spell_mandokir_decapitate_SpellScript : public SpellScript
        {
            PrepareSpellScript(spell_mandokir_decapitate_SpellScript);

            void HandleScript(SpellEffIndex /*effIndex*/)
            {
                GetCaster()->CastSpell(GetHitUnit(), uint32(GetEffectValue()), true);
            }

            void Register()
            {
                OnEffectHitTarget += SpellEffectFn(spell_mandokir_decapitate_SpellScript::HandleScript, EFFECT_0, SPELL_EFFECT_DUMMY);
            }
        };

        SpellScript* GetSpellScript() const
        {
            return new spell_mandokir_decapitate_SpellScript();
        }
};

class spell_mandokir_bloodletting : public SpellScriptLoader
{
    public:
        spell_mandokir_bloodletting() : SpellScriptLoader("spell_mandokir_bloodletting") { }

    private:
        class spell_mandokir_bloodletting_AuraScript : public AuraScript
        {
            PrepareAuraScript(spell_mandokir_bloodletting_AuraScript);

            void HandleEffectPeriodic(AuraEffect const* aurEff)
            {
                if (GetCaster())
                {
                    int32 damage = std::max<int32>(7500, GetTarget()->CountPctFromCurHealth(aurEff->GetAmount()));
                    GetCaster()->CastCustomSpell(GetTarget(), SPELL_BLOODLETTING_DAMAGE, &damage, 0, 0, true);
                    GetTarget()->CastCustomSpell(GetCaster(), SPELL_BLOODLETTING_HEAL, &damage, 0, 0, true);
                }
            }

            void Register()
            {
                OnEffectPeriodic += AuraEffectPeriodicFn(spell_mandokir_bloodletting_AuraScript::HandleEffectPeriodic, EFFECT_0, SPELL_AURA_PERIODIC_DUMMY);
            }
        };

        AuraScript* GetAuraScript() const
        {
            return new spell_mandokir_bloodletting_AuraScript();
        }
};

class spell_mandokir_spirit_vengeance_cancel : public SpellScriptLoader
{
    public:
        spell_mandokir_spirit_vengeance_cancel() : SpellScriptLoader("spell_mandokir_spirit_vengeance_cancel") { }

    private:
        class spell_mandokir_spirit_vengeance_cancel_SpellScript : public SpellScript
        {
            PrepareSpellScript(spell_mandokir_spirit_vengeance_cancel_SpellScript);

            void HandleScript(SpellEffIndex /*effIndex*/)
            {
                GetHitUnit()->RemoveAura(uint32(GetEffectValue()));
            }

            void Register()
            {
                OnEffectHitTarget += SpellEffectFn(spell_mandokir_spirit_vengeance_cancel_SpellScript::HandleScript, EFFECT_0, SPELL_EFFECT_SCRIPT_EFFECT);
                OnEffectHitTarget += SpellEffectFn(spell_mandokir_spirit_vengeance_cancel_SpellScript::HandleScript, EFFECT_1, SPELL_EFFECT_DUMMY);
            }
        };

        SpellScript* GetSpellScript() const
        {
            return new spell_mandokir_spirit_vengeance_cancel_SpellScript();
        }
};

class spell_mandokir_ohgan_orders : public SpellScriptLoader
{
    public:
        spell_mandokir_ohgan_orders() : SpellScriptLoader("spell_mandokir_ohgan_orders") { }

    private:
        class spell_mandokir_ohgan_orders_SpellScript : public SpellScript
        {
            PrepareSpellScript(spell_mandokir_ohgan_orders_SpellScript);

            void HandleScript(SpellEffIndex /*effIndex*/)
            {
                GetCaster()->CastSpell(GetHitUnit(), uint32(GetEffectValue()), true);
            }

            void Register()
            {
                OnEffectHitTarget += SpellEffectFn(spell_mandokir_ohgan_orders_SpellScript::HandleScript, EFFECT_0, SPELL_EFFECT_DUMMY);
            }
        };

        SpellScript* GetSpellScript() const
        {
            return new spell_mandokir_ohgan_orders_SpellScript();
        }
};

class spell_mandokir_ohgan_orders_trigger : public SpellScriptLoader
{
    public:
        spell_mandokir_ohgan_orders_trigger() : SpellScriptLoader("spell_mandokir_ohgan_orders_trigger") { }

    private:
        class spell_mandokir_ohgan_orders_trigger_SpellScript : public SpellScript
        {
            PrepareSpellScript(spell_mandokir_ohgan_orders_trigger_SpellScript);

            void HandleScript(SpellEffIndex effIndex)
            {
                GetHitUnit()->CastSpell(GetCaster(), GetSpellInfo()->Effects[effIndex].BasePoints, true);
            }

            void Register()
            {
                OnEffectHitTarget += SpellEffectFn(spell_mandokir_ohgan_orders_trigger_SpellScript::HandleScript, EFFECT_1, SPELL_EFFECT_SCRIPT_EFFECT);
            }
        };

        SpellScript* GetSpellScript() const
        {
            return new spell_mandokir_ohgan_orders_trigger_SpellScript();
        }
};

class spell_mandokir_reanimate_ohgan : public SpellScriptLoader
{
    public:
        spell_mandokir_reanimate_ohgan() : SpellScriptLoader("spell_mandokir_reanimate_ohgan") { }

    private:
        class spell_mandokir_reanimate_ohgan_SpellScript : public SpellScript
        {
            PrepareSpellScript(spell_mandokir_reanimate_ohgan_SpellScript);

            void HandleScript(SpellEffIndex /*effIndex*/)
            {
                GetHitUnit()->GetAI()->DoAction(ACTION_REANIMATE_OHGAN);
            }

            void Register()
            {
                OnEffectHitTarget += SpellEffectFn(spell_mandokir_reanimate_ohgan_SpellScript::HandleScript, EFFECT_1, SPELL_EFFECT_SCRIPT_EFFECT);
            }
        };

        SpellScript* GetSpellScript() const
        {
            return new spell_mandokir_reanimate_ohgan_SpellScript();
        }
};

class spell_clear_all : public SpellScriptLoader
{
    public:
        spell_clear_all() : SpellScriptLoader("spell_clear_all") { }

    private:
        class spell_clear_all_SpellScript : public SpellScript
        {
            PrepareSpellScript(spell_clear_all_SpellScript);

            void HandleScript(SpellEffIndex /*effIndex*/)
            {
                GetCaster()->GetAI()->DoAction(ACTION_CLEAR_ALL);
            }

            void Register()
            {
                OnEffectHitTarget += SpellEffectFn(spell_clear_all_SpellScript::HandleScript, EFFECT_0, SPELL_EFFECT_SCRIPT_EFFECT);
            }
        };

        SpellScript* GetSpellScript() const
        {
            return new spell_clear_all_SpellScript();
        }
};

class spell_ohgan_permanent_feign_death : public SpellScriptLoader
{
    public:
        spell_ohgan_permanent_feign_death() : SpellScriptLoader("spell_ohgan_permanent_feign_death") { }

    private:
        class spell_ohgan_permanent_feign_deathAuraScript : public AuraScript
        {
            PrepareAuraScript(spell_ohgan_permanent_feign_deathAuraScript)

            void EffectDeath(AuraEffect const * /*aurEff*/, AuraEffectHandleModes /*mode*/)
            {
                GetTarget()->SetFlag(UNIT_FIELD_FLAGS_2, UNIT_FLAG2_FEIGN_DEATH);
                GetTarget()->SetFlag(UNIT_DYNAMIC_FLAGS, UNIT_DYNFLAG_DEAD);
                GetTarget()->SetFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_NON_ATTACKABLE);

                if (GetTarget()->ToCreature())
                    GetTarget()->ToCreature()->SetReactState(REACT_PASSIVE);
            }

            void EffectRevive(AuraEffect const * /*aurEff*/, AuraEffectHandleModes /*mode*/)
            {
                GetTarget()->RemoveFlag(UNIT_FIELD_FLAGS_2, UNIT_FLAG2_FEIGN_DEATH);
                GetTarget()->RemoveFlag(UNIT_DYNAMIC_FLAGS, UNIT_DYNFLAG_DEAD);
                GetTarget()->RemoveFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_NON_ATTACKABLE);

                if (GetTarget()->ToCreature())
                    GetTarget()->ToCreature()->SetReactState(REACT_AGGRESSIVE);
            }

            void Register()
            {
                OnEffectApply += AuraEffectApplyFn(spell_ohgan_permanent_feign_deathAuraScript::EffectDeath, EFFECT_0, SPELL_AURA_DUMMY, AURA_EFFECT_HANDLE_REAL);
                OnEffectRemove += AuraEffectRemoveFn(spell_ohgan_permanent_feign_deathAuraScript::EffectRevive, EFFECT_0, SPELL_AURA_DUMMY, AURA_EFFECT_HANDLE_REAL);
            }
        };

        AuraScript *GetAuraScript() const
        {
            return new spell_ohgan_permanent_feign_deathAuraScript();
        }
};

class DevastatingSlamTargetSelector : public std::unary_function<Unit *, bool>
{
    public:
        DevastatingSlamTargetSelector(Creature* me, const Unit* victim) : _me(me), _victim(victim) {}

        bool operator() (WorldObject* target)
        {
            if (target == _victim && _me->getThreatManager().getThreatList().size() > 1)
                return true;

            if (target->GetTypeId() != TYPEID_PLAYER)
                return true;

            return false;
        }

        Creature* _me;
        Unit const* _victim;
};

class spell_mandokir_devastating_slam : public SpellScriptLoader
{
    public:
        spell_mandokir_devastating_slam() : SpellScriptLoader("spell_mandokir_devastating_slam") { }

        class spell_mandokir_devastating_slam_SpellScript : public SpellScript
        {
            PrepareSpellScript(spell_mandokir_devastating_slam_SpellScript);

            void FilterTargets(std::list<WorldObject*>& targets)
            {
                targets.remove_if(DevastatingSlamTargetSelector(GetCaster()->ToCreature(), GetCaster()->GetVictim()));
                if (targets.empty())
                    return;

                WorldObject* target = Trinity::Containers::SelectRandomContainerElement(targets);
                targets.clear();
                targets.push_back(target);
            }

            void HandleScript(SpellEffIndex /*effIndex*/)
            {
                Unit* caster = GetCaster();
                float angle = 0.0f;
                float x, y, z;

                if (Player* target = GetHitPlayer())
                {
                    caster->AttackStop();
                    caster->SetOrientation(caster->GetAngle(target));
                    caster->SetFacingTo(caster->GetAngle(target));

                    caster->CastSpell(caster, SPELL_DEVASTATING_SLAM, false);

                    // HACK: Need better way for pos calculation
                    for (uint8 i = 0; i <= 50; ++i)
                    {
                        angle = float(rand_norm()) * static_cast<float>(M_PI * 35.0f / 180.0f) - static_cast<float>(M_PI * 17.5f / 180.0f);
                        caster->GetClosePoint(x, y, z, 4.0f, frand(-2.5f, 50.0f), angle);

                        caster->CastSpell(x, y, z, SPELL_DEVASTATING_SLAM_DAMAGE, true);
                    }
                }
            }

            void Register()
            {
                OnObjectAreaTargetSelect += SpellObjectAreaTargetSelectFn(spell_mandokir_devastating_slam_SpellScript::FilterTargets, EFFECT_0, TARGET_UNIT_SRC_AREA_ENEMY);
                OnEffectHitTarget += SpellEffectFn(spell_mandokir_devastating_slam_SpellScript::HandleScript, EFFECT_0, SPELL_EFFECT_FORCE_CAST);
            }
        };

        SpellScript* GetSpellScript() const
        {
            return new spell_mandokir_devastating_slam_SpellScript();
        }
};

class achievement_ohganot_so_fast : public AchievementCriteriaScript
{
   public:
       achievement_ohganot_so_fast() : AchievementCriteriaScript("achievement_ohganot_so_fast") { }

    private:
       bool OnCheck(Player* source, Unit* target)
       {
           return target && target->GetAI()->GetData(DATA_OHGANOT_SO_FAST);
       }
};

void AddSC_boss_mandokir()
{
    new boss_mandokir();
    new npc_ohgan();
    new npc_chained_spirit();

    new spell_mandokir_decapitate();
    new spell_mandokir_bloodletting();
    new spell_mandokir_spirit_vengeance_cancel();
    new spell_mandokir_ohgan_orders();
    new spell_mandokir_ohgan_orders_trigger();
    new spell_mandokir_reanimate_ohgan();
    new spell_clear_all();
    new spell_ohgan_permanent_feign_death();

    new achievement_ohganot_so_fast();
}
