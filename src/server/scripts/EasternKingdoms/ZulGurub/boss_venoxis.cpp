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

#include "ScriptMgr.h"
#include "ScriptedCreature.h"
#include "MoveSplineInit.h"
#include "SpellScript.h"
#include "zulgurub.h"

enum Spells
{
    SPELL_VENOM_TOTEM              = 96534,
    SPELL_VENOXIS_ULT_COSMETIC     = 96634,
    SPELL_VENOM_EFFUSION           = 96678,
    SPELL_VENOM_EFFUSION_TRIGGERED = 96681,
    SPELL_POOL_OF_ACID_TEARS       = 96520,
    SPELL_VENOM_TOTEM_BEAM_CENTER  = 97098,
    SPELL_BLOODVENOM               = 97099,
    SPELL_BLOODVENOM_INST          = 97110,
    SPELL_BLOODVENOM_TRIGGERED     = 96842,
    SPELL_WORD_HETHISS             = 96560,
    SPELL_WHISPERS_HETHISS         = 96466,
    SPELL_TOXIC_LINK               = 96475,
    SPELL_TOXIC_LINK_TOTEM         = 96477,
    SPELL_BLESSING_SNAKEGOD        = 97354,
    SPELL_REMOVE_DODGE_PARRY       = 96602,
    SPELL_VENOM_WITHDRAWAL         = 96653,
    SPELL_BREATH_OF_HETHISS        = 96509,
    SPELL_POOL_ACRID               = 96515,
    SPELL_TRANSFOR_REMOVE          = 96617, // Remove Transform from Venoxis
};

enum Events
{
    EVENT_VENOMOUS_EFFUSION                     = 1,
    EVENT_REMOVE_EFFUSION_VISUAL                = 2,
    EVENT_WHISPERS_OF_HETHISS                   = 3,
    EVENT_TOXIC_LINK                            = 4,
    EVENT_BLESSING_OF_THE_SNAKE_GOD             = 5,
    EVENT_POOL_OF_ACRID_TEARS                   = 6,
    EVENT_BREATH_OF_HETHISS                     = 7,
    EVENT_TRANSFORM_REMOVAL_PRIMER              = 8,
    EVENT_VENOM_WITHDRAWAL                      = 9,
    EVENT_RETURN_TO_FIGHT                       = 10,
    EVENT_SET_BLOODVENOM_PLAYER                 = 11,
    EVENT_CHANGE_BLOODVENOM_PLAYER              = 12,
    EVENT_CHECK_ROOM_POSITION                   = 13,
    // Bloodvenom
    EVENT_MOVE_PLAYER                           = 1,
    EVENT_PAUSE                                 = 2,
};

enum Texts
{
    SAY_AGGRO               = 0,
    SAY_TOXIC_LINK          = 1,
    SAY_TRANSFROM           = 2,
    SAY_BLOODVENOM          = 3,
    EMOTE_BLOODVENOM        = 4,
    EMOTE_VENOM_WITHDRAWAL  = 5,
    SAY_DEATH               = 6,
    SAY_PLAYER_KILL         = 7,
};

const uint32 TotemSpell[2]=
{
    96536,
    96537,
};

const Position TotemSP[2]=
{
    {-12011.0f, -1705.97f, 43.4762f, 0.523599f},
    {-12021.8f, -1688.28f, 43.4524f, 0.558505f},
};

const Position EffusionSP[2]=
{
    {-12004.82f, -1699.144f, 32.36689f, 0.0f},
    {-12014.38f, -1684.675f, 32.36689f, 0.0f},
};

const Position BoosSP = {-12020.4f, -1699.78f, 39.6222f, 0.698132f};

const float RoomCenter[2] = {-12006.8f, -1699.29f};

class boss_venoxis : public CreatureScript
{
    public:
        boss_venoxis() : CreatureScript("boss_venoxis") { }

    private:
        CreatureAI* GetAI(Creature* creature) const
        {
            return GetZulGurubAI<boss_venoxisAI>(creature);
        }

