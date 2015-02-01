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
 * Scripts for spells with SPELLFAMILY_DEATHKNIGHT and SPELLFAMILY_GENERIC spells used by deathknight players.
 * Ordered alphabetically using scriptname.
 * Scriptnames of files in this file should be prefixed with "spell_dk_".
 */

#include "Player.h"
#include "Pet.h"
#include "ScriptMgr.h"
#include "SpellScript.h"
#include "SpellAuraEffects.h"

enum DeathKnightSpells
{
    SPELL_DK_ANTI_MAGIC_SHELL_TALENT            = 51052,
    SPELL_DK_BLOOD_BOIL_TRIGGERED               = 65658,
    SPELL_DK_BLOOD_GORGED_HEAL                  = 50454,
    SPELL_DK_CORPSE_EXPLOSION_TRIGGERED         = 43999,
    SPELL_DK_CORPSE_EXPLOSION_VISUAL            = 51270,
    SPELL_DK_DEATH_COIL_DAMAGE                  = 47632,
    SPELL_DK_DEATH_COIL_HEAL                    = 47633,
    SPELL_DK_DEATH_STRIKE_HEAL                  = 45470,
    SPELL_DK_GHOUL_EXPLODE                      = 47496,
    SPELL_DK_GLYPH_OF_ICEBOUND_FORTITUDE        = 58625,
    SPELL_DK_RUNIC_POWER_ENERGIZE               = 49088,
    SPELL_DK_SCOURGE_STRIKE_TRIGGERED           = 70890,
    SPELL_DK_WILL_OF_THE_NECROPOLIS_TALENT_R1   = 49189,
    SPELL_DK_WILL_OF_THE_NECROPOLIS_AURA_R1     = 52284,
    SPELL_DK_UNHOLY_PRESENCE                    = 48265,
    SPELL_DK_IMPROVED_UNHOLY_PRESENCE_TRIGGERED = 63622,
    SPELL_DK_ITEM_SIGIL_VENGEFUL_HEART          = 64962,
    SPELL_DK_ITEM_T8_MELEE_4P_BONUS             = 64736,
    SPELL_DK_GLYPH_OF_SCOURGE_STRIKE            = 58642
};

enum DeathKnightSpellIcons
{
    DK_ICON_ID_IMPROVED_DEATH_STRIKE            = 2751
};

enum DKPetCalculate
{
    SPELL_DEATH_KNIGHT_RUNE_WEAPON_02   = 51906,
    SPELL_DEATH_KNIGHT_PET_SCALING_01   = 54566,
    SPELL_DEATH_KNIGHT_PET_SCALING_02   = 51996,
    SPELL_DEATH_KNIGHT_PET_SCALING_03   = 61697,
    SPELL_NIGHT_OF_THE_DEAD             = 55620,
    ENTRY_ARMY_OF_THE_DEAD_GHOUL        = 24207,
    SPELL_DEATH_KNIGHT_GLYPH_OF_GHOUL   = 58686,
};

// 50462 - Anti-Magic Shell (on raid member)
class spell_dk_anti_magic_shell_raid : public SpellScriptLoader
{
    public:
        spell_dk_anti_magic_shell_raid() : SpellScriptLoader("spell_dk_anti_magic_shell_raid") { }

        class spell_dk_anti_magic_shell_raid_AuraScript : public AuraScript
        {
            PrepareAuraScript(spell_dk_anti_magic_shell_raid_AuraScript);

            uint32 absorbPct;

            bool Load()
            {
                absorbPct = GetSpellInfo()->Effects[EFFECT_0].CalcValue(GetCaster());
                return true;
            }

            void CalculateAmount(AuraEffect const* /*aurEff*/, int32 & amount, bool & /*canBeRecalculated*/)
            {
                // TODO: this should absorb limited amount of damage, but no info on calculation formula
                amount = -1;
            }

            void Absorb(AuraEffect* /*aurEff*/, DamageInfo & dmgInfo, uint32 & absorbAmount)
            {
                absorbAmount = CalculatePct(dmgInfo.GetDamage(), absorbPct);
            }

            void Register()
            {
                DoEffectCalcAmount += AuraEffectCalcAmountFn(spell_dk_anti_magic_shell_raid_AuraScript::CalculateAmount, EFFECT_0, SPELL_AURA_SCHOOL_ABSORB);
                OnEffectAbsorb += AuraEffectAbsorbFn(spell_dk_anti_magic_shell_raid_AuraScript::Absorb, EFFECT_0);
            }
        };

        AuraScript* GetAuraScript() const
        {
            return new spell_dk_anti_magic_shell_raid_AuraScript();
        }
};

// Necrotic strike
class spell_dk_necrotic : public SpellScriptLoader
{
    public:
        spell_dk_necrotic() : SpellScriptLoader("spell_dk_necrotic") { }

        class spell_dk_necrotic_AuraScript : public AuraScript
        {
            PrepareAuraScript(spell_dk_necrotic_AuraScript);

            void CalculateAmount(AuraEffect const* /*aurEff*/, int32 & amount, bool & /*canBeRecalculated*/)
            {
                if (Unit* caster = GetCaster())
                {
                    int32 absorbAmount = caster->GetTotalAttackPowerValue(BASE_ATTACK) * 0.7f;
                    caster->ApplyResilience(GetUnitOwner(), &absorbAmount, false);
                    if (AuraEffect* necrotic = GetUnitOwner()->GetAuraEffect(73975, EFFECT_0, caster->GetGUID()))
                        absorbAmount += necrotic->GetAmount();
                    amount = int32(absorbAmount);
                }
            }

            void Register()
            {
                DoEffectCalcAmount += AuraEffectCalcAmountFn(spell_dk_necrotic_AuraScript::CalculateAmount, EFFECT_0, SPELL_AURA_SCHOOL_HEAL_ABSORB);
            }
        };

        AuraScript* GetAuraScript() const
        {
            return new spell_dk_necrotic_AuraScript();
        }
};

// Festering strike
class spell_dk_festering : public SpellScriptLoader
{
    public:
        spell_dk_festering() : SpellScriptLoader("spell_dk_festering") { }

        class spell_dk_festering_SpellScript : public SpellScript
        {
            PrepareSpellScript(spell_dk_festering_SpellScript);

            void OnHitTarget(SpellEffIndex /*effIndex*/)
            {
                Unit* caster = GetCaster();
                Unit* target = GetHitUnit();
                Aura* aura = NULL;
                int32 newDuration = GetSpellInfo()->Effects[EFFECT_2].BasePoints * 1000;
                // Increase chains of ice
                if (aura = target->GetAura(45524, caster->GetGUID()))
                    aura->SetDuration(std::min(newDuration + aura->GetDuration(), aura->GetMaxDuration()), false, true);
                // Increase Blood plague
                if (aura = target->GetAura(55078, caster->GetGUID()))
                    aura->SetDuration(std::min(newDuration + aura->GetDuration(), aura->GetMaxDuration()), false, true);
                // Increase frost fever
                if (aura = target->GetAura(55095, caster->GetGUID()))
                    aura->SetDuration(std::min(newDuration + aura->GetDuration(), aura->GetMaxDuration()), false, true);
                // Increase Ebon Plague
                if (aura = target->GetAura(65142, caster->GetGUID()))
                    aura->SetDuration(std::min(newDuration + aura->GetDuration(), aura->GetMaxDuration()), false, true);
            }

            void Register()
            {
                OnEffectHitTarget += SpellEffectFn(spell_dk_festering_SpellScript::OnHitTarget, EFFECT_2, SPELL_EFFECT_SCRIPT_EFFECT);
            }
        };

