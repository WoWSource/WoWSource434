/*
* Copyright (C) 2010-2011 Project Trinity <http://www.projecttrinity.org/>
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

#include "ScriptPCH.h"
#include "blackrock_caverns.h"

#include "SpellAuraEffects.h"

#define ENCOUNTERS 5

/*Boss Encounters
  Rom'Ogg Bonecrusher
  Corla, Herald of Twilight
  Karsh Steelbender
  Beauty
  Ascendant Lord Obsidius
*/

class npc_raz_the_crazed : public CreatureScript
{
    public:

        npc_raz_the_crazed() : CreatureScript("npc_raz_the_crazed")
        {
        }

        struct npc_raz_the_crazedAI : public ScriptedAI
        {
            npc_raz_the_crazedAI(Creature* creature) : ScriptedAI(creature)
            {
                creature->CastSpell(creature, SPELL_SHADOW_PRISON, true);
                uiPhase = 0;
            }

            uint8 uiPhase;

            void MovementInform(uint32 uiMoveType, uint32 uiPointId)
            {
                if (!me->HasAura(SPELL_AGGRO_NEARBY_TARGETS)) // Reapply aura
                    me->CastSpell(me, SPELL_AGGRO_NEARBY_TARGETS, true);

                if (uiPointId == 400) // Jumped near to the last boss
                {
                    if (Creature * lord = GetClosestCreatureWithEntry(me, BOSS_ASCENDANT_LORD_OBSIDIUS, 100.0f))
                    {
                        // Apply killing aura
                        lord->CastSpell(me, 82393, true);
                        me->AddAura(75054, me);
                    }
                }
                else if (uiPointId == 300)
                {
                    me->GetMotionMaster()->MoveJump(257.176f, 648.703f, 96.130f, 20.0f, 5.0f);
                }
                else if (uiPointId == 200) // After second jump down
                {
                    me->GetMotionMaster()->Clear();
                    me->GetMotionMaster()->MovePath(39671, false);
                }
                else if (uiMoveType == POINT_MOTION_TYPE && uiPointId != 100)
                {
                    // Start movement
                    me->SetSpeed(MOVE_WALK, 3.5f);
                    me->SetSpeed(MOVE_RUN, 3.5f);
                    me->GetMotionMaster()->MovePath(39670, false);
                    me->CastSpell(me, SPELL_AGGRO_NEARBY_TARGETS, true);
                }
                else
                {
                    if (uiMoveType == WAYPOINT_MOTION_TYPE)
                    {
                        if (uiPointId == 5 && uiPhase == 0) // First jump down
                        {
                            me->GetMotionMaster()->MoveJump(406.270081f, 906.590271f, 163.427017f, 15.0f, 25.0f, 100);
                            me->GetMotionMaster()->MoveIdle();
                            uiPhase = 1;
                        }
                        else if (uiPointId == 2 && uiPhase == 2)
                        {
                            uiPhase = 3;
                            me->GetMotionMaster()->MoveJump(225.531f, 724.160f, 105.719f, 20.0f, 10.0f, 300);
                            me->GetMotionMaster()->MoveIdle();
                        }
                    }
                }
            }

            void UpdateAI(uint32 const /*diff*/)
            {
                if (!UpdateVictim())
                    return;

                DoMeleeAttackIfReady();
            }
        };

        CreatureAI* GetAI(Creature* creature) const
        {
            return new npc_raz_the_crazedAI(creature);
        }
};

class at_raz_jump_down : public AreaTriggerScript
{
    public:
        at_raz_jump_down() : AreaTriggerScript("at_raz_jump_down") { }

        bool OnTrigger(Player* player, AreaTriggerEntry const* /*areaTrigger*/)
        {
            InstanceScript* instance = player->GetInstanceScript();

            if (Creature * raz = instance->instance->GetCreature(instance->GetData64(DATA_RAZ_THE_CRAZED)))
            {
                if (npc_raz_the_crazed::npc_raz_the_crazedAI* razAI = CAST_AI(npc_raz_the_crazed::npc_raz_the_crazedAI, raz->GetAI()))
                {
                    if (razAI->uiPhase == 1)
                    {
                        razAI->uiPhase = 2;
                        raz->GetMotionMaster()->MoveJump(403.945f, 816.761f, 102.408f, 30.0f, 10.0f, 200);
                    }
                    if (razAI->uiPhase == 3) // Last boss jump
                    {
                        raz->GetMotionMaster()->Clear();
                        raz->GetMotionMaster()->MoveJump(303.831f, 564.210f, 66.381f, 30.0f, 10.0f, 400);
                        raz->GetMotionMaster()->MoveIdle();
                        razAI->uiPhase = 4;
                    }
                }
            }

            return true;
        }
};