        struct boss_venoxisAI : public BossAI
        {
            boss_venoxisAI(Creature* creature) : BossAI(creature, DATA_VENOXIS), AcridTears(creature), BloodVenoms(creature) { }

            uint64 TotemGUID[2];
            SummonList AcridTears;
            SummonList BloodVenoms;

            void InitializeAI()
            {
                for (int i = 0; i < 2; ++i)
                    if (Creature* totem = me->SummonCreature(NPC_GENERAL_PURPOSE_BUNNY, TotemSP[i]))
                    {
                        TotemGUID[i] = totem->GetGUID();
                        totem->SetControlled(true, UNIT_STATE_STUNNED);
                        totem->CastSpell(totem, SPELL_VENOM_TOTEM, false);
                        totem->CastSpell(me, TotemSpell[i], false);
                    }

                Reset();
            }

            void Reset()
            {
                _Reset();
                AcridTears.DespawnAll();
                BloodVenoms.DespawnAll();
                me->SetReactState(REACT_AGGRESSIVE);
                instance->SendEncounterUnit(ENCOUNTER_FRAME_DISENGAGE, me);

                if (GameObject* forcefield = me->FindNearestGameObject(GO_THE_CACHE_OF_MADNESS_DOOR, 150.0f))
                    me->RemoveGameObject(forcefield, true);
            }

            void SetBloodvenomTarget()
            {
                ThreatContainer::StorageType threatList = me->getThreatManager().getThreatList();
                ThreatContainer::StorageType::const_iterator hostileReference = threatList.begin();
                SummonList::const_iterator bloodvenomGUID = BloodVenoms.begin();

                while (bloodvenomGUID != BloodVenoms.end() && hostileReference != threatList.end())
                {
                    if (Creature* bloodvenom = ObjectAccessor::GetCreature(*me, *bloodvenomGUID))
                        bloodvenom->AI()->SetGUID((*hostileReference)->getUnitGuid());

                    ++hostileReference;
                    ++bloodvenomGUID;
                }
            }

            void JustReachedHome()
            {
                if (!me->isInCombat())
                {
                    for (int i = 0; i < 2; ++i)
                        if (Creature* totem = ObjectAccessor::GetCreature(*me, TotemGUID[i]))
                            totem->CastSpell(me, TotemSpell[i], false);
                }
                else
                {
                    for (SummonList::iterator itr = AcridTears.begin(); itr != AcridTears.end(); ++itr)
                    {
                        if (Creature* acrid = ObjectAccessor::GetCreature(*me, *itr))
                        {
                            acrid->CastSpell(acrid, SPELL_VENOXIS_ULT_COSMETIC, false);
                            acrid->RemoveAllAuras();
                            acrid->DespawnOrUnsummon(2000);
                        }
                    }

                    Talk(SAY_BLOODVENOM);
                    Talk(EMOTE_BLOODVENOM);
                    me->CastSpell((Unit*)NULL, SPELL_BLOODVENOM_TRIGGERED, false);
                    events.ScheduleEvent(EVENT_VENOM_WITHDRAWAL, 15000);
                    events.ScheduleEvent(EVENT_SET_BLOODVENOM_PLAYER, 4000);
                }
            }

            void VenomousEffusion()
            {
                for (int i = 0; i < 2; ++i)
                    if (Creature* effusion_stalker = me->SummonCreature(52302, EffusionSP[i]))
                    {
                        if (Creature* totem = ObjectAccessor::GetCreature(*me, TotemGUID[i]))
                            totem->CastSpell(effusion_stalker, TotemSpell[i], false);

                        effusion_stalker->CastSpell(effusion_stalker, SPELL_VENOM_EFFUSION, false, NULL, NULL, me->GetGUID());
                    }

                events.ScheduleEvent(EVENT_REMOVE_EFFUSION_VISUAL, 7000);
            }