        SpellScript* GetSpellScript() const
        {
            return new spell_dk_festering_SpellScript;
        }
};

// Chilblains
class spell_dk_chilblains : public SpellScriptLoader
{
    public:
        spell_dk_chilblains() : SpellScriptLoader("spell_dk_chilblains") { }

        class spell_dk_chilblains_SpellScript : public SpellScript
        {
            PrepareSpellScript(spell_dk_chilblains_SpellScript);

            void OnHitTarget(SpellEffIndex /*effIndex*/)
            {
                Unit* caster = GetCaster();
                Unit* target = GetHitUnit();
                if (AuraEffect* chilblains = caster->GetAuraEffect(SPELL_AURA_ADD_TARGET_TRIGGER, SPELLFAMILY_DEATHKNIGHT, 143, EFFECT_0))
                {
                    uint32 triggeredId = 0;
                    switch (chilblains->GetId())
                    {
                        case 50040:
                            triggeredId = 96293;
                            break;
                        case 50041:
                            triggeredId = 96294;
                            break;
                    }
                    caster->CastSpell(target, triggeredId, true);
                }
            }

            void Register()
            {
                OnEffectHitTarget += SpellEffectFn(spell_dk_chilblains_SpellScript::OnHitTarget, EFFECT_0, SPELL_EFFECT_APPLY_AURA);
            }
        };

        SpellScript* GetSpellScript() const
        {
            return new spell_dk_chilblains_SpellScript;
        }
};

// Pillar of frost
class spell_dk_pillar_of_frost : public SpellScriptLoader
{
    public:
        spell_dk_pillar_of_frost() : SpellScriptLoader("spell_dk_pillar_of_frost") { }

        class spell_dk_pillar_of_frost_AuraScript : public AuraScript
        {
            PrepareAuraScript(spell_dk_pillar_of_frost_AuraScript);

            void HandleEffectApply(AuraEffect const* /*aurEff*/, AuraEffectHandleModes /*mode*/)
            {
                Unit* target = GetTarget();
                if (AuraEffect* glyph = target->GetDummyAuraEffect(SPELLFAMILY_DEATHKNIGHT, 4919, EFFECT_0))
                    target->CastSpell(target, 90259, true);
            }

            void HandleEffectRemove(AuraEffect const* /*aurEff*/, AuraEffectHandleModes /*mode*/)
            {
                GetTarget()->RemoveAurasDueToSpell(90259);
            }

            void Register()
            {
                AfterEffectApply += AuraEffectApplyFn(spell_dk_pillar_of_frost_AuraScript::HandleEffectApply, EFFECT_0, SPELL_AURA_MOD_TOTAL_STAT_PERCENTAGE, AURA_EFFECT_HANDLE_REAL_OR_REAPPLY_MASK);
                AfterEffectRemove += AuraEffectRemoveFn(spell_dk_pillar_of_frost_AuraScript::HandleEffectRemove, EFFECT_0, SPELL_AURA_MOD_TOTAL_STAT_PERCENTAGE, AURA_EFFECT_HANDLE_REAL_OR_REAPPLY_MASK);
            }
        };

        AuraScript* GetAuraScript() const
        {
            return new spell_dk_pillar_of_frost_AuraScript();
        }
};

// Strangulate
class spell_dk_strangulate : public SpellScriptLoader
{
    public:
        spell_dk_strangulate() : SpellScriptLoader("spell_dk_strangulate") { }

        class spell_dk_strangulate_SpellScript : public SpellScript
        {
            PrepareSpellScript(spell_dk_strangulate_SpellScript);

            bool Load()
            {
                wasCasting = false;
                return true;
            }

            void HandleBeforeHit()
            {
                if (GetHitUnit())
                    wasCasting = GetHitUnit()->IsNonMeleeSpellCasted(false, false, true);
            }

            void OnHitTarget(SpellEffIndex /*effIndex*/)
            {
                if (!wasCasting)
                    return;

                Unit* caster = GetCaster();
                Unit* target = GetHitUnit();
                if (AuraEffect* glyph = caster->GetDummyAuraEffect(SPELLFAMILY_DEATHKNIGHT, 2027, EFFECT_0))
                    if (Aura* strangulate = GetHitAura())
                    {
                        strangulate->SetMaxDuration(strangulate->GetDuration() + glyph->GetAmount());
                        strangulate->SetDuration(strangulate->GetDuration() + glyph->GetAmount());
                    }
            }

            void Register()
            {
                BeforeHit += SpellHitFn(spell_dk_strangulate_SpellScript::HandleBeforeHit);
                OnEffectHitTarget += SpellEffectFn(spell_dk_strangulate_SpellScript::OnHitTarget, EFFECT_0, SPELL_EFFECT_APPLY_AURA);
            }

            bool wasCasting;
        };

        SpellScript* GetSpellScript() const
        {
            return new spell_dk_strangulate_SpellScript;
        }
};

// 48707 - Anti-Magic Shell (on self)
class spell_dk_anti_magic_shell_self : public SpellScriptLoader
{
    public:
        spell_dk_anti_magic_shell_self() : SpellScriptLoader("spell_dk_anti_magic_shell_self") { }

        class spell_dk_anti_magic_shell_self_AuraScript : public AuraScript
        {
            PrepareAuraScript(spell_dk_anti_magic_shell_self_AuraScript);

            uint32 hpPct, totalAbsorb;
            bool Load()
            {
                hpPct = GetSpellInfo()->Effects[EFFECT_1].CalcValue(GetCaster());
                totalAbsorb = GetCaster()->CountPctFromMaxHealth(hpPct);
                return true;
            }

            void Absorb(AuraEffect* aurEff, DamageInfo & dmgInfo, uint32 & absorbAmount)
            {
                PreventDefaultAction();
                absorbAmount = std::min(CalculatePct(dmgInfo.GetDamage(), absorbAmount), GetTarget()->CountPctFromMaxHealth(hpPct));
                if (absorbAmount > totalAbsorb)
                {
                    absorbAmount = totalAbsorb;
                    aurEff->GetBase()->Remove();
                }
                else
                    totalAbsorb -= absorbAmount;

                dmgInfo.AbsorbDamage(absorbAmount);

                Unit* target = GetTarget();
                // Magic Suppression
                if (AuraEffect* aurEff = target->GetAuraEffectOfRankedSpell(49224, EFFECT_0))
                {
                    float damagePerRp = 540.0f;
                    switch (aurEff->GetId())
                    {
                        case 49610:
                            damagePerRp *= 0.66f;
                            break;
                        case 49611:
                            damagePerRp *= 0.33f;
                            break;
                    }

                    // Guessed
                    int32 energizeAmount = (absorbAmount / damagePerRp) * 10.0f;
                    target->CastCustomSpell(target, SPELL_DK_RUNIC_POWER_ENERGIZE, &energizeAmount, NULL, NULL, true, NULL, aurEff);
                }
            }

            void Register()
            {
                OnEffectAbsorb += AuraEffectAbsorbFn(spell_dk_anti_magic_shell_self_AuraScript::Absorb, EFFECT_0);
            }
        };

        AuraScript* GetAuraScript() const
        {
            return new spell_dk_anti_magic_shell_self_AuraScript();
        }
};

// 50461 - Anti-Magic Zone
class spell_dk_anti_magic_zone : public SpellScriptLoader
{
    public:
        spell_dk_anti_magic_zone() : SpellScriptLoader("spell_dk_anti_magic_zone") { }

        class spell_dk_anti_magic_zone_AuraScript : public AuraScript
        {
            PrepareAuraScript(spell_dk_anti_magic_zone_AuraScript);

            uint32 absorbPct;

