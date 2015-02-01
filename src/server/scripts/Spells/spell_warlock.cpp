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

/*
 * Scripts for spells with SPELLFAMILY_WARLOCK and SPELLFAMILY_GENERIC spells used by warlock players.
 * Ordered alphabetically using scriptname.
 * Scriptnames of files in this file should be prefixed with "spell_warl_".
 */

#include "Player.h"
#include "Pet.h"
#include "ScriptMgr.h"
#include "SpellScript.h"
#include "SpellAuraEffects.h"
#include "SpellAuras.h"

enum WarlockSpells
{
    SPELL_WARLOCK_BANE_OF_DOOM_EFFECT               = 18662,
    SPELL_WARLOCK_CURSE_OF_DOOM_EFFECT              = 18662,
    SPELL_WARLOCK_DEMONIC_CIRCLE_ALLOW_CAST         = 62388,
    SPELL_WARLOCK_DEMONIC_CIRCLE_SUMMON             = 48018,
    SPELL_WARLOCK_DEMONIC_CIRCLE_TELEPORT           = 48020,
    SPELL_WARLOCK_DEMONIC_EMPOWERMENT_FELGUARD      = 54508,
    SPELL_WARLOCK_DEMONIC_EMPOWERMENT_FELHUNTER     = 54509,
    SPELL_WARLOCK_DEMONIC_EMPOWERMENT_IMP           = 54444,
    SPELL_WARLOCK_DEMONIC_EMPOWERMENT_SUCCUBUS      = 54435,
    SPELL_WARLOCK_DEMONIC_EMPOWERMENT_VOIDWALKER    = 54443,
    SPELL_WARLOCK_DEMON_SOUL_IMP                    = 79459,
    SPELL_WARLOCK_DEMON_SOUL_FELHUNTER              = 79460,
    SPELL_WARLOCK_DEMON_SOUL_FELGUARD               = 79462,
    SPELL_WARLOCK_DEMON_SOUL_SUCCUBUS               = 79463,
    SPELL_WARLOCK_DEMON_SOUL_VOIDWALKER             = 79464,
    SPELL_WARLOCK_FEL_SYNERGY_HEAL                  = 54181,
    SPELL_WARLOCK_HAUNT                             = 48181,
    SPELL_WARLOCK_HAUNT_HEAL                        = 48210,
    SPELL_WARLOCK_IMMOLATE                          = 348,
    SPELL_WARLOCK_CREATE_HEALTHSTONE                = 34130,
    SPELL_WARLOCK_IMPROVED_HEALTH_FUNNEL_BUFF_R1    = 60955,
    SPELL_WARLOCK_IMPROVED_HEALTH_FUNNEL_BUFF_R2    = 60956,
    SPELL_WARLOCK_IMPROVED_HEALTH_FUNNEL_R1         = 18703,
    SPELL_WARLOCK_IMPROVED_HEALTH_FUNNEL_R2         = 18704,
    SPELL_WARLOCK_IMPROVED_SOULFIRE                 = 85383,
    SPELL_WARLOCK_LIFE_TAP_ENERGIZE                 = 31818,
    SPELL_WARLOCK_LIFE_TAP_ENERGIZE_2               = 32553,
    SPELL_WARLOCK_SOULSHATTER                       = 32835,
    SPELL_WARLOCK_UNSTABLE_AFFLICTION               = 30108,
    SPELL_WARLOCK_UNSTABLE_AFFLICTION_DISPEL        = 31117,
    SPELL_WARLOCK_JINX_ENERGY                       = 85540,
    SPELL_WARLOCK_JINX_RAGE                         = 85539,
    SPELL_WARLOCK_JINX_RUNIC_POWER                  = 85541,
    SPELL_WARLOCK_JINX_FOCUS                        = 85542,
    SPELL_WARLOCK_JINX_R1                           = 18179,
    SPELL_WARLOCK_JINX_R2                           = 85479,
    SPELL_WARLOCK_JINX_CoE_R1                       = 85547,
    SPELL_WARLOCK_JINX_CoE_R2                       = 86105,
    SPELL_WARLOCK_CoE                               = 1490,
    SPELL_WARLOCK_DRAIN_LIFE_HEALTH                 = 89653,
    SPELL_WARLOCK_SOUL_SWAP_OVERRIDE                = 86211,
    SPELL_WARLOCK_SOUL_SWAP_EXHALE                  = 86213,
    SPELL_WARLOCK_SOUL_SWAP_GLYPH                   = 56226,
    SPELL_WARLOCK_SOUL_SWAP_COOLDOWN                = 94229,
    SPELL_WARLOCK_SOUL_SWAP_VISUAL                  = 92795,
    SPELL_WARLOCK_SOUL_SWAP_FREE_DOTS_COSTS         = 92794,
    SPELL_WARLOCK_SOULBURN                          = 74434,
    SPELL_WARLOCK_SOULBURN_SEED_OF_CORRUPTION       = 86664,
    SPELL_WARLOCK_SOULBURN_SOC_VISUAL               = 93313,
    SPELL_WARLOCK_SOULSHARD_ADD                     = 87388,
    SPELL_WARLOCK_PANDEMIC                          = 92931,
    SPELL_WARLOCK_SOULBURN_SEARING_PAIN             = 79440,
    SPELL_WARLOCK_SOULBURN_DEMONIC_CIRCLE           = 79438,
    SPELL_WARLOCK_SOUL_HARVEST_SOULSHARD            = 101977,
    SPELL_WARLOCK_FEL_ARMOR_HEAL                    = 96379,
    SPELL_WARLOCK_SHADOW_WARD                       = 6229,
    SPELL_WARLOCK_NETHER_WARD                       = 91711,
    SPELL_WARLOCK_METHAMORPHOSIS_FORM               = 47241,
    SPELL_WARLOCK_NETHER_WARD_TALENT                = 91713,
    SPELL_WARLOCK_BANE_OF_HAVOC_CASTER_AURA         = 85466,
    SPELL_WARLOCK_BANE_OF_HAVOC_TRIGGERED           = 85455,
};

enum WarlockSpellIcons
{
    WARLOCK_ICON_ID_IMPROVED_LIFE_TAP               = 208,
    WARLOCK_ICON_ID_MANA_FEED                       = 1982,
    WARLOCK_ICON_SOULFIRE                           = 184
};

enum WarlockPetCalculate
{
    SPELL_PET_PASSIVE_CRIT             = 35695,
    SPELL_PET_PASSIVE_DAMAGE_TAKEN     = 35697,
    SPELL_WARLOCK_PET_SCALING_01       = 34947,
    SPELL_WARLOCK_PET_SCALING_02       = 34956,
    SPELL_WARLOCK_PET_SCALING_03       = 34957,
    SPELL_WARLOCK_PET_SCALING_04       = 34958,
    SPELL_WARLOCK_PET_SCALING_05       = 61013,
    ENTRY_FELGUARD                     = 17252,
    ENTRY_VOIDWALKER                   = 1860,
    ENTRY_FELHUNTER                    = 417,
    ENTRY_SUCCUBUS                     = 1863,
    ENTRY_IMP                          = 416,
    SPELL_WARLOCK_GLYPH_OF_VOIDWALKER  = 56247,
};

// 710 - Banish
/// Updated 4.3.4
class spell_warl_banish : public SpellScriptLoader
{
    public:
        spell_warl_banish() : SpellScriptLoader("spell_warl_banish") { }

        class spell_warl_banish_SpellScript : public SpellScript
        {
            PrepareSpellScript(spell_warl_banish_SpellScript);

            bool Load()
            {
                _removed = false;
                return true;
            }

            void HandleBanish()
            {
                /// Casting Banish on a banished target will cancel the effect
                /// Check if the target already has Banish, if so, do nothing.
                if (Unit* target = GetHitUnit())
                {
                    if (target->GetAuraEffect(SPELL_AURA_SCHOOL_IMMUNITY, SPELLFAMILY_WARLOCK, 0, 0x08000000, 0))
                    {
                        // No need to remove old aura since its removed due to not stack by current Banish aura
                        PreventHitDefaultEffect(EFFECT_0);
                        PreventHitDefaultEffect(EFFECT_1);
                        PreventHitDefaultEffect(EFFECT_2);
                        _removed = true;
                    }
                }
            }

            void RemoveAura()
            {
                if (_removed)
                    PreventHitAura();
            }

            void Register()
            {
                BeforeHit += SpellHitFn(spell_warl_banish_SpellScript::HandleBanish);
                AfterHit += SpellHitFn(spell_warl_banish_SpellScript::RemoveAura);
            }

            bool _removed;
        };

        SpellScript* GetSpellScript() const
        {
            return new spell_warl_banish_SpellScript();
        }
};

// Soul Stone
class spell_warl_soul_stone : public SpellScriptLoader
{
    public:
        spell_warl_soul_stone() : SpellScriptLoader("spell_warl_soul_stone") { }

        class spell_warl_soul_stone_SpellScript : public SpellScript
        {
            PrepareSpellScript(spell_warl_soul_stone_SpellScript);

            void HandleAura(SpellEffIndex effIndex)
            {
                if (!GetHitUnit()->isAlive())
                    PreventHitAura();
            }

            void Register()
            {
                OnEffectHitTarget += SpellEffectFn(spell_warl_soul_stone_SpellScript::HandleAura, EFFECT_0, SPELL_EFFECT_APPLY_AURA);
            }
        };

        SpellScript* GetSpellScript() const
        {
            return new spell_warl_soul_stone_SpellScript();
        }
};

class spell_warl_curse_of_weakness : public SpellScriptLoader
{
    public:
        spell_warl_curse_of_weakness() : SpellScriptLoader("spell_warl_curse_of_weakness") { }

        class spell_warl_curse_of_weakness_SpellScript : public SpellScript
        {
            PrepareSpellScript(spell_warl_curse_of_weakness_SpellScript);

            bool Validate(SpellInfo const* /*spellInfo*/)
            {
                if (!sSpellMgr->GetSpellInfo(SPELL_WARLOCK_JINX_ENERGY))
                    return false;
                if (!sSpellMgr->GetSpellInfo(SPELL_WARLOCK_JINX_RAGE))
                    return false;
                if (!sSpellMgr->GetSpellInfo(SPELL_WARLOCK_JINX_RUNIC_POWER))
                    return false;
                if (!sSpellMgr->GetSpellInfo(SPELL_WARLOCK_JINX_FOCUS))
                    return false;
                if (!sSpellMgr->GetSpellInfo(SPELL_WARLOCK_JINX_R1))
                    return false;
                if (!sSpellMgr->GetSpellInfo(SPELL_WARLOCK_JINX_R2))
                    return false;

                return true;
            }

            void HandleJinx()
            {
                Unit* caster = GetCaster();
                Unit* target = GetHitUnit();
                if (!caster || ! target)
                    return;

                uint32 trigerred_spell = 0;
                switch (target->getPowerType())
                {
                    case POWER_RAGE:
                        trigerred_spell = SPELL_WARLOCK_JINX_RAGE;
                        break;
                    case POWER_FOCUS:
                        trigerred_spell = SPELL_WARLOCK_JINX_FOCUS;
                        break;
                    case POWER_ENERGY:
                        trigerred_spell = SPELL_WARLOCK_JINX_ENERGY;
                        break;
                    case POWER_RUNIC_POWER:
                        trigerred_spell = SPELL_WARLOCK_JINX_RUNIC_POWER;
                        break;
                    default:
                        return;
                }

                if (AuraEffect const* aurEff = caster->GetDummyAuraEffect(SPELLFAMILY_WARLOCK, 5002, EFFECT_1))
                {
                    int32 basepoints0 = aurEff->GetAmount();
                    caster->CastCustomSpell(target, trigerred_spell, &basepoints0, NULL, NULL, true);
                }
            }

            void Register()
            {
                OnHit += SpellHitFn(spell_warl_curse_of_weakness_SpellScript::HandleJinx);
            }
        };

        SpellScript* GetSpellScript() const
        {
            return new spell_warl_curse_of_weakness_SpellScript();
        }
};

// Rain of fire
class spell_warl_rain_of_fire : public SpellScriptLoader
{
    public:
        spell_warl_rain_of_fire() : SpellScriptLoader("spell_warl_rain_of_fire") { }

        class spell_warl_rain_of_fire_SpellScript : public SpellScript
        {
            PrepareSpellScript(spell_warl_rain_of_fire_SpellScript);

            void HandleHit(SpellEffIndex /*effIndex*/)
            {
                Unit* caster = GetCaster();
                if (AuraEffect const* aurEff = caster->GetAuraEffect(SPELL_AURA_PROC_TRIGGER_SPELL, SPELLFAMILY_WARLOCK, 11, EFFECT_0))
                    if (roll_chance_i(aurEff->GetSpellInfo()->Effects[EFFECT_1].BasePoints))
                        caster->CastSpell(GetHitUnit(), 85387, true);
            }

            void Register()
            {
                OnEffectHitTarget += SpellEffectFn(spell_warl_rain_of_fire_SpellScript::HandleHit, EFFECT_0, SPELL_EFFECT_SCHOOL_DAMAGE);
            }
        };

        SpellScript* GetSpellScript() const
        {
            return new spell_warl_rain_of_fire_SpellScript();
        }
};

// Seduce
class spell_warl_seduction : public SpellScriptLoader
{
    public:
        spell_warl_seduction() : SpellScriptLoader("spell_warl_seduction") { }

        class spell_warl_seduction_SpellScript : public SpellScript
        {
            PrepareSpellScript(spell_warl_seduction_SpellScript);

            void HandleOnHit()
            {
                Unit* caster = GetCaster()->GetOwner();
                if (!caster)
                    return;
                Unit* target = GetHitUnit();
                // Glyph removes dots on hit
                if (AuraEffect* glyph = caster->GetDummyAuraEffect(SPELLFAMILY_WARLOCK, 48, EFFECT_0))
                {
                    if (!target)
                        return;

                    target->RemoveAurasByType(SPELL_AURA_PERIODIC_DAMAGE, 0, target->GetAura(32409)); // SW:D shall not be removed.
                    target->RemoveAurasByType(SPELL_AURA_PERIODIC_DAMAGE_PERCENT);
                    target->RemoveAurasByType(SPELL_AURA_PERIODIC_LEECH);
                }
            }

            void Register()
            {
                OnHit += SpellHitFn(spell_warl_seduction_SpellScript::HandleOnHit);
            }
        };

        SpellScript* GetSpellScript() const
        {
            return new spell_warl_seduction_SpellScript();
        }
};

// 6201 - Create Healthstone
class spell_warl_create_healthstone : public SpellScriptLoader
{
    public:
        spell_warl_create_healthstone() : SpellScriptLoader("spell_warl_create_healthstone") { }

        class spell_warl_create_healthstone_SpellScript : public SpellScript
        {
            PrepareSpellScript(spell_warl_create_healthstone_SpellScript);