            void JustSummoned(Creature* summon)
            {
                switch (summon->GetEntry())
                {
                    case NPC_GENERAL_PURPOSE_BUNNY:
                        return;
                    case NPC_POOL_OF_ACID_TEARS:
                        summon->SetReactState(REACT_PASSIVE);
                        summon->CastSpell(summon, SPELL_POOL_OF_ACID_TEARS, false);
                        summon->SetInCombatWithZone();
                        AcridTears.Summon(summon);
                        return;
                    case NPC_BLOOD_VENOM:
                        summon->SetReactState(REACT_PASSIVE);
                        summon->CastSpell(summon, SPELL_BLOODVENOM_INST, false);
                        summon->CastSpell(summon, SPELL_BLOODVENOM, false);
                        summon->CastSpell(me, SPELL_VENOM_TOTEM_BEAM_CENTER, false);
                        BloodVenoms.Summon(summon);
                        return;
                    case NPC_VENOMOOUS_EFFUSION:
                        summon->CastSpell(summon, SPELL_VENOM_EFFUSION_TRIGGERED, false);
                        break;
                }

                summon->SetInCombatWithZone();
                summons.Summon(summon);
            }

            void EnterCombat(Unit* /*who*/)
            {
                _EnterCombat();
                Talk(SAY_AGGRO);
                me->CastSpell((Unit*)NULL, SPELL_WORD_HETHISS, false);
                events.ScheduleEvent(EVENT_VENOMOUS_EFFUSION, 1000);
                events.ScheduleEvent(EVENT_BLESSING_OF_THE_SNAKE_GOD, 30000);
                events.ScheduleEvent(EVENT_WHISPERS_OF_HETHISS, urand(5000, 10000));
                events.ScheduleEvent(EVENT_TOXIC_LINK, 10000);
//                events.ScheduleEvent(EVENT_CHECK_ROOM_POSITION, 1000);
                instance->SendEncounterUnit(ENCOUNTER_FRAME_ENGAGE, me);

                for (int i = 0; i < 2; ++i)
                    if (Creature* totem = ObjectAccessor::GetCreature(*me, TotemGUID[i]))
                        totem->CastStop();

                me->SummonGameObject(GO_THE_CACHE_OF_MADNESS_DOOR, -11938.6f, -1843.32f, 61.7272f, 0.0899053f, 0, 0, 0, 0, 0);
            }

            void KilledUnit(Unit* victim)
            {
                if (victim->GetTypeId() == TYPEID_PLAYER)
                    Talk(SAY_PLAYER_KILL);
            }