            bool Load()
            {
                absorbPct = GetSpellInfo()->Effects[EFFECT_0].CalcValue(GetCaster());
                return true;
            }

            bool Validate(SpellInfo const* /*spellInfo*/)
            {
                if (!sSpellMgr->GetSpellInfo(SPELL_DK_ANTI_MAGIC_SHELL_TALENT))
                    return false;
                return true;
            }

            void CalculateAmount(AuraEffect const* /*aurEff*/, int32 & amount, bool & /*canBeRecalculated*/)
            {
                if (!GetCaster()->GetOwner())
                    return;

                SpellInfo const* talentSpell = sSpellMgr->GetSpellInfo(SPELL_DK_ANTI_MAGIC_SHELL_TALENT);
                amount = talentSpell->Effects[EFFECT_0].CalcValue(GetCaster());
                if (Unit* owner = GetCaster()->GetOwner())
                    amount += int32(2 * owner->GetTotalAttackPowerValue(BASE_ATTACK));
            }

            void Absorb(AuraEffect* /*aurEff*/, DamageInfo & dmgInfo, uint32 & absorbAmount)
            {
                 absorbAmount = CalculatePct(dmgInfo.GetDamage(), absorbPct);
            }

            void Register()
            {
                 DoEffectCalcAmount += AuraEffectCalcAmountFn(spell_dk_anti_magic_zone_AuraScript::CalculateAmount, EFFECT_0, SPELL_AURA_SCHOOL_ABSORB);
                 OnEffectAbsorb += AuraEffectAbsorbFn(spell_dk_anti_magic_zone_AuraScript::Absorb, EFFECT_0);
            }
        };

        AuraScript* GetAuraScript() const
        {
            return new spell_dk_anti_magic_zone_AuraScript();
        }
};

class spell_dk_blood_boil : public SpellScriptLoader
{
    class script_impl : public SpellScript
    {
        PrepareSpellScript(script_impl);

        enum
        {
            SPELL_BLOOD_BOIL    = 65658,
            SPELL_BLOOD_PLAGUE  = 55078,
            SPELL_FROST_FEVER   = 55095,
        };

        bool Load()
        {
            Unit const* const caster = GetCaster();
            return caster->GetTypeId() == TYPEID_PLAYER && caster->getClass() == CLASS_DEATH_KNIGHT;
        }

        void HandleHit()
        {
            Unit const* const target = GetHitUnit();
            if (target && (target->HasAura(SPELL_FROST_FEVER) || target->HasAura(SPELL_BLOOD_PLAGUE)))
            {
                int32 value = GetHitDamage();
                value += value * 0.5f + GetCaster()->GetTotalAttackPowerValue(BASE_ATTACK) * 0.035f;
                SetHitDamage(value);
            }
        }

        void HandleAfterHit()
        {
            Unit* const caster = GetCaster();
            caster->CastSpell(caster, SPELL_BLOOD_BOIL, true);
        }

        void Register()
        {
            OnHit += SpellHitFn(script_impl::HandleHit);
            AfterHit += SpellHitFn(script_impl::HandleAfterHit);
        }
    };

public:
    spell_dk_blood_boil()
        : SpellScriptLoader("spell_dk_blood_boil")
    {
    }

    SpellScript* GetSpellScript() const
    {
        return new script_impl();
    }
};

// Improved Unholy presence
class spell_dk_imp_presences : public SpellScriptLoader
{
    public:
        spell_dk_imp_presences() : SpellScriptLoader("spell_dk_imp_presences") { }

        class spell_dk_imp_presences_SpellScript : public SpellScript
        {
            PrepareSpellScript(spell_dk_imp_presences_SpellScript);

            void HandleOnHit()
            {
                GetCaster()->SetPower(POWER_RUNIC_POWER, 0);
                // Improved Unholy Presence
                if (AuraEffect* aur = GetCaster()->GetDummyAuraEffect(SPELLFAMILY_DEATHKNIGHT, 2633, EFFECT_0))
                {
                    if (GetSpellInfo()->Id != 48265)
                    {
                        int32 bp0 = aur->GetAmount();
                        GetCaster()->CastCustomSpell(GetCaster(), 63622, &bp0, NULL, NULL, true);
                    }
                    else
                        GetCaster()->RemoveAurasDueToSpell(63622);
                }

                // Improved Blood Presence
                if (AuraEffect* aur = GetCaster()->GetDummyAuraEffect(SPELLFAMILY_GENERIC, 2636, EFFECT_0))
                {
                    if (GetSpellInfo()->Id != 48263)
                    {
                        int32 bp0 = aur->GetAmount();
                        GetCaster()->CastCustomSpell(GetCaster(), 63611, &bp0, NULL, NULL, true);
                    }
                    else
                        GetCaster()->RemoveAurasDueToSpell(63611);
                }

                // Improved Frost Presence
                if (AuraEffect* aur = GetCaster()->GetDummyAuraEffect(SPELLFAMILY_DEATHKNIGHT, 2632, EFFECT_0))
                {
                    if (GetSpellInfo()->Id != 48266)
                    {
                        int32 bp0 = aur->GetAmount();
                        GetCaster()->CastCustomSpell(GetCaster(), 63621, &bp0, NULL, NULL, true);
                    }
                    else
                        GetCaster()->RemoveAurasDueToSpell(63621);
                }
            }

            void Register()
            {
                OnHit += SpellHitFn(spell_dk_imp_presences_SpellScript::HandleOnHit);
            }
        };

        SpellScript* GetSpellScript() const
        {
            return new spell_dk_imp_presences_SpellScript();
        }
};

// Dark transformation
class spell_dk_dark_transformation : public SpellScriptLoader
{
    public:
        spell_dk_dark_transformation() : SpellScriptLoader("spell_dk_dark_transformation") { }

        class spell_dk_dark_transformation_SpellScript : public SpellScript
        {
            PrepareSpellScript(spell_dk_dark_transformation_SpellScript);

            void HandleOnHit()
            {
                GetHitUnit()->RemoveAurasDueToSpell(91342);
                GetCaster()->RemoveAurasDueToSpell(93426);
            }

            void Register()
            {
                OnHit += SpellHitFn(spell_dk_dark_transformation_SpellScript::HandleOnHit);
            }
        };

        SpellScript* GetSpellScript() const
        {
            return new spell_dk_dark_transformation_SpellScript();
        }
};

// Claw
class spell_dk_pet_spells : public SpellScriptLoader
{
    public:
        spell_dk_pet_spells() : SpellScriptLoader("spell_dk_pet_spells") { }

        class spell_dk_pet_spells_SpellScript : public SpellScript
        {
            PrepareSpellScript(spell_dk_pet_spells_SpellScript);

            void HandleDummy(SpellEffIndex /*effIndex*/)
            {
                bool darkTransformation = GetCaster()->HasAura(63560);
                int32 triggered_spell_id = 0;
                switch (GetSpellInfo()->Id)
                {
                    case 47481: // Gnaw
                        triggered_spell_id = darkTransformation ? 91797 : 91800;
                        break;
                    case 47468: // Claw
                        triggered_spell_id = darkTransformation ? 91778 : 91776;
                        break;
                    case 47482: // Leap
                        triggered_spell_id = darkTransformation ? 91802 : 91809;
                        break;
                    case 47484: // Huddle
                        triggered_spell_id = darkTransformation ? 91837 : 91838;
                        break;
                }
                if (triggered_spell_id)
                    GetCaster()->CastSpell(GetHitUnit(), triggered_spell_id, true);
            }

            void Register()
            {
                OnEffectHitTarget += SpellEffectFn(spell_dk_pet_spells_SpellScript::HandleDummy, EFFECT_0, SPELL_EFFECT_DUMMY);
            }
        };