class instance_blackrock_caverns : public InstanceMapScript
{
public:
    instance_blackrock_caverns() : InstanceMapScript("instance_blackrock_caverns", 645) { }

    InstanceScript* GetInstanceScript(InstanceMap* map) const
    {
        return new instance_blackrock_cavernsInstanceMapScript(map);
    }

    struct instance_blackrock_cavernsInstanceMapScript : public InstanceScript
    {
        instance_blackrock_cavernsInstanceMapScript(InstanceMap* map) : InstanceScript(map) { }

        uint32 uiEncounter[ENCOUNTERS];

        uint64 uiRomoggBonecrusher;
        uint64 uiCorla;
        uint64 uiKarshSteelbender;
        uint64 uiBeauty;
        uint64 uiAscendantLordObsidius;

        uint64 uiRazTheCrazed;

        void Initialize()
        {
             uiRomoggBonecrusher = 0;
             uiCorla = 0;
             uiKarshSteelbender = 0;
             uiBeauty = 0;
             uiAscendantLordObsidius = 0;

             uiRazTheCrazed = 0;

             for (uint8 i = 0 ; i < ENCOUNTERS; ++i)
                 uiEncounter[i] = NOT_STARTED;
        }

        bool IsEncounterInProgress() const
        {
            for (uint8 i = 0; i < ENCOUNTERS; ++i)
            {
                if (uiEncounter[i] == NOT_STARTED)
                    return true;
            }
            return false;
        }

        void OnCreatureCreate(Creature* creature)
        {
            switch (creature->GetEntry())
            {
                case BOSS_ROMOGG_BONECRUSHER:
                    uiRomoggBonecrusher = creature->GetGUID();
                    break;
                case BOSS_CORLA:
                    uiCorla = creature->GetGUID();
                    break;
                case BOSS_KARSH_STEELBENDER:
                    uiKarshSteelbender = creature->GetGUID();
                    break;
                case BOSS_BEAUTY:
                    uiBeauty = creature->GetGUID();
                    break;
                case BOSS_ASCENDANT_LORD_OBSIDIUS:
                    uiAscendantLordObsidius = creature->GetGUID();
                    break;
                case NPC_RAZ_THE_CRAZED:
                    uiRazTheCrazed = creature->GetGUID();
                    break;
            }
        }

        uint64 GetData64(uint32 identifier) const
        {
            switch (identifier)
            {
                case DATA_ROMOGG_BONECRUSHER:
                    return uiRomoggBonecrusher;
                case DATA_CORLA:
                    return uiCorla;
                case DATA_KARSH_STEELBENDER:
                    return uiKarshSteelbender;
                case DATA_BEAUTY:
                    return uiBeauty;
                case DATA_ASCENDANT_LORD_OBSIDIUS:
                    return uiAscendantLordObsidius;
                case DATA_RAZ_THE_CRAZED:
                    return uiRazTheCrazed;
            }
            return 0;
        }

        void SetData(uint32 type, uint32 data)
        {
            switch (type)
            {
                case DATA_ROMOGG_BONECRUSHER_EVENT:
                    uiEncounter[0] = data;
                    break;
                case DATA_CORLA_EVENT:
                    uiEncounter[1] = data;
                    break;
                case DATA_KARSH_STEELBENDER_EVENT:
                    uiEncounter[2] = data;
                    break;
                case DATA_BEAUTY_EVENT:
                    uiEncounter[3] = data;
                    break;
                case DATA_ASCENDANT_LORD_OBSIDIUS_EVENT:
                    uiEncounter[4] = data;
                    break;
            }
            if (data == DONE)
                SaveToDB();
        }

        uint32 GetData(uint32 type) const
        {
            switch (type)
            {
                case DATA_ROMOGG_BONECRUSHER_EVENT:
                    return uiEncounter[0];
                case DATA_CORLA_EVENT:
                    return uiEncounter[1];
                case DATA_KARSH_STEELBENDER_EVENT:
                    return uiEncounter[2];
                case DATA_BEAUTY_EVENT:
                    return uiEncounter[3];
                case DATA_ASCENDANT_LORD_OBSIDIUS_EVENT:
                    return uiEncounter[4];
            }
            return 0;
        }

        std::string GetSaveData()
        {
            OUT_SAVE_INST_DATA;

            std::string str_data;
            std::ostringstream saveStream;
            saveStream << "B C" << uiEncounter[0] << " " << uiEncounter[1]  << " " << uiEncounter[2]  << " " << uiEncounter[3] << " " << uiEncounter[4];
            str_data = saveStream.str();

            OUT_SAVE_INST_DATA_COMPLETE;
            return str_data;
        }