            void JustDied(Unit* /*killer*/)
            {
                _JustDied();
                Talk(SAY_DEATH);
                AcridTears.DespawnAll();
                BloodVenoms.DespawnAll();
                instance->SendEncounterUnit(ENCOUNTER_FRAME_DISENGAGE, me);

                if (GameObject* forcefield = me->FindNearestGameObject(GO_THE_CACHE_OF_MADNESS_DOOR, 150.0f))
                    me->RemoveGameObject(forcefield, true);
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
                        case EVENT_CHECK_ROOM_POSITION:
                            {
                                if (me->GetExactDist2dSq(RoomCenter[0], RoomCenter[1]) > 1500.0f || !BoosSP.HasInArc(M_PI, me))
                                {
                                    BossAI::EnterEvadeMode();
                                    return;
                                }

                                events.ScheduleEvent(EVENT_CHECK_ROOM_POSITION, 1000);
                            }
                            break;
                        case EVENT_VENOMOUS_EFFUSION:
                            VenomousEffusion();
                            break;
                        case EVENT_REMOVE_EFFUSION_VISUAL:
                            {
                                for (int i = 0; i < 2; ++i)
                                    if (Creature* totem = ObjectAccessor::GetCreature(*me, TotemGUID[i]))
                                        totem->CastStop();
                            }
                            break;
                        case EVENT_WHISPERS_OF_HETHISS:
                            {
                                if (Unit* target = SelectTarget(SELECT_TARGET_RANDOM, 0, 50.0f, true))
                                    me->CastSpell(target, SPELL_WHISPERS_HETHISS, false);

                                events.ScheduleEvent(EVENT_WHISPERS_OF_HETHISS, urand(10000, 20000));
                            }
                            break;
                        case EVENT_TOXIC_LINK:
                            Talk(SAY_TOXIC_LINK);
                            me->CastSpell((Unit*)NULL, SPELL_TOXIC_LINK, false);
                            events.ScheduleEvent(EVENT_TOXIC_LINK, 10000);
                            break;
                        case EVENT_BLESSING_OF_THE_SNAKE_GOD:
                            Talk(SAY_TRANSFROM);
                            me->CastSpell(me, SPELL_REMOVE_DODGE_PARRY, false);
                            me->CastSpell(me, SPELL_BLESSING_SNAKEGOD, false);
                            events.Reset();
                            events.ScheduleEvent(EVENT_POOL_OF_ACRID_TEARS, urand(3000, 7000));
                            events.ScheduleEvent(EVENT_BREATH_OF_HETHISS, urand(4000, 8000));
                            events.ScheduleEvent(EVENT_TRANSFORM_REMOVAL_PRIMER, 30000);
                            events.ScheduleEvent(EVENT_CHECK_ROOM_POSITION, 1000);
                            break;
                        case EVENT_POOL_OF_ACRID_TEARS:
                            {
                                if (Unit* target = SelectTarget(SELECT_TARGET_FARTHEST, 0, 100.0f, true))
                                    me->CastSpell(target, SPELL_POOL_ACRID, false);

                                events.ScheduleEvent(EVENT_POOL_OF_ACRID_TEARS, urand(3000, 7000));
                            }
                            break;
                        case EVENT_BREATH_OF_HETHISS:
                            {
                                if (Unit* target = SelectTarget(SELECT_TARGET_RANDOM, 0, 15.0f, true))
                                {
                                    me->SetReactState(REACT_PASSIVE);
                                    me->AttackStop();
                                    me->StopMoving();
                                    me->SetFacingTo(me->GetAngle(target));
                                    events.ScheduleEvent(EVENT_RETURN_TO_FIGHT, 3000);
                                }

                                me->CastSpell(me, SPELL_BREATH_OF_HETHISS, false);
                                events.ScheduleEvent(EVENT_BREATH_OF_HETHISS, urand(5000, 10000));
                            }
                            break;
                        case EVENT_RETURN_TO_FIGHT:
                            me->SetReactState(REACT_AGGRESSIVE);
                            break;
                        case EVENT_TRANSFORM_REMOVAL_PRIMER:
                            me->CastSpell(me, SPELL_TRANSFOR_REMOVE, false);
                            me->SetReactState(REACT_PASSIVE);
                            me->AttackStop();
                            me->GetMotionMaster()->MoveTargetedHome();
                            events.Reset();
                            break;
                        case EVENT_VENOM_WITHDRAWAL:
                            {
                                for (SummonList::iterator itr = summons.begin(); itr != summons.end(); )
                                {
                                    if (Creature* summon = ObjectAccessor::GetCreature(*me, *itr))
                                    {
                                        summon->CastStop();
                                        summon->RemoveAllAuras();
                                        summon->DespawnOrUnsummon(2000);
                                    }

                                    itr = summons.erase(itr);
                                }

                                Talk(EMOTE_VENOM_WITHDRAWAL);
                                BloodVenoms.DespawnAll();
                                me->RemoveAura(SPELL_BLESSING_SNAKEGOD);
                                me->RemoveAura(SPELL_REMOVE_DODGE_PARRY);
                                me->CastSpell(me, SPELL_VENOM_WITHDRAWAL, false);
                                me->SetReactState(REACT_AGGRESSIVE);
                                events.ScheduleEvent(EVENT_VENOMOUS_EFFUSION, 11000);
                                events.ScheduleEvent(EVENT_BLESSING_OF_THE_SNAKE_GOD, 40000);
                                events.ScheduleEvent(EVENT_WHISPERS_OF_HETHISS, urand(15000, 20000));
                                events.ScheduleEvent(EVENT_TOXIC_LINK, urand(12000, 15000));
                                events.ScheduleEvent(EVENT_CHECK_ROOM_POSITION, 1000);
                            }
                            break;
                        case EVENT_SET_BLOODVENOM_PLAYER:
                            SetBloodvenomTarget();
                            events.ScheduleEvent(EVENT_CHANGE_BLOODVENOM_PLAYER, 6000);
                            break;
                        case EVENT_CHANGE_BLOODVENOM_PLAYER:
                            SetBloodvenomTarget();
                            break;
                    }
                }