        SpellScript* GetSpellScript() const
        {
            return new spell_dk_pet_spells_SpellScript();
        }
};

class spell_dk_pestilence : public SpellScriptLoader
{
    class script_impl : public SpellScript
    {
        PrepareSpellScript(script_impl);

        enum
        {
            SPELL_DOT_REDUCTION         = 76243,
            SPELL_GLYPH_OF_PESTILENCE   = 63334,
            SPELL_PESTILENCE_ANIMATION  = 91939,
            SPELL_BLOOD_PLAGUE          = 55078,
            SPELL_FROST_FEVER           = 55095,
        };

        Unit* m_explTarget;

        bool Load()
        {
            m_explTarget = GetExplTargetUnit();
            return m_explTarget;
        }

        void FilterTargets(std::list<WorldObject*>& targets)
        {
            targets.remove(m_explTarget);
        }

        void HandleScript(SpellEffIndex)
        {
            Unit* const caster = GetCaster();
            Unit* const target = GetHitUnit();
            if (!target)
                return;

            caster->CastSpell(caster, SPELL_DOT_REDUCTION, true);

            // Blood Plague
            if (m_explTarget->HasAura(SPELL_BLOOD_PLAGUE, caster->GetGUID()))
                caster->CastSpell(target, SPELL_BLOOD_PLAGUE, true);

            // Frost Fever
            if (m_explTarget->HasAura(SPELL_FROST_FEVER, caster->GetGUID()))
                caster->CastSpell(target, SPELL_FROST_FEVER, true);

            caster->CastSpell(target, SPELL_PESTILENCE_ANIMATION, true);
        }

        void Register()
        {
            OnObjectAreaTargetSelect += SpellObjectAreaTargetSelectFn(script_impl::FilterTargets, EFFECT_2, TARGET_UNIT_DEST_AREA_ENEMY);
            OnEffectHitTarget += SpellEffectFn(script_impl::HandleScript, EFFECT_2, SPELL_EFFECT_SCRIPT_EFFECT);
        }
    };

public:
    spell_dk_pestilence()
        : SpellScriptLoader("spell_dk_pestilence")
    {
    }

    SpellScript* GetSpellScript() const
    {
        return new script_impl();
    }
};

// Death's advance
class spell_dk_deaths_advance : public SpellScriptLoader
{
    public:
        spell_dk_deaths_advance() : SpellScriptLoader("spell_dk_deaths_advance") { }

        class spell_dk_deaths_advance_AuraScript : public AuraScript
        {
            PrepareAuraScript(spell_dk_deaths_advance_AuraScript);

            void HandleEffectPeriodic(AuraEffect const* /*aurEff*/)
            {
                Player* player = GetCaster()->ToPlayer();
                if (!player)
                    return;

                uint8 unholyRunesOnCooldown = 0;
                for (uint32 i = 2; i < 4; ++i)
                    if (player->GetRuneCooldown(i))
                        unholyRunesOnCooldown++;
                if (unholyRunesOnCooldown == 2)
                    RefreshDuration();
            }

            void Register()
            {
                OnEffectPeriodic += AuraEffectPeriodicFn(spell_dk_deaths_advance_AuraScript::HandleEffectPeriodic, EFFECT_1, SPELL_AURA_PERIODIC_DUMMY);
            }
        };

        AuraScript* GetAuraScript() const
        {
            return new spell_dk_deaths_advance_AuraScript();
        }
};

// 50453 - Bloodworms Health Leech
class spell_dk_blood_gorged : public SpellScriptLoader
{
    public:
        spell_dk_blood_gorged() : SpellScriptLoader("spell_dk_blood_gorged") { }

        class spell_dk_blood_gorged_AuraScript : public AuraScript
        {
            PrepareAuraScript(spell_dk_blood_gorged_AuraScript);

            bool Validate(SpellInfo const* /*spellInfo*/)
            {
                if (!sSpellMgr->GetSpellInfo(SPELL_DK_BLOOD_GORGED_HEAL))
                    return false;
                return true;
            }

            bool Load()
            {
                _procTarget = NULL;
                return true;
            }

            bool CheckProc(ProcEventInfo& /*eventInfo*/)
            {
                _procTarget = GetTarget()->GetOwner();
                return _procTarget;
            }

            void HandleProc(AuraEffect const* aurEff, ProcEventInfo& eventInfo)
            {
                PreventDefaultAction();
                int32 bp = int32(eventInfo.GetDamageInfo()->GetDamage() * 1.5f);
                GetTarget()->CastCustomSpell(SPELL_DK_BLOOD_GORGED_HEAL, SPELLVALUE_BASE_POINT0, bp, _procTarget, true, NULL, aurEff);
            }

            void Register()
            {
                DoCheckProc += AuraCheckProcFn(spell_dk_blood_gorged_AuraScript::CheckProc);
                OnEffectProc += AuraEffectProcFn(spell_dk_blood_gorged_AuraScript::HandleProc, EFFECT_0, SPELL_AURA_DUMMY);
            }

        private:
            Unit* _procTarget;
        };

        AuraScript* GetAuraScript() const
        {
            return new spell_dk_blood_gorged_AuraScript();
        }
};

// Resilient Infection
class spell_sha_resilient_infection : public SpellScriptLoader
{
    public:
        spell_sha_resilient_infection() : SpellScriptLoader("spell_sha_resilient_infection") { }

        class spell_sha_resilient_infection_AuraScript : public AuraScript
        {
            PrepareAuraScript(spell_sha_resilient_infection_AuraScript);

            void HandleDispel(DispelInfo* /*dispelInfo*/)
            {
                if (Unit* caster = GetCaster())
                {
                    // Resilient Infection
                    if (AuraEffect const* aurEff = caster->GetDummyAuraEffect(SPELLFAMILY_GENERIC, 1910, EFFECT_0))
                        if (roll_chance_i(aurEff->GetAmount()))
                        {
                            int32 bp0 = 0;
                            int32 bp1 = 0;
                            if (GetId() == 55095)
                                bp0 = 1;
                            else
                                bp1 = 1;

                            caster->CastCustomSpell(caster, 90721, &bp0, &bp1, NULL, true);
                        }
                }
            }

            void Register()
            {
                AfterDispel += AuraDispelFn(spell_sha_resilient_infection_AuraScript::HandleDispel);
            }
        };

        AuraScript* GetAuraScript() const
        {
            return new spell_sha_resilient_infection_AuraScript();
        }
};

// -47541, 52375, 59134, -62900 - Death Coil
class spell_dk_death_coil : public SpellScriptLoader
{
    public:
        spell_dk_death_coil() : SpellScriptLoader("spell_dk_death_coil") { }

        class spell_dk_death_coil_SpellScript : public SpellScript
        {
            PrepareSpellScript(spell_dk_death_coil_SpellScript);

            bool Validate(SpellInfo const* /*spell*/)
            {
                if (!sSpellMgr->GetSpellInfo(SPELL_DK_DEATH_COIL_DAMAGE) || !sSpellMgr->GetSpellInfo(SPELL_DK_DEATH_COIL_HEAL))
                    return false;
                return true;
            }

            void HandleDummy(SpellEffIndex /*effIndex*/)
            {
                int32 damage = GetEffectValue();
                Unit* caster = GetCaster();
                if (Unit* target = GetHitUnit())
                {
                    if (caster->IsFriendlyTo(target))
                    {
                        int32 bp = int32(damage * 3.5f);
                        caster->CastCustomSpell(target, SPELL_DK_DEATH_COIL_HEAL, &bp, NULL, NULL, true);
                    }
                    else
                    {
                        if (AuraEffect const* auraEffect = caster->GetAuraEffect(SPELL_DK_ITEM_SIGIL_VENGEFUL_HEART, EFFECT_1))
                            damage += auraEffect->GetBaseAmount();
                        caster->CastCustomSpell(target, SPELL_DK_DEATH_COIL_DAMAGE, &damage, NULL, NULL, true);
                    }
                }
            }