            bool Validate(SpellInfo const* /*spellInfo*/)
            {
                if (!sSpellMgr->GetSpellInfo(SPELL_WARLOCK_CREATE_HEALTHSTONE))
                    return false;

                return true;
            }

            SpellCastResult CheckCast()
            {
                if (Player* caster = GetCaster()->ToPlayer())
                {
                    uint32 healthstoneId = sSpellMgr->GetSpellInfo(SPELL_WARLOCK_CREATE_HEALTHSTONE)->Effects[0].ItemType;
                    ItemPosCountVec dest;
                    InventoryResult msg = caster->CanStoreNewItem(NULL_BAG, NULL_SLOT, dest, healthstoneId, 1, NULL);
                    if (msg != EQUIP_ERR_OK)
                        return SPELL_FAILED_TOO_MANY_OF_ITEM;
                }
                return SPELL_CAST_OK;
            }

            void HandleScriptEffect(SpellEffIndex effIndex)
            {
                if (Unit* caster = GetCaster())
                    caster->CastSpell(caster, SPELL_WARLOCK_CREATE_HEALTHSTONE, true);
            }

            void Register()
            {
                OnEffectHitTarget += SpellEffectFn(spell_warl_create_healthstone_SpellScript::HandleScriptEffect, EFFECT_0, SPELL_EFFECT_SCRIPT_EFFECT);
                OnCheckCast += SpellCheckCastFn(spell_warl_create_healthstone_SpellScript::CheckCast);
            }
        };

        SpellScript* GetSpellScript() const
        {
            return new spell_warl_create_healthstone_SpellScript();
        }
};

// 603 - Bane of Doom
/// Updated 4.3.4
class spell_warl_bane_of_doom : public SpellScriptLoader
{
    public:
        spell_warl_bane_of_doom() : SpellScriptLoader("spell_warl_bane_of_doom") { }

        class spell_warl_curse_of_doom_AuraScript : public AuraScript
        {
            PrepareAuraScript(spell_warl_curse_of_doom_AuraScript);

            bool Validate(SpellInfo const* /*spellInfo*/)
            {
                if (!sSpellMgr->GetSpellInfo(SPELL_WARLOCK_BANE_OF_DOOM_EFFECT))
                    return false;
                return true;
            }

            bool Load()
            {
                return GetCaster() && GetCaster()->GetTypeId() == TYPEID_PLAYER;
            }

            void OnRemove(AuraEffect const* aurEff, AuraEffectHandleModes /*mode*/)
            {
                if (!GetCaster())
                    return;

                AuraRemoveMode removeMode = GetTargetApplication()->GetRemoveMode();
                if (removeMode != AURA_REMOVE_BY_DEATH || !IsExpired())
                    return;

                if (GetCaster()->ToPlayer()->isHonorOrXPTarget(GetTarget()))
                    GetCaster()->CastSpell(GetTarget(), SPELL_WARLOCK_BANE_OF_DOOM_EFFECT, true, NULL, aurEff);
            }

            void Register()
            {
                 AfterEffectRemove += AuraEffectRemoveFn(spell_warl_curse_of_doom_AuraScript::OnRemove, EFFECT_0, SPELL_AURA_PERIODIC_DAMAGE, AURA_EFFECT_HANDLE_REAL);
            }
        };

        AuraScript* GetAuraScript() const
        {
            return new spell_warl_curse_of_doom_AuraScript();
        }
};

// 48018 - Demonic Circle: Summon
/// Updated 4.3.4
class spell_warl_demonic_circle_summon : public SpellScriptLoader
{
    public:
        spell_warl_demonic_circle_summon() : SpellScriptLoader("spell_warl_demonic_circle_summon") { }

        class spell_warl_demonic_circle_summon_AuraScript : public AuraScript
        {
            PrepareAuraScript(spell_warl_demonic_circle_summon_AuraScript);

            void HandleRemove(AuraEffect const* /*aurEff*/, AuraEffectHandleModes mode)
            {
                // If effect is removed by expire remove the summoned demonic circle too.
                if (!(mode & AURA_EFFECT_HANDLE_REAPPLY))
                    GetTarget()->RemoveGameObject(GetId(), true);

                GetTarget()->RemoveAura(SPELL_WARLOCK_DEMONIC_CIRCLE_ALLOW_CAST);
            }

            void HandleDummyTick(AuraEffect const* /*aurEff*/)
            {
                if (GameObject* circle = GetTarget()->GetGameObject(GetId()))
                {
                    // Here we check if player is in demonic circle teleport range, if so add
                    // WARLOCK_DEMONIC_CIRCLE_ALLOW_CAST; allowing him to cast the WARLOCK_DEMONIC_CIRCLE_TELEPORT.
                    // If not in range remove the WARLOCK_DEMONIC_CIRCLE_ALLOW_CAST.

                    SpellInfo const* spellInfo = sSpellMgr->GetSpellInfo(SPELL_WARLOCK_DEMONIC_CIRCLE_TELEPORT);

                    if (GetTarget()->IsWithinDist(circle, spellInfo->GetMaxRange(true)))
                    {
                        if (!GetTarget()->HasAura(SPELL_WARLOCK_DEMONIC_CIRCLE_ALLOW_CAST))
                            GetTarget()->CastSpell(GetTarget(), SPELL_WARLOCK_DEMONIC_CIRCLE_ALLOW_CAST, true);
                    }
                    else
                        GetTarget()->RemoveAura(SPELL_WARLOCK_DEMONIC_CIRCLE_ALLOW_CAST);
                }
            }

            void Register()
            {
                OnEffectRemove += AuraEffectApplyFn(spell_warl_demonic_circle_summon_AuraScript::HandleRemove, EFFECT_0, SPELL_AURA_PERIODIC_DUMMY, AURA_EFFECT_HANDLE_REAL_OR_REAPPLY_MASK);
                OnEffectPeriodic += AuraEffectPeriodicFn(spell_warl_demonic_circle_summon_AuraScript::HandleDummyTick, EFFECT_0, SPELL_AURA_PERIODIC_DUMMY);
            }
        };

        AuraScript* GetAuraScript() const
        {
            return new spell_warl_demonic_circle_summon_AuraScript();
        }
};

// 48020 - Demonic Circle: Teleport
/// Updated 4.3.4
class spell_warl_demonic_circle_teleport : public SpellScriptLoader
{
    public:
        spell_warl_demonic_circle_teleport() : SpellScriptLoader("spell_warl_demonic_circle_teleport") { }

        class spell_warl_demonic_circle_teleport_AuraScript : public AuraScript
        {
            PrepareAuraScript(spell_warl_demonic_circle_teleport_AuraScript);

            void HandleTeleport(AuraEffect const* aurEff, AuraEffectHandleModes /*mode*/)
            {
                if (Player* player = GetTarget()->ToPlayer())
                {
                    if (GameObject* circle = player->GetGameObject(SPELL_WARLOCK_DEMONIC_CIRCLE_SUMMON))
                    {
                        player->NearTeleportTo(circle->GetPositionX(), circle->GetPositionY(), circle->GetPositionZ(), circle->GetOrientation());
                        player->RemoveMovementImpairingAuras();
                        if (aurEff->GetBase()->IsChangeBySoulBurn())
                            player->CastSpell(player, SPELL_WARLOCK_SOULBURN_DEMONIC_CIRCLE, true);
                    }
                }
            }

            void Register()
            {
                OnEffectApply += AuraEffectApplyFn(spell_warl_demonic_circle_teleport_AuraScript::HandleTeleport, EFFECT_0, SPELL_AURA_MECHANIC_IMMUNITY, AURA_EFFECT_HANDLE_REAL);
            }
        };

        class spell_warl_demonic_circle_teleport_SpellScript : public SpellScript
        {
            PrepareSpellScript(spell_warl_demonic_circle_teleport_SpellScript);

            void HandleOnCast()
            {
                if (Unit* caster = GetCaster())
                    if (caster->HasAura(SPELL_WARLOCK_SOULBURN))
                    {
                        GetSpell()->SetChangeBySoulBurn(true);
                        caster->RemoveAurasDueToSpell(SPELL_WARLOCK_SOULBURN);
                    }
            }

            void Register()
            {
                OnCast += SpellCastFn(spell_warl_demonic_circle_teleport_SpellScript::HandleOnCast);
            }
        };

        SpellScript* GetSpellScript() const
        {
            return new spell_warl_demonic_circle_teleport_SpellScript();
        }

        AuraScript* GetAuraScript() const
        {
            return new spell_warl_demonic_circle_teleport_AuraScript();
        }
};

class spell_warl_bane_of_havoc : public SpellScriptLoader
{
public:
    spell_warl_bane_of_havoc() : SpellScriptLoader("spell_warl_bane_of_havoc") { }
    
    class spell_warl_bane_of_havoc_AuraScript : public AuraScript
    {
        PrepareAuraScript(spell_warl_bane_of_havoc_AuraScript);
        
        void HandleApply(AuraEffect const* aurEff, AuraEffectHandleModes /*mode*/)
        {
            if (Unit* caster = GetCaster())
            {
                caster->CastSpell(caster, SPELL_WARLOCK_BANE_OF_HAVOC_CASTER_AURA, true);
            }
        }
        
        void Register()
        {
            OnEffectApply += AuraEffectApplyFn(spell_warl_bane_of_havoc_AuraScript::HandleApply, EFFECT_0, SPELL_AURA_DUMMY, AURA_EFFECT_HANDLE_REAL);
        }
    };
    
    AuraScript* GetAuraScript() const
    {
        return new spell_warl_bane_of_havoc_AuraScript();
    }
};

// 77801 - Demon Soul - Updated to 4.3.4
class spell_warl_demon_soul : public SpellScriptLoader
{
    public:
        spell_warl_demon_soul() : SpellScriptLoader("spell_warl_demon_soul") { }

        class spell_warl_demon_soul_SpellScript : public SpellScript
        {
            PrepareSpellScript(spell_warl_demon_soul_SpellScript);

            bool Validate(SpellInfo const* /*spellInfo*/)
            {
                if (!sSpellMgr->GetSpellInfo(SPELL_WARLOCK_DEMON_SOUL_IMP))
                    return false;
                if (!sSpellMgr->GetSpellInfo(SPELL_WARLOCK_DEMON_SOUL_FELHUNTER))
                    return false;
                if (!sSpellMgr->GetSpellInfo(SPELL_WARLOCK_DEMON_SOUL_FELGUARD))
                    return false;
                if (!sSpellMgr->GetSpellInfo(SPELL_WARLOCK_DEMON_SOUL_SUCCUBUS))
                    return false;
                if (!sSpellMgr->GetSpellInfo(SPELL_WARLOCK_DEMON_SOUL_VOIDWALKER))
                    return false;
                return true;
            }

            void OnHitTarget(SpellEffIndex /*effIndex*/)
            {
                Unit* caster = GetCaster();
                if (Creature* targetCreature = GetHitCreature())
                {
                    if (targetCreature->isPet())
                    {
                        CreatureTemplate const* ci = sObjectMgr->GetCreatureTemplate(targetCreature->GetEntry());
                        switch (ci->family)
                        {
                            case CREATURE_FAMILY_SUCCUBUS:
                                caster->CastSpell(caster, SPELL_WARLOCK_DEMON_SOUL_SUCCUBUS);
                                break;
                            case CREATURE_FAMILY_VOIDWALKER:
                                caster->CastSpell(caster, SPELL_WARLOCK_DEMON_SOUL_VOIDWALKER);
                                break;
                            case CREATURE_FAMILY_FELGUARD:
                                caster->CastSpell(caster, SPELL_WARLOCK_DEMON_SOUL_FELGUARD);
                                break;
                            case CREATURE_FAMILY_FELHUNTER:
                                caster->CastSpell(caster, SPELL_WARLOCK_DEMON_SOUL_FELHUNTER);
                                break;
                            case CREATURE_FAMILY_IMP:
                                caster->CastSpell(caster, SPELL_WARLOCK_DEMON_SOUL_IMP);
                                break;
                        }
                    }
                }
            }

            void Register()
            {
                OnEffectHitTarget += SpellEffectFn(spell_warl_demon_soul_SpellScript::OnHitTarget, EFFECT_0, SPELL_EFFECT_SCRIPT_EFFECT);
            }
        };

        SpellScript* GetSpellScript() const
        {
            return new spell_warl_demon_soul_SpellScript;
        }
};

// 47193 - Demonic Empowerment
/// Updated 4.3.4
class spell_warl_demonic_empowerment : public SpellScriptLoader
{
    public:
        spell_warl_demonic_empowerment() : SpellScriptLoader("spell_warl_demonic_empowerment") { }

        class spell_warl_demonic_empowerment_SpellScript : public SpellScript
        {
            PrepareSpellScript(spell_warl_demonic_empowerment_SpellScript);

            bool Validate(SpellInfo const* /*spellInfo*/)
            {
                if (!sSpellMgr->GetSpellInfo(SPELL_WARLOCK_DEMONIC_EMPOWERMENT_SUCCUBUS) || !sSpellMgr->GetSpellInfo(SPELL_WARLOCK_DEMONIC_EMPOWERMENT_VOIDWALKER) || !sSpellMgr->GetSpellInfo(SPELL_WARLOCK_DEMONIC_EMPOWERMENT_FELGUARD) || !sSpellMgr->GetSpellInfo(SPELL_WARLOCK_DEMONIC_EMPOWERMENT_FELHUNTER) || !sSpellMgr->GetSpellInfo(SPELL_WARLOCK_DEMONIC_EMPOWERMENT_IMP))
                    return false;
                return true;
            }

            void HandleScriptEffect(SpellEffIndex /*effIndex*/)
            {
                if (Creature* targetCreature = GetHitCreature())
                {
                    if (targetCreature->isPet())
                    {
                        CreatureTemplate const* ci = sObjectMgr->GetCreatureTemplate(targetCreature->GetEntry());
                        switch (ci->family)
                        {
                            case CREATURE_FAMILY_SUCCUBUS:
                                targetCreature->CastSpell(targetCreature, SPELL_WARLOCK_DEMONIC_EMPOWERMENT_SUCCUBUS, true);
                                break;
                            case CREATURE_FAMILY_VOIDWALKER:
                            {
                                SpellInfo const* spellInfo = sSpellMgr->GetSpellInfo(SPELL_WARLOCK_DEMONIC_EMPOWERMENT_VOIDWALKER);
                                int32 hp = int32(targetCreature->CountPctFromMaxHealth(GetCaster()->CalculateSpellDamage(targetCreature, spellInfo, 0)));
                                targetCreature->CastCustomSpell(targetCreature, SPELL_WARLOCK_DEMONIC_EMPOWERMENT_VOIDWALKER, &hp, NULL, NULL, true);
                                break;
                            }
                            case CREATURE_FAMILY_FELGUARD:
                                targetCreature->CastSpell(targetCreature, SPELL_WARLOCK_DEMONIC_EMPOWERMENT_FELGUARD, true);
                                break;
                            case CREATURE_FAMILY_FELHUNTER:
                                targetCreature->CastSpell(targetCreature, SPELL_WARLOCK_DEMONIC_EMPOWERMENT_FELHUNTER, true);
                                break;
                            case CREATURE_FAMILY_IMP:
                                targetCreature->CastSpell(targetCreature, SPELL_WARLOCK_DEMONIC_EMPOWERMENT_IMP, true);
                                break;
                        }
                    }
                }
            }