                DoMeleeAttackIfReady();
            }
        };
};

class npc_venoxis_bloodvenom : public CreatureScript
{
    public:
        npc_venoxis_bloodvenom() : CreatureScript("npc_venoxis_bloodvenom") { }

    private:
        CreatureAI* GetAI(Creature* creature) const
        {
            return GetZulGurubAI<npc_venoxis_bloodvenomAI>(creature);
        }

        struct npc_venoxis_bloodvenomAI : public ScriptedAI
        {
            npc_venoxis_bloodvenomAI(Creature* creature) : ScriptedAI(creature) { }

            EventMap events;
            uint64 FollowTargetGUID;

            void InitializeAI()
            {
                events.ScheduleEvent(EVENT_MOVE_PLAYER, 300);
            }

            void SetGUID(uint64 guid, int32 /*type*/)
            {
                FollowTargetGUID = guid;
            }

            void UpdateAI(uint32 const diff)
            {
                events.Update(diff);

                if (events.ExecuteEvent() == EVENT_MOVE_PLAYER)
                {
                    if (Unit* target = ObjectAccessor::GetUnit(*me, FollowTargetGUID))
                        if (me->GetExactDist(target) >= 1.0f)
                        {
                            Movement::MoveSplineInit init(me);
                            init.MoveTo(target->GetPositionX(), target->GetPositionY(), target->GetPositionZ());
                            init.Launch();
                        }

                    events.ScheduleEvent(EVENT_MOVE_PLAYER, 300);
                }
            }
        };
};

class spell_venomous_effusion_summon : public SpellScriptLoader
{
    public:
        spell_venomous_effusion_summon() : SpellScriptLoader("spell_venomous_effusion_summon") { }

    private:
        class spell_venomous_effusion_summon_SpellScript : public SpellScript
        {
            PrepareSpellScript(spell_venomous_effusion_summon_SpellScript);

            void ModDestHeight(SpellEffIndex /*effIndex*/)
            {
                Position offset = {0.0f, 0.0f, 1.5f, 0.0f};
                const_cast<WorldLocation*>(GetExplTargetDest())->RelocateOffset(offset);
                GetHitDest()->RelocateOffset(offset);
            }

            void Register()
            {
                OnEffectLaunch += SpellEffectFn(spell_venomous_effusion_summon_SpellScript::ModDestHeight, EFFECT_1, SPELL_EFFECT_SUMMON);
            }
        };

        SpellScript* GetSpellScript() const
        {
            return new spell_venomous_effusion_summon_SpellScript();
        }
};

class spell_toxic_link_selector : public SpellScriptLoader
{
    public:
        spell_toxic_link_selector() : SpellScriptLoader("spell_toxic_link_selector") { }

    private:
        class spell_toxic_link_selector_SpellScript : public SpellScript
        {
            PrepareSpellScript(spell_toxic_link_selector_SpellScript)

            void TriggerSpell(SpellEffIndex effIndex)
            {
                GetCaster()->CastSpell(GetHitUnit(), GetSpellInfo()->Effects[effIndex].BasePoints, true);
            }

            void Register()
            {
                OnEffectHitTarget += SpellEffectFn(spell_toxic_link_selector_SpellScript::TriggerSpell, EFFECT_0, SPELL_EFFECT_DUMMY);
            }
        };

        SpellScript *GetSpellScript() const
        {
            return new spell_toxic_link_selector_SpellScript();
        }
};

class spell_toxic_link_visual : public SpellScriptLoader
{
    public:
        spell_toxic_link_visual() : SpellScriptLoader("spell_toxic_link_visual") { }

    private:
        class spell_toxic_link_visual_SpellScript : public SpellScript
        {
            PrepareSpellScript(spell_toxic_link_visual_SpellScript);

            void FilterTargets(std::list<WorldObject*>& unitList)
            {
                if (unitList.empty())
                    GetCaster()->RemoveAura(SPELL_TOXIC_LINK_TOTEM);
            }