            SpellCastResult CheckCast()
            {
                Unit* caster = GetCaster();
                if (Unit* target = GetExplTargetUnit())
                {
                    if (!caster->IsFriendlyTo(target) && !caster->isInFront(target))
                        return SPELL_FAILED_UNIT_NOT_INFRONT;

                    if (target->IsFriendlyTo(caster) && target->GetCreatureType() != CREATURE_TYPE_UNDEAD)
                        return SPELL_FAILED_BAD_TARGETS;
                }
                else
                    return SPELL_FAILED_BAD_TARGETS;

                return SPELL_CAST_OK;
            }

            void Register()
            {
                OnCheckCast += SpellCheckCastFn(spell_dk_death_coil_SpellScript::CheckCast);
                OnEffectHitTarget += SpellEffectFn(spell_dk_death_coil_SpellScript::HandleDummy, EFFECT_0, SPELL_EFFECT_DUMMY);
            }
        };

        SpellScript* GetSpellScript() const
        {
            return new spell_dk_death_coil_SpellScript();
        }
};

// 52751 - Death Gate
class spell_dk_death_gate : public SpellScriptLoader
{
    public:
        spell_dk_death_gate() : SpellScriptLoader("spell_dk_death_gate") {}

        class spell_dk_death_gate_SpellScript : public SpellScript
        {
            PrepareSpellScript(spell_dk_death_gate_SpellScript);

            SpellCastResult CheckClass()
            {
                if (GetCaster()->getClass() != CLASS_DEATH_KNIGHT)
                {
                    SetCustomCastResultMessage(SPELL_CUSTOM_ERROR_MUST_BE_DEATH_KNIGHT);
                    return SPELL_FAILED_CUSTOM_ERROR;
                }

                return SPELL_CAST_OK;
            }

            void HandleScript(SpellEffIndex effIndex)
            {
                PreventHitDefaultEffect(effIndex);
                if (Unit* target = GetHitUnit())
                    target->CastSpell(target, GetEffectValue(), false);
            }

            void Register()
            {
                OnCheckCast += SpellCheckCastFn(spell_dk_death_gate_SpellScript::CheckClass);
                OnEffectHitTarget += SpellEffectFn(spell_dk_death_gate_SpellScript::HandleScript, EFFECT_0, SPELL_EFFECT_SCRIPT_EFFECT);
            }
        };

        SpellScript* GetSpellScript() const
        {
            return new spell_dk_death_gate_SpellScript();
        }
};

// 49560 - Death Grip
class spell_dk_death_grip : public SpellScriptLoader
{
    public:
        spell_dk_death_grip() : SpellScriptLoader("spell_dk_death_grip") { }

        class spell_dk_death_grip_SpellScript : public SpellScript
        {
            PrepareSpellScript(spell_dk_death_grip_SpellScript);

            void HandleDummy(SpellEffIndex /*effIndex*/)
            {
                int32 damage = GetEffectValue();
                Position const* pos = GetExplTargetDest();
                if (Unit* target = GetHitUnit())
                    if (!target->HasAuraType(SPELL_AURA_DEFLECT_SPELLS)) // Deterrence
                        target->CastSpell(pos->GetPositionX(), pos->GetPositionY(), pos->GetPositionZ(), damage, true);
            }

            void Register()
            {
                OnEffectHitTarget += SpellEffectFn(spell_dk_death_grip_SpellScript::HandleDummy, EFFECT_0, SPELL_EFFECT_DUMMY);
            }

        };

        SpellScript* GetSpellScript() const
        {
            return new spell_dk_death_grip_SpellScript();
        }
};

// Death grip dummy
class spell_dk_death_grip_dummy : public SpellScriptLoader
{
    public:
        spell_dk_death_grip_dummy() : SpellScriptLoader("spell_dk_death_grip_dummy") { }

        class spell_dk_death_grip_dummy_SpellScript : public SpellScript
        {
            PrepareSpellScript(spell_dk_death_grip_dummy_SpellScript);

            void HandleDummy(SpellEffIndex /*effIndex*/)
            {
                Unit* caster = GetCaster();
                caster->CastSpell(GetHitUnit(), 49560, true);
            }

            void HandleOnHit()
            {
                SpellInfo const* triggered = sSpellMgr->GetSpellInfo(49560);
                Unit* caster = GetCaster();
                // Glyph of resilient grip
                if (AuraEffect* glyph = caster->GetAuraEffect(59309, EFFECT_0, caster->GetGUID()))
                    if (GetHitUnit()->IsImmunedToSpell(triggered))
                        caster->CastSpell(caster, 90289, true);
            }

            void Register()
            {
                OnEffectHitTarget += SpellEffectFn(spell_dk_death_grip_dummy_SpellScript::HandleDummy, EFFECT_0, SPELL_EFFECT_DUMMY);
                OnHit += SpellHitFn(spell_dk_death_grip_dummy_SpellScript::HandleOnHit);
            }
        };

        SpellScript* GetSpellScript() const
        {
            return new spell_dk_death_grip_dummy_SpellScript();
        }
};

// 48743 - Death Pact
class spell_dk_death_pact : public SpellScriptLoader
{
    public:
        spell_dk_death_pact() : SpellScriptLoader("spell_dk_death_pact") { }

        class spell_dk_death_pact_SpellScript : public SpellScript
        {
            PrepareSpellScript(spell_dk_death_pact_SpellScript);

            SpellCastResult CheckCast()
            {
                // Check if we have valid targets, otherwise skip spell casting here
                if (Player* player = GetCaster()->ToPlayer())
                    for (Unit::ControlList::const_iterator itr = player->m_Controlled.begin(); itr != player->m_Controlled.end(); ++itr)
                        if (Creature* undeadPet = (*itr)->ToCreature())
                            if (undeadPet->isAlive() &&
                                undeadPet->GetOwnerGUID() == player->GetGUID() &&
                                undeadPet->GetCreatureType() == CREATURE_TYPE_UNDEAD &&
                                undeadPet->IsWithinDist(player, 100.0f, false))
                                return SPELL_CAST_OK;

                return SPELL_FAILED_NO_PET;
            }

            void FilterTargets(std::list<WorldObject*>& targetList)
            {
                Unit* target = NULL;
                if (Unit* selected = GetSelectedUnit())
                    if (selected->isAlive() && selected->GetOwnerGUID() == GetCaster()->GetGUID() && selected->GetCreatureType() == CREATURE_TYPE_UNDEAD)
                        target = selected;
                
                if (!target)
                {
                    for (std::list<WorldObject*>::iterator itr = targetList.begin(); itr != targetList.end(); ++itr)
                    {
                        if (Unit* unit = (*itr)->ToUnit())
                            if (unit->isAlive() && unit->GetOwnerGUID() == GetCaster()->GetGUID() && unit->GetCreatureType() == CREATURE_TYPE_UNDEAD)
                            {
                                target = unit;
                                break;
                            }
                    }
                }

                targetList.clear();
                if (target)
                    targetList.push_back(target);
            }

            void Register()
            {
                OnCheckCast += SpellCheckCastFn(spell_dk_death_pact_SpellScript::CheckCast);
                OnObjectAreaTargetSelect += SpellObjectAreaTargetSelectFn(spell_dk_death_pact_SpellScript::FilterTargets, EFFECT_1, TARGET_UNIT_DEST_AREA_ALLY);
            }
        };