            void Register()
            {
                OnEffectHitTarget += SpellEffectFn(spell_warl_demonic_empowerment_SpellScript::HandleScriptEffect, EFFECT_0, SPELL_EFFECT_SCRIPT_EFFECT);
            }
        };

        SpellScript* GetSpellScript() const
        {
            return new spell_warl_demonic_empowerment_SpellScript();
        }
};

// 47422 - Everlasting Affliction
/// Updated 4.3.4
class spell_warl_everlasting_affliction : public SpellScriptLoader
{
    public:
        spell_warl_everlasting_affliction() : SpellScriptLoader("spell_warl_everlasting_affliction") { }

        class spell_warl_everlasting_affliction_SpellScript : public SpellScript
        {
            PrepareSpellScript(spell_warl_everlasting_affliction_SpellScript);

            void HandleScriptEffect(SpellEffIndex /*effIndex*/)
            {
                if (Unit* unitTarget = GetHitUnit())
                    // Refresh corruption on target
                    if (AuraEffect* aur = unitTarget->GetAuraEffect(SPELL_AURA_PERIODIC_DAMAGE, SPELLFAMILY_WARLOCK, 0x2, 0, 0, GetCaster()->GetGUID()))
                        aur->GetBase()->RefreshDuration();
            }

            void Register()
            {
                OnEffectHitTarget += SpellEffectFn(spell_warl_everlasting_affliction_SpellScript::HandleScriptEffect, EFFECT_0, SPELL_EFFECT_SCRIPT_EFFECT);
            }
        };

        SpellScript* GetSpellScript() const
        {
            return new spell_warl_everlasting_affliction_SpellScript();
        }
};

// 6353 Soulfire
// Updated 4.3.4
class spell_warl_soulfire : public SpellScriptLoader
{
    public:
        spell_warl_soulfire() : SpellScriptLoader("spell_warl_soulfire") { }

        class spell_warl_soulfire_SpellScript : public SpellScript
        {
            PrepareSpellScript(spell_warl_soulfire_SpellScript);

            bool Validate(SpellInfo const* /*spellInfo*/)
            {
                if (!sSpellMgr->GetSpellInfo(SPELL_WARLOCK_IMPROVED_SOULFIRE))
                    return false;
                return true;
            }

            void HandleDummy(SpellEffIndex /*effIndex*/)
            {
                if (Unit* caster = GetCaster())
                    if (AuraEffect* aurEff = caster->GetAuraEffect(SPELL_AURA_DUMMY, SPELLFAMILY_WARLOCK, WARLOCK_ICON_SOULFIRE, EFFECT_0))
                    {
                        int32 basepoints = aurEff->GetAmount();
                        caster->CastCustomSpell(caster, SPELL_WARLOCK_IMPROVED_SOULFIRE, &basepoints, NULL, NULL, true);
                    }
            }

            void Register()
            {
                OnEffectHitTarget += SpellEffectFn(spell_warl_soulfire_SpellScript::HandleDummy, EFFECT_0, SPELL_EFFECT_SCHOOL_DAMAGE);
            }
        };

        SpellScript* GetSpellScript() const
        {
            return new spell_warl_soulfire_SpellScript();
        }
};

// 77799 - Fel Flame - Updated to 4.3.4
class spell_warl_fel_flame : public SpellScriptLoader
{
    public:
        spell_warl_fel_flame() : SpellScriptLoader("spell_warl_fel_flame") { }

        class spell_warl_fel_flame_SpellScript : public SpellScript
        {
            PrepareSpellScript(spell_warl_fel_flame_SpellScript);

            void OnHitTarget(SpellEffIndex /*effIndex*/)
            {
                Unit* caster = GetCaster();
                Unit* target = GetHitUnit();
                Aura* aura = target->GetAura(SPELL_WARLOCK_UNSTABLE_AFFLICTION, caster->GetGUID());
                if (!aura)
                    aura = target->GetAura(SPELL_WARLOCK_IMMOLATE, caster->GetGUID());

                if (!aura)
                    return;

                int32 newDuration = aura->GetDuration() + GetSpellInfo()->Effects[EFFECT_1].CalcValue() * 1000;
                aura->SetDuration(std::min(newDuration, aura->GetMaxDuration()), false, true);
            }

            void Register()
            {
                OnEffectHitTarget += SpellEffectFn(spell_warl_fel_flame_SpellScript::OnHitTarget, EFFECT_1, SPELL_EFFECT_SCRIPT_EFFECT);
            }
        };

        SpellScript* GetSpellScript() const
        {
            return new spell_warl_fel_flame_SpellScript;
        }
};

// -47230 - Fel Synergy
class spell_warl_fel_synergy : public SpellScriptLoader
{
    public:
        spell_warl_fel_synergy() : SpellScriptLoader("spell_warl_fel_synergy") { }

        class spell_warl_fel_synergy_AuraScript : public AuraScript
        {
            PrepareAuraScript(spell_warl_fel_synergy_AuraScript);

            bool Validate(SpellInfo const* /*spellInfo*/)
            {
                if (!sSpellMgr->GetSpellInfo(SPELL_WARLOCK_FEL_SYNERGY_HEAL))
                    return false;
                return true;
            }

            bool CheckProc(ProcEventInfo& eventInfo)
            {
                return GetTarget()->GetGuardianPet() && eventInfo.GetDamageInfo()->GetDamage();
            }

            void OnProc(AuraEffect const* aurEff, ProcEventInfo& eventInfo)
            {
                PreventDefaultAction();

                int32 heal = CalculatePct(int32(eventInfo.GetDamageInfo()->GetDamage()), aurEff->GetAmount());
                GetTarget()->CastCustomSpell(SPELL_WARLOCK_FEL_SYNERGY_HEAL, SPELLVALUE_BASE_POINT0, heal, (Unit*)NULL, true, NULL, aurEff); // TARGET_UNIT_PET
            }

            void Register()
            {
                DoCheckProc += AuraCheckProcFn(spell_warl_fel_synergy_AuraScript::CheckProc);
                OnEffectProc += AuraEffectProcFn(spell_warl_fel_synergy_AuraScript::OnProc, EFFECT_0, SPELL_AURA_DUMMY);
            }
        };

        AuraScript* GetAuraScript() const
        {
            return new spell_warl_fel_synergy_AuraScript();
        }
};

// 48181 - Haunt
/// Updated 4.3.4
class spell_warl_haunt : public SpellScriptLoader
{
    public:
        spell_warl_haunt() : SpellScriptLoader("spell_warl_haunt") { }

        class spell_warl_haunt_SpellScript : public SpellScript
        {
            PrepareSpellScript(spell_warl_haunt_SpellScript);

            void HandleOnHit()
            {
                if (Aura* aura = GetHitAura())
                    if (AuraEffect* aurEff = aura->GetEffect(EFFECT_1))
                        aurEff->SetAmount(CalculatePct(aurEff->GetAmount(), GetHitDamage()));
            }

            void Register()
            {
                OnHit += SpellHitFn(spell_warl_haunt_SpellScript::HandleOnHit);
            }
        };

        class spell_warl_haunt_AuraScript : public AuraScript
        {
            PrepareAuraScript(spell_warl_haunt_AuraScript);

            bool Validate(SpellInfo const* /*spellInfo*/)
            {
                if (!sSpellMgr->GetSpellInfo(SPELL_WARLOCK_HAUNT_HEAL))
                    return false;
                return true;
            }

            void HandleRemove(AuraEffect const* aurEff, AuraEffectHandleModes /*mode*/)
            {
                if (Unit* caster = GetCaster())
                {
                    int32 amount = aurEff->GetAmount();
                    GetTarget()->CastCustomSpell(caster, SPELL_WARLOCK_HAUNT_HEAL, &amount, NULL, NULL, true, NULL, aurEff, GetCasterGUID());
                }
            }

            void Register()
            {
                OnEffectRemove += AuraEffectApplyFn(spell_warl_haunt_AuraScript::HandleRemove, EFFECT_1, SPELL_AURA_DUMMY, AURA_EFFECT_HANDLE_REAL_OR_REAPPLY_MASK);
            }
        };

        SpellScript* GetSpellScript() const
        {
            return new spell_warl_haunt_SpellScript();
        }

        AuraScript* GetAuraScript() const
        {
            return new spell_warl_haunt_AuraScript();
        }
};

// 755 - Health Funnel
/// Updated 4.3.4
class spell_warl_health_funnel : public SpellScriptLoader
{
    public:
        spell_warl_health_funnel() : SpellScriptLoader("spell_warl_health_funnel") { }

        class spell_warl_health_funnel_AuraScript : public AuraScript
        {
            PrepareAuraScript(spell_warl_health_funnel_AuraScript);

            void ApplyEffect(AuraEffect const* /*aurEff*/, AuraEffectHandleModes /*mode*/)
            {
                Unit* caster = GetCaster();
                if (!caster)
                    return;

                Unit* target = GetTarget();
                if (caster->HasAura(SPELL_WARLOCK_IMPROVED_HEALTH_FUNNEL_R2))
                    target->CastSpell(target, SPELL_WARLOCK_IMPROVED_HEALTH_FUNNEL_BUFF_R2, true);
                else if (caster->HasAura(SPELL_WARLOCK_IMPROVED_HEALTH_FUNNEL_R1))
                    target->CastSpell(target, SPELL_WARLOCK_IMPROVED_HEALTH_FUNNEL_BUFF_R1, true);
            }

            void RemoveEffect(AuraEffect const* /*aurEff*/, AuraEffectHandleModes /*mode*/)
            {
                Unit* target = GetTarget();
                target->RemoveAurasDueToSpell(SPELL_WARLOCK_IMPROVED_HEALTH_FUNNEL_BUFF_R1);
                target->RemoveAurasDueToSpell(SPELL_WARLOCK_IMPROVED_HEALTH_FUNNEL_BUFF_R2);
            }

            void DealFunnelDamage(AuraEffect const* /*aurEff*/)
            {
                Unit* caster = GetCaster();
                if (!caster)
                    return;

                uint32 funnelDamage = caster->CountPctFromMaxHealth(GetSpellInfo()->Effects[EFFECT_2].BasePoints);
                if (AuraEffect* aurEff = caster->GetAuraEffect(SPELL_AURA_ADD_PCT_MODIFIER, SPELLFAMILY_WARLOCK, 153, EFFECT_1))
                    funnelDamage = AddPct(funnelDamage, aurEff->GetAmount());

                uint32 funnelAbsorb = 0;
                caster->DealDamageMods(caster, funnelDamage, &funnelAbsorb);
                caster->SendSpellNonMeleeDamageLog(caster, GetId(), funnelDamage, GetSpellInfo()->GetSchoolMask(), funnelAbsorb, 0, false, 0, false);

                CleanDamage cleanDamage = CleanDamage(0, 0, BASE_ATTACK, MELEE_HIT_NORMAL);
                caster->DealDamage(caster, funnelDamage, &cleanDamage, NODAMAGE, GetSpellInfo()->GetSchoolMask(), GetSpellInfo(), true);
            }

            void Register()
            {
                OnEffectRemove += AuraEffectRemoveFn(spell_warl_health_funnel_AuraScript::RemoveEffect, EFFECT_0, SPELL_AURA_OBS_MOD_HEALTH, AURA_EFFECT_HANDLE_REAL);
                OnEffectApply += AuraEffectApplyFn(spell_warl_health_funnel_AuraScript::ApplyEffect, EFFECT_0, SPELL_AURA_OBS_MOD_HEALTH, AURA_EFFECT_HANDLE_REAL);
                OnEffectPeriodic += AuraEffectPeriodicFn(spell_warl_health_funnel_AuraScript::DealFunnelDamage, EFFECT_0, SPELL_AURA_OBS_MOD_HEALTH);
            }
        };

        AuraScript* GetAuraScript() const
        {
            return new spell_warl_health_funnel_AuraScript();
        }
};

// 1454 - Life Tap
/// Updated 4.3.4
class spell_warl_life_tap : public SpellScriptLoader
{
    public:
        spell_warl_life_tap() : SpellScriptLoader("spell_warl_life_tap") { }

        class spell_warl_life_tap_SpellScript : public SpellScript
        {
            PrepareSpellScript(spell_warl_life_tap_SpellScript);

            bool Load()
            {
                return GetCaster()->GetTypeId() == TYPEID_PLAYER;
            }

            bool Validate(SpellInfo const* /*spellInfo*/)
            {
                if (!sSpellMgr->GetSpellInfo(SPELL_WARLOCK_LIFE_TAP_ENERGIZE) || !sSpellMgr->GetSpellInfo(SPELL_WARLOCK_LIFE_TAP_ENERGIZE_2))
                    return false;
                return true;
            }

            void HandleDummy(SpellEffIndex /*effIndex*/)
            {
                Player* caster = GetCaster()->ToPlayer();
                if (Unit* target = GetHitUnit())
                {
                    int32 damage = caster->CountPctFromMaxHealth(GetSpellInfo()->Effects[EFFECT_2].CalcValue());
                    int32 mana = CalculatePct(damage, GetSpellInfo()->Effects[EFFECT_1].CalcValue());

                    // Shouldn't Appear in Combat Log
                    target->ModifyHealth(-damage);

                    // Improved Life Tap mod
                    if (AuraEffect const* aurEff = caster->GetDummyAuraEffect(SPELLFAMILY_WARLOCK, WARLOCK_ICON_ID_IMPROVED_LIFE_TAP, 0))
                        AddPct(mana, aurEff->GetAmount());

                    caster->CastCustomSpell(target, SPELL_WARLOCK_LIFE_TAP_ENERGIZE, &mana, NULL, NULL, false);

                    // Mana Feed
                    if (AuraEffect const* aurEff = caster->GetAuraEffect(SPELL_AURA_ADD_FLAT_MODIFIER, SPELLFAMILY_WARLOCK, WARLOCK_ICON_ID_MANA_FEED, 0))
                    {
                        int32 manaFeedVal = aurEff->GetAmount();
                        ApplyPct(manaFeedVal, mana);
                        if (caster->GetGuardianPet())
                            caster->CastCustomSpell(caster, SPELL_WARLOCK_LIFE_TAP_ENERGIZE_2, &manaFeedVal, NULL, NULL, true, NULL);
                    }
                }
            }