            void Register()
            {
                OnObjectAreaTargetSelect += SpellObjectAreaTargetSelectFn(spell_toxic_link_visual_SpellScript::FilterTargets, EFFECT_0, TARGET_UNIT_SRC_AREA_ALLY);
            }
        };

        SpellScript* GetSpellScript() const
        {
            return new spell_toxic_link_visual_SpellScript();
        }
};

class spell_toxic_link_aura : public SpellScriptLoader
{
    public:
        spell_toxic_link_aura() : SpellScriptLoader("spell_toxic_link_aura") { }

    private:
        class spell_toxic_link_aura_AuraScript : public AuraScript
        {
            PrepareAuraScript(spell_toxic_link_aura_AuraScript);

            void OnRemove(AuraEffect const* /*aurEff*/, AuraEffectHandleModes /*mode*/)
            {
                GetTarget()->CastSpell((Unit*)NULL, GetSpellInfo()->Effects[EFFECT_2].BasePoints, true);
            }

            void Register()
            {
                OnEffectRemove += AuraEffectRemoveFn(spell_toxic_link_aura_AuraScript::OnRemove, EFFECT_0, SPELL_AURA_PERIODIC_TRIGGER_SPELL, AURA_EFFECT_HANDLE_REAL);
            }
        };

        AuraScript* GetAuraScript() const
        {
            return new spell_toxic_link_aura_AuraScript();
        }
};

class spell_whispers_of_hethiss : public SpellScriptLoader
{
    public:
        spell_whispers_of_hethiss() : SpellScriptLoader("spell_whispers_of_hethiss") { }

    private:
        class spell_whispers_of_hethiss_AuraScript : public AuraScript
        {
            PrepareAuraScript(spell_whispers_of_hethiss_AuraScript)

            void HandlePeriodicTick(AuraEffect const* /*aurEff*/)
            {
                PreventDefaultAction();

                if (Unit* target = ObjectAccessor::GetUnit(*GetCaster(), GetCaster()->GetUInt64Value(UNIT_FIELD_CHANNEL_OBJECT)))
                    GetCaster()->CastSpell(target, GetSpellInfo()->Effects[EFFECT_1].TriggerSpell, true);
            }

            void Register()
            {
                OnEffectPeriodic += AuraEffectPeriodicFn(spell_whispers_of_hethiss_AuraScript::HandlePeriodicTick, EFFECT_1, SPELL_AURA_PERIODIC_TRIGGER_SPELL);
            }
        };

        AuraScript* GetAuraScript() const
        {
            return new spell_whispers_of_hethiss_AuraScript();
        }
};

class spell_venom_withdrawal : public SpellScriptLoader
{
    public:
        spell_venom_withdrawal() : SpellScriptLoader("spell_venom_withdrawal") { }

    private:
        class spell_venom_withdrawal_SpellScript : public SpellScript
        {
            PrepareSpellScript(spell_venom_withdrawal_SpellScript);

            void LeapBack(SpellEffIndex effIndex)
            {
                PreventHitDefaultEffect(effIndex);

                if (Unit* caster = GetCaster())
                {
                    float speedxy = float(GetSpellInfo()->Effects[effIndex].MiscValue) / 10;
                    float speedz = float(GetSpellInfo()->Effects[effIndex].BasePoints / 10);
                    caster->JumpTo(speedxy, speedz, true);
                }
            }

            void Register()
            {
                OnEffectHitTarget += SpellEffectFn(spell_venom_withdrawal_SpellScript::LeapBack, EFFECT_2, SPELL_EFFECT_LEAP_BACK);
            }
        };

        SpellScript* GetSpellScript() const
        {
            return new spell_venom_withdrawal_SpellScript();
        }
};

void AddSC_boss_venoxis()
{
    new boss_venoxis();
    new npc_venoxis_bloodvenom();

    new spell_venomous_effusion_summon();
    new spell_toxic_link_selector();
    new spell_toxic_link_visual();
    new spell_toxic_link_aura();
    new spell_whispers_of_hethiss();
    new spell_venom_withdrawal();
}