        SpellScript* GetSpellScript() const
        {
            return new spell_dk_death_pact_SpellScript();
        }
};

// -49998 - Death Strike
class spell_dk_death_strike : public SpellScriptLoader
{
    public:
        spell_dk_death_strike() : SpellScriptLoader("spell_dk_death_strike") { }

        class spell_dk_death_strike_SpellScript : public SpellScript
        {
            PrepareSpellScript(spell_dk_death_strike_SpellScript);

            bool Validate(SpellInfo const* /*spellInfo*/)
            {
                if (!sSpellMgr->GetSpellInfo(SPELL_DK_DEATH_STRIKE_HEAL))
                    return false;
                return true;
            }

            void HandleOnHit()
            {
                Unit* caster = GetCaster();
                if (Unit* target = GetHitUnit())
                {
                    uint32 count = target->GetDiseasesByCaster(caster->GetGUID());
                    int32 bp = caster->CountPctFromMaxHealth(GetSpellInfo()->Effects[EFFECT_2].BasePoints);
                    // 20% of damage taken in the last 5 seconds
                    if (AuraEffect* aurEff = caster->GetAuraEffect(89832, EFFECT_0))
                        bp += aurEff->GetAmount() * 0.2f;
                    // Improved Death Strike
                    if (AuraEffect const* aurEff = caster->GetDummyAuraEffect(SPELLFAMILY_DEATHKNIGHT, DK_ICON_ID_IMPROVED_DEATH_STRIKE, EFFECT_2))
                        AddPct(bp, aurEff->GetAmount());
                    // Dark Succor
                    if (bp < (int32)(caster->GetMaxHealth() / 5))
                    {
                        if (Aura* succor = caster->GetAura(101568))
                        {
                            if (!caster->HasAura(48263))
                            {
                                bp = caster->GetMaxHealth() / 5;
                                caster->RemoveAura(succor);
                            }
                        }
                    }
                    // Mastery adds blood shield when in blood presence
                    if (AuraEffect const* mastery = caster->GetDummyAuraEffect(SPELLFAMILY_DEATHKNIGHT, 2624, EFFECT_0))
                        if (caster->HasAura(48263))
                        {
                            int32 bp0 = bp * (mastery->GetAmount() / 100.0f);
                            caster->CastCustomSpell(caster, 77535, &bp0, NULL, NULL, true);
                        }
                    caster->CastCustomSpell(caster, SPELL_DK_DEATH_STRIKE_HEAL, &bp, NULL, NULL, false);
                }
            }

            void Register()
            {
                OnHit += SpellHitFn(spell_dk_death_strike_SpellScript::HandleOnHit);
            }

        };

        SpellScript* GetSpellScript() const
        {
            return new spell_dk_death_strike_SpellScript();
        }
};

// 47496 - Explode, Ghoul spell for Corpse Explosion
class spell_dk_ghoul_explode : public SpellScriptLoader
{
    public:
        spell_dk_ghoul_explode() : SpellScriptLoader("spell_dk_ghoul_explode") { }

        class spell_dk_ghoul_explode_SpellScript : public SpellScript
        {
            PrepareSpellScript(spell_dk_ghoul_explode_SpellScript);

            bool Validate(SpellInfo const* /*spellInfo*/)
            {
                if (!sSpellMgr->GetSpellInfo(SPELL_DK_CORPSE_EXPLOSION_TRIGGERED))
                    return false;
                return true;
            }

            void Suicide(SpellEffIndex /*effIndex*/)
            {
                if (Unit* unitTarget = GetHitUnit())
                {
                    // Corpse Explosion (Suicide)
                    unitTarget->CastSpell(unitTarget, SPELL_DK_CORPSE_EXPLOSION_TRIGGERED, true);
                }
            }

            void Register()
            {
                OnEffectHitTarget += SpellEffectFn(spell_dk_ghoul_explode_SpellScript::Suicide, EFFECT_1, SPELL_EFFECT_SCHOOL_DAMAGE);
            }
        };

        SpellScript* GetSpellScript() const
        {
            return new spell_dk_ghoul_explode_SpellScript();
        }
};


// 59754 Rune Tap - Party
class spell_dk_rune_tap_party : public SpellScriptLoader
{
    public:
        spell_dk_rune_tap_party() : SpellScriptLoader("spell_dk_rune_tap_party") { }

        class spell_dk_rune_tap_party_SpellScript : public SpellScript
        {
            PrepareSpellScript(spell_dk_rune_tap_party_SpellScript);

            void CheckTargets(std::list<WorldObject*>& targets)
            {
                targets.remove(GetCaster());
            }

            void Register()
            {
                OnObjectAreaTargetSelect += SpellObjectAreaTargetSelectFn(spell_dk_rune_tap_party_SpellScript::CheckTargets, EFFECT_0, TARGET_UNIT_CASTER_AREA_PARTY);
            }
        };

        SpellScript* GetSpellScript() const
        {
            return new spell_dk_rune_tap_party_SpellScript();
        }
};

// 55090 - Scourge Strike (55265, 55270, 55271)
class spell_dk_scourge_strike : public SpellScriptLoader
{
    public:
        spell_dk_scourge_strike() : SpellScriptLoader("spell_dk_scourge_strike") { }

        class spell_dk_scourge_strike_SpellScript : public SpellScript
        {
            PrepareSpellScript(spell_dk_scourge_strike_SpellScript);
            float multiplier;

            bool Load()
            {
                multiplier = 1.0f;
                return true;
            }

            bool Validate(SpellInfo const* /*spellInfo*/)
            {
                if (!sSpellMgr->GetSpellInfo(SPELL_DK_SCOURGE_STRIKE_TRIGGERED))
                    return false;
                return true;
            }

            void HandleDummy(SpellEffIndex /*effIndex*/)
            {
                Unit* caster = GetCaster();
                if (Unit* unitTarget = GetHitUnit())
                {
                    multiplier = (GetEffectValue() * unitTarget->GetDiseasesByCaster(caster->GetGUID()) / 100.f);
                    // Death Knight T8 Melee 4P Bonus
                    if (AuraEffect const* aurEff = caster->GetAuraEffect(SPELL_DK_ITEM_T8_MELEE_4P_BONUS, EFFECT_0))
                        AddPct(multiplier, aurEff->GetAmount());
                }
            }

            void HandleAfterHit()
            {
                Unit* caster = GetCaster();
                if (Unit* unitTarget = GetHitUnit())
                {
                    int32 bp = GetTrueDamage() * multiplier;
                    AuraEffect const* aurEff = caster->GetAuraEffect(SPELL_DK_GLYPH_OF_SCOURGE_STRIKE, EFFECT_0);
                    // Glyph of Scourge Strike
                    if (aurEff)
                        AddPct(bp, aurEff->GetAmount());

                    // Unholy Mastery
                    if (aurEff = caster->GetAuraEffect(77515, EFFECT_0))
                        AddPct(bp, aurEff->GetAmount());

                    caster->CastCustomSpell(unitTarget, SPELL_DK_SCOURGE_STRIKE_TRIGGERED, &bp, NULL, NULL, true);
                }
            }

            void Register()
            {
                OnEffectHitTarget += SpellEffectFn(spell_dk_scourge_strike_SpellScript::HandleDummy, EFFECT_2, SPELL_EFFECT_DUMMY);
                AfterHit += SpellHitFn(spell_dk_scourge_strike_SpellScript::HandleAfterHit);
            }
        };

        SpellScript* GetSpellScript() const
        {
            return new spell_dk_scourge_strike_SpellScript();
        }
};