            SpellCastResult CheckCast()
            {
                if (int32(GetCaster()->GetHealth()) > int32(GetCaster()->CountPctFromMaxHealth(GetSpellInfo()->Effects[EFFECT_2].CalcValue())))
                    return SPELL_CAST_OK;
                return SPELL_FAILED_FIZZLE;
            }

            void Register()
            {
                OnEffectHitTarget += SpellEffectFn(spell_warl_life_tap_SpellScript::HandleDummy, EFFECT_0, SPELL_EFFECT_DUMMY);
                OnCheckCast += SpellCheckCastFn(spell_warl_life_tap_SpellScript::CheckCast);
            }
        };

        SpellScript* GetSpellScript() const
        {
            return new spell_warl_life_tap_SpellScript();
        }
};

// 27285 - Seed of Corruption
/// Updated 4.3.4
class spell_warl_seed_of_corruption : public SpellScriptLoader
{
    public:
        spell_warl_seed_of_corruption() : SpellScriptLoader("spell_warl_seed_of_corruption") { }

        class spell_warl_seed_of_corruption_SpellScript : public SpellScript
        {
            PrepareSpellScript(spell_warl_seed_of_corruption_SpellScript);

            bool inSoulBurn;

            bool Load()
            {
                inSoulBurn = false;
                return true;
            }

            void FilterTargets(std::list<WorldObject*>& targets)
            {
                if (GetExplTargetUnit())
                    targets.remove(GetExplTargetUnit());
            }

            void HandleOnHit()
            {
                Unit* caster = GetCaster();
                if (!caster)
                    return;

                if (!inSoulBurn)
                {
                    if (GetSpell()->IsChangeBySoulBurn())
                    {
                        inSoulBurn = true;
                        caster->CastSpell(caster, SPELL_WARLOCK_SOULSHARD_ADD, true);
                    }
                }

                if (inSoulBurn)
                    if (Unit* target = GetHitUnit())
                        caster->CastSpell(target, 172, true);
            }

            void Register()
            {
                OnObjectAreaTargetSelect += SpellObjectAreaTargetSelectFn(spell_warl_seed_of_corruption_SpellScript::FilterTargets, EFFECT_0, TARGET_UNIT_DEST_AREA_ENEMY);
                OnHit += SpellHitFn(spell_warl_seed_of_corruption_SpellScript::HandleOnHit);
            }
        };

        SpellScript* GetSpellScript() const
        {
            return new spell_warl_seed_of_corruption_SpellScript();
        }
};

// -7235 - Shadow Ward
// -91711 - Nether Ward
class spell_warl_wards : public SpellScriptLoader
{
    public:
        spell_warl_wards() : SpellScriptLoader("spell_warl_wards") { }

        class spell_warl_wards_AuraScript : public AuraScript
        {
            PrepareAuraScript(spell_warl_wards_AuraScript);

            void CalculateAmount(AuraEffect const* /*aurEff*/, int32& amount, bool& canBeRecalculated)
            {
                canBeRecalculated = false;
                if (Unit* caster = GetCaster())
                {
                    // +80.70% from sp bonus
                    float bonus = 0.807f;

                    bonus *= caster->SpellBaseHealingBonusDone(GetSpellInfo()->GetSchoolMask());
                    bonus *= caster->CalculateLevelPenalty(GetSpellInfo());

                    amount += int32(bonus);
                }
            }

            void Absorb(AuraEffect* /*aurEff*/, DamageInfo & dmgInfo, uint32 & absorbAmount)
            {
                Unit* target = GetTarget();
                if (!target)
                    return;

                if (AuraEffect* talent = target->GetDummyAuraEffect(SPELLFAMILY_WARLOCK, 1985, EFFECT_0))
                {
                    uint32 spellId = 0;
                    switch (dmgInfo.GetSchoolMask())
                    {
                        case SPELL_SCHOOL_MASK_HOLY:
                            spellId = 54370;
                            break;
                        case SPELL_SCHOOL_MASK_FIRE:
                            spellId = 54371;
                            break;
                        case SPELL_SCHOOL_MASK_FROST:
                            spellId = 54372;
                            break;
                        case SPELL_SCHOOL_MASK_ARCANE:
                            spellId = 54373;
                            break;
                        case SPELL_SCHOOL_MASK_SHADOW:
                            spellId = 54374;
                            break;
                        case SPELL_SCHOOL_MASK_NATURE:
                            spellId = 54375;
                            break;
                    }
                    if (spellId)
                    {
                        int32 bp0 = -talent->GetAmount();
                        target->CastCustomSpell(target, spellId, &bp0, NULL, NULL, true);
                    }
                }
            }

            void Register()
            {
                DoEffectCalcAmount += AuraEffectCalcAmountFn(spell_warl_wards_AuraScript::CalculateAmount, EFFECT_0, SPELL_AURA_SCHOOL_ABSORB);
                OnEffectAbsorb += AuraEffectAbsorbFn(spell_warl_wards_AuraScript::Absorb, EFFECT_0);
            }
        };

        AuraScript* GetAuraScript() const
        {
            return new spell_warl_wards_AuraScript();
        }
};

// 29858 - Soulshatter
/// Updated 4.3.4
class spell_warl_soulshatter : public SpellScriptLoader
{
    public:
        spell_warl_soulshatter() : SpellScriptLoader("spell_warl_soulshatter") { }

        class spell_warl_soulshatter_SpellScript : public SpellScript
        {
            PrepareSpellScript(spell_warl_soulshatter_SpellScript);

            bool Validate(SpellInfo const* /*spellInfo*/)
            {
                if (!sSpellMgr->GetSpellInfo(SPELL_WARLOCK_SOULSHATTER))
                    return false;
                return true;
            }

            void HandleDummy(SpellEffIndex /*effIndex*/)
            {
                Unit* caster = GetCaster();
                if (Unit* target = GetHitUnit())
                    if (target->CanHaveThreatList() && target->getThreatManager().getThreat(caster) > 0.0f)
                        caster->CastSpell(target, SPELL_WARLOCK_SOULSHATTER, true);
            }

            void Register()
            {
                OnEffectHitTarget += SpellEffectFn(spell_warl_soulshatter_SpellScript::HandleDummy, EFFECT_0, SPELL_EFFECT_DUMMY);
            }
        };

        SpellScript* GetSpellScript() const
        {
            return new spell_warl_soulshatter_SpellScript();
        }
};

// 30108, 34438, 34439, 35183 - Unstable Affliction
/// Updated 4.3.4
class spell_warl_unstable_affliction : public SpellScriptLoader
{
    public:
        spell_warl_unstable_affliction() : SpellScriptLoader("spell_warl_unstable_affliction") { }

        class spell_warl_unstable_affliction_AuraScript : public AuraScript
        {
            PrepareAuraScript(spell_warl_unstable_affliction_AuraScript);

            bool Validate(SpellInfo const* /*spellInfo*/)
            {
                if (!sSpellMgr->GetSpellInfo(SPELL_WARLOCK_UNSTABLE_AFFLICTION_DISPEL))
                    return false;

                return true;
            }

            void HandleDispel(DispelInfo* dispelInfo)
            {
                if (Unit* caster = GetCaster())
                    if (AuraEffect const* aurEff = GetFirstEffectOfType(SPELL_AURA_PERIODIC_DAMAGE))
                    {
                        int32 damage = aurEff->GetAmount() * 9;
                        // backfire damage and silence
                        caster->CastCustomSpell(dispelInfo->GetDispeller(), SPELL_WARLOCK_UNSTABLE_AFFLICTION_DISPEL, &damage, NULL, NULL, true, NULL, aurEff);
                    }
            }

            void Register()
            {
                AfterDispel += AuraDispelFn(spell_warl_unstable_affliction_AuraScript::HandleDispel);
            }
        };

        AuraScript* GetAuraScript() const
        {
            return new spell_warl_unstable_affliction_AuraScript();
        }
};

class spell_warl_curse_of_the_elements : public SpellScriptLoader
{
    public:
        spell_warl_curse_of_the_elements() : SpellScriptLoader("spell_warl_curse_of_the_elements") { }

        class spell_warl_curse_of_the_elements_AuraScript : public AuraScript
        {
            PrepareAuraScript(spell_warl_curse_of_the_elements_AuraScript);

            bool Validate(SpellInfo const* /*spellInfo*/)
            {
                if (!sSpellMgr->GetSpellInfo(SPELL_WARLOCK_JINX_R1))
                    return false;
                if (!sSpellMgr->GetSpellInfo(SPELL_WARLOCK_JINX_R2))
                    return false;
                if (!sSpellMgr->GetSpellInfo(SPELL_WARLOCK_JINX_CoE_R1))
                    return false;
                if (!sSpellMgr->GetSpellInfo(SPELL_WARLOCK_JINX_CoE_R2))
                    return false;

                return true;
            }

            void HandleEffectPeriodic(AuraEffect const* /*aurEff*/)
            {
                Unit* caster = GetCaster();
                Unit* target = GetTarget();
                if (!caster || ! target)
                    return;

                if (caster->HasAura(SPELL_WARLOCK_JINX_R2))
                    caster->CastSpell(target, SPELL_WARLOCK_JINX_CoE_R2, true);
                else if (caster->HasAura(SPELL_WARLOCK_JINX_R1))
                    caster->CastSpell(target, SPELL_WARLOCK_JINX_CoE_R1, true);
            }

            void Register()
            {
                OnEffectPeriodic += AuraEffectPeriodicFn(spell_warl_curse_of_the_elements_AuraScript::HandleEffectPeriodic, EFFECT_2, SPELL_AURA_PERIODIC_DUMMY);
            }
        };

        AuraScript* GetAuraScript() const
        {
            return new spell_warl_curse_of_the_elements_AuraScript();
        }
};

class spell_warl_jinx_coe : public SpellScriptLoader
{
    public:
        spell_warl_jinx_coe() : SpellScriptLoader("spell_warl_jinx_coe") { }

        class spell_warl_jinx_coe_SpellScript : public SpellScript
        {
            PrepareSpellScript(spell_warl_jinx_coe_SpellScript);

            void FilterTargetsInitial(std::list<WorldObject*>& targets)
            {
                targets.remove_if(Trinity::UnitAuraCheck(true, SPELL_WARLOCK_CoE));
                if (targets.size() > 15)
                    targets.resize(15);

                sharedTargets = targets;
            }

            void FilterTargetsSubsequent(std::list<WorldObject*>& targets)
            {
                targets = sharedTargets;
            }

            void Register()
            {
                OnObjectAreaTargetSelect += SpellObjectAreaTargetSelectFn(spell_warl_jinx_coe_SpellScript::FilterTargetsInitial, EFFECT_0, TARGET_UNIT_DEST_AREA_ENEMY);
                OnObjectAreaTargetSelect += SpellObjectAreaTargetSelectFn(spell_warl_jinx_coe_SpellScript::FilterTargetsSubsequent, EFFECT_1, TARGET_UNIT_DEST_AREA_ENEMY);
            }

            std::list<WorldObject*> sharedTargets;
        };

        SpellScript* GetSpellScript() const
        {
            return new spell_warl_jinx_coe_SpellScript();
        }
};

class spell_warl_drain_life : public SpellScriptLoader
{
    public:
        spell_warl_drain_life() : SpellScriptLoader("spell_warl_drain_life") { }

        class spell_warl_drain_life_AuraScript : public AuraScript
        {
            PrepareAuraScript(spell_warl_drain_life_AuraScript);

            bool Validate(SpellInfo const* /*spellInfo*/)
            {
                if (!sSpellMgr->GetSpellInfo(SPELL_WARLOCK_DRAIN_LIFE_HEALTH))
                    return false;

                return true;
            }

            void onPeriodicTick(AuraEffect const* /*aurEff*/)
            {
                if (Unit* caster = GetCaster())
                {
                    int32 baseAmount = caster->CalculateSpellDamage(caster, sSpellMgr->GetSpellInfo(SPELL_WARLOCK_DRAIN_LIFE_HEALTH), 0);

                    if (AuraEffect const* aurEff = caster->GetAuraEffect(SPELL_AURA_DUMMY, SPELLFAMILY_WARLOCK, 3223, 0))
                        if (caster->HealthBelowPct(25))
                            baseAmount += int32(aurEff->GetAmount());

                    caster->CastCustomSpell(caster, SPELL_WARLOCK_DRAIN_LIFE_HEALTH, &baseAmount, NULL, NULL, true);
                }
            }

            void Register()
            {
                OnEffectPeriodic += AuraEffectPeriodicFn(spell_warl_drain_life_AuraScript::onPeriodicTick, EFFECT_0, SPELL_AURA_PERIODIC_DAMAGE);
            }
        };

        AuraScript* GetAuraScript() const
        {
            return new spell_warl_drain_life_AuraScript();
        }
};

class spell_warl_soul_swap : public SpellScriptLoader
{
    public:
        spell_warl_soul_swap() : SpellScriptLoader("spell_warl_soul_swap") { }

        class spell_warl_soul_swap_SpellScript : public SpellScript
        {
            PrepareSpellScript(spell_warl_soul_swap_SpellScript);

            bool Validate(SpellInfo const* /*spellInfo*/)
            {
                if (!sSpellMgr->GetSpellInfo(SPELL_WARLOCK_SOUL_SWAP_OVERRIDE))
                    return false;
                if (!sSpellMgr->GetSpellInfo(SPELL_WARLOCK_SOUL_SWAP_GLYPH))
                    return false;
                if (!sSpellMgr->GetSpellInfo(SPELL_WARLOCK_SOUL_SWAP_COOLDOWN))
                    return false;
                if (!sSpellMgr->GetSpellInfo(SPELL_WARLOCK_SOUL_SWAP_VISUAL))
                    return false;
                if (!sSpellMgr->GetSpellInfo(SPELL_WARLOCK_SOUL_SWAP_FREE_DOTS_COSTS))
                    return false;

                return true;
            }

            std::list<uint32> dotsList;

            void HandleSoulSwap(SpellEffIndex /*effIndex*/)
            {
                Unit* caster = GetCaster();
                Unit* target = GetHitUnit();
                if (!caster || ! target)
                    return;

                caster->CastSpell(caster, SPELL_WARLOCK_SOUL_SWAP_OVERRIDE, true);
                target->CastSpell(caster, SPELL_WARLOCK_SOUL_SWAP_VISUAL, true);

                for (std::list<uint32>::const_iterator itr = dotsList.begin(); itr != dotsList.end(); itr++)
                {
                    if (!caster->HasAura(SPELL_WARLOCK_SOUL_SWAP_GLYPH))
                        target->RemoveAura(*itr, caster->GetGUID());
                }

                caster->SaveSoulSwapDotsListAndTarget(target->GetGUID(), dotsList);
            }