        void Load(const char* in)
        {
            if (!in)
            {
                OUT_LOAD_INST_DATA_FAIL;
                return;
            }

            OUT_LOAD_INST_DATA(in);

            char dataHead1, dataHead2;
            uint16 data0, data1, data2, data3, data4;

            std::istringstream loadStream(in);
            loadStream >> dataHead1 >> dataHead2 >> data0 >> data1 >> data2 >> data3 >> data4;

            if (dataHead1 == 'B' && dataHead2 == 'C')
            {
                uiEncounter[0] = data0;
                uiEncounter[1] = data1;
                uiEncounter[2] = data2;
                uiEncounter[3] = data3;
                uiEncounter[4] = data4;

                for (uint8 i=0; i < ENCOUNTERS; ++i)
                    if (uiEncounter[i] == IN_PROGRESS)
                        uiEncounter[i] = NOT_STARTED;
            }
            else OUT_LOAD_INST_DATA_FAIL;

            OUT_LOAD_INST_DATA_COMPLETE;
        }
    };
};

enum ZealotType
{
    TYPE_MAGE       = 0,
    TYPE_WARLOCK    = 1,
    TYPE_ROGUE      = 2,
    TYPE_WARRIOR    = 3,
};

enum ZealotSpells
{
    SPELL_REND                  = 76594,
    SPELL_MORTAL_STRIKE         = 13737,

    SPELL_ARCANE_BARRAGE        = 76589,
    SPELL_AURA_OF_ARCANE_HASTE  = 76591,

    SPELL_SHADOW_BOLT           = 76584,
    SPELL_SHADOW_NOVA           = 76588,

    SPELL_KICK                  = 76583,
    SPELL_GOUGE                 = 76582,
};

class mob_twilight_zaelot: public CreatureScript
{
public:
    mob_twilight_zaelot() : CreatureScript("mob_twilight_zaelot_trash") { }

    CreatureAI* GetAI(Creature* creature) const
    {
        return new mob_twilight_zaelotAI (creature);
    }

    struct mob_twilight_zaelotAI : public ScriptedAI
    {
        mob_twilight_zaelotAI(Creature* creature) : ScriptedAI(creature)
        {
            type = urand(0, 3);
        }

        uint8 type;

        // Warlock
        uint32 ShadowBoltTimer;
        uint32 ShadowNovaTimer;
        // Mage
        uint32 ArcaneBarrageTimer;
        uint32 ArcaneHasteTimer;
        // Warrior
        uint32 RendTimer;
        uint32 MortalStrikeTimer;
        // Rogue
        uint32 KickTimer;
        uint32 GougeTimer;

        void Reset()
        {
            ShadowBoltTimer = urand(1000, 3000);
            ShadowNovaTimer = urand(5000, 10000);

            ArcaneBarrageTimer = urand(1000, 3000);
            ArcaneHasteTimer = urand(4000, 15000);

            RendTimer = urand(5000, 15000);
            MortalStrikeTimer = urand(6000, 12000);

            KickTimer = urand(3000, 13000);
            GougeTimer = urand(6000, 16000);
        }

        void UpdateAI(uint32 const Diff)
        {
            if (!UpdateVictim())
                return;

            if (me->IsNonMeleeSpellCasted(false))
                return;

            switch (type)
            {
                case TYPE_MAGE:
                {
                    if (ArcaneBarrageTimer <= Diff)
                    {
                        if (Unit * target = SelectTarget(SELECT_TARGET_RANDOM, 0, 40.0f, true))
                            me->CastSpell(target, SPELL_ARCANE_BARRAGE, false);
                        ArcaneBarrageTimer = urand(4000, 60000);
                    }
                    else ArcaneBarrageTimer -= Diff;

                    if (ArcaneHasteTimer <= Diff)
                    {
                        DoCast(SPELL_AURA_OF_ARCANE_HASTE);
                        ArcaneHasteTimer = urand(13000, 20000);
                    }
                    else ArcaneHasteTimer -= Diff;
                }
                break;

                case TYPE_WARLOCK:
                {
                    if (ShadowBoltTimer <= Diff)
                    {
                        if (Unit * target = SelectTarget(SELECT_TARGET_RANDOM, 0, 40.0f, true))
                            me->CastSpell(target, SPELL_SHADOW_BOLT, false);
                        ShadowBoltTimer = urand(3000, 6000);
                    }
                    else ShadowBoltTimer -= Diff;

                    if (ShadowNovaTimer <= Diff)
                    {
                        DoCast(SPELL_SHADOW_NOVA);
                        ShadowNovaTimer = urand(14000, 20000);
                    }
                    else ShadowNovaTimer -= Diff;
                }
                break;

                case TYPE_ROGUE:
                {
                    if (KickTimer <= Diff)
                    {
                        if (Unit * target = SelectTarget(SELECT_TARGET_RANDOM, 0, 40.0f, true))
                            me->CastSpell(target, SPELL_KICK, true);
                        KickTimer = urand(10000, 20000);
                    }
                    else KickTimer -= Diff;

                    if (GougeTimer <= Diff)
                    {
                        DoCastVictim(SPELL_GOUGE);
                        GougeTimer = urand(14000, 20000);
                    }
                    else GougeTimer -= Diff;
                }
                break;

                case TYPE_WARRIOR:
                {
                    if (RendTimer <= Diff)
                    {
                        DoCastVictim(SPELL_REND);
                        RendTimer = urand(10000, 20000);
                    }
                    else RendTimer -= Diff;

                    if (MortalStrikeTimer <= Diff)
                    {
                        DoCastVictim(SPELL_MORTAL_STRIKE);
                        MortalStrikeTimer = urand(8000, 18000);
                    }
                    else MortalStrikeTimer -= Diff;
                }
                break;
            }

            DoMeleeAttackIfReady();
        }
    };
};