// 55233 - Vampiric Blood
class spell_dk_vampiric_blood : public SpellScriptLoader
{
    public:
        spell_dk_vampiric_blood() : SpellScriptLoader("spell_dk_vampiric_blood") { }

        class spell_dk_vampiric_blood_AuraScript : public AuraScript
        {
            PrepareAuraScript(spell_dk_vampiric_blood_AuraScript);

            void CalculateAmount(AuraEffect const* /*aurEff*/, int32& amount, bool& /*canBeRecalculated*/)
            {
                amount = GetUnitOwner()->CountPctFromMaxHealth(amount);
            }

            void Register()
            {
                DoEffectCalcAmount += AuraEffectCalcAmountFn(spell_dk_vampiric_blood_AuraScript::CalculateAmount, EFFECT_1, SPELL_AURA_MOD_INCREASE_HEALTH);
            }
        };

        AuraScript* GetAuraScript() const
        {
            return new spell_dk_vampiric_blood_AuraScript();
        }
};

class spell_dk_avoidance_passive : public SpellScriptLoader
{
public:
    spell_dk_avoidance_passive() : SpellScriptLoader("spell_dk_avoidance_passive") { }

    class spell_dk_avoidance_passive_AuraScript : public AuraScript
    {
        PrepareAuraScript(spell_dk_avoidance_passive_AuraScript);

        bool Load()
        {
            if (!GetCaster() || !GetCaster()->GetOwner() || GetCaster()->GetOwner()->GetTypeId() != TYPEID_PLAYER)
                return false;
            return true;
        }

        void CalculateAvoidanceAmount(AuraEffect const* /* aurEff */, int32& amount, bool& /*canBeRecalculated*/)
        {
            if (Unit* pet = GetUnitOwner())
            {
                if (Unit* owner = pet->GetOwner())
                {
                    // Army of the dead ghoul
                    if (pet->GetEntry() == ENTRY_ARMY_OF_THE_DEAD_GHOUL)
                        amount = -90;
                    // Night of the dead
                    else if (Aura* aur = owner->GetAuraOfRankedSpell(SPELL_NIGHT_OF_THE_DEAD))
                        amount = aur->GetSpellInfo()->Effects[EFFECT_2].CalcValue();
                }
            }
        }

        void Register()
        {
            DoEffectCalcAmount += AuraEffectCalcAmountFn(spell_dk_avoidance_passive_AuraScript::CalculateAvoidanceAmount, EFFECT_0, SPELL_AURA_MOD_CREATURE_AOE_DAMAGE_AVOIDANCE);
        }
    };

    AuraScript* GetAuraScript() const
    {
        return new spell_dk_avoidance_passive_AuraScript();
    }
};

class spell_dk_pet_scaling_01 : public SpellScriptLoader
{
public:
    spell_dk_pet_scaling_01() : SpellScriptLoader("spell_dk_pet_scaling_01") { }

    class spell_dk_pet_scaling_01_AuraScript : public AuraScript
    {
        PrepareAuraScript(spell_dk_pet_scaling_01_AuraScript);

        bool Load()
        {
            if (!GetCaster() || !GetCaster()->GetOwner() || GetCaster()->GetOwner()->GetTypeId() != TYPEID_PLAYER)
                return false;
            _tempHealth = 0;
            return true;
        }

        void CalculateStaminaAmount(AuraEffect const* /* aurEff */, int32& amount, bool& /*canBeRecalculated*/)
        {
            if (Unit* pet = GetUnitOwner())
            {
                if (pet->isGuardian())
                {
                    if (Unit* owner = pet->GetOwner())
                    {
                        float mod = 1.0f;
                        // Glyph of Raise dead
                        if (AuraEffect const* aurEff = owner->GetAuraEffect(58686, 0))
                            AddPct(mod, aurEff->GetAmount());

                        float ownerBonus = float(owner->GetStat(STAT_STAMINA)) * mod;
                        amount += ownerBonus;
                    }
                }
            }
        }

        void ApplyEffect(AuraEffect const* /* aurEff */, AuraEffectHandleModes /*mode*/)
        {
            if (Unit* pet = GetUnitOwner())
                if (_tempHealth)
                    pet->SetHealth(_tempHealth);
        }

        void RemoveEffect(AuraEffect const* /* aurEff */, AuraEffectHandleModes /*mode*/)
        {
            if (Unit* pet = GetUnitOwner())
                _tempHealth = pet->GetHealth();
        }

        void CalculateStrengthAmount(AuraEffect const* /* aurEff */, int32& amount, bool& /*canBeRecalculated*/)
        {
            if (Unit* pet = GetUnitOwner())
            {
                if (!pet->isGuardian())
                    return;

                Unit* owner = pet->GetOwner();
                if (!owner)
                    return;

                float mod = 1.0f;
                // Glyph of raise dead
                if (AuraEffect const* aurEff = owner->GetAuraEffect(58686, 0))
                    AddPct(mod, aurEff->GetAmount());

                float ownerBonus = float(owner->GetStat(STAT_STRENGTH)) * mod;
                amount += ownerBonus;
            }
        }

        void Register()
        {
            OnEffectRemove += AuraEffectRemoveFn(spell_dk_pet_scaling_01_AuraScript::RemoveEffect, EFFECT_0, SPELL_AURA_MOD_STAT, AURA_EFFECT_HANDLE_CHANGE_AMOUNT_MASK);
            AfterEffectApply += AuraEffectApplyFn(spell_dk_pet_scaling_01_AuraScript::ApplyEffect, EFFECT_0, SPELL_AURA_MOD_STAT, AURA_EFFECT_HANDLE_CHANGE_AMOUNT_MASK);
            DoEffectCalcAmount += AuraEffectCalcAmountFn(spell_dk_pet_scaling_01_AuraScript::CalculateStaminaAmount, EFFECT_0, SPELL_AURA_MOD_STAT);
            DoEffectCalcAmount += AuraEffectCalcAmountFn(spell_dk_pet_scaling_01_AuraScript::CalculateStrengthAmount, EFFECT_1, SPELL_AURA_MOD_STAT);
        }

    private:
        uint32 _tempHealth;
    };

    AuraScript* GetAuraScript() const
    {
        return new spell_dk_pet_scaling_01_AuraScript();
    }
};

class spell_dk_pet_scaling_02 : public SpellScriptLoader
{
public:
    spell_dk_pet_scaling_02() : SpellScriptLoader("spell_dk_pet_scaling_02") { }

    class spell_dk_pet_scaling_02_AuraScript : public AuraScript
    {
        PrepareAuraScript(spell_dk_pet_scaling_02_AuraScript);

        bool Load()
        {
            if (!GetCaster() || !GetCaster()->GetOwner() || GetCaster()->GetOwner()->GetTypeId() != TYPEID_PLAYER)
                return false;
            return true;
        }

        void CalculateAmountMeleeHaste(AuraEffect const* /* aurEff */, int32& amount, bool& /*canBeRecalculated*/)
        {
            if (!GetCaster() || !GetCaster()->GetOwner())
                return;
            if (Player* owner = GetCaster()->GetOwner()->ToPlayer())
            {
                // For others recalculate it from:
                float HasteMelee = 0.0f;
                HasteMelee += (1-owner->m_modAttackSpeedPct[BASE_ATTACK])*100;
                amount += int32(HasteMelee);
            }
        }