            SpellCastResult CheckCast()
            {
                Unit* caster = GetCaster();
                Unit* target = GetExplTargetUnit();
                if (!caster || ! target)
                    return SPELL_FAILED_BAD_TARGETS;

                Unit::AuraEffectList const& auras = target->GetAuraDotsByCaster(caster->GetGUID());
                if (auras.empty())
                    return SPELL_FAILED_BAD_TARGETS;

                for (Unit::AuraEffectList::const_iterator itr = auras.begin(); itr != auras.end(); ++itr)
                {
                    if ((*itr)->GetSpellInfo()->SpellFamilyFlags & sSpellMgr->GetSpellInfo(SPELL_WARLOCK_SOUL_SWAP_FREE_DOTS_COSTS)->Effects[0].SpellClassMask)
                        dotsList.push_back((*itr)->GetId());
                }

                return dotsList.empty() ? SPELL_FAILED_BAD_TARGETS : SPELL_CAST_OK;
            }

            void Register()
            {
                OnEffectHitTarget += SpellEffectFn(spell_warl_soul_swap_SpellScript::HandleSoulSwap, EFFECT_0, SPELL_EFFECT_SCHOOL_DAMAGE);
                OnCheckCast += SpellCheckCastFn(spell_warl_soul_swap_SpellScript::CheckCast);
            }
        };

        SpellScript* GetSpellScript() const
        {
            return new spell_warl_soul_swap_SpellScript();
        }
};

class spell_warl_soul_swap_exhale : public SpellScriptLoader
{
    public:
        spell_warl_soul_swap_exhale() : SpellScriptLoader("spell_warl_soul_swap_exhale") { }

        class spell_warl_soul_swap_exhale_SpellScript : public SpellScript
        {
            PrepareSpellScript(spell_warl_soul_swap_exhale_SpellScript);

            bool Validate(SpellInfo const* /*spellInfo*/)
            {
                if (!sSpellMgr->GetSpellInfo(SPELL_WARLOCK_SOUL_SWAP_OVERRIDE))
                    return false;
                if (!sSpellMgr->GetSpellInfo(SPELL_WARLOCK_SOUL_SWAP_FREE_DOTS_COSTS))
                    return false;

                return true;
            }

            std::list<uint32> dotsList;

            void HandleOnCast()
            {
                if (Unit* caster = GetCaster())
                {
                    dotsList = caster->GetSoulSwapDotsList();
                    caster->RemoveAurasDueToSpell(SPELL_WARLOCK_SOUL_SWAP_OVERRIDE);

                    if (caster->HasAura(SPELL_WARLOCK_SOUL_SWAP_GLYPH))
                        caster->CastSpell(caster, SPELL_WARLOCK_SOUL_SWAP_COOLDOWN, false);
                }
            }

            void HandleSoulSwapExhale(SpellEffIndex /*effIndex*/)
            {
                Unit* caster = GetCaster();
                Unit* target = GetHitUnit();
                if (!caster || ! target)
                    return;

                if (dotsList.empty())
                    return;

                for (std::list<uint32>::const_iterator itr = dotsList.begin(); itr != dotsList.end(); ++itr)
                {
                    caster->CastSpell(caster, SPELL_WARLOCK_SOUL_SWAP_FREE_DOTS_COSTS, true);
                    caster->CastSpell(target, *itr, true);
                }

                caster->RemoveAurasDueToSpell(SPELL_WARLOCK_SOUL_SWAP_FREE_DOTS_COSTS); // use for prevention
            }

            SpellCastResult CheckCast()
            {
                Unit* caster = GetCaster();
                Unit* target = GetExplTargetUnit();
                if (!caster || ! target)
                    return SPELL_FAILED_BAD_TARGETS;

                if (caster->GetSoulSwapTarget() == target->GetGUID())
                    return SPELL_FAILED_BAD_TARGETS;

                return SPELL_CAST_OK;
            }

            void Register()
            {
                OnEffectLaunchTarget += SpellEffectFn(spell_warl_soul_swap_exhale_SpellScript::HandleSoulSwapExhale, EFFECT_0, SPELL_EFFECT_SCHOOL_DAMAGE);
                OnCheckCast += SpellCheckCastFn(spell_warl_soul_swap_exhale_SpellScript::CheckCast);
                OnCast += SpellCastFn(spell_warl_soul_swap_exhale_SpellScript::HandleOnCast);
            }
        };

        SpellScript* GetSpellScript() const
        {
            return new spell_warl_soul_swap_exhale_SpellScript();
        }
};

class spell_warl_soul_swap_override : public SpellScriptLoader
{
    public:
        spell_warl_soul_swap_override() : SpellScriptLoader("spell_warl_soul_swap_override") { }

        class spell_warl_soul_swap_override_AuraScript : public AuraScript
        {
            PrepareAuraScript(spell_warl_soul_swap_override_AuraScript);

            bool Validate(SpellInfo const* spellInfo)
            {
                return true;
            }

            void AfterRemove(AuraEffect const* /*aurEff*/, AuraEffectHandleModes /*mode*/)
            {
                if (Unit* caster = GetCaster())
                    caster->ClearSoulSwapDotsList();
            }

            void Register()
            {
                AfterEffectRemove += AuraEffectRemoveFn(spell_warl_soul_swap_override_AuraScript::AfterRemove, EFFECT_0, SPELL_AURA_OVERRIDE_ACTIONBAR_SPELLS, AURA_EFFECT_HANDLE_REAL);
            }
        };

        AuraScript* GetAuraScript() const
        {
            return new spell_warl_soul_swap_override_AuraScript();
        }
};

class spell_warl_soulburn : public SpellScriptLoader
{
    public:
        spell_warl_soulburn() : SpellScriptLoader("spell_warl_soulburn") { }

        class spell_warl_soulburn_SpellScript : public SpellScript
        {
            PrepareSpellScript(spell_warl_soulburn_SpellScript);

            bool Validate(SpellInfo const* /*spellInfo*/)
            {
                if (!sSpellMgr->GetSpellInfo(SPELL_WARLOCK_SOULBURN_SOC_VISUAL))
                    return false;
                if (!sSpellMgr->GetSpellInfo(SPELL_WARLOCK_SOULBURN_SEED_OF_CORRUPTION))
                    return false;

                return true;
            }

            void HandleOnCast()
            {
                if (Unit* caster = GetCaster())
                    if (caster->HasSpell(SPELL_WARLOCK_SOULBURN_SEED_OF_CORRUPTION))
                        caster->CastSpell(caster, SPELL_WARLOCK_SOULBURN_SOC_VISUAL, true);
            }

            void Register()
            {
                OnCast += SpellCastFn(spell_warl_soulburn_SpellScript::HandleOnCast);
            }
        };

        SpellScript* GetSpellScript() const
        {
            return new spell_warl_soulburn_SpellScript();
        }
};

class spell_warl_soulburn_seed_of_corruption : public SpellScriptLoader
{
    public:
        spell_warl_soulburn_seed_of_corruption() : SpellScriptLoader("spell_warl_soulburn_seed_of_corruption") { }

        class spell_warl_soulburn_seed_of_corruption_SpellScript : public SpellScript
        {
            PrepareSpellScript(spell_warl_soulburn_seed_of_corruption_SpellScript);

            bool Validate(SpellInfo const* /*spellInfo*/)
            {
                if (!sSpellMgr->GetSpellInfo(SPELL_WARLOCK_SOULBURN))
                    return false;
                if (!sSpellMgr->GetSpellInfo(SPELL_WARLOCK_SOULBURN_SEED_OF_CORRUPTION))
                    return false;

                return true;
            }

            void HandleOnCast()
            {
                if (Unit* caster = GetCaster())
                    if (caster->HasAura(SPELL_WARLOCK_SOULBURN) && caster->HasSpell(SPELL_WARLOCK_SOULBURN_SEED_OF_CORRUPTION))
                    {
                        GetSpell()->SetChangeBySoulBurn(true);
                        caster->RemoveAurasDueToSpell(SPELL_WARLOCK_SOULBURN);
                    }
            }

            void Register()
            {
                OnCast += SpellCastFn(spell_warl_soulburn_seed_of_corruption_SpellScript::HandleOnCast);
            }
        };

        SpellScript* GetSpellScript() const
        {
            return new spell_warl_soulburn_seed_of_corruption_SpellScript();
        }
};

class spell_warl_pandemic : public SpellScriptLoader
{
    public:
        spell_warl_pandemic() : SpellScriptLoader("spell_warl_pandemic") { }

        class spell_warl_pandemic_SpellScript : public SpellScript
        {
            PrepareSpellScript(spell_warl_pandemic_SpellScript);

            void HandleScriptEffect(SpellEffIndex /*effIndex*/)
            {
                if (Unit* unitTarget = GetHitUnit())
                    // Refresh untable affliction on target
                    if (AuraEffect* aur = unitTarget->GetAuraEffect(SPELL_AURA_PERIODIC_DAMAGE, SPELLFAMILY_WARLOCK, 0, 0x100, 0, GetCaster()->GetGUID()))
                        aur->GetBase()->RefreshDuration();
            }

            void Register()
            {
                OnEffectHitTarget += SpellEffectFn(spell_warl_pandemic_SpellScript::HandleScriptEffect, EFFECT_0, SPELL_EFFECT_SCRIPT_EFFECT);
            }
        };

        SpellScript* GetSpellScript() const
        {
            return new spell_warl_pandemic_SpellScript();
        }
};

class spell_warl_drain_soul : public SpellScriptLoader
{
    public:
        spell_warl_drain_soul() : SpellScriptLoader("spell_warl_drain_soul") { }

        class spell_warl_drain_soul_AuraScript : public AuraScript
        {
            PrepareAuraScript(spell_warl_drain_soul_AuraScript);

            bool Validate(SpellInfo const* spellInfo)
            {
                if (!sSpellMgr->GetSpellInfo(SPELL_WARLOCK_PANDEMIC))
                    return false;

                return true;
            }

            void onPeriodicTick(AuraEffect const* /*aurEff*/)
            {
                Unit* target = GetTarget();
                Unit* caster = GetCaster();
                if (!target || ! caster)
                    return;
                
                if (target->HealthBelowPct(25))
                    if (AuraEffect const* aurEff = caster->GetAuraEffect(SPELL_AURA_ADD_FLAT_MODIFIER, SPELLFAMILY_WARLOCK, 4554, EFFECT_1))
                        if (Aura *aura = aurEff->GetBase())
                            if (roll_chance_i(aura->GetSpellInfo()->Effects[EFFECT_0].BasePoints))
                                caster->CastSpell(target, SPELL_WARLOCK_PANDEMIC, true);

            }

            void RemoveEffect(AuraEffect const* aurEff, AuraEffectHandleModes /*mode*/)
            {
                if (GetTargetApplication()->GetRemoveMode() == AURA_REMOVE_BY_DEATH)
                    if (Player *caster = GetCaster()->ToPlayer())
                        if (Unit *target = GetTarget())
                            if (caster->isHonorOrXPTarget(target))
                                caster->CastSpell(caster, 79264, true, 0, aurEff);
            }

            void Register()
            {
                OnEffectPeriodic += AuraEffectPeriodicFn(spell_warl_drain_soul_AuraScript::onPeriodicTick, EFFECT_0, SPELL_AURA_PERIODIC_DAMAGE);
                OnEffectRemove += AuraEffectRemoveFn(spell_warl_drain_soul_AuraScript::RemoveEffect, EFFECT_0, SPELL_AURA_PERIODIC_DAMAGE, AURA_EFFECT_HANDLE_REAL);
            }
        };

        AuraScript* GetAuraScript() const
        {
            return new spell_warl_drain_soul_AuraScript();
        }
};

class spell_warl_soulburn_healthstone : public SpellScriptLoader
{
    public:
        spell_warl_soulburn_healthstone() : SpellScriptLoader("spell_warl_soulburn_healthstone") { }

        class spell_warl_soulburn_healthstone_AuraScript : public AuraScript
        {
            PrepareAuraScript(spell_warl_soulburn_healthstone_AuraScript);

            void CalculateAmount(AuraEffect const* /*aurEff*/, int32& amount, bool& /*canBeRecalculated*/)
            {
                amount = GetUnitOwner()->CountPctFromMaxHealth(amount);
            }

            void Register()
            {
                DoEffectCalcAmount += AuraEffectCalcAmountFn(spell_warl_soulburn_healthstone_AuraScript::CalculateAmount, EFFECT_0, SPELL_AURA_MOD_INCREASE_HEALTH);
            }
        };

        AuraScript* GetAuraScript() const
        {
            return new spell_warl_soulburn_healthstone_AuraScript();
        }
};

class spell_warl_searing_pain : public SpellScriptLoader
{
    public:
        spell_warl_searing_pain() : SpellScriptLoader("spell_warl_searing_pain") { }

        class spell_warl_searing_pain_SpellScript : public SpellScript
        {
            PrepareSpellScript(spell_warl_searing_pain_SpellScript);

            bool Validate(SpellInfo const* spellInfo)
            {
                if (!sSpellMgr->GetSpellInfo(SPELL_WARLOCK_SOULBURN_SEARING_PAIN))
                    return false;

                return true;
            }

            void HandleOnHit(SpellEffIndex /*effIndex*/)
            {
                if (Player* caster = GetCaster()->ToPlayer())
                    if (Unit* unitTarget = GetHitUnit())
                        if (GetSpell()->IsChangeBySoulBurn())
                            caster->CastSpell(caster, SPELL_WARLOCK_SOULBURN_SEARING_PAIN, true);
            }

            void HandleOnCast()
            {
                if (Unit* caster = GetCaster())
                    if (caster->HasAura(SPELL_WARLOCK_SOULBURN))
                    {
                        GetSpell()->SetChangeBySoulBurn(true);
                        caster->RemoveAurasDueToSpell(SPELL_WARLOCK_SOULBURN);
                    }
            }

            void Register()
            {
                OnEffectHitTarget += SpellEffectFn(spell_warl_searing_pain_SpellScript::HandleOnHit, EFFECT_0, SPELL_EFFECT_SCHOOL_DAMAGE);
                OnCast += SpellCastFn(spell_warl_searing_pain_SpellScript::HandleOnCast);
            }
        };

        SpellScript* GetSpellScript() const
        {
            return new spell_warl_searing_pain_SpellScript();
        }
};

class spell_warl_soul_harvest : public SpellScriptLoader
{
    public:
        spell_warl_soul_harvest() : SpellScriptLoader("spell_warl_soul_harvest") { }

        class spell_warl_soul_harvest_AuraScript : public AuraScript
        {
            PrepareAuraScript(spell_warl_soul_harvest_AuraScript);

            bool Validate(SpellInfo const* spellInfo)
            {
                if (!sSpellMgr->GetSpellInfo(SPELL_WARLOCK_SOUL_HARVEST_SOULSHARD))
                    return false;

                return true;
            }