class spell_fire_strike_target_check : public SpellScriptLoader
{
    public:
        spell_fire_strike_target_check() : SpellScriptLoader("spell_fire_strike_target_check") { }

        class spell_fire_strike_target_check_SpellScript : public SpellScript
        {
            PrepareSpellScript(spell_fire_strike_target_check_SpellScript);

            void CheckTarget()
            {
                if (Unit* target = GetHitUnit())
                    GetCaster()->CastSpell(target, 76324, true);
            }

            void FilterTargets(std::list<WorldObject*>& unitList)
            {
                if (!unitList.empty()) // Select only 1 from list
                {
                    std::list<WorldObject*>::iterator randitr = unitList.begin();
                    std::advance(randitr, urand(0, unitList.size()-1));
                    WorldObject* selected = (*randitr);
                    unitList.clear();
                    unitList.push_back(selected);
                }
            }

            void Register()
            {
                OnObjectAreaTargetSelect += SpellObjectAreaTargetSelectFn(spell_fire_strike_target_check_SpellScript::FilterTargets, EFFECT_0, TARGET_UNIT_DEST_AREA_ENEMY);
                OnHit += SpellHitFn(spell_fire_strike_target_check_SpellScript::CheckTarget);
            }
        };

        SpellScript* GetSpellScript() const
        {
            return new spell_fire_strike_target_check_SpellScript();
        }
};

class spell_shadow_of_obsidius_absorb : public SpellScriptLoader
{
    class script_impl : public AuraScript
    {
        PrepareAuraScript(script_impl);

        void CalculateAmount(AuraEffect const *, int32 &amount, bool &)
        {
            amount = -1;
        }

        void Absorb(AuraEffect *, DamageInfo &dmgInfo, uint32 &absorbAmount)
        {
            // Absorb all damage
            absorbAmount = dmgInfo.GetDamage();
        }

        void Register()
        {
            DoEffectCalcAmount += AuraEffectCalcAmountFn(script_impl::CalculateAmount, EFFECT_0, SPELL_AURA_SCHOOL_ABSORB);
            OnEffectAbsorb += AuraEffectAbsorbFn(script_impl::Absorb, EFFECT_0);
        }
    };

public:
    spell_shadow_of_obsidius_absorb()
        : SpellScriptLoader("shadow_of_obsidius_absorb")
    { }

    AuraScript * GetAuraScript() const
    {
        return new script_impl();
    }
};

#define SPELL_SHADOW_PRISON_DAMAGE 76687

class spell_shadow_prison : public SpellScriptLoader
{
    class script_impl : public AuraScript
    {
        PrepareAuraScript(script_impl);

        void HandleEffectPeriodic(AuraEffect const* aurEff)
        {
            if (!GetCaster())
                return;

            Unit* target = GetTarget();
            if (!target)
                return;

            if(target->isMoving())
            {
                int32 dmgBase = aurEff->GetAmount();
                target->CastCustomSpell(target, SPELL_SHADOW_PRISON_DAMAGE, &dmgBase, 0, 0, true, NULL, NULL, GetCaster()->GetGUID());
                const_cast<AuraEffect*>(aurEff)->SetAmount(dmgBase + aurEff->GetSpellInfo()->Effects[EFFECT_1].BasePoints);
            }
        }

        void Register()
        {
            OnEffectPeriodic += AuraEffectPeriodicFn(script_impl::HandleEffectPeriodic, EFFECT_0, SPELL_AURA_PERIODIC_DUMMY);
        }
    };

public:
    spell_shadow_prison()
        : SpellScriptLoader("spell_shadow_prison")
    { }

    AuraScript * GetAuraScript() const
    {
        return new script_impl();
    }
};

void AddSC_instance_blackrock_caverns()
{
    new instance_blackrock_caverns();
    // Trash and general instance handling
    new spell_fire_strike_target_check();
    new spell_shadow_of_obsidius_absorb();
    new spell_shadow_prison();
    new mob_twilight_zaelot();
    new npc_raz_the_crazed();
    new at_raz_jump_down();
}