        void CalculateBonusDamagePct(AuraEffect const* /* aurEff */, int32& amount, bool& /*canBeRecalculated*/)
        {
            if (!GetCaster() || !GetCaster()->GetOwner())
                return;

            if (Player* owner = GetCaster()->GetOwner()->ToPlayer())
            {
                Unit::AuraEffectList const& auraDamagePctList = owner->GetAuraEffectsByType(SPELL_AURA_MOD_DAMAGE_PERCENT_DONE);
                for (Unit::AuraEffectList::const_iterator itr = auraDamagePctList.begin(); itr != auraDamagePctList.end(); ++itr)
                    if ((*itr)->GetMiscValue() & SPELL_SCHOOL_MASK_NORMAL)
                        amount += (*itr)->GetAmount();
            }
        }
        

        void Register()
        {
            DoEffectCalcAmount += AuraEffectCalcAmountFn(spell_dk_pet_scaling_02_AuraScript::CalculateBonusDamagePct, EFFECT_0, SPELL_AURA_MOD_DAMAGE_PERCENT_DONE);
            DoEffectCalcAmount += AuraEffectCalcAmountFn(spell_dk_pet_scaling_02_AuraScript::CalculateAmountMeleeHaste, EFFECT_1, SPELL_AURA_MELEE_SLOW);
        }
    };

    AuraScript* GetAuraScript() const
    {
        return new spell_dk_pet_scaling_02_AuraScript();
    }
};

class spell_dk_pet_scaling_03 : public SpellScriptLoader
{
public:
    spell_dk_pet_scaling_03() : SpellScriptLoader("spell_dk_pet_scaling_03") { }

    class spell_dk_pet_scaling_03_AuraScript : public AuraScript
    {
        PrepareAuraScript(spell_dk_pet_scaling_03_AuraScript);

        bool Load()
        {
            if (!GetCaster() || !GetCaster()->GetOwner() || GetCaster()->GetOwner()->GetTypeId() != TYPEID_PLAYER)
                return false;
            return true;
        }

        void CalculateAmountMeleeHit(AuraEffect const* /* aurEff */, int32& amount, bool& /*canBeRecalculated*/)
        {
            if (!GetCaster() || !GetCaster()->GetOwner())
                return;
            if (Player* owner = GetCaster()->GetOwner()->ToPlayer())
            {
                // For others recalculate it from:
                float HitMelee = 0.0f;
                // Increase hit from SPELL_AURA_MOD_HIT_CHANCE
                HitMelee += owner->GetTotalAuraModifier(SPELL_AURA_MOD_HIT_CHANCE);
                // Increase hit melee from meele hit ratings
                HitMelee += owner->GetRatingBonusValue(CR_HIT_MELEE);

                amount += int32(HitMelee);
            }
        }

        void CalculateAmountSpellHit(AuraEffect const* /* aurEff */, int32& amount, bool& /*canBeRecalculated*/)
        {
            if (!GetCaster() || !GetCaster()->GetOwner())
                return;
            if (Player* owner = GetCaster()->GetOwner()->ToPlayer())
            {
                // For others recalculate it from:
                float HitSpell = 0.0f;
                // Increase hit from SPELL_AURA_MOD_SPELL_HIT_CHANCE
                HitSpell += owner->GetTotalAuraModifier(SPELL_AURA_MOD_SPELL_HIT_CHANCE);
                // Increase hit spell from spell hit ratings
                HitSpell += owner->GetRatingBonusValue(CR_HIT_SPELL);

                amount += int32(HitSpell);
            }
        }

        void Register()
        {
            DoEffectCalcAmount += AuraEffectCalcAmountFn(spell_dk_pet_scaling_03_AuraScript::CalculateAmountMeleeHit, EFFECT_0, SPELL_AURA_MOD_HIT_CHANCE);
            DoEffectCalcAmount += AuraEffectCalcAmountFn(spell_dk_pet_scaling_03_AuraScript::CalculateAmountSpellHit, EFFECT_1, SPELL_AURA_MOD_SPELL_HIT_CHANCE);
        }
    };

    AuraScript* GetAuraScript() const
    {
        return new spell_dk_pet_scaling_03_AuraScript();
    }
};

class spell_dk_rune_weapon_scaling_02 : public SpellScriptLoader
{
public:
    spell_dk_rune_weapon_scaling_02() : SpellScriptLoader("spell_dk_rune_weapon_scaling_02") { }

    class spell_dk_rune_weapon_scaling_02_AuraScript : public AuraScript
    {
        PrepareAuraScript(spell_dk_rune_weapon_scaling_02_AuraScript);

        bool Load()
        {
            if (!GetCaster() || !GetCaster()->GetOwner() || GetCaster()->GetOwner()->GetTypeId() != TYPEID_PLAYER)
                return false;
            return true;
        }

        void CalculateDamageDoneAmount(AuraEffect const* /* aurEff */, int32& amount, bool& /*canBeRecalculated*/)
        {
            if (Unit* pet = GetUnitOwner())
            {
                Unit* owner = pet->GetOwner();
                if (!owner)
                    return;

                if (pet->isGuardian())
                    ((Guardian*)pet)->SetSpellBonusDamage(owner->GetTotalAttackPowerValue(BASE_ATTACK));

                amount += owner->CalculateDamage(BASE_ATTACK, true, true);
            }
        }

        void CalculateAmountMeleeHaste(AuraEffect const* /* aurEff */, int32& amount, bool& /*canBeRecalculated*/)
        {
            if (!GetCaster() || !GetCaster()->GetOwner())
                return;
            if (Player* owner = GetCaster()->GetOwner()->ToPlayer())
            {
                // For others recalculate it from:
                float HasteMelee = 0.0f;
                // Increase hit from SPELL_AURA_MOD_HIT_CHANCE
                HasteMelee += (1-owner->m_modAttackSpeedPct[BASE_ATTACK])*100;

                amount += int32(HasteMelee);
            }
        }

        void Register()
        {
            DoEffectCalcAmount += AuraEffectCalcAmountFn(spell_dk_rune_weapon_scaling_02_AuraScript::CalculateDamageDoneAmount, EFFECT_0, SPELL_AURA_MOD_DAMAGE_DONE);
            DoEffectCalcAmount += AuraEffectCalcAmountFn(spell_dk_rune_weapon_scaling_02_AuraScript::CalculateAmountMeleeHaste, EFFECT_1, SPELL_AURA_MELEE_SLOW);
        }
    };

    AuraScript* GetAuraScript() const
    {
        return new spell_dk_rune_weapon_scaling_02_AuraScript();
    }
};


void AddSC_deathknight_spell_scripts()
{
    new spell_dk_anti_magic_shell_raid();
    new spell_dk_anti_magic_shell_self();
    new spell_dk_anti_magic_zone();
    new spell_dk_blood_boil();
    new spell_dk_blood_gorged();
    new spell_dk_death_coil();
    new spell_dk_death_gate();
    new spell_dk_death_grip();
    new spell_dk_death_pact();
    new spell_dk_death_strike();
    new spell_dk_ghoul_explode();
    new spell_dk_rune_tap_party();
    new spell_dk_scourge_strike();
    new spell_dk_vampiric_blood();
    new spell_sha_resilient_infection();
    new spell_dk_pestilence();
    new spell_dk_deaths_advance();
    new spell_dk_imp_presences();
    new spell_dk_pet_spells();
    new spell_dk_dark_transformation();
    new spell_dk_necrotic();
    new spell_dk_festering();
    new spell_dk_chilblains();
    new spell_dk_strangulate();
    new spell_dk_avoidance_passive();
    new spell_dk_pet_scaling_01();
    new spell_dk_pet_scaling_02();
    new spell_dk_pet_scaling_03();
    new spell_dk_rune_weapon_scaling_02();
    new spell_dk_death_grip_dummy();
    new spell_dk_pillar_of_frost();
}