            void onPeriodicTick(AuraEffect const* /*aurEff*/)
            {
                if (Unit* caster = GetCaster())
                    if (caster->GetPower(POWER_SOUL_SHARDS) != caster->GetMaxPower(POWER_SOUL_SHARDS))
                        caster->CastSpell(caster, SPELL_WARLOCK_SOUL_HARVEST_SOULSHARD, true);
            }

            void Register()
            {
                OnEffectPeriodic += AuraEffectPeriodicFn(spell_warl_soul_harvest_AuraScript::onPeriodicTick, EFFECT_0, SPELL_AURA_PERIODIC_DUMMY);
            }
        };

        AuraScript* GetAuraScript() const
        {
            return new spell_warl_soul_harvest_AuraScript();
        }
};

class spell_warl_fel_armor : public SpellScriptLoader
{
    public:
        spell_warl_fel_armor() : SpellScriptLoader("spell_warl_fel_armor") { }

        class spell_warl_fel_armor_AuraScript : public AuraScript
        {
            PrepareAuraScript(spell_warl_fel_armor_AuraScript);

            bool Validate(SpellInfo const* /*spellInfo*/)
            {
                if (!sSpellMgr->GetSpellInfo(SPELL_WARLOCK_FEL_ARMOR_HEAL))
                    return false;
                if (!sSpellMgr->GetSpellInfo(SPELL_WARLOCK_SHADOW_WARD))
                    return false;
                if (!sSpellMgr->GetSpellInfo(SPELL_WARLOCK_NETHER_WARD))
                    return false;
                if (!sSpellMgr->GetSpellInfo(SPELL_WARLOCK_NETHER_WARD_TALENT))
                    return false;

                return true;
            }

            bool CheckProc(ProcEventInfo& eventInfo)
            {
                return eventInfo.GetDamageInfo()->GetDamage() && !eventInfo.GetDamageInfo()->GetSpellInfo()->IsTargetingArea() &&
                    !eventInfo.GetDamageInfo()->GetSpellInfo()->IsAffectingArea() && !eventInfo.GetDamageInfo()->GetSpellInfo()->IsEffectAffectingOrTargetingArea();
            }

            void OnProc(AuraEffect const* aurEff, ProcEventInfo& eventInfo)
            {
                Unit *caster = GetCaster();
                if (!caster)
                    return;

                PreventDefaultAction();

                int32 heal = CalculatePct(int32(eventInfo.GetDamageInfo()->GetDamage()), aurEff->GetAmount());
                caster->CastCustomSpell(caster, SPELL_WARLOCK_FEL_ARMOR_HEAL, &heal, NULL, NULL, true, NULL, aurEff, GetCasterGUID());
            }

            void CalculateAmount(AuraEffect const* /*aurEff*/, int32& amount, bool& canBeRecalculated)
            {
                canBeRecalculated = false;
                if (Unit* caster = GetCaster())
                {
                    if (caster->HasAura(SPELL_WARLOCK_NETHER_WARD_TALENT)) // Nether Ward
                        amount = SPELL_WARLOCK_NETHER_WARD;
                    else
                        amount = SPELL_WARLOCK_SHADOW_WARD;
                }
                else
                    amount = SPELL_WARLOCK_SHADOW_WARD;
            }

            void Register()
            {
                DoCheckProc += AuraCheckProcFn(spell_warl_fel_armor_AuraScript::CheckProc);
                OnEffectProc += AuraEffectProcFn(spell_warl_fel_armor_AuraScript::OnProc, EFFECT_1, SPELL_AURA_DUMMY);
                DoEffectCalcAmount += AuraEffectCalcAmountFn(spell_warl_fel_armor_AuraScript::CalculateAmount, EFFECT_2, SPELL_AURA_OVERRIDE_ACTIONBAR_SPELLS);
            }
        };

        AuraScript* GetAuraScript() const
        {
            return new spell_warl_fel_armor_AuraScript();
        }
};

class spell_warl_mana_feed : public SpellScriptLoader
{
    public:
        spell_warl_mana_feed() : SpellScriptLoader("spell_warl_mana_feed") { }

        class spell_warl_mana_feed_SpellScript : public SpellScript
        {
            PrepareSpellScript(spell_warl_mana_feed_SpellScript);

            void HandleEnergize(SpellEffIndex effIndex)
            {
                Unit* target = GetHitUnit();
                if (!target)
                    return;

                if (AuraEffect* aur = target->GetAuraEffect(SPELL_AURA_DUMMY, SPELLFAMILY_WARLOCK, 1982, EFFECT_2))
                    SetEffectDamage(aur->GetAmount());
            }

            void Register()
            {
                OnEffectHitTarget += SpellEffectFn(spell_warl_mana_feed_SpellScript::HandleEnergize, EFFECT_0, SPELL_EFFECT_ENERGIZE_PCT);
            }
        };

        SpellScript* GetSpellScript() const
        {
            return new spell_warl_mana_feed_SpellScript();
        }
};

class spell_warl_impending_doom : public SpellScriptLoader
{
    public:
        spell_warl_impending_doom() : SpellScriptLoader("spell_warl_impending_doom") { }

        class spell_warl_impending_doom_AuraScript : public AuraScript
        {
            PrepareAuraScript(spell_warl_impending_doom_AuraScript);

            bool Validate(SpellInfo const* /*spellInfo*/)
            {
                if (!sSpellMgr->GetSpellInfo(SPELL_WARLOCK_METHAMORPHOSIS_FORM))
                    return false;

                return true;
            }

            bool CheckProc(ProcEventInfo& /*eventInfo*/)
            {
                return GetCaster()->ToPlayer() && GetCaster()->ToPlayer()->HasSpellCooldown(SPELL_WARLOCK_METHAMORPHOSIS_FORM);
            }

            void OnProc(AuraEffect const* aurEff, ProcEventInfo& /*eventInfo*/)
            {
                Player* caster = GetCaster()->ToPlayer();
                if (!caster)
                    return;

                PreventDefaultAction();

                int32 cooldown = caster->GetSpellCooldownDelay(SPELL_WARLOCK_METHAMORPHOSIS_FORM);
                int32 amount = aurEff->GetAmount();

                if (cooldown < amount)
                    cooldown = 0;
                else
                    cooldown -= amount;

                caster->AddSpellCooldown(SPELL_WARLOCK_METHAMORPHOSIS_FORM, 0, uint32(time(NULL) + cooldown));
                WorldPacket data(SMSG_MODIFY_COOLDOWN, 4+8+4);
                data << uint32(SPELL_WARLOCK_METHAMORPHOSIS_FORM);
                data << uint64(caster->GetGUID());
                data << int32(-(amount*1000));
                caster->GetSession()->SendPacket(&data);
            }

            void Register()
            {
                DoCheckProc += AuraCheckProcFn(spell_warl_impending_doom_AuraScript::CheckProc);
                OnEffectProc += AuraEffectProcFn(spell_warl_impending_doom_AuraScript::OnProc, EFFECT_1, SPELL_AURA_DUMMY);
            }
        };

        AuraScript* GetAuraScript() const
        {
            return new spell_warl_impending_doom_AuraScript();
        }
};

class spell_warl_cremation : public SpellScriptLoader
{
    public:
        spell_warl_cremation() : SpellScriptLoader("spell_warl_cremation") { }

        class spell_warl_cremation_SpellScript : public SpellScript
        {
            PrepareSpellScript(spell_warl_cremation_SpellScript);

            void HandleScriptEffect(SpellEffIndex /*effIndex*/)
            {
				Unit* caster = GetCaster();
				
				if (!caster)
			    	return;
					
                if (Unit* unitTarget = GetHitUnit())
                    // Refresh immolate on target
                    if (AuraEffect* aur = unitTarget->GetAuraEffect(SPELL_AURA_PERIODIC_DAMAGE, SPELLFAMILY_WARLOCK, 0x00000004, 0, 0, GetCaster()->GetGUID()))
                        aur->GetBase()->RefreshDuration();
            }

            void Register()
            {
                OnEffectHitTarget += SpellEffectFn(spell_warl_cremation_SpellScript::HandleScriptEffect, EFFECT_0, SPELL_EFFECT_SCRIPT_EFFECT);
            }
        };

        SpellScript* GetSpellScript() const
        {
            return new spell_warl_cremation_SpellScript();
        }
};

/*PET SCALING*/
class spell_warl_pet_scaling_01 : public SpellScriptLoader
{
public:
    spell_warl_pet_scaling_01() : SpellScriptLoader("spell_warl_pet_scaling_01") { }

    class spell_warl_pet_scaling_01_AuraScript : public AuraScript
    {
        PrepareAuraScript(spell_warl_pet_scaling_01_AuraScript);

        bool Load()
        {
            if (!GetCaster() || !GetCaster()->GetOwner() || GetCaster()->GetOwner()->GetTypeId() != TYPEID_PLAYER)
                return false;
            return true;
        }

        void ApplyEffect(AuraEffect const* /* aurEff */, AuraEffectHandleModes /*mode*/)
        {
            if (Unit* pet = GetUnitOwner())
                pet->SetHealth(pet->GetMaxHealth());
        }

        void RemoveEffect(AuraEffect const* /* aurEff */, AuraEffectHandleModes /*mode*/)
        {
            if (Unit* pet = GetUnitOwner())
                _tempHealth = pet->GetHealth();
        }

        void CalculateMaxHealthAmount(AuraEffect const* /* aurEff */, int32& amount, bool& /*canBeRecalculated*/)
        {
            if (Unit* pet = GetUnitOwner())
                if (pet->isPet())
                    if (Unit* owner = pet->ToPet()->GetOwner())
                        amount += owner->ToPlayer()->GetHealthBonusFromStamina() * 0.75f;
        }

        void CalculateAttackPowerAmount(AuraEffect const* /* aurEff */, int32& amount, bool& /*canBeRecalculated*/)
        {
            if (Unit* pet = GetUnitOwner())
                if (pet->isPet())
                    if (Unit* owner = pet->ToPet()->GetOwner())
                    {
                        int32 fire  = int32(owner->GetUInt32Value(PLAYER_FIELD_MOD_DAMAGE_DONE_POS + SPELL_SCHOOL_FIRE))
                            - owner->GetUInt32Value(PLAYER_FIELD_MOD_DAMAGE_DONE_NEG + SPELL_SCHOOL_FIRE);
                        int32 shadow = int32(owner->GetUInt32Value(PLAYER_FIELD_MOD_DAMAGE_DONE_POS + SPELL_SCHOOL_SHADOW))
                            - owner->GetUInt32Value(PLAYER_FIELD_MOD_DAMAGE_DONE_NEG + SPELL_SCHOOL_SHADOW);
                        int32 maximum  = (fire > shadow) ? fire : shadow;
                        if (maximum < 0)
                            maximum = 0;
                        float bonusAP = maximum * 0.57f;
                        amount += bonusAP;
                        // Glyph of felguard
                        if (pet->GetEntry() == ENTRY_FELGUARD)
                        {
                            if (AuraEffect* ect = owner->GetAuraEffect(56246, EFFECT_0))
                            {
                                float base_attPower = pet->GetModifierValue(UNIT_MOD_ATTACK_POWER, BASE_VALUE) * pet->GetModifierValue(UNIT_MOD_ATTACK_POWER, BASE_PCT);
                                amount += CalculatePct(amount+base_attPower, ect->GetAmount());
                            }
                        }
                    }
        }

        void CalculateDamageDoneAmount(AuraEffect const* /* aurEff */, int32& amount, bool& /*canBeRecalculated*/)
        {
            if (Unit* pet = GetUnitOwner())
                if (pet->isPet())
                    if (Unit* owner = pet->ToPet()->GetOwner())
                    {
                        //the damage bonus used for pets is either fire or shadow damage, whatever is higher
                        int32 fire  = int32(owner->GetUInt32Value(PLAYER_FIELD_MOD_DAMAGE_DONE_POS + SPELL_SCHOOL_FIRE))
                            - owner->GetUInt32Value(PLAYER_FIELD_MOD_DAMAGE_DONE_NEG + SPELL_SCHOOL_FIRE);
                        int32 shadow = int32(owner->GetUInt32Value(PLAYER_FIELD_MOD_DAMAGE_DONE_POS + SPELL_SCHOOL_SHADOW))
                            - owner->GetUInt32Value(PLAYER_FIELD_MOD_DAMAGE_DONE_NEG + SPELL_SCHOOL_SHADOW);
                        int32 maximum  = (fire > shadow) ? fire : shadow;
                        float bonusDamage = 0.0f;
                        if (maximum > 0)
                            bonusDamage = maximum * 0.15f;
                        amount += bonusDamage;
                    }
        }

        void Register()
        {
            OnEffectRemove += AuraEffectRemoveFn(spell_warl_pet_scaling_01_AuraScript::RemoveEffect, EFFECT_0, SPELL_AURA_MOD_STAT, AURA_EFFECT_HANDLE_CHANGE_AMOUNT_MASK);
            AfterEffectApply += AuraEffectApplyFn(spell_warl_pet_scaling_01_AuraScript::ApplyEffect, EFFECT_0, SPELL_AURA_MOD_STAT, AURA_EFFECT_HANDLE_CHANGE_AMOUNT_MASK);
            DoEffectCalcAmount += AuraEffectCalcAmountFn(spell_warl_pet_scaling_01_AuraScript::CalculateMaxHealthAmount, EFFECT_0, SPELL_AURA_MOD_MAX_HEALTH);
            DoEffectCalcAmount += AuraEffectCalcAmountFn(spell_warl_pet_scaling_01_AuraScript::CalculateAttackPowerAmount, EFFECT_1, SPELL_AURA_MOD_ATTACK_POWER);
            DoEffectCalcAmount += AuraEffectCalcAmountFn(spell_warl_pet_scaling_01_AuraScript::CalculateDamageDoneAmount, EFFECT_2, SPELL_AURA_MOD_DAMAGE_DONE);
        }

    private:
        uint32 _tempHealth;
    };

    AuraScript* GetAuraScript() const
    {
        return new spell_warl_pet_scaling_01_AuraScript();
    }
};

class spell_warl_pet_scaling_02 : public SpellScriptLoader
{
public:
    spell_warl_pet_scaling_02() : SpellScriptLoader("spell_warl_pet_scaling_02") { }

    class spell_warl_pet_scaling_02_AuraScript : public AuraScript
    {
        PrepareAuraScript(spell_warl_pet_scaling_02_AuraScript);

        bool Load()
        {
            if (!GetCaster() || !GetCaster()->GetOwner() || GetCaster()->GetOwner()->GetTypeId() != TYPEID_PLAYER)
                return false;
            _tempMana = 0;
            return true;
        }

        void ApplyEffect(AuraEffect const* /* aurEff */, AuraEffectHandleModes /*mode*/)
        {
            if (Unit* pet = GetUnitOwner())
                if (_tempMana)
                    pet->SetPower(POWER_MANA, _tempMana);
        }

        void RemoveEffect(AuraEffect const* /* aurEff */, AuraEffectHandleModes /*mode*/)
        {
            if (Unit* pet = GetUnitOwner())
                _tempMana = pet->GetPower(POWER_MANA);
        }

        void CalculateEnergyAmount(AuraEffect const* /* aurEff */, int32& amount, bool& /*canBeRecalculated*/)
        {
            if (Unit* pet = GetUnitOwner())
                if (pet->isPet())
                    if (Unit* owner = pet->ToPet()->GetOwner())
                    {
                        float ownerBonus = 0.0f;
                        float multiplicator = 15.0f;

                        ownerBonus = CalculatePct(owner->GetStat(STAT_INTELLECT), 30);
                        switch (pet->ToCreature()->GetEntry())
                        {
                            case ENTRY_IMP:
                                multiplicator = 4.95f;
                                // Burning Embers
                                if (AuraEffect* aurEff = owner->GetAuraEffect(SPELL_AURA_DUMMY, SPELLFAMILY_WARLOCK, 5116, 0)) 
                                    owner->AddAura(aurEff->GetSpellInfo()->Id, pet);
                                break;
                            case ENTRY_VOIDWALKER:
                            case ENTRY_SUCCUBUS:
                            case ENTRY_FELHUNTER:
                            case ENTRY_FELGUARD:
                                multiplicator = 11.5f;
                                break;
                            default:
                                multiplicator = 15.0f;
                                break;
                        }
                        amount += int32(ownerBonus * multiplicator);
                    }
        }

        void CalculateArmorAmount(AuraEffect const* /* aurEff */, int32& amount, bool& /*canBeRecalculated*/)
        {
            if (Unit* pet = GetUnitOwner())
                if (pet->isPet())
                    if (Unit* owner = pet->ToPet()->GetOwner())
                    {
                        float ownerBonus = 0.0f;
                        ownerBonus = CalculatePct(owner->GetArmor(), 35);
                        amount += ownerBonus;
                    }
        }

        void CalculateFireResistanceAmount(AuraEffect const* /* aurEff */, int32& amount, bool& /*canBeRecalculated*/)
        {
            if (Unit* pet = GetUnitOwner())
                if (pet->isPet())
                    if (Unit* owner = pet->ToPet()->GetOwner())
                    {
                        float ownerBonus = 0.0f;
                        ownerBonus = CalculatePct(owner->GetResistance(SPELL_SCHOOL_FIRE), 40);
                        amount += ownerBonus;
                    }
        }

        void Register()
        {
            OnEffectRemove += AuraEffectRemoveFn(spell_warl_pet_scaling_02_AuraScript::RemoveEffect, EFFECT_0, SPELL_AURA_MOD_STAT, AURA_EFFECT_HANDLE_CHANGE_AMOUNT_MASK);
            AfterEffectApply += AuraEffectApplyFn(spell_warl_pet_scaling_02_AuraScript::ApplyEffect, EFFECT_0, SPELL_AURA_MOD_STAT, AURA_EFFECT_HANDLE_CHANGE_AMOUNT_MASK);
            DoEffectCalcAmount += AuraEffectCalcAmountFn(spell_warl_pet_scaling_02_AuraScript::CalculateEnergyAmount, EFFECT_0, SPELL_AURA_MOD_INCREASE_ENERGY);
            DoEffectCalcAmount += AuraEffectCalcAmountFn(spell_warl_pet_scaling_02_AuraScript::CalculateArmorAmount, EFFECT_1, SPELL_AURA_MOD_RESISTANCE);
            DoEffectCalcAmount += AuraEffectCalcAmountFn(spell_warl_pet_scaling_02_AuraScript::CalculateFireResistanceAmount, EFFECT_2, SPELL_AURA_MOD_RESISTANCE);
        }

    private:
        uint32 _tempMana;
    };

    AuraScript* GetAuraScript() const
    {
        return new spell_warl_pet_scaling_02_AuraScript();
    }
};


class spell_warl_pet_scaling_03 : public SpellScriptLoader
{
public:
    spell_warl_pet_scaling_03() : SpellScriptLoader("spell_warl_pet_scaling_03") { }

    class spell_warl_pet_scaling_03_AuraScript : public AuraScript
    {
        PrepareAuraScript(spell_warl_pet_scaling_03_AuraScript);

        bool Load()
        {
            if (!GetCaster() || !GetCaster()->GetOwner() || GetCaster()->GetOwner()->GetTypeId() != TYPEID_PLAYER)
                return false;
            return true;
        }

        void CalculateFrostResistanceAmount(AuraEffect const* /* aurEff */, int32& amount, bool& /*canBeRecalculated*/)
        {
            if (Unit* pet = GetUnitOwner())
                if (pet->isPet())
                    if (Unit* owner = pet->ToPet()->GetOwner())
                    {
                        float ownerBonus = 0.0f;
                        ownerBonus = CalculatePct(owner->GetResistance(SPELL_SCHOOL_FROST), 40);
                        amount += ownerBonus;
                    }
        }

        void CalculateArcaneResistanceAmount(AuraEffect const* /* aurEff */, int32& amount, bool& /*canBeRecalculated*/)
        {
            if (Unit* pet = GetUnitOwner())
                if (pet->isPet())
                    if (Unit* owner = pet->ToPet()->GetOwner())
                    {
                        float ownerBonus = 0.0f;
                        ownerBonus = CalculatePct(owner->GetResistance(SPELL_SCHOOL_ARCANE), 40);
                        amount += ownerBonus;
                    }
        }

        void CalculateNatureResistanceAmount(AuraEffect const* /* aurEff */, int32& amount, bool& /*canBeRecalculated*/)
        {
            if (Unit* pet = GetUnitOwner())
                if (pet->isPet())
                    if (Unit* owner = pet->ToPet()->GetOwner())
                    {
                        float ownerBonus = 0.0f;
                        ownerBonus = CalculatePct(owner->GetResistance(SPELL_SCHOOL_NATURE), 40);
                        amount += ownerBonus;
                    }
        }

        void Register()
        {
            DoEffectCalcAmount += AuraEffectCalcAmountFn(spell_warl_pet_scaling_03_AuraScript::CalculateFrostResistanceAmount, EFFECT_0, SPELL_AURA_MOD_RESISTANCE);
            DoEffectCalcAmount += AuraEffectCalcAmountFn(spell_warl_pet_scaling_03_AuraScript::CalculateArcaneResistanceAmount, EFFECT_1, SPELL_AURA_MOD_RESISTANCE);
            DoEffectCalcAmount += AuraEffectCalcAmountFn(spell_warl_pet_scaling_03_AuraScript::CalculateNatureResistanceAmount, EFFECT_2, SPELL_AURA_MOD_RESISTANCE);
        }
    };

    AuraScript* GetAuraScript() const
    {
        return new spell_warl_pet_scaling_03_AuraScript();
    }
};



class spell_warl_pet_scaling_04 : public SpellScriptLoader
{
public:
    spell_warl_pet_scaling_04() : SpellScriptLoader("spell_warl_pet_scaling_04") { }

    class spell_warl_pet_scaling_04_AuraScript : public AuraScript
    {
        PrepareAuraScript(spell_warl_pet_scaling_04_AuraScript);

        bool Load()
        {
            if (!GetCaster() || !GetCaster()->GetOwner() || GetCaster()->GetOwner()->GetTypeId() != TYPEID_PLAYER)
                return false;
            return true;
        }

        void CalculateShadowResistanceAmount(AuraEffect const* /* aurEff */, int32& amount, bool& /*canBeRecalculated*/)
        {
            if (Unit* pet = GetUnitOwner())
                if (pet->isPet())
                    if (Unit* owner = pet->ToPet()->GetOwner())
                    {
                        float ownerBonus = 0.0f;
                        ownerBonus = CalculatePct(owner->GetResistance(SPELL_SCHOOL_SHADOW), 40);
                        amount += ownerBonus;
                    }
        }

        void Register()
        {
            DoEffectCalcAmount += AuraEffectCalcAmountFn(spell_warl_pet_scaling_04_AuraScript::CalculateShadowResistanceAmount, EFFECT_0, SPELL_AURA_MOD_RESISTANCE);
        }
    };

    AuraScript* GetAuraScript() const
    {
        return new spell_warl_pet_scaling_04_AuraScript();
    }
};

class spell_warl_pet_scaling_05 : public SpellScriptLoader
{
public:
    spell_warl_pet_scaling_05() : SpellScriptLoader("spell_warl_pet_scaling_05") { }

    class spell_warl_pet_scaling_05_AuraScript : public AuraScript
    {
        PrepareAuraScript(spell_warl_pet_scaling_05_AuraScript);

        bool Load()
        {
            if (!GetCaster() || !GetCaster()->GetOwner() || GetCaster()->GetOwner()->GetTypeId() != TYPEID_PLAYER)
                return false;
            return true;
        }

        void CalculateAmountMeleeHit(AuraEffect const* /* aurEff */, int32& amount, bool& /*canBeRecalculated*/)
        {
            if (Player* owner = GetCaster()->GetOwner()->ToPlayer())
            {
                float HitMelee = 0.0f;
                HitMelee += owner->GetTotalAuraModifier(SPELL_AURA_MOD_SPELL_HIT_CHANCE);
                HitMelee += owner->GetRatingBonusValue(CR_HIT_SPELL);
                amount += int32(HitMelee);
            }
        }

        void CalculateAmountSpellHit(AuraEffect const* /* aurEff */, int32& amount, bool& /*canBeRecalculated*/)
        {
            if (Player* owner = GetCaster()->GetOwner()->ToPlayer())
            {
                float HitSpell = 0.0f;
                HitSpell += owner->GetTotalAuraModifier(SPELL_AURA_MOD_SPELL_HIT_CHANCE);
                HitSpell += owner->GetRatingBonusValue(CR_HIT_SPELL);
                amount += int32(HitSpell);
            }
        }

        void CalculateAmountExpertise(AuraEffect const* /* aurEff */, int32& amount, bool& /*canBeRecalculated*/)
        {
            if (Player* owner = GetCaster()->GetOwner()->ToPlayer())
            {
                float Expertise = 0.0f;
                Expertise += owner->GetTotalAuraModifier(SPELL_AURA_MOD_SPELL_HIT_CHANCE);
                Expertise += owner->GetRatingBonusValue(CR_HIT_SPELL);
                amount += int32(Expertise);
            }
        }

        void Register()
        {
            DoEffectCalcAmount += AuraEffectCalcAmountFn(spell_warl_pet_scaling_05_AuraScript::CalculateAmountMeleeHit, EFFECT_0, SPELL_AURA_MOD_HIT_CHANCE);
            DoEffectCalcAmount += AuraEffectCalcAmountFn(spell_warl_pet_scaling_05_AuraScript::CalculateAmountSpellHit, EFFECT_1, SPELL_AURA_MOD_SPELL_HIT_CHANCE);
            DoEffectCalcAmount += AuraEffectCalcAmountFn(spell_warl_pet_scaling_05_AuraScript::CalculateAmountExpertise, EFFECT_2, SPELL_AURA_MOD_EXPERTISE);
        }
    };

    AuraScript* GetAuraScript() const
    {
        return new spell_warl_pet_scaling_05_AuraScript();
    }
};

class spell_warl_pet_passive : public SpellScriptLoader
{
public:
    spell_warl_pet_passive() : SpellScriptLoader("spell_warl_pet_passive") { }

    class spell_warl_pet_passive_AuraScript : public AuraScript
    {
        PrepareAuraScript(spell_warl_pet_passive_AuraScript);

        bool Load()
        {
            if (!GetCaster() || !GetCaster()->GetOwner() || GetCaster()->GetOwner()->GetTypeId() != TYPEID_PLAYER)
                return false;
            return true;
        }

        void CalculateAmountCritSpell(AuraEffect const* /* aurEff */, int32& amount, bool& /*canBeRecalculated*/)
        {
            if (Player* owner = GetCaster()->GetOwner()->ToPlayer())
            {
                float CritSpell = 0.0f;
                CritSpell += owner->GetSpellCritFromIntellect();
                CritSpell += owner->GetTotalAuraModifier(SPELL_AURA_MOD_SPELL_CRIT_CHANCE);
                CritSpell += owner->GetTotalAuraModifier(SPELL_AURA_MOD_CRIT_PCT);
                CritSpell += owner->GetRatingBonusValue(CR_CRIT_SPELL);
                if (AuraApplication* improvedDemonicTacticsApp = owner->GetAuraApplicationOfRankedSpell(54347))
                    if (Aura* improvedDemonicTactics = improvedDemonicTacticsApp->GetBase())
                        if (AuraEffect* improvedDemonicTacticsEffect = improvedDemonicTactics->GetEffect(EFFECT_0))
                            amount += CalculatePct(CritSpell, improvedDemonicTacticsEffect->GetAmount());
            }
        }

        void CalculateAmountCritMelee(AuraEffect const* /* aurEff */, int32& amount, bool& /*canBeRecalculated*/)
        {
            if (Player* owner = GetCaster()->GetOwner()->ToPlayer())
            {
                float CritMelee = 0.0f;
                CritMelee += owner->GetMeleeCritFromAgility();
                CritMelee += owner->GetTotalAuraModifier(SPELL_AURA_MOD_WEAPON_CRIT_PERCENT);
                CritMelee += owner->GetTotalAuraModifier(SPELL_AURA_MOD_CRIT_PCT);
                CritMelee += owner->GetRatingBonusValue(CR_CRIT_MELEE);
                if (AuraApplication* improvedDemonicTacticsApp = owner->GetAuraApplicationOfRankedSpell(54347))
                    if (Aura* improvedDemonicTactics = improvedDemonicTacticsApp->GetBase())
                        if (AuraEffect* improvedDemonicTacticsEffect = improvedDemonicTactics->GetEffect(EFFECT_0))
                            amount += CalculatePct(CritMelee, improvedDemonicTacticsEffect->GetAmount());
            }
        }

        void Register()
        {
            DoEffectCalcAmount += AuraEffectCalcAmountFn(spell_warl_pet_passive_AuraScript::CalculateAmountCritSpell, EFFECT_0, SPELL_AURA_MOD_SPELL_CRIT_CHANCE);
            DoEffectCalcAmount += AuraEffectCalcAmountFn(spell_warl_pet_passive_AuraScript::CalculateAmountCritMelee, EFFECT_1, SPELL_AURA_MOD_WEAPON_CRIT_PERCENT);
        }
    };

    AuraScript* GetAuraScript() const
    {
        return new spell_warl_pet_passive_AuraScript();
    }
};

class spell_warl_pet_passive_damage_done : public SpellScriptLoader
{
public:
    spell_warl_pet_passive_damage_done() : SpellScriptLoader("spell_warl_pet_passive_damage_done") { }

    class spell_warl_pet_passive_damage_done_AuraScript : public AuraScript
    {
        PrepareAuraScript(spell_warl_pet_passive_damage_done_AuraScript);

        bool Load()
        {
            if (!GetCaster() || !GetCaster()->GetOwner() || GetCaster()->GetOwner()->GetTypeId() != TYPEID_PLAYER)
                return false;
            return true;
        }

        void CalculateAmountDamageDone(AuraEffect const* /* aurEff */, int32& amount, bool& /*canBeRecalculated*/)
        {
            if (!GetCaster() || !GetCaster()->GetOwner())
                return;
            if (GetCaster()->GetOwner()->ToPlayer())
            {
                switch (GetCaster()->GetEntry())
                {
                    case ENTRY_VOIDWALKER:
                        amount += -16;
                        break;
                    case ENTRY_SUCCUBUS:
                    case ENTRY_FELGUARD:
                        amount += 5;
                        break;
                }
            }
        }

        void Register()
        {
            DoEffectCalcAmount += AuraEffectCalcAmountFn(spell_warl_pet_passive_damage_done_AuraScript::CalculateAmountDamageDone, EFFECT_0, SPELL_AURA_MOD_DAMAGE_PERCENT_DONE);
            DoEffectCalcAmount += AuraEffectCalcAmountFn(spell_warl_pet_passive_damage_done_AuraScript::CalculateAmountDamageDone, EFFECT_1, SPELL_AURA_MOD_DAMAGE_PERCENT_DONE);
        }
    };

    AuraScript* GetAuraScript() const
    {
        return new spell_warl_pet_passive_damage_done_AuraScript();
    }
};

class spell_warl_pet_passive_voidwalker : public SpellScriptLoader
{
public:
    spell_warl_pet_passive_voidwalker() : SpellScriptLoader("spell_warl_pet_passive_voidwalker") { }

    class spell_warl_pet_passive_voidwalker_AuraScript : public AuraScript
    {
        PrepareAuraScript(spell_warl_pet_passive_voidwalker_AuraScript);

        bool Load()
        {
            if (!GetCaster() || !GetCaster()->GetOwner() || GetCaster()->GetOwner()->GetTypeId() != TYPEID_PLAYER)
                return false;
            return true;
        }

        void CalculateAmount(AuraEffect const* /* aurEff */, int32& amount, bool& /*canBeRecalculated*/)
        {
            if (Unit* pet = GetUnitOwner())
                if (pet->isPet())
                    if (Unit* owner = pet->ToPet()->GetOwner())
                        if (AuraEffect* ect = owner->GetAuraEffect(SPELL_WARLOCK_GLYPH_OF_VOIDWALKER, EFFECT_0))
                            amount += ect->GetAmount();
        }

        void Register()
        {
            DoEffectCalcAmount += AuraEffectCalcAmountFn(spell_warl_pet_passive_voidwalker_AuraScript::CalculateAmount, EFFECT_0, SPELL_AURA_MOD_TOTAL_STAT_PERCENTAGE);
        }
    };

    AuraScript* GetAuraScript() const
    {
        return new spell_warl_pet_passive_voidwalker_AuraScript();
    }
};
/*END PET SCALING*/

class spell_warl_shadowburn : public SpellScriptLoader
{
    public:
        spell_warl_shadowburn() : SpellScriptLoader("spell_warl_shadowburn") { }

        class spell_warl_shadowburn_SpellScript : public SpellScript
        {
            PrepareSpellScript(spell_warl_shadowburn_SpellScript);

            void HandleDamage()
            {
                Unit* caster = GetCaster();
                if (caster->GetTypeId() != TYPEID_PLAYER)
                    return;

                Unit* target = GetHitUnit();
                int32 damage = GetHitDamage();
                // Glyph cooldown reset when target was failed to kill
                if (AuraEffect* glyph = caster->GetDummyAuraEffect(SPELLFAMILY_WARLOCK, 1590, EFFECT_0))
                    if (target && target->GetHealthPct() <= glyph->GetAmount() && !caster->HasAura(91001))
                        if (int32(target->GetHealth()) > damage)
                        {
                            caster->ToPlayer()->RemoveSpellCooldown(17877, true);
                            caster->CastSpell(caster, 91001, true);
                        }

            }

            void Register()
            {
                OnHit += SpellHitFn(spell_warl_shadowburn_SpellScript::HandleDamage);
            }
        };

        SpellScript* GetSpellScript() const
        {
            return new spell_warl_shadowburn_SpellScript();
        }
};

class spell_warl_demon_armor : public SpellScriptLoader
{
    public:
        spell_warl_demon_armor() : SpellScriptLoader("spell_warl_demon_armor") { }

        class spell_warl_demon_armor_AuraScript : public AuraScript
        {
            PrepareAuraScript(spell_warl_demon_armor_AuraScript);

            bool Validate(SpellInfo const* /*spellInfo*/)
            {
                if (!sSpellMgr->GetSpellInfo(SPELL_WARLOCK_SHADOW_WARD))
                    return false;
                if (!sSpellMgr->GetSpellInfo(SPELL_WARLOCK_NETHER_WARD))
                    return false;
                if (!sSpellMgr->GetSpellInfo(SPELL_WARLOCK_NETHER_WARD_TALENT))
                    return false;

                return true;
            }

            void CalculateAmount(AuraEffect const* /*aurEff*/, int32& amount, bool& canBeRecalculated)
            {
                canBeRecalculated = false;
                if (Unit* caster = GetCaster())
                {
                    if (caster->HasAura(SPELL_WARLOCK_NETHER_WARD_TALENT)) // Nether Ward
                        amount = SPELL_WARLOCK_NETHER_WARD;
                    else
                        amount = SPELL_WARLOCK_SHADOW_WARD;
                }
                else
                    amount = SPELL_WARLOCK_SHADOW_WARD;
            }

            void Register()
            {
                DoEffectCalcAmount += AuraEffectCalcAmountFn(spell_warl_demon_armor_AuraScript::CalculateAmount, EFFECT_2, SPELL_AURA_OVERRIDE_ACTIONBAR_SPELLS);
            }
        };

        AuraScript* GetAuraScript() const
        {
            return new spell_warl_demon_armor_AuraScript();
        }
};

class spell_warl_soul_link : public SpellScriptLoader
{
    public:
        spell_warl_soul_link() : SpellScriptLoader("spell_warl_soul_link") { }

        class spell_warl_soul_link_AuraScript : public AuraScript
        {
            PrepareAuraScript(spell_warl_soul_link_AuraScript);

            void CalculateAmount(AuraEffect const* aurEff, int32& amount, bool& canBeRecalculated)
            {
                if (Unit* caster = GetCaster())
                    if (Unit* owner = caster->GetOwner())
                        if (AuraEffect* glyph = owner->GetDummyAuraEffect(SPELLFAMILY_WARLOCK, 173, EFFECT_0))
                            amount += glyph->GetAmount();
            }

            void Register()
            {
                DoEffectCalcAmount += AuraEffectCalcAmountFn(spell_warl_soul_link_AuraScript::CalculateAmount, EFFECT_0, SPELL_AURA_SPLIT_DAMAGE_PCT);
            }
        };

        AuraScript* GetAuraScript() const
        {
            return new spell_warl_soul_link_AuraScript();
        }
};

class spell_warl_shadowburn_aura : public SpellScriptLoader
{
    public:
        spell_warl_shadowburn_aura() : SpellScriptLoader("spell_warl_shadowburn_aura") { }

        class spell_warl_shadowburn_aura_AuraScript : public AuraScript
        {
            PrepareAuraScript(spell_warl_shadowburn_aura_AuraScript);

            bool Validate(SpellInfo const* /*spellInfo*/)
            {
                if (!sSpellMgr->GetSpellInfo(SPELL_WARLOCK_SOULSHARD_ADD))
                    return false;

                return true;
            }

            void RemoveEffect(AuraEffect const* aurEff, AuraEffectHandleModes /*mode*/)
            {
                if (GetTargetApplication()->GetRemoveMode() == AURA_REMOVE_BY_DEATH)
                    if (Player *caster = GetCaster()->ToPlayer())
                        if (Unit *target = GetTarget())
                            if (caster->isHonorOrXPTarget(target))
                            {
                                int32 basepoints = aurEff->GetAmount();
                                caster->CastCustomSpell(caster, SPELL_WARLOCK_SOULSHARD_ADD, &basepoints, NULL, NULL, true, NULL, aurEff);
                            }
            }

            void Register()
            {
                OnEffectRemove += AuraEffectRemoveFn(spell_warl_shadowburn_aura_AuraScript::RemoveEffect, EFFECT_0, SPELL_AURA_DUMMY, AURA_EFFECT_HANDLE_REAL);
            }
        };

        AuraScript* GetAuraScript() const
        {
            return new spell_warl_shadowburn_aura_AuraScript();
        }
};

class spell_warl_pet_damage_spells : public SpellScriptLoader
{
    public:
        spell_warl_pet_damage_spells() : SpellScriptLoader("spell_warl_pet_damage_spells") { }

        class spell_warl_pet_damage_spells_SpellScript : public SpellScript
        {
            PrepareSpellScript(spell_warl_pet_damage_spells_SpellScript);

            void HandleHit(SpellEffIndex /*effIndex*/)
            {
                if (Unit* owner = GetCaster()->GetOwner())
                {
                    float coeff = 1.0f;
                    switch (GetSpellInfo()->Id)
                    {
                        // Firebolt
                        case 3110:
                            coeff = 0.657f;
                            break;
                        // Shadow bite
                        case 54049:
                            coeff = 1.228f;
                            break;
                        // Whiplash
                        case 6360:
                            coeff = 0.85f;
                            break;
                        // Lash of pain
                        case 7814:
                            coeff = 0.612f;
                            break;
                    }
                    int32 baseDamage = GetEffectValue() + (coeff * owner->SpellBaseDamageBonusDone(GetSpellInfo()->GetSchoolMask()) * 0.5f);
                    SetEffectDamage(baseDamage);
                }
            }

            void Register()
            {
                OnEffectLaunchTarget += SpellEffectFn(spell_warl_pet_damage_spells_SpellScript::HandleHit, EFFECT_0, SPELL_EFFECT_SCHOOL_DAMAGE);
            }
        };

        SpellScript* GetSpellScript() const
        {
            return new spell_warl_pet_damage_spells_SpellScript();
        }
};

// Fear
class spell_warl_fear: public SpellScriptLoader
{
    public:
        spell_warl_fear() : SpellScriptLoader("spell_warl_fear") { }

        class spell_warl_fear_SpellScript : public SpellScript
        {
            PrepareSpellScript(spell_warl_fear_SpellScript);

            void HandleOnHit()
            {
                if (!GetHitUnit())
                    return;

                AuraEffect* glyph = GetCaster()->GetAuraEffect(56244, EFFECT_0);
                if (Aura* debuff = GetHitAura())
                {
                    if (debuff->GetApplicationOfTarget(GetHitUnit()->GetGUID()))
                    {
                        // Remove root effect
                        if (AuraEffect* root = debuff->GetEffect(EFFECT_2))
                            if (!glyph)
                                root->HandleEffect(GetHitUnit(), AURA_REMOVE_BY_DEFAULT, false);
                    }
                }

                if (glyph)
                    GetCaster()->CastSpell(GetCaster(), 91168);
            }

            void Register()
            {
                OnHit += SpellHitFn(spell_warl_fear_SpellScript::HandleOnHit);
            }
        };

        SpellScript* GetSpellScript() const
        {
            return new spell_warl_fear_SpellScript();
        }
};

// Devour Magic
class spell_warl_devour: public SpellScriptLoader
{
    public:
        spell_warl_devour() : SpellScriptLoader("spell_warl_devour") { }

        class spell_warl_devour_SpellScript : public SpellScript
        {
            PrepareSpellScript(spell_warl_devour_SpellScript);

            void HandleOnDispel()
            {
                Unit* caster = GetCaster();
                if (Unit* owner = caster->GetOwner())
                {
                    SpellInfo const* spellInfo = sSpellMgr->GetSpellInfo(19658); 
                    int32 heal_amount = spellInfo->Effects[EFFECT_0].CalcValue(caster) + (owner->SpellBaseDamageBonusDone(spellInfo->GetSchoolMask()) * 0.5f * 0.3f);
                    caster->CastCustomSpell(caster, 19658, &heal_amount, NULL, NULL, true);
                    // Glyph of Felhunter
                    if (owner->GetAura(56249))
                        owner->CastCustomSpell(owner, 19658, &heal_amount, NULL, NULL, true);
                }
            }

            void Register()
            {
                OnSuccessfulDispel += SpellDispelFn(spell_warl_devour_SpellScript::HandleOnDispel);
            }
        };

        SpellScript* GetSpellScript() const
        {
            return new spell_warl_devour_SpellScript();
        }
};

void AddSC_warlock_spell_scripts()
{
    new spell_warl_bane_of_doom();
    new spell_warl_banish();
    new spell_warl_curse_of_weakness();
    new spell_warl_create_healthstone();
    new spell_warl_demonic_circle_summon();
    new spell_warl_demonic_circle_teleport();
    new spell_warl_demonic_empowerment();
    new spell_warl_demon_soul();
    new spell_warl_everlasting_affliction();
    new spell_warl_fel_flame();
    new spell_warl_fel_synergy();
    new spell_warl_haunt();
    new spell_warl_health_funnel();
    new spell_warl_life_tap();
    new spell_warl_seed_of_corruption();
    new spell_warl_soulshatter();
    new spell_warl_unstable_affliction();
    new spell_warl_curse_of_the_elements();
    new spell_warl_jinx_coe();
    new spell_warl_drain_life();
    new spell_warl_soul_swap();
    new spell_warl_soul_swap_exhale();
    new spell_warl_soul_swap_override();
    new spell_warl_soulburn();
    new spell_warl_soulburn_seed_of_corruption();
    new spell_warl_pandemic();
    new spell_warl_drain_soul();
    new spell_warl_soulburn_healthstone();
    new spell_warl_searing_pain();
    new spell_warl_soul_harvest();
    new spell_warl_fel_armor();
    new spell_warl_mana_feed();
    new spell_warl_bane_of_havoc();
    new spell_warl_impending_doom();
    new spell_warl_cremation();
    new spell_warl_pet_scaling_01();
    new spell_warl_pet_scaling_02();
    new spell_warl_pet_scaling_03();
    new spell_warl_pet_scaling_04();
    new spell_warl_pet_scaling_05();
    new spell_warl_pet_passive();
    new spell_warl_pet_passive_damage_done();
    new spell_warl_pet_passive_voidwalker();
    new spell_warl_demon_armor();
    new spell_warl_soulfire();
    new spell_warl_rain_of_fire();
    new spell_warl_seduction();
    new spell_warl_shadowburn();
    new spell_warl_soul_link();
    new spell_warl_shadowburn_aura();
    new spell_warl_pet_damage_spells();
    new spell_warl_wards();
    new spell_warl_fear();
    new spell_warl_devour();
    new spell_warl_soul_stone();
}
