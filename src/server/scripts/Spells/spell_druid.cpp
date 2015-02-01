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
 * Scripts for spells with SPELLFAMILY_DRUID and SPELLFAMILY_GENERIC spells used by druid players.
 * Ordered alphabetically using scriptname.
 * Scriptnames of files in this file should be prefixed with "spell_dru_".
 */

#include "Player.h"
#include "ScriptMgr.h"
#include "SpellScript.h"
#include "SpellAuraEffects.h"
#include "Containers.h"

enum DruidSpells
{
    SPELL_DRUID_NPC_WILD_MUSHROOM             = 47649,
    SPELL_DRUID_FUNGAL_GROWTH_GRAPHIC         = 94339 ,
    SPELL_DRUID_FUNGAL_GROWTH_SUMMON          = 81291 ,
    SPELL_DRUID_SPELL_WILD_MUSHROOM_SUICIDE   = 92853,
    SPELL_DRUID_SPELL_WILD_MUSHROOM_DAMAGE    = 78777,
    SPELL_DRUID_MOONFIRE                      = 8921,
    SPELL_DRUID_SUNFIRE                       = 93402,
    SPELL_DRUID_WRATH                         = 5176,
    SPELL_DRUID_STARFIRE                      = 2912,
    SPELL_DRUID_STARSURGE                     = 78674,
    SPELL_DRUID_ECLIPSE_GENERAL_ENERGIZE      = 89265,
    SPELL_DRUID_STARSURGE_ENERGIZE            = 86605,
    SPELL_DRUID_LUNAR_ECLIPSE_MARKER          = 67484, // Will make the yellow arrow on eclipse bar point to the blue side (lunar)
    SPELL_DRUID_SOLAR_ECLIPSE_MARKER          = 67483, // Will make the yellow arrow on eclipse bar point to the yellow side (solar)
    SPELL_DRUID_SOLAR_ECLIPSE                 = 48517,
    SPELL_DRUID_LUNAR_ECLIPSE                 = 48518,
    SPELL_DRUID_ENRAGE_MOD_DAMAGE             = 51185,
    SPELL_DRUID_GLYPH_OF_TYPHOON              = 62135,
    SPELL_DRUID_IDOL_OF_FERAL_SHADOWS         = 34241,
    SPELL_DRUID_IDOL_OF_WORSHIP               = 60774,
    SPELL_DRUID_INCREASED_MOONFIRE_DURATION   = 38414,
    SPELL_DRUID_KING_OF_THE_JUNGLE            = 48492,
    SPELL_DRUID_LIFEBLOOM_ENERGIZE            = 64372,
    SPELL_DRUID_LIFEBLOOM_FINAL_HEAL          = 33778,
    SPELL_DRUID_LIVING_SEED_HEAL              = 48503,
    SPELL_DRUID_LIVING_SEED_PROC              = 48504,
    SPELL_DRUID_NATURES_SPLENDOR              = 57865,
    SPELL_DRUID_SURVIVAL_INSTINCTS            = 50322,
    SPELL_DRUID_SAVAGE_ROAR                   = 62071,
    SPELL_DRUID_TIGER_S_FURY_ENERGIZE         = 51178,
    SPELL_DRUID_ITEM_T8_BALANCE_RELIC         = 64950,
	SPELL_DRUID_LIFEBLOOM_HOT                 = 33763
};

// 2912, 5176, 78674 - Starfire, Wrath, and Starsurge
class spell_dru_eclipse_energize : public SpellScriptLoader
{
    public:
        spell_dru_eclipse_energize() : SpellScriptLoader("spell_dru_eclipse_energize") { }

        class spell_dru_eclipse_energize_SpellScript : public SpellScript
        {
            PrepareSpellScript(spell_dru_eclipse_energize_SpellScript);

            int32 energizeAmount;

            bool Load()
            {
                if (GetCaster()->GetTypeId() != TYPEID_PLAYER)
                    return false;

                if (GetCaster()->ToPlayer()->getClass() != CLASS_DRUID)
                    return false;

                energizeAmount = 0;

                return true;
            }

            void HandleEnergize(SpellEffIndex effIndex)
            {
                Player* caster = GetCaster()->ToPlayer();

                // No boomy, no deal.
                if (caster->GetPrimaryTalentTree(caster->GetActiveSpec()) != TALENT_TREE_DRUID_BALANCE)
                    return;

                bool lunar = false;
                bool solar = false;
                bool hasActiveMarker = caster->HasAura(SPELL_DRUID_LUNAR_ECLIPSE_MARKER) || caster->HasAura(SPELL_DRUID_SOLAR_ECLIPSE_MARKER);
                switch (GetSpellInfo()->Id)
                {
                    case SPELL_DRUID_SUNFIRE:
                    case SPELL_DRUID_MOONFIRE:
                    {
                        // Lunar shower
                        if (!caster->GetAuraEffect(SPELL_AURA_ADD_PCT_MODIFIER, SPELLFAMILY_DRUID, 3698, EFFECT_0))
                            return;
                        energizeAmount = 8;
                        break;
                    }
                    case SPELL_DRUID_WRATH:
                    case SPELL_DRUID_STARFIRE:
                    {
                        energizeAmount = GetSpellInfo()->Effects[EFFECT_1].BasePoints;
                        // Don't add Euphoria beenfit when already in eclipse state
                        if (caster->HasAura(SPELL_DRUID_SOLAR_ECLIPSE) || caster->HasAura(SPELL_DRUID_LUNAR_ECLIPSE))
                            break;
                        // Euphoria
                        if (AuraEffect* euphoria = caster->GetDummyAuraEffect(SPELLFAMILY_DRUID, 4431, EFFECT_0))
                            if (roll_chance_i(euphoria->GetAmount()))
                                energizeAmount *= 2;
                        break;
                    }
                    default:
                        energizeAmount = GetSpellInfo()->Effects[EFFECT_1].BasePoints;
                        break;
                }                

                switch (GetSpellInfo()->Id)
                {
                    case SPELL_DRUID_SUNFIRE:
                    case SPELL_DRUID_WRATH:
                    {
                        energizeAmount = -energizeAmount;
                        // If we are set to fill the lunar side or we've just logged in with 0 power..
                        if ((!caster->HasAura(SPELL_DRUID_SOLAR_ECLIPSE_MARKER) && caster->HasAura(SPELL_DRUID_LUNAR_ECLIPSE_MARKER))
                            || (!hasActiveMarker && caster->GetPower(POWER_ECLIPSE) == 0))
                        {
                            caster->CastCustomSpell(caster, SPELL_DRUID_ECLIPSE_GENERAL_ENERGIZE, &energizeAmount, 0, 0, true);
                            // If the energize was due to 0 power, cast the eclipse marker aura
                            if (!caster->HasAura(SPELL_DRUID_LUNAR_ECLIPSE_MARKER))
                                caster->CastSpell(caster, SPELL_DRUID_LUNAR_ECLIPSE_MARKER, true);
                            
                            lunar = caster->GetPower(POWER_ECLIPSE) == -100;
                        }
                        // The energizing effect brought us out of the solar eclipse, remove the aura
                        if (caster->HasAura(SPELL_DRUID_SOLAR_ECLIPSE) && caster->GetPower(POWER_ECLIPSE) <= 0)
                        {
                            caster->RemoveAurasDueToSpell(SPELL_DRUID_SOLAR_ECLIPSE);
                            caster->RemoveAurasDueToSpell(94338);
                        }
                        break;
                    }
                    case SPELL_DRUID_STARFIRE:
                    {
                        // If we are set to fill the solar side or we've just logged in with 0 power..
                        if ((!caster->HasAura(SPELL_DRUID_LUNAR_ECLIPSE_MARKER) && caster->HasAura(SPELL_DRUID_SOLAR_ECLIPSE_MARKER))
                            || (!hasActiveMarker && caster->GetPower(POWER_ECLIPSE) == 0))
                        {
                            caster->CastCustomSpell(caster, SPELL_DRUID_ECLIPSE_GENERAL_ENERGIZE, &energizeAmount, 0, 0, true);
                            // If the energize was due to 0 power, cast the eclipse marker aura
                            if (!caster->HasAura(SPELL_DRUID_SOLAR_ECLIPSE_MARKER))
                                caster->CastSpell(caster, SPELL_DRUID_SOLAR_ECLIPSE_MARKER, true);

                            solar = caster->GetPower(POWER_ECLIPSE) == 100;
                        }
                        // The energizing effect brought us out of the lunar eclipse, remove the aura
                        if (caster->HasAura(SPELL_DRUID_LUNAR_ECLIPSE) && caster->GetPower(POWER_ECLIPSE) >= 0)
                            caster->RemoveAura(SPELL_DRUID_LUNAR_ECLIPSE);
                        break;
                    }
                    case SPELL_DRUID_MOONFIRE:
                    case SPELL_DRUID_STARSURGE:
                    {
                        // If we are set to fill the solar side or we've just logged in with 0 power (confirmed with sniffs)
                        if ((!caster->HasAura(SPELL_DRUID_LUNAR_ECLIPSE_MARKER) && caster->HasAura(SPELL_DRUID_SOLAR_ECLIPSE_MARKER))
                            || (!hasActiveMarker && caster->GetPower(POWER_ECLIPSE) == 0))
                        {
                            if (GetSpellInfo()->Id == SPELL_DRUID_STARSURGE)
                                caster->CastCustomSpell(caster, SPELL_DRUID_STARSURGE_ENERGIZE, &energizeAmount, 0, 0, true);
                            else
                                caster->CastCustomSpell(caster, SPELL_DRUID_ECLIPSE_GENERAL_ENERGIZE, &energizeAmount, 0, 0, true);
                            solar = caster->GetPower(POWER_ECLIPSE) == 100;

                            // If the energize was due to 0 power, cast the eclipse marker aura
                            if (!caster->HasAura(SPELL_DRUID_SOLAR_ECLIPSE_MARKER))
                                caster->CastSpell(caster, SPELL_DRUID_SOLAR_ECLIPSE_MARKER, true);
                        }
                        else if (!caster->HasAura(SPELL_DRUID_SOLAR_ECLIPSE_MARKER) && caster->HasAura(SPELL_DRUID_LUNAR_ECLIPSE_MARKER))
                        {
                            energizeAmount = -energizeAmount;
                            if (GetSpellInfo()->Id == SPELL_DRUID_STARSURGE)
                                caster->CastCustomSpell(caster, SPELL_DRUID_STARSURGE_ENERGIZE, &energizeAmount, 0, 0, true);
                            else
                                caster->CastCustomSpell(caster, SPELL_DRUID_ECLIPSE_GENERAL_ENERGIZE, &energizeAmount, 0, 0, true);
                            lunar = caster->GetPower(POWER_ECLIPSE) == -100;
                        }

                        // The energizing effect brought us out of the lunar eclipse, remove the aura
                        if (caster->HasAura(SPELL_DRUID_LUNAR_ECLIPSE) && caster->GetPower(POWER_ECLIPSE) >= 0)
                            caster->RemoveAura(SPELL_DRUID_LUNAR_ECLIPSE);
                        // The energizing effect brought us out of the solar eclipse, remove the aura
                        else if (caster->HasAura(SPELL_DRUID_SOLAR_ECLIPSE) && caster->GetPower(POWER_ECLIPSE) <= 0)
                        {
                            caster->RemoveAurasDueToSpell(SPELL_DRUID_SOLAR_ECLIPSE);
                            caster->RemoveAurasDueToSpell(94338);
                        }
                        break;
                    }
                }
                if (solar || lunar)
                {
                    // Euphoria mana regen
                    if (AuraEffect* euphoria = caster->GetDummyAuraEffect(SPELLFAMILY_DRUID, 4431, EFFECT_2))
                        caster->CastCustomSpell(81070, SPELLVALUE_BASE_POINT0, euphoria->GetAmount(), caster, TRIGGERED_FULL_MASK);
                    
                    // Remove Nature's grace cooldown
                    caster->RemoveSpellCooldown(16886);
                    
                    if (solar)
                        // Sunfire
                        if (AuraEffect* sunfire = caster->GetDummyAuraEffect(SPELLFAMILY_DRUID, 3262, EFFECT_0))
                            caster->CastCustomSpell(94338, SPELLVALUE_BASE_POINT0, 93402, caster, TRIGGERED_FULL_MASK);
                }
            }

            void Register()
            {
                OnEffectHitTarget += SpellEffectFn(spell_dru_eclipse_energize_SpellScript::HandleEnergize, EFFECT_0, SPELL_EFFECT_SCHOOL_DAMAGE);
                OnEffectHitTarget += SpellEffectFn(spell_dru_eclipse_energize_SpellScript::HandleEnergize, EFFECT_1, SPELL_EFFECT_SCHOOL_DAMAGE);
            }
        };

        SpellScript* GetSpellScript() const
        {
            return new spell_dru_eclipse_energize_SpellScript;
        }
};

class spell_dru_energize : public SpellScriptLoader
{
    public:
        spell_dru_energize() : SpellScriptLoader("spell_dru_energize") { }

        class spell_dru_energize_SpellScript : public SpellScript
        {
            PrepareSpellScript(spell_dru_energize_SpellScript);

            bool Load()
            {
                if (GetCaster()->GetTypeId() != TYPEID_PLAYER)
                    return false;

                if (GetCaster()->ToPlayer()->getClass() != CLASS_DRUID)
                    return false;

                return true;
            }

            void OnHit()
            {
                Player* caster = GetCaster()->ToPlayer();

                // No boomy, no deal.
                if (caster->GetPrimaryTalentTree(caster->GetActiveSpec()) != TALENT_TREE_DRUID_BALANCE)
                    return;

                bool lunar = caster->GetPower(POWER_ECLIPSE) == -100;
                bool solar = caster->GetPower(POWER_ECLIPSE) == 100;
                if (caster->GetPower(POWER_ECLIPSE) > 0)
                {
                    if (!caster->HasAura(SPELL_DRUID_SOLAR_ECLIPSE_MARKER))
                        caster->CastSpell(caster, SPELL_DRUID_SOLAR_ECLIPSE_MARKER, true);
                }
                else if (caster->GetPower(POWER_ECLIPSE) < 0)
                {
                    if (!caster->HasAura(SPELL_DRUID_LUNAR_ECLIPSE_MARKER))
                        caster->CastSpell(caster, SPELL_DRUID_LUNAR_ECLIPSE_MARKER, true);
                }
                if (solar || lunar)
                {
                    // Euphoria mana regen
                    if (AuraEffect* euphoria = caster->GetDummyAuraEffect(SPELLFAMILY_DRUID, 4431, EFFECT_2))
                        caster->CastCustomSpell(81070, SPELLVALUE_BASE_POINT0, euphoria->GetAmount(), caster, TRIGGERED_FULL_MASK);
                    
                    // Remove Nature's grace cooldown
                    caster->RemoveSpellCooldown(16886);
                    
                    if (solar)
                    {
                        // Sunfire
                        if (AuraEffect* sunfire = caster->GetDummyAuraEffect(SPELLFAMILY_DRUID, 3262, EFFECT_0))
                            caster->CastCustomSpell(94338, SPELLVALUE_BASE_POINT0, 93402, caster, TRIGGERED_FULL_MASK);

                        if (caster->HasAura(SPELL_DRUID_SOLAR_ECLIPSE_MARKER))
                        {
                            caster->RemoveAurasDueToSpell(SPELL_DRUID_SOLAR_ECLIPSE_MARKER);
                            caster->CastSpell(caster, SPELL_DRUID_LUNAR_ECLIPSE_MARKER, true);
                        }

                        caster->CastSpell(caster, 48517, true);
                    }

                    if (lunar)
                    {
                        if (caster->HasAura(SPELL_DRUID_LUNAR_ECLIPSE_MARKER))
                        {
                            caster->RemoveAurasDueToSpell(SPELL_DRUID_LUNAR_ECLIPSE_MARKER);
                            caster->CastSpell(caster, SPELL_DRUID_SOLAR_ECLIPSE_MARKER, true);
                        }

                        caster->CastSpell(caster, 48518, true);
                    }
                }
            }

            void Register()
            {
                AfterHit += SpellHitFn(spell_dru_energize_SpellScript::OnHit);
            }
        };

        SpellScript* GetSpellScript() const
        {
            return new spell_dru_energize_SpellScript;
        }
};

// Starsurege
class spell_dru_starsurge : public SpellScriptLoader
{
    public:
        spell_dru_starsurge() : SpellScriptLoader("spell_dru_starsurge") { }

        class spell_dru_starsurge_SpellScript : public SpellScript
        {
            PrepareSpellScript(spell_dru_starsurge_SpellScript);

            void HandleProc(SpellEffIndex /*effIndex*/)
            {
                Unit* caster = GetCaster();
                if (!caster || caster->GetTypeId() != TYPEID_PLAYER)
                    return;

                if (caster->HasAura(93400))
                    // Check if starsurge was affected by active starsurge proc
                    if (Spell* spell = GetSpell())
                        if (spell->m_appliedAuras.find(93400) == spell->m_appliedAuras.end())
                            caster->ToPlayer()->RemoveSpellCooldown(78674, true);
            }

            void Register()
            {
                OnEffectHitTarget += SpellEffectFn(spell_dru_starsurge_SpellScript::HandleProc, EFFECT_0, SPELL_EFFECT_SCHOOL_DAMAGE);
            }
        };

        SpellScript* GetSpellScript() const
        {
            return new spell_dru_starsurge_SpellScript();
        }
};

// -1850 - Dash
class spell_dru_dash : public SpellScriptLoader
{
    public:
        spell_dru_dash() : SpellScriptLoader("spell_dru_dash") { }

        class spell_dru_dash_AuraScript : public AuraScript
        {
            PrepareAuraScript(spell_dru_dash_AuraScript);

            void CalculateAmount(AuraEffect const* /*aurEff*/, int32& amount, bool& /*canBeRecalculated*/)
            {
                // do not set speed if not in cat form
                if (GetUnitOwner()->GetShapeshiftForm() != FORM_CAT)
                    amount = 0;
            }

            void Register()
            {
                DoEffectCalcAmount += AuraEffectCalcAmountFn(spell_dru_dash_AuraScript::CalculateAmount, EFFECT_0, SPELL_AURA_MOD_INCREASE_SPEED);
            }
        };

        AuraScript* GetAuraScript() const
        {
            return new spell_dru_dash_AuraScript();
        }
};

class spell_dru_innervate : public SpellScriptLoader
{
    class script_impl : public AuraScript
    {
        PrepareAuraScript(script_impl);

        enum
        {
            SPELL_INNERVATE_MANA        = 54833,
            SPELL_GLYPH_OF_INNERVATE    = 54832,
            SPELL_DREAMSTATE            = 33597,
        };

        bool m_self;

        bool Load()
        {
            Unit const* const caster = GetCaster();
            Unit const* const owner = GetUnitOwner();
            m_self = (caster == owner);
            return owner && caster && caster->GetTypeId() == TYPEID_PLAYER;
        }

        void CalculateAmount(AuraEffect const* aurEff, int32& amount, bool&)
        {
            Unit const* const caster = GetCaster();
            Unit const* const owner = GetUnitOwner();

            int32 value = m_self ? amount : amount / 4.0f;
            if (m_self)
            {
                // Dreamstate
                if (AuraEffect const* const eff = caster->GetAuraEffectOfRankedSpell(SPELL_DREAMSTATE, EFFECT_0))
                    value += eff->GetAmount();
            }

            int32 const mana = CalculatePct(caster->GetMaxPower(POWER_MANA), value);
            amount = int32(mana / aurEff->GetTotalTicks());
        }

        void OnApply(AuraEffect const*, AuraEffectHandleModes)
        {
            if (m_self)
                return;

            // Glyph of Innervate
            Unit* const caster = GetCaster();
            if (caster->HasAura(SPELL_GLYPH_OF_INNERVATE))
                caster->CastSpell(caster, SPELL_INNERVATE_MANA, true);
        }

        void Register()
        {
            DoEffectCalcAmount += AuraEffectCalcAmountFn(script_impl::CalculateAmount, EFFECT_0, SPELL_AURA_PERIODIC_ENERGIZE);
            OnEffectApply += AuraEffectApplyFn(script_impl::OnApply, EFFECT_0, SPELL_AURA_PERIODIC_ENERGIZE, AURA_EFFECT_HANDLE_REAL);
        }
    };

public:
    spell_dru_innervate()
        : SpellScriptLoader("spell_dru_innervate")
    {
    }

    AuraScript* GetAuraScript() const
    {
        return new script_impl();
    }
};

// Solar Beam
class spell_dru_beam : public SpellScriptLoader
{
    public:
        spell_dru_beam() : SpellScriptLoader("spell_dru_beam") { }

        class spell_dru_beam_AuraScript : public AuraScript
        {
            PrepareAuraScript(spell_dru_beam_AuraScript);

            bool Validate(SpellInfo const* /*spellInfo*/) { return true; }

            void HandleEffectPeriodic(AuraEffect const* aurEff)
            {
                if (DynamicObject* dyn = GetTarget()->GetDynObject(aurEff->GetId()))
                    GetTarget()->CastSpell(dyn->GetPositionX(), dyn->GetPositionY(), dyn->GetPositionZ(), 81261, true);
            }

            void Register()
            {
                OnEffectPeriodic += AuraEffectPeriodicFn(spell_dru_beam_AuraScript::HandleEffectPeriodic, EFFECT_2, SPELL_AURA_PERIODIC_DUMMY);
            }
        };

        AuraScript* GetAuraScript() const
        {
            return new spell_dru_beam_AuraScript();
        }
};

// Rejuvenation
class spell_dru_rejuv : public SpellScriptLoader
{
    public:
        spell_dru_rejuv() : SpellScriptLoader("spell_dru_rejuv") { }

        class spell_dru_rejuv_AuraScript : public AuraScript
        {
            PrepareAuraScript(spell_dru_rejuv_AuraScript);

            void AfterRemove(AuraEffect const* aurEff, AuraEffectHandleModes /*mode*/)
            {
                if (Unit* caster = GetCaster())
                {
                    if (!caster->ToPlayer())
                        return;
                    
                    uint32 rejuvCount = 0;
                    Player::appliedAurasList const& auras = caster->ToPlayer()->appliedAuras;

                    if (auras.empty())
                        return;
                    
                    for (Player::appliedAurasList::const_iterator itr = auras.begin(); itr != auras.end(); itr++)
                    {
                        Aura* aura = (*itr);
                        if (aura->GetId() == GetSpellInfo()->Id)
                            rejuvCount++;
                    }
                    if (rejuvCount < 3)
                        caster->RemoveAurasDueToSpell(96206);
                }
            }

            void AfterApply(AuraEffect const* aurEff, AuraEffectHandleModes /*mode*/)
            {
                if (Unit* caster = GetCaster())
                {
                    if (AuraEffect* naturesBounty = caster->GetAuraEffect(SPELL_AURA_ADD_FLAT_MODIFIER, SPELLFAMILY_DRUID, 197, EFFECT_0))
                    {
                        uint32 rejuvCount = 0;
                        Player::appliedAurasList const& auras = caster->ToPlayer()->appliedAuras;
                        for (Player::appliedAurasList::const_iterator itr = auras.begin(); itr != auras.end(); itr++)
                        {
                            Aura* aura = (*itr);
                            if (aura->GetId() == GetSpellInfo()->Id)
                                rejuvCount++;
                        }
                        if (rejuvCount >= 3)
                        {
                            int32 bp0 = -naturesBounty->GetSpellInfo()->Effects[EFFECT_1].BasePoints;
                            caster->CastCustomSpell(caster, 96206, &bp0, NULL, NULL, true);
                        }
                    }

                    // Gift of the earthmother
                    if (AuraEffect* earthMother = caster->GetDummyAuraEffect(SPELLFAMILY_DRUID, 3186, EFFECT_0))
                    {
                        int32 bp0 = caster->SpellHealingBonusDone(GetUnitOwner(), GetSpellInfo(), aurEff->GetAmount(), DOT);
                        bp0 = (bp0 * aurEff->GetTotalTicks()) * (earthMother->GetAmount() / 100.0f);
                        caster->CastCustomSpell(GetUnitOwner(), 64801, &bp0, NULL, NULL, true);
                    }
                }
            }

            void Register()
            {
                AfterEffectApply += AuraEffectApplyFn(spell_dru_rejuv_AuraScript::AfterApply, EFFECT_0, SPELL_AURA_PERIODIC_HEAL, AURA_EFFECT_HANDLE_REAL);
                AfterEffectRemove += AuraEffectRemoveFn(spell_dru_rejuv_AuraScript::AfterRemove, EFFECT_0, SPELL_AURA_PERIODIC_HEAL, AURA_EFFECT_HANDLE_REAL);
            }
        };

        AuraScript* GetAuraScript() const
        {
            return new spell_dru_rejuv_AuraScript();
        }
};

// 88747 - Wild mushroom
class spell_dru_wild_mushroom : public SpellScriptLoader
{
    public:
        spell_dru_wild_mushroom() : SpellScriptLoader("spell_dru_wild_mushroom") { }

        class spell_dru_wild_mushroom_SpellScript : public SpellScript
        {
            PrepareSpellScript(spell_dru_wild_mushroom_SpellScript)

            void HandleSummon(SpellEffIndex effIndex)
            {
                PreventHitDefaultEffect(effIndex);

                Unit* caster = GetCaster();
                WorldLocation const* targetDest = GetExplTargetDest();
                SpellInfo const* spellInfo = GetSpellInfo();

                if(caster && targetDest)
                {
                    if (Player* player = caster->ToPlayer())
                    {
                        Position pos;
                        std::list<Creature*> list;

                        Trinity::AnyUnfriendlyCreatureInUnitRangeCheck check(player, SPELL_DRUID_NPC_WILD_MUSHROOM, 100.0f);
                        Trinity::CreatureListSearcher<Trinity::AnyUnfriendlyCreatureInUnitRangeCheck> searcher(player, list, check);
                        player->VisitNearbyGridObject(100.0f, searcher);

                        // Max 3 Wild Mushroom
                        if ((int32)list.size() >= GetEffectValue())
                        {
                            if(list.back())
                            {
                                if(TempSummon* temp = list.back()->ToTempSummon())
                                {
                                    temp->UnSummon();
                                }
                            }
                        }
                        
                        // Summon position
                        targetDest->GetPosition(&pos);

                        // Summon properties
                        const SummonPropertiesEntry* properties = sSummonPropertiesStore.LookupEntry(spellInfo->Effects[effIndex].MiscValueB);
                                                      
                        TempSummon* summon = player->SummonCreature(spellInfo->Effects[0].MiscValue, pos, TEMPSUMMON_TIMED_OR_CORPSE_DESPAWN, spellInfo->GetDuration());

                        if (!summon)
                            return;
                    
                        summon->SetUInt64Value(UNIT_FIELD_SUMMONEDBY, player->GetGUID());
                        summon->setFaction(player->getFaction());
                        summon->SetUInt32Value(UNIT_CREATED_BY_SPELL, GetSpellInfo()->Id);
                        summon->SetMaxHealth(5);
                        summon->SetFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_DISABLE_MOVE);
                        summon->StopMoving();
                        summon->SetControlled(true, UNIT_STATE_STUNNED);
                    }
                }
            }

            void Register()
            {
                OnEffectHit += SpellEffectFn(spell_dru_wild_mushroom_SpellScript::HandleSummon, EFFECT_0, SPELL_EFFECT_SUMMON);
            }
        };

        SpellScript* GetSpellScript() const
        {
            return new spell_dru_wild_mushroom_SpellScript();
        }
};

// 88751 - Wild mushroom : Detonate
class spell_dru_wild_mushroom_detonation : public SpellScriptLoader
{
    public:
        spell_dru_wild_mushroom_detonation() : SpellScriptLoader("spell_dru_wild_mushroom_detonation") { }

        class spell_dru_wild_mushroom_detonation_SpellScript : public SpellScript
        {
            PrepareSpellScript(spell_dru_wild_mushroom_detonation_SpellScript)

            bool Load()
            {
                spellRange = GetSpellInfo()->GetMaxRange();

                if (!spellRange)
                    return false;

                if(Unit* caster = GetCaster())
                {
                    if(Player* player = caster->ToPlayer())
                    {
                        std::list<Creature*> list;
                        
                        Trinity::AnyUnfriendlyCreatureInUnitRangeCheck check(player, SPELL_DRUID_NPC_WILD_MUSHROOM, 100.0f);
                        Trinity::CreatureListSearcher<Trinity::AnyUnfriendlyCreatureInUnitRangeCheck> searcher(player, mushroomList, check);
                        player->VisitNearbyGridObject(100.0f, searcher);

                        return true;
                    }
                }

                return false;
            }

            SpellCastResult CheckCast()
            {
                if(Unit* caster = GetCaster())
                {
                    if(Player* player = caster->ToPlayer())
                    {
                        if (mushroomList.empty())
                            return SPELL_FAILED_CANT_DO_THAT_RIGHT_NOW;

                        bool inRange = false;

                        for (std::list<Creature*>::const_iterator i = mushroomList.begin(); i != mushroomList.end(); i)
                        {
                            if((*i))
                            {
                                Position pos;
                                (*i)->GetPosition(&pos);

                                // Must have at least one mushroom within 40 yards
                                if (player->IsWithinDist3d(&pos, spellRange))
                                {
                                    return SPELL_CAST_OK;
                                }
                            }

                            i++;
                        }

                        SetCustomCastResultMessage(SPELL_CUSTOM_ERROR_TARGET_TOO_FAR);
                        return SPELL_FAILED_CUSTOM_ERROR;
                    }
                }
                return SPELL_FAILED_CASTER_DEAD;
            }

            void HandleDummy(SpellEffIndex /*effIndex*/)
            {
                if(Unit* caster = GetCaster())
                {
                    if(Player* player = caster->ToPlayer())
                    {
                        int32 fungalGrowthSlowSpellId;
                        
                        for (std::list<Creature*>::const_iterator i = mushroomList.begin(); i != mushroomList.end(); i)
                        {
                            Position pos;
                            Creature* tempMushroom = (*i);
                            
                            if(tempMushroom)
                            {
                                tempMushroom->GetPosition(&pos);

                                // Explosion visual and suicide
                                tempMushroom->CastSpell(tempMushroom, SPELL_DRUID_SPELL_WILD_MUSHROOM_SUICIDE, true);

                                // Explosion damage
                                player->CastSpell(
                                    pos.GetPositionX(), 
                                    pos.GetPositionY(), 
                                    pos.GetPositionZ(), 
                                    SPELL_DRUID_SPELL_WILD_MUSHROOM_DAMAGE, true);

                                // Fungal Growth
                                if (AuraEffect* aurEff = caster->GetDummyAuraEffect(SPELLFAMILY_DRUID, 2681, EFFECT_0))
                                {
                                    fungalGrowthSlowSpellId = aurEff->GetMiscValue();
                                    
                                    // Summon fungal
                                    player->CastSpell(
                                        pos.GetPositionX(), 
                                        pos.GetPositionY(), 
                                        pos.GetPositionZ(), 
                                        SPELL_DRUID_FUNGAL_GROWTH_SUMMON, true);
                                }
                            }

                            i++;
                        }

                        // Frees the memory
                        mushroomList.clear();

                        // Fungal Growth spells
                        if(fungalGrowthSlowSpellId)
                        {
                            if(SpellInfo const* fungalGrowthSpellInfo = sSpellMgr->GetSpellInfo(SPELL_DRUID_FUNGAL_GROWTH_SUMMON))
                            {
                                Trinity::AnyUnfriendlyCreatureInUnitRangeCheck check(player, fungalGrowthSpellInfo->Effects[0].MiscValue, 100.0f);
                                Trinity::CreatureListSearcher<Trinity::AnyUnfriendlyCreatureInUnitRangeCheck> searcher(player, mushroomList, check);
                                player->VisitNearbyGridObject(100.0f, searcher);

                                for (std::list<Creature*>::const_iterator i = mushroomList.begin(); i != mushroomList.end(); i)
                                {
                                    Creature* tempFungal = (*i);

                                    if(tempFungal)
                                    {
                                        tempFungal->SetUInt64Value(UNIT_FIELD_SUMMONEDBY, player->GetGUID());
                                        tempFungal->SetUInt32Value(UNIT_CREATED_BY_SPELL, SPELL_DRUID_FUNGAL_GROWTH_SUMMON);
                                        tempFungal->SetMaxHealth(5);
                                        tempFungal->SetFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_DISABLE_MOVE);
                                        tempFungal->SetFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_NOT_SELECTABLE);
                                        tempFungal->StopMoving();
                                        tempFungal->SetControlled(true, UNIT_STATE_STUNNED);
                                        
                                        // Graphical effect
                                        tempFungal->CastSpell(tempFungal, SPELL_DRUID_FUNGAL_GROWTH_GRAPHIC, true);
                                        
                                        // Slow effect
                                        tempFungal->CastSpell(tempFungal, fungalGrowthSlowSpellId, true);
                                    }
                                
                                    i++;
                                }
                            }
                        }
                    }
                }
            }
            
        private:
            float spellRange;
            std::list<Creature*> mushroomList;

            void Register() 
            {
                OnCheckCast += SpellCheckCastFn(spell_dru_wild_mushroom_detonation_SpellScript::CheckCast);
                OnEffectHitTarget += SpellEffectFn(spell_dru_wild_mushroom_detonation_SpellScript::HandleDummy, EFFECT_0, SPELL_EFFECT_DUMMY);
            }
        };

        SpellScript* GetSpellScript() const
        {
            return new spell_dru_wild_mushroom_detonation_SpellScript();
        }
};


// -5229 - Enrage
class spell_dru_enrage : public SpellScriptLoader
{
    public:
        spell_dru_enrage() : SpellScriptLoader("spell_dru_enrage") { }

        class spell_dru_enrage_SpellScript : public SpellScript
        {
            PrepareSpellScript(spell_dru_enrage_SpellScript);

            void OnHit()
            {
                // King of the jungle
                if (AuraEffect const* aurEff = GetHitUnit()->GetDummyAuraEffect(SPELLFAMILY_DRUID, 2850, EFFECT_1))
                    GetHitUnit()->CastCustomSpell(SPELL_DRUID_ENRAGE_MOD_DAMAGE, SPELLVALUE_BASE_POINT0, aurEff->GetAmount(), GetHitUnit(), true);
            }

            void Register()
            {
                AfterHit += SpellHitFn(spell_dru_enrage_SpellScript::OnHit);
            }
        };

        SpellScript* GetSpellScript() const
        {
            return new spell_dru_enrage_SpellScript();
        }
};

// Stampede
class spell_dru_stampede : public SpellScriptLoader
{
    public:
        spell_dru_stampede() : SpellScriptLoader("spell_dru_stampede") { }

        class spell_dru_stampede_SpellScript : public SpellScript
        {
            PrepareSpellScript(spell_dru_stampede_SpellScript);

            void OnHit()
            {
                Unit* caster = GetCaster();
                uint32 triggeredId = 0;
                switch (GetSpellInfo()->Id)
                {                    
                    case 16979: // Feral charge bear
                        if (AuraEffect* stampede = caster->GetDummyAuraEffect(SPELLFAMILY_DRUID, 3930, EFFECT_0))
                        {
                            switch (stampede->GetAmount())
                            {
                                case 5:
                                    triggeredId = 81016;
                                    break;
                                case 10:
                                    triggeredId = 81017;
                                    break;
                            }
                        }
                        break;
                    case 49376: // Feral charge cat
                        if (AuraEffect* stampede = caster->GetDummyAuraEffect(SPELLFAMILY_DRUID, 3930, EFFECT_1))
                        {
                            switch (stampede->GetAmount())
                            {
                                case 3:
                                    triggeredId = 81021;
                                    break;
                                case 6:
                                    triggeredId = 81022;
                                    break;
                            }
                            caster->CastSpell(caster, 109881, true);
                        }
                        break;
                }
                if (triggeredId)
                    caster->CastSpell(caster, triggeredId, true);
            }

            void Register()
            {
                AfterHit += SpellHitFn(spell_dru_stampede_SpellScript::OnHit);
            }
        };

        SpellScript* GetSpellScript() const
        {
            return new spell_dru_stampede_SpellScript();
        }
};

// Feral swiftness
class spell_dru_feral_swiftness : public SpellScriptLoader
{
    public:
        spell_dru_feral_swiftness() : SpellScriptLoader("spell_dru_feral_swiftness") { }

        class spell_dru_feral_swiftness_SpellScript : public SpellScript
        {
            PrepareSpellScript(spell_dru_feral_swiftness_SpellScript);

            void OnHit()
            {
                if (AuraEffect const* feralSwiftness = GetCaster()->GetAuraEffect(SPELL_AURA_MOD_DODGE_PERCENT, SPELLFAMILY_DRUID, 67, EFFECT_0))
                {
                    uint32 chance = feralSwiftness->GetId() == 24867 ? 50 : 100;
                    if (roll_chance_i(chance))
                        GetCaster()->CastSpell(GetHitUnit(), 97985, true);
                }
            }

            void Register()
            {
                AfterHit += SpellHitFn(spell_dru_feral_swiftness_SpellScript::OnHit);
            }
        };

        SpellScript* GetSpellScript() const
        {
            return new spell_dru_feral_swiftness_SpellScript();
        }
};

// Skull Bash
class spell_dru_skull_bash : public SpellScriptLoader
{
    public:
        spell_dru_skull_bash() : SpellScriptLoader("spell_dru_skull_bash") { }

        class spell_dru_skull_bash_SpellScript : public SpellScript
        {
            PrepareSpellScript(spell_dru_skull_bash_SpellScript);

            void HandleDummy(SpellEffIndex /*effIndex*/)
            {
                Unit* caster = GetCaster();
                Unit* victim = GetHitUnit();
                caster->CastSpell(victim, 93983, true);
                caster->CastSpell(victim, 93985, true);
                // Brutal impact
                if (AuraEffect* aurEff = caster->GetAuraEffect(SPELL_AURA_ADD_FLAT_MODIFIER, SPELLFAMILY_DRUID, 473, EFFECT_0))
                {
                    uint32 triggeredId = 0;
                    switch (aurEff->GetId())
                    {
                        case 16940:
                            triggeredId = 82364;
                            break;
                        case 16941:
                            triggeredId = 82365;
                            break;
                    }
                    caster->CastSpell(victim, triggeredId, true);
                }
            }

            void Register()
            {
                OnEffectHitTarget += SpellEffectFn(spell_dru_skull_bash_SpellScript::HandleDummy, EFFECT_0, SPELL_EFFECT_DUMMY);
            }
        };

        SpellScript* GetSpellScript() const
        {
            return new spell_dru_skull_bash_SpellScript();
        }
};

// Primal madness
class spell_dru_primal_madness : public SpellScriptLoader
{
    public:
        spell_dru_primal_madness() : SpellScriptLoader("spell_dru_primal_madness") { }

        class spell_dru_primal_madness_AuraScript : public AuraScript
        {
            PrepareAuraScript(spell_dru_primal_madness_AuraScript);

            void OnApply(AuraEffect const* aurEff, AuraEffectHandleModes /*mode*/)
            {
                // Primal Madness
                if (AuraEffect const* primalMadness = GetCaster()->GetAuraEffect(SPELL_AURA_ADD_FLAT_MODIFIER, SPELLFAMILY_DRUID, 1181, EFFECT_1))
                {
                    uint32 triggeredId = 0;
                    switch (primalMadness->GetId())
                    {
                        case 80316:
                            triggeredId = 80879;
                            break;
                        case 80317:
                            triggeredId = 80886;
                            break;
                    }
                    GetCaster()->CastSpell(GetCaster(), triggeredId, true);
                }
            }

            void OnRemove(AuraEffect const* /*aurEff*/, AuraEffectHandleModes /*mode*/)
            {
                if (Unit* caster = GetCaster())
                {
                    caster->RemoveAurasDueToSpell(80879);
                    caster->RemoveAurasDueToSpell(80886);
                }
            }

            void Register()
            {
                AfterEffectApply += AuraEffectApplyFn(spell_dru_primal_madness_AuraScript::OnApply, EFFECT_0, SPELL_AURA_ADD_PCT_MODIFIER, AURA_EFFECT_HANDLE_REAL);
                AfterEffectApply += AuraEffectApplyFn(spell_dru_primal_madness_AuraScript::OnApply, EFFECT_0, SPELL_AURA_MOD_DAMAGE_PERCENT_DONE, AURA_EFFECT_HANDLE_REAL);
                AfterEffectRemove += AuraEffectRemoveFn(spell_dru_primal_madness_AuraScript::OnRemove, EFFECT_0, SPELL_AURA_ADD_PCT_MODIFIER, AURA_EFFECT_HANDLE_REAL);
                AfterEffectRemove += AuraEffectRemoveFn(spell_dru_primal_madness_AuraScript::OnRemove, EFFECT_0, SPELL_AURA_MOD_DAMAGE_PERCENT_DONE, AURA_EFFECT_HANDLE_REAL);
            }
        };

        AuraScript* GetAuraScript() const
        {
            return new spell_dru_primal_madness_AuraScript();
        }
};

// Feral aggression
class spell_dru_feral_aggression : public SpellScriptLoader
{
    public:
        spell_dru_feral_aggression() : SpellScriptLoader("spell_dru_feral_aggression") { }


        class spell_dru_feral_aggression_SpellScript : public SpellScript
        {
            PrepareSpellScript(spell_dru_feral_aggression_SpellScript);

            void OnHitTarget(SpellEffIndex effIndex)
            {
                if (Unit* caster = GetCaster())
                    if (AuraEffect const* feralSwiftness = caster->GetDummyAuraEffect(SPELLFAMILY_DRUID, 960, EFFECT_0))
                    {
                        PreventHitDefaultEffect(effIndex);
                        caster->CastCustomSpell(91565, SPELLVALUE_AURA_STACK, feralSwiftness->GetAmount(), GetHitUnit(), true);
                    }
            }

            void Register()
            {
                OnEffectHitTarget += SpellEffectFn(spell_dru_feral_aggression_SpellScript::OnHitTarget, EFFECT_0, SPELL_EFFECT_TRIGGER_SPELL);
            }
        };

        SpellScript* GetSpellScript() const
        {
            return new spell_dru_feral_aggression_SpellScript();
        }
};

// Barkskin
class spell_dru_barkskin : public SpellScriptLoader
{
    public:
        spell_dru_barkskin() : SpellScriptLoader("spell_dru_barkskin") { }

        class spell_dru_barkskin_SpellScript : public SpellScript
        {
            PrepareSpellScript(spell_dru_barkskin_SpellScript);

            void HandleOnHit()
            {
                Unit* caster = GetCaster();
                if (AuraEffect const* glyph = caster->GetDummyAuraEffect(SPELLFAMILY_DRUID, 689, EFFECT_0))
                    caster->CastSpell(caster, 63058, true);
            }

            void Register()
            {
                OnHit += SpellHitFn(spell_dru_barkskin_SpellScript::HandleOnHit);
            }
        };

        SpellScript* GetSpellScript() const
        {
            return new spell_dru_barkskin_SpellScript();
        }
};

// Harmony (Mastery)
class spell_dru_harmony : public SpellScriptLoader
{
    public:
        spell_dru_harmony() : SpellScriptLoader("spell_dru_harmony") { }

        class spell_dru_harmony_SpellScript : public SpellScript
        {
            PrepareSpellScript(spell_dru_harmony_SpellScript);

            void OnHit()
            {
                if (AuraEffect const* mastery = GetCaster()->GetAuraEffect(77495, EFFECT_0, GetCaster()->GetGUID()))
                {
                    int32 bp0 = mastery->GetAmount();
                    GetCaster()->CastCustomSpell(GetCaster(), 100977, &bp0, NULL, NULL, true);
                }             
            }

            void Register()
            {
                AfterHit += SpellHitFn(spell_dru_harmony_SpellScript::OnHit);
            }
        };

        SpellScript* GetSpellScript() const
        {
            return new spell_dru_harmony_SpellScript();
        }
};

// Empowered touch
class spell_dru_emp_touch : public SpellScriptLoader
{
    public:
        spell_dru_emp_touch() : SpellScriptLoader("spell_dru_emp_touch") { }

        class spell_dru_emp_touch_SpellScript : public SpellScript
        {
            PrepareSpellScript(spell_dru_emp_touch_SpellScript);

            void OnHit()
            {
                if (AuraEffect const* aurEff = GetCaster()->GetDummyAuraEffect(SPELLFAMILY_DRUID, 2251, EFFECT_1))
                    if (roll_chance_i(aurEff->GetAmount()))
                        GetCaster()->CastSpell(GetHitUnit(), 88433, true);
            }

            void Register()
            {
                AfterHit += SpellHitFn(spell_dru_emp_touch_SpellScript::OnHit);
            }
        };

        SpellScript* GetSpellScript() const
        {
            return new spell_dru_emp_touch_SpellScript();
        }
};

// Empowered touch
class spell_dru_emp_touch_triggered : public SpellScriptLoader
{
    public:
        spell_dru_emp_touch_triggered() : SpellScriptLoader("spell_dru_emp_touch_triggered") { }

        class spell_dru_emp_touch_triggered_SpellScript : public SpellScript
        {
            PrepareSpellScript(spell_dru_emp_touch_triggered_SpellScript);

            void OnHit()
            {
                if (GetCaster())
                    if (Aura* aura = GetCaster()->GetAura(33763, GetCaster()->GetGUID()))
                        aura->RefreshDuration();
            }

            void Register()
            {
                AfterHit += SpellHitFn(spell_dru_emp_touch_triggered_SpellScript::OnHit);
            }
        };

        SpellScript* GetSpellScript() const
        {
            return new spell_dru_emp_touch_triggered_SpellScript();
        }
};

// Efflorescence
class spell_sha_efflorescence : public SpellScriptLoader
{
    public:
        spell_sha_efflorescence() : SpellScriptLoader("spell_sha_efflorescence") { }

        class spell_sha_efflorescence_AuraScript : public AuraScript
        {
            PrepareAuraScript(spell_sha_efflorescence_AuraScript);

            void HandleEffectPeriodic(AuraEffect const* aurEff)
            {
                if (DynamicObject* dyn = GetTarget()->GetDynObject(aurEff->GetId()))
                    GetTarget()->CastSpell(dyn->GetPositionX(), dyn->GetPositionY(), dyn->GetPositionZ(), 81269, true);
            }

            void Register()
            {
                OnEffectPeriodic += AuraEffectPeriodicFn(spell_sha_efflorescence_AuraScript::HandleEffectPeriodic, EFFECT_1, SPELL_AURA_PERIODIC_DUMMY);
            }
        };

        AuraScript* GetAuraScript() const
        {
            return new spell_sha_efflorescence_AuraScript();
        }
};

// Efflorescence heal
class spell_sha_efflorescence_heal : public SpellScriptLoader
{
    public:
        spell_sha_efflorescence_heal() : SpellScriptLoader("spell_sha_efflorescence_heal") { }

        class spell_sha_efflorescence_heal_SpellScript : public SpellScript
        {
            PrepareSpellScript(spell_sha_efflorescence_heal_SpellScript);

            void FilterTargets(std::list<WorldObject*>& targets)
            {
                std::list<Unit*> temp;
                for (std::list<WorldObject*>::iterator itr = targets.begin(); itr != targets.end(); itr++)
                    if (Unit* unit = (*itr)->ToUnit())
                        temp.push_back(unit);

                targets.clear();
                temp.sort(Trinity::HealthPctOrderPred());
                if (temp.size() > 3)
                    temp.resize(3);                
                for (std::list<Unit*>::iterator itr = temp.begin(); itr != temp.end(); itr++)
                    targets.push_back((WorldObject*)(*itr));
            }

            void HandleHeal(SpellEffIndex /*effIndex*/)
            {
                if (AuraEffect* aurEff = GetCaster()->GetAuraEffect(81262, EFFECT_1, GetCaster()->GetGUID()))
                    SetHitHeal(aurEff->GetAmount());
            }

            void Register()
            {
                OnObjectAreaTargetSelect += SpellObjectAreaTargetSelectFn(spell_sha_efflorescence_heal_SpellScript::FilterTargets, EFFECT_0, TARGET_UNIT_DEST_AREA_ALLY);
                OnEffectHitTarget += SpellEffectFn(spell_sha_efflorescence_heal_SpellScript::HandleHeal, EFFECT_0, SPELL_EFFECT_HEAL);
            }
        };

        SpellScript* GetSpellScript() const
        {
            return new spell_sha_efflorescence_heal_SpellScript();
        }
};

// Starfire
class spell_dru_glyph_of_starfire : public SpellScriptLoader
{
    public:
        spell_dru_glyph_of_starfire() : SpellScriptLoader("spell_dru_glyph_of_starfire") { }

        class spell_dru_glyph_of_starfire_SpellScript : public SpellScript
        {
            PrepareSpellScript(spell_dru_glyph_of_starfire_SpellScript);

            void HandleOnHit()
            {
                Unit* caster = GetCaster();
                if (Unit* unitTarget = GetHitUnit())
                    if (AuraEffect* glyph = caster->GetAuraEffect(54845, EFFECT_0, caster->GetGUID()))
                        if (Aura* moonfire = unitTarget->GetAura(8921, caster->GetGUID()))
                        {
                            // Only last applied moonfire can be increased
                            if (caster->appliedAuras.back() != moonfire)
                                return;

                            uint32 countMin = moonfire->GetMaxDuration();
                            uint32 countMax = moonfire->GetSpellInfo()->GetMaxDuration() + 9000;
                            if (caster->HasAura(SPELL_DRUID_INCREASED_MOONFIRE_DURATION))
                                countMax += 3000;

                            if (countMin < countMax)
                            {
                                moonfire->SetDuration(uint32(moonfire->GetDuration() + 3000), false, true);
                                moonfire->SetMaxDuration(countMin + 3000);
                            }
                        }
            }

            void Register()
            {
                OnHit += SpellHitFn(spell_dru_glyph_of_starfire_SpellScript::HandleOnHit);
            }
        };

        SpellScript* GetSpellScript() const
        {
            return new spell_dru_glyph_of_starfire_SpellScript();
        }
};

// 34246 - Idol of the Emerald Queen
// 60779 - Idol of Lush Moss
class spell_dru_idol_lifebloom : public SpellScriptLoader
{
    public:
        spell_dru_idol_lifebloom() : SpellScriptLoader("spell_dru_idol_lifebloom") { }

        class spell_dru_idol_lifebloom_AuraScript : public AuraScript
        {
            PrepareAuraScript(spell_dru_idol_lifebloom_AuraScript);

            void HandleEffectCalcSpellMod(AuraEffect const* aurEff, SpellModifier*& spellMod)
            {
                if (!spellMod)
                {
                    spellMod = new SpellModifier(GetAura());
                    spellMod->op = SPELLMOD_DOT;
                    spellMod->type = SPELLMOD_FLAT;
                    spellMod->spellId = GetId();
                    spellMod->mask = GetSpellInfo()->Effects[aurEff->GetEffIndex()].SpellClassMask;
                }
                spellMod->value = aurEff->GetAmount() / 7;
            }

            void Register()
            {
                DoEffectCalcSpellMod += AuraEffectCalcSpellModFn(spell_dru_idol_lifebloom_AuraScript::HandleEffectCalcSpellMod, EFFECT_0, SPELL_AURA_DUMMY);
            }
        };

        AuraScript* GetAuraScript() const
        {
            return new spell_dru_idol_lifebloom_AuraScript();
        }
};

// -5570 - Insect Swarm
class spell_dru_insect_swarm : public SpellScriptLoader
{
    public:
        spell_dru_insect_swarm() : SpellScriptLoader("spell_dru_insect_swarm") { }

        class spell_dru_insect_swarm_AuraScript : public AuraScript
        {
            PrepareAuraScript(spell_dru_insect_swarm_AuraScript);

            void CalculateAmount(AuraEffect const* aurEff, int32 & amount, bool & /*canBeRecalculated*/)
            {
                if (Unit* caster = GetCaster())
                    if (AuraEffect const* relicAurEff = caster->GetAuraEffect(SPELL_DRUID_ITEM_T8_BALANCE_RELIC, EFFECT_0))
                        amount += relicAurEff->GetAmount() / aurEff->GetTotalTicks();
            }

            void Register()
            {
                 DoEffectCalcAmount += AuraEffectCalcAmountFn(spell_dru_insect_swarm_AuraScript::CalculateAmount, EFFECT_0, SPELL_AURA_PERIODIC_DAMAGE);
            }
        };

        AuraScript* GetAuraScript() const
        {
            return new spell_dru_insect_swarm_AuraScript();
        }
};

class spell_dru_lifebloom : public SpellScriptLoader
{
    public:
        spell_dru_lifebloom() : SpellScriptLoader("spell_dru_lifebloom") { }

        class spell_dru_lifebloom_AuraScript : public AuraScript
        {
            PrepareAuraScript(spell_dru_lifebloom_AuraScript);

            bool Validate(SpellInfo const* /*spell*/)
            {
                if (!sSpellMgr->GetSpellInfo(SPELL_DRUID_LIFEBLOOM_FINAL_HEAL))
                    return false;
                if (!sSpellMgr->GetSpellInfo(SPELL_DRUID_LIFEBLOOM_ENERGIZE))
                    return false;
                return true;
            }

            void AfterRemove(AuraEffect const* aurEff, AuraEffectHandleModes /*mode*/)
            {
                // Final heal only on duration end
                if (GetTargetApplication()->GetRemoveMode() != AURA_REMOVE_BY_EXPIRE)
                    return;

                // final heal
                int32 stack = GetStackAmount();
                int32 healAmount = aurEff->GetAmount();
                if (Unit* caster = GetCaster())
                {
                    healAmount = caster->SpellHealingBonusDone(GetTarget(), GetSpellInfo(), healAmount, HEAL, stack);
                    healAmount = GetTarget()->SpellHealingBonusTaken(GetSpellInfo(), healAmount, HEAL, stack, GetCasterGUID());

                    GetTarget()->CastCustomSpell(GetTarget(), SPELL_DRUID_LIFEBLOOM_FINAL_HEAL, &healAmount, NULL, NULL, true, NULL, aurEff, GetCasterGUID());

                    // restore mana
                    int32 returnMana = CalculatePct(caster->GetCreateMana(), GetSpellInfo()->ManaCostPercentage) * stack / 2;
                    caster->CastCustomSpell(caster, SPELL_DRUID_LIFEBLOOM_ENERGIZE, &returnMana, NULL, NULL, true, NULL, aurEff, GetCasterGUID());
                    return;
                }

                GetTarget()->CastCustomSpell(GetTarget(), SPELL_DRUID_LIFEBLOOM_FINAL_HEAL, &healAmount, NULL, NULL, true, NULL, aurEff, GetCasterGUID());
            }

            void HandleDispel(DispelInfo* dispelInfo)
            {
                if (Unit* target = GetUnitOwner())
                {
                    if (AuraEffect const* aurEff = GetEffect(EFFECT_1))
                    {
                        // final heal
                        int32 healAmount = aurEff->GetAmount();
                        if (Unit* caster = GetCaster())
                        {
                            healAmount = caster->SpellHealingBonusDone(target, GetSpellInfo(), healAmount, HEAL, dispelInfo->GetRemovedCharges());
                            healAmount = target->SpellHealingBonusTaken(GetSpellInfo(), healAmount, HEAL, dispelInfo->GetRemovedCharges(), GetCasterGUID());
                            target->CastCustomSpell(target, SPELL_DRUID_LIFEBLOOM_FINAL_HEAL, &healAmount, NULL, NULL, true, NULL, NULL, GetCasterGUID());

                            // restore mana
                            int32 returnMana = CalculatePct(caster->GetCreateMana(), GetSpellInfo()->ManaCostPercentage) * dispelInfo->GetRemovedCharges() / 2;
                            caster->CastCustomSpell(caster, SPELL_DRUID_LIFEBLOOM_ENERGIZE, &returnMana, NULL, NULL, true, NULL, NULL, GetCasterGUID());
                            return;
                        }

                        target->CastCustomSpell(target, SPELL_DRUID_LIFEBLOOM_FINAL_HEAL, &healAmount, NULL, NULL, true, NULL, NULL, GetCasterGUID());
                    }
                }
            }

            void Register()
            {
                AfterEffectRemove += AuraEffectRemoveFn(spell_dru_lifebloom_AuraScript::AfterRemove, EFFECT_1, SPELL_AURA_DUMMY, AURA_EFFECT_HANDLE_REAL);
                AfterDispel += AuraDispelFn(spell_dru_lifebloom_AuraScript::HandleDispel);
            }
        };

        AuraScript* GetAuraScript() const
        {
            return new spell_dru_lifebloom_AuraScript();
        }
};

// Faerie Fire
class spell_dru_faerie_fire : public SpellScriptLoader
{
    public:
        spell_dru_faerie_fire() : SpellScriptLoader("spell_dru_faerie_fire") { }

        class spell_dru_faerie_fire_AuraScript : public AuraScript
        {
            PrepareAuraScript(spell_dru_faerie_fire_AuraScript);

            void HandleDispel(DispelInfo* dispelInfo)
            {
                if (Unit* target = GetUnitOwner())
                    target->RemoveAura(GetId());
            }

            void Register()
            {
                AfterDispel += AuraDispelFn(spell_dru_faerie_fire_AuraScript::HandleDispel);
            }
        };

        AuraScript* GetAuraScript() const
        {
            return new spell_dru_faerie_fire_AuraScript();
        }
};

// -48496 - Living Seed
class spell_dru_living_seed : public SpellScriptLoader
{
    public:
        spell_dru_living_seed() : SpellScriptLoader("spell_dru_living_seed") { }

        class spell_dru_living_seed_AuraScript : public AuraScript
        {
            PrepareAuraScript(spell_dru_living_seed_AuraScript);

            bool Validate(SpellInfo const* /*spellInfo*/)
            {
                if (!sSpellMgr->GetSpellInfo(SPELL_DRUID_LIVING_SEED_PROC))
                    return false;
                return true;
            }

            void HandleProc(AuraEffect const* aurEff, ProcEventInfo& eventInfo)
            {
                PreventDefaultAction();
                int32 amount = CalculatePct(eventInfo.GetHealInfo()->GetHeal(), aurEff->GetAmount());
                GetTarget()->CastCustomSpell(SPELL_DRUID_LIVING_SEED_PROC, SPELLVALUE_BASE_POINT0, amount, eventInfo.GetProcTarget(), true, NULL, aurEff);
            }

            void Register()
            {
                OnEffectProc += AuraEffectProcFn(spell_dru_living_seed_AuraScript::HandleProc, EFFECT_0, SPELL_AURA_DUMMY);
            }
        };

        AuraScript* GetAuraScript() const
        {
            return new spell_dru_living_seed_AuraScript();
        }
};

// 48504 - Living Seed (Proc)
class spell_dru_living_seed_proc : public SpellScriptLoader
{
    public:
        spell_dru_living_seed_proc() : SpellScriptLoader("spell_dru_living_seed_proc") { }

        class spell_dru_living_seed_proc_AuraScript : public AuraScript
        {
            PrepareAuraScript(spell_dru_living_seed_proc_AuraScript);

            bool Validate(SpellInfo const* /*spellInfo*/)
            {
                if (!sSpellMgr->GetSpellInfo(SPELL_DRUID_LIVING_SEED_HEAL))
                    return false;
                return true;
            }

            void HandleProc(AuraEffect const* aurEff, ProcEventInfo& /*eventInfo*/)
            {
                PreventDefaultAction();
                GetTarget()->CastCustomSpell(SPELL_DRUID_LIVING_SEED_HEAL, SPELLVALUE_BASE_POINT0, aurEff->GetAmount(), GetTarget(), true, NULL, aurEff);
            }

            void Register()
            {
                OnEffectProc += AuraEffectProcFn(spell_dru_living_seed_proc_AuraScript::HandleProc, EFFECT_0, SPELL_AURA_DUMMY);
            }
        };

        AuraScript* GetAuraScript() const
        {
            return new spell_dru_living_seed_proc_AuraScript();
        }
};

// 48391 - Owlkin Frenzy
class spell_dru_owlkin_frenzy : public SpellScriptLoader
{
    public:
        spell_dru_owlkin_frenzy() : SpellScriptLoader("spell_dru_owlkin_frenzy") { }

        class spell_dru_owlkin_frenzy_AuraScript : public AuraScript
        {
            PrepareAuraScript(spell_dru_owlkin_frenzy_AuraScript);

            void CalculateAmount(AuraEffect const* /*aurEff*/, int32& amount, bool& /*canBeRecalculated*/)
            {
                amount = CalculatePct(GetUnitOwner()->GetCreatePowers(POWER_MANA), amount);
            }

            void Register()
            {
                DoEffectCalcAmount += AuraEffectCalcAmountFn(spell_dru_owlkin_frenzy_AuraScript::CalculateAmount, EFFECT_2, SPELL_AURA_PERIODIC_ENERGIZE);
            }
        };

        AuraScript* GetAuraScript() const
        {
            return new spell_dru_owlkin_frenzy_AuraScript();
        }
};

/*class spell_druid_frenzied_regeneration : public SpellScriptLoader
{
    class script_impl : public AuraScript
    {
        PrepareAuraScript(script_impl);

        enum
        {
            SPELL_FRENZIED_REGENERATION_HEAL        = 22845,
            SPELL_GLYPH_OF_FRENZIED_REGENERATION    = 54810,
        };

        void HandlePeriodic(AuraEffect const* aurEff)
        {
            Unit* const target = GetTarget();
            if (target->getPowerType() != POWER_RAGE)
                return;

            // Glyph of Frenzied Regeneration
            if (target->HasAura(SPELL_GLYPH_OF_FRENZIED_REGENERATION))
                return;

            uint32 const power = target->GetPower(POWER_RAGE);
            if (!power)
                return;

            int32 const rage = std::min<uint32>(power, 100);
            float const pct = rage * aurEff->GetAmount() / 1000;
            int32 const health = CalculatePct(target->GetMaxHealth(), pct);

            target->CastCustomSpell(target, SPELL_FRENZIED_REGENERATION_HEAL, &health, NULL, NULL, true, 0, aurEff);
            target->SetPower(POWER_RAGE, power - rage);
        }

        void Register()
        {
            OnEffectPeriodic += AuraEffectPeriodicFn(script_impl::HandlePeriodic, EFFECT_0, SPELL_AURA_PERIODIC_DUMMY);
        }
    };

public:
    spell_druid_frenzied_regeneration()
        : SpellScriptLoader("spell_druid_frenzied_regeneration")
    {
    }

    AuraScript* GetAuraScript() const
    {
        return new script_impl();
    }
};*/

// -16972 - Predatory Strikes
class spell_dru_predatory_strikes : public SpellScriptLoader
{
    public:
        spell_dru_predatory_strikes() : SpellScriptLoader("spell_dru_predatory_strikes") { }

        class spell_dru_predatory_strikes_AuraScript : public AuraScript
        {
            PrepareAuraScript(spell_dru_predatory_strikes_AuraScript);

            void UpdateAmount(AuraEffect const* /*aurEff*/, AuraEffectHandleModes /*mode*/)
            {
                if (Player* target = GetTarget()->ToPlayer())
                    target->UpdateAttackPowerAndDamage();
            }

            void Register()
            {
                AfterEffectApply += AuraEffectApplyFn(spell_dru_predatory_strikes_AuraScript::UpdateAmount, EFFECT_ALL, SPELL_AURA_DUMMY, AURA_EFFECT_HANDLE_CHANGE_AMOUNT_MASK);
                AfterEffectRemove += AuraEffectRemoveFn(spell_dru_predatory_strikes_AuraScript::UpdateAmount, EFFECT_ALL, SPELL_AURA_DUMMY, AURA_EFFECT_HANDLE_CHANGE_AMOUNT_MASK);
            }
        };

        AuraScript* GetAuraScript() const
        {
            return new spell_dru_predatory_strikes_AuraScript();
        }
};

// -1079 - Rip
class spell_dru_rip : public SpellScriptLoader
{
    public:
        spell_dru_rip() : SpellScriptLoader("spell_dru_rip") { }

        class spell_dru_rip_AuraScript : public AuraScript
        {
            PrepareAuraScript(spell_dru_rip_AuraScript);

            bool Load()
            {
                Unit* caster = GetCaster();
                return caster && caster->GetTypeId() == TYPEID_PLAYER;
            }

            void CalculateAmount(AuraEffect const* /*aurEff*/, int32& amount, bool& canBeRecalculated)
            {
                canBeRecalculated = false;

                if (Unit* caster = GetCaster())
                {
                    // 0.0207 * $AP * cp
                    uint8 cp = caster->ToPlayer()->GetComboPoints();
                    float coeff = cp * 0.0207f;
                    amount += int32(caster->GetTotalAttackPowerValue(BASE_ATTACK) * coeff);
                }
            }

            void Register()
            {
                DoEffectCalcAmount += AuraEffectCalcAmountFn(spell_dru_rip_AuraScript::CalculateAmount, EFFECT_0, SPELL_AURA_PERIODIC_DAMAGE);
            }
        };

        AuraScript* GetAuraScript() const
        {
            return new spell_dru_rip_AuraScript();
        }
};

// - Rake
class spell_dru_rake : public SpellScriptLoader
{
    public:
        spell_dru_rake() : SpellScriptLoader("spell_dru_rake") { }

        class spell_dru_rake_AuraScript : public AuraScript
        {
            PrepareAuraScript(spell_dru_rake_AuraScript);


            void CalculateAmount(AuraEffect const* aurEff, int32& amount, bool& canBeRecalculated)
            {
                canBeRecalculated = false;

                if (Unit* caster = GetCaster())
                {
                    // Ap modifier based on talent - 0.147 per talent point
                    float coeff = 0.441f;
                    if (AuraEffect* carnage = caster->GetAuraEffect(SPELL_AURA_ADD_FLAT_MODIFIER, SPELLFAMILY_DRUID, 2725, EFFECT_0))
                        coeff += 0.147f * (carnage->GetAmount() / 3000);
                    amount += int32(caster->GetTotalAttackPowerValue(BASE_ATTACK) * coeff) / aurEff->GetTotalTicks();
                }
            }

            void Register()
            {
                DoEffectCalcAmount += AuraEffectCalcAmountFn(spell_dru_rake_AuraScript::CalculateAmount, EFFECT_1, SPELL_AURA_PERIODIC_DAMAGE);
            }
        };

        AuraScript* GetAuraScript() const
        {
            return new spell_dru_rake_AuraScript();
        }
};

// 52610 - Savage Roar
class spell_dru_savage_roar : public SpellScriptLoader
{
    public:
        spell_dru_savage_roar() : SpellScriptLoader("spell_dru_savage_roar") { }

        class spell_dru_savage_roar_SpellScript : public SpellScript
        {
            PrepareSpellScript(spell_dru_savage_roar_SpellScript);

            SpellCastResult CheckCast()
            {
                Unit* caster = GetCaster();
                if (caster->GetShapeshiftForm() != FORM_CAT)
                    return SPELL_FAILED_ONLY_SHAPESHIFT;

                return SPELL_CAST_OK;
            }

            void Register()
            {
                OnCheckCast += SpellCheckCastFn(spell_dru_savage_roar_SpellScript::CheckCast);
            }
        };

        class spell_dru_savage_roar_AuraScript : public AuraScript
        {
            PrepareAuraScript(spell_dru_savage_roar_AuraScript);

            bool Validate(SpellInfo const* /*spellInfo*/)
            {
                if (!sSpellMgr->GetSpellInfo(SPELL_DRUID_SAVAGE_ROAR))
                    return false;
                return true;
            }

            void AfterApply(AuraEffect const* aurEff, AuraEffectHandleModes /*mode*/)
            {
                Unit* target = GetTarget();
                target->CastSpell(target, SPELL_DRUID_SAVAGE_ROAR, true, NULL, aurEff, GetCasterGUID());
            }

            void AfterRemove(AuraEffect const* /*aurEff*/, AuraEffectHandleModes /*mode*/)
            {
                GetTarget()->RemoveAurasDueToSpell(SPELL_DRUID_SAVAGE_ROAR);
            }

            void Register()
            {
                AfterEffectApply += AuraEffectApplyFn(spell_dru_savage_roar_AuraScript::AfterApply, EFFECT_1, SPELL_AURA_DUMMY, AURA_EFFECT_HANDLE_REAL);
                AfterEffectRemove += AuraEffectRemoveFn(spell_dru_savage_roar_AuraScript::AfterRemove, EFFECT_1, SPELL_AURA_DUMMY, AURA_EFFECT_HANDLE_REAL);
            }
        };

        SpellScript* GetSpellScript() const
        {
            return new spell_dru_savage_roar_SpellScript();
        }

        AuraScript* GetAuraScript() const
        {
            return new spell_dru_savage_roar_AuraScript();
        }
};

// -50286 - Starfall (Dummy)
class spell_dru_starfall_dummy : public SpellScriptLoader
{
    class CombatCheck
    {
        Unit* const m_caster;
    public:
        CombatCheck(Unit* caster) : m_caster(caster)
        {
        }

        bool operator() (WorldObject* unit)
        {
            if (unit->GetTypeId() != TYPEID_UNIT && unit->GetTypeId() != TYPEID_PLAYER)
                return true;

            return unit->ToUnit()->HasBreakableByDamageCrowdControlAura(m_caster);
        }
    };

    class script_impl : public SpellScript
    {
        PrepareSpellScript(script_impl);

        enum { MAX_SHARDS = 20 };

        bool Load()
        {
            return GetTriggeringSpell();
        }

        void FilterTargets(std::list<WorldObject*>& targets)
        {
            targets.remove_if(CombatCheck(GetCaster()));
        }

        void HandleDummy(SpellEffIndex)
        {
            Unit* const caster = GetCaster();
            uint32 const triggeringSpellId = GetTriggeringSpell()->Id;
            AuraEffect* const aurEff = caster->GetAuraEffect(triggeringSpellId, EFFECT_0);
            if (!aurEff)
                return;

            // Shapeshifting into an animal form or mounting cancels the effect
            // Or if 20 Shards were fired
            if (caster->GetCreatureType() == CREATURE_TYPE_BEAST || caster->IsMounted() || aurEff->userData() >= MAX_SHARDS)
            {
                caster->RemoveAurasDueToSpell(triggeringSpellId);
                return;
            }


            // Any effect which causes you to lose control of your character will supress the starfall effect.
            if (caster->HasUnitState(UNIT_STATE_CONTROLLED))
                return;

            caster->CastSpell(GetHitUnit(), uint32(GetEffectValue()), true);
            aurEff->setUserData(aurEff->userData() + 1);
        }

        void Register()
        {
            OnObjectAreaTargetSelect += SpellObjectAreaTargetSelectFn(script_impl::FilterTargets, EFFECT_0, TARGET_UNIT_SRC_AREA_ENEMY);
            OnEffectHitTarget += SpellEffectFn(script_impl::HandleDummy, EFFECT_0, SPELL_EFFECT_DUMMY);
        }
    };

public:
    spell_dru_starfall_dummy()
        : SpellScriptLoader("spell_dru_starfall_dummy")
    {
    }

    SpellScript* GetSpellScript() const
    {
        return new script_impl();
    }
};

// 61336 - Survival Instincts
class spell_dru_survival_instincts : public SpellScriptLoader
{
    public:
        spell_dru_survival_instincts() : SpellScriptLoader("spell_dru_survival_instincts") { }

        class spell_dru_survival_instincts_SpellScript : public SpellScript
        {
            PrepareSpellScript(spell_dru_survival_instincts_SpellScript);

            SpellCastResult CheckCast()
            {
                Unit* caster = GetCaster();
                if (!caster->IsInFeralForm())
                    return SPELL_FAILED_ONLY_SHAPESHIFT;

                return SPELL_CAST_OK;
            }

            void Register()
            {
                OnCheckCast += SpellCheckCastFn(spell_dru_survival_instincts_SpellScript::CheckCast);
            }
        };

        class spell_dru_survival_instincts_AuraScript : public AuraScript
        {
            PrepareAuraScript(spell_dru_survival_instincts_AuraScript);

            bool Validate(SpellInfo const* /*spell*/)
            {
                if (!sSpellMgr->GetSpellInfo(SPELL_DRUID_SURVIVAL_INSTINCTS))
                    return false;
                return true;
            }

            void AfterApply(AuraEffect const* aurEff, AuraEffectHandleModes /*mode*/)
            {
                Unit* target = GetTarget();
                target->CastSpell(target, SPELL_DRUID_SURVIVAL_INSTINCTS, true);
            }

            void Register()
            {
                AfterEffectApply += AuraEffectApplyFn(spell_dru_survival_instincts_AuraScript::AfterApply, EFFECT_0, SPELL_AURA_DUMMY, AURA_EFFECT_HANDLE_CHANGE_AMOUNT_MASK);
            }
        };

        SpellScript* GetSpellScript() const
        {
            return new spell_dru_survival_instincts_SpellScript();
        }

        AuraScript* GetAuraScript() const
        {
            return new spell_dru_survival_instincts_AuraScript();
        }
};

// 40121 - Swift Flight Form (Passive)
class spell_dru_swift_flight_passive : public SpellScriptLoader
{
    public:
        spell_dru_swift_flight_passive() : SpellScriptLoader("spell_dru_swift_flight_passive") { }

        class spell_dru_swift_flight_passive_AuraScript : public AuraScript
        {
            PrepareAuraScript(spell_dru_swift_flight_passive_AuraScript);

            bool Load()
            {
                return GetCaster()->GetTypeId() == TYPEID_PLAYER;
            }

            void CalculateAmount(AuraEffect const* /*aurEff*/, int32 & amount, bool & /*canBeRecalculated*/)
            {
                if (Player* caster = GetCaster()->ToPlayer())
                    if (caster->GetSkillValue(SKILL_RIDING) >= 375)
                        amount = 310;
            }

            void Register()
            {
                DoEffectCalcAmount += AuraEffectCalcAmountFn(spell_dru_swift_flight_passive_AuraScript::CalculateAmount, EFFECT_1, SPELL_AURA_MOD_INCREASE_VEHICLE_FLIGHT_SPEED);
            }
        };

        AuraScript* GetAuraScript() const
        {
            return new spell_dru_swift_flight_passive_AuraScript();
        }
};

// -5217 - Tiger's Fury
class spell_dru_tiger_s_fury : public SpellScriptLoader
{
public:
	spell_dru_tiger_s_fury() : SpellScriptLoader("spell_dru_tiger_s_fury") { }

	class spell_dru_tiger_s_fury_AuraScript : public AuraScript
	{
		PrepareAuraScript(spell_dru_tiger_s_fury_AuraScript);

		bool Validate(SpellInfo const* /*spell*/)
		{
			if (!sSpellMgr->GetSpellInfo(5217))
				return false;
			return true;
		}

		void AfterApply(AuraEffect const* /*aurEff*/, AuraEffectHandleModes /*mode*/)
		{
			if (Unit* caster = GetCaster())
			{
				if (AuraEffect* aurEff = caster->GetDummyAuraEffect(SPELLFAMILY_DRUID, 2850, 1))
				{
					int32 bp0 = aurEff->GetAmount();
					caster->CastCustomSpell(caster, 51178, &bp0, 0, 0, true); // King of the Jungle
				}
				if (caster->HasAura(80316)) // Primal Madness (rank 1)
					caster->CastSpell(caster, 80879, true);

				if (caster->HasAura(80317)) // Primal Madness (rank 1)
					caster->CastSpell(caster, 80886, true);
			}
		}

		void AfterRemove(AuraEffect const* /*aurEff*/, AuraEffectHandleModes /*mode*/)
		{
			if (Unit* caster = GetCaster())
			{
				if (caster->HasAura(80316))
					caster->RemoveAura(80879);

				if (caster->HasAura(80317))
					caster->RemoveAura(80886);
			}
		}

		void Register()
		{
			AfterEffectApply += AuraEffectApplyFn(spell_dru_tiger_s_fury_AuraScript::AfterApply, EFFECT_0, SPELL_AURA_MOD_DAMAGE_PERCENT_DONE, AURA_EFFECT_HANDLE_REAL);
			AfterEffectRemove += AuraEffectRemoveFn(spell_dru_tiger_s_fury_AuraScript::AfterRemove, EFFECT_0, SPELL_AURA_MOD_DAMAGE_PERCENT_DONE, AURA_EFFECT_HANDLE_REAL);
		}
	};

	AuraScript* GetAuraScript() const
	{
		return new spell_dru_tiger_s_fury_AuraScript();
	}
};

// -61391 - Typhoon
class spell_dru_typhoon : public SpellScriptLoader
{
    public:
        spell_dru_typhoon() : SpellScriptLoader("spell_dru_typhoon") { }

        class spell_dru_typhoon_SpellScript : public SpellScript
        {
            PrepareSpellScript(spell_dru_typhoon_SpellScript);

            void HandleKnockBack(SpellEffIndex effIndex)
            {
                // Glyph of Typhoon
                if (GetCaster()->HasAura(SPELL_DRUID_GLYPH_OF_TYPHOON))
                    PreventHitDefaultEffect(effIndex);
            }

            void Register()
            {
                OnEffectHitTarget += SpellEffectFn(spell_dru_typhoon_SpellScript::HandleKnockBack, EFFECT_0, SPELL_EFFECT_KNOCK_BACK);
            }
        };

        SpellScript* GetSpellScript() const
        {
            return new spell_dru_typhoon_SpellScript();
        }
};

// 70691 - Item T10 Restoration 4P Bonus
class spell_dru_t10_restoration_4p_bonus : public SpellScriptLoader
{
    public:
        spell_dru_t10_restoration_4p_bonus() : SpellScriptLoader("spell_dru_t10_restoration_4p_bonus") { }

        class spell_dru_t10_restoration_4p_bonus_SpellScript : public SpellScript
        {
            PrepareSpellScript(spell_dru_t10_restoration_4p_bonus_SpellScript);

            bool Load()
            {
                return GetCaster()->GetTypeId() == TYPEID_PLAYER;
            }

            void FilterTargets(std::list<WorldObject*>& targets)
            {
                if (!GetCaster()->ToPlayer()->GetGroup())
                {
                    targets.clear();
                    targets.push_back(GetCaster());
                }
                else
                {
                    targets.remove(GetExplTargetUnit());
                    std::list<Unit*> tempTargets;
                    for (std::list<WorldObject*>::const_iterator itr = targets.begin(); itr != targets.end(); ++itr)
                        if ((*itr)->GetTypeId() == TYPEID_PLAYER && GetCaster()->IsInRaidWith((*itr)->ToUnit()))
                            tempTargets.push_back((*itr)->ToUnit());

                    if (tempTargets.empty())
                    {
                        targets.clear();
                        FinishCast(SPELL_FAILED_DONT_REPORT);
                        return;
                    }

                    Unit* target = Trinity::Containers::SelectRandomContainerElement(tempTargets);
                    targets.clear();
                    targets.push_back(target);
                }
            }

            void Register()
            {
                OnObjectAreaTargetSelect += SpellObjectAreaTargetSelectFn(spell_dru_t10_restoration_4p_bonus_SpellScript::FilterTargets, EFFECT_0, TARGET_UNIT_DEST_AREA_ALLY);
            }
        };

        SpellScript* GetSpellScript() const
        {
            return new spell_dru_t10_restoration_4p_bonus_SpellScript();
        }
};

// Thorns
class spell_dru_thorns : public SpellScriptLoader
{
    public:
        spell_dru_thorns() : SpellScriptLoader("spell_dru_thorns") { }

        class spell_dru_thorns_AuraScript : public AuraScript
        {
            PrepareAuraScript(spell_dru_thorns_AuraScript);

            void CalculateAmount(AuraEffect const* /*aurEff*/, int32& amount, bool& /*canBeRecalculated*/)
            {
                if (Unit* caster = GetCaster())
                {
                    float coeff = 0.168f;
                    if (caster->HasSpell(33876))
                        amount = caster->GetTotalAttackPowerValue(BASE_ATTACK) * coeff;
                    else
                        amount = caster->SpellBaseDamageBonusDone(GetSpellInfo()->GetSchoolMask()) * coeff;
                }
            }

            void Register()
            {
                DoEffectCalcAmount += AuraEffectCalcAmountFn(spell_dru_thorns_AuraScript::CalculateAmount, EFFECT_0, SPELL_AURA_DAMAGE_SHIELD);
            }
        };

        AuraScript* GetAuraScript() const
        {
            return new spell_dru_thorns_AuraScript();
        }
};

class spell_druid_t12_4p_restoration_item_bonus : public SpellScriptLoader
{
    class script_impl : public AuraScript
    {
        PrepareAuraScript(script_impl);

        enum { SPELL_RUNIC_ENERGIZE = 61258 };

        bool CheckProc(ProcEventInfo& procInfo)
        {
            SpellInfo const* const spellInfo = procInfo.GetSpellInfo();
            if (!spellInfo)
                return false;

            HealInfo* const healInfo = procInfo.GetHealInfo();
            if (!healInfo || !healInfo->GetHeal())
                return false;

            return SelectFriendlyTarget();
        }


        void HandleProc(AuraEffect const*, ProcEventInfo& procInfo)
        {
            Unit* const target = SelectFriendlyTarget();

            int32 const value = int32(procInfo.GetHealInfo()->GetHeal());
            GetTarget()->CastCustomSpell(target, procInfo.GetSpellInfo()->Id, &value, NULL, NULL, true);
        }


        Unit* SelectFriendlyTarget() const
        {
            Unit* unit = NULL;
            Unit* const target = GetTarget();

            Trinity::MostHPMissingInRange check(target, 15.0f, 1);
            Trinity::UnitLastSearcher<Trinity::MostHPMissingInRange> searcher(target, unit, check);
            target->VisitNearbyObject(15.0f, searcher);

            return unit;
        }


        void Register()
        {
            DoCheckProc += AuraCheckProcFn(script_impl::CheckProc);
            OnEffectProc += AuraEffectProcFn(script_impl::HandleProc, EFFECT_0, SPELL_AURA_DUMMY);
        }
    };

public:
    spell_druid_t12_4p_restoration_item_bonus()
        : SpellScriptLoader("spell_druid_t12_4p_restoration_item_bonus")
    {
    }

    AuraScript* GetAuraScript() const
    {
        return new script_impl();
    }
};

class spell_druid_ferocious_bite : public SpellScriptLoader
{
    class script_impl : public SpellScript
    {
        PrepareSpellScript(script_impl);

        enum { SPELL_BLOOD_IN_THE_WATER = 80318 };

        void HandleScript(SpellEffIndex)
        {
            Unit* const caster = GetCaster();
            Unit* const target = GetHitUnit();
            if (caster->GetTypeId() != TYPEID_PLAYER || !target)
                return;

            // Blood in the Water
            if (AuraEffect const* const aurEff = caster->GetAuraEffectOfRankedSpell(SPELL_BLOOD_IN_THE_WATER, EFFECT_0))
            {
                uint8 const rank = sSpellMgr->GetSpellRank(aurEff->GetId());
                uint32 const spellId = aurEff->GetSpellInfo()->Effects[aurEff->GetEffIndex()].TriggerSpell;

                if (target->HealthBelowPct(aurEff->GetAmount() + 1) && roll_chance_i(50 * rank))
                    caster->CastSpell(caster, spellId, true);
            }
        }


        void Register()
        {
            OnEffectHitTarget += SpellEffectFn(script_impl::HandleScript, EFFECT_0, SPELL_EFFECT_SCHOOL_DAMAGE);
        }
    };

public:
    spell_druid_ferocious_bite()
        : SpellScriptLoader("spell_druid_ferocious_bite")
    {
    }

    SpellScript* GetSpellScript() const
    {
        return new script_impl();
    }
};

class spell_druid_blood_in_the_water : public SpellScriptLoader
{
    class script_impl : public SpellScript
    {
        PrepareSpellScript(script_impl);

        enum { SPELL_RIP = 1079 };

        void HandleScript(SpellEffIndex effIndex)
        {
            Unit* const target = GetHitUnit();
            if (!target)
                return;

            // Rip
            if (Aura* const aura = target->GetAura(SPELL_RIP, GetCaster()->GetGUID()))
                aura->RefreshDuration();
        }

        void Register()
        {
            OnEffectHitTarget += SpellEffectFn(script_impl::HandleScript, EFFECT_0, SPELL_EFFECT_SCRIPT_EFFECT);
        }
    };

public:
    spell_druid_blood_in_the_water()
        : SpellScriptLoader("spell_druid_blood_in_the_water")
    {
    }

    SpellScript* GetSpellScript() const
    {
        return new script_impl();
    }
};

class spell_druid_shred_maul : public SpellScriptLoader
{
    class script_impl : public SpellScript
    {
        PrepareSpellScript(script_impl);

        enum { SPELL_REND_AND_TEAR = 48432 };

        void HandleOnHit()
        {
            if (AuraEffect const* const aurEff = GetCaster()->GetAuraEffectOfRankedSpell(SPELL_REND_AND_TEAR, EFFECT_0))
            {
                Unit* const target = GetHitUnit();
                if (!target || !target->HasAuraState(AURA_STATE_BLEEDING))
                    return;

                int32 value = GetHitDamage();
                AddPct(value, aurEff->GetAmount());
                SetHitDamage(value);
            }
        }


        void Register()
        {
            OnHit += SpellHitFn(script_impl::HandleOnHit);
        }
    };

public:
    spell_druid_shred_maul()
        : SpellScriptLoader("spell_druid_shred_maul")
    {
    }

    SpellScript* GetSpellScript() const
    {
        return new script_impl();
    }
};

class spell_druid_regrowth : public SpellScriptLoader
{
    class script_impl : public AuraScript
    {
        PrepareAuraScript(script_impl);

        enum { SPELL_GLYPH_OF_REGROWTH = 54743 };

        bool Load()
        {
            Unit const* const caster = GetCaster();
            return caster && caster->GetTypeId() == TYPEID_PLAYER;
        }


        void OnPeriodic(AuraEffect const* aurEff)
        {
            Unit const* const caster = GetCaster();
            if (caster)
                if(AuraEffect const* const regrowth = caster->GetAuraEffect(SPELL_GLYPH_OF_REGROWTH, EFFECT_0))
                    if (!GetTarget()->HealthAbovePct(regrowth->GetAmount()))
                        aurEff->GetBase()->RefreshDuration();
        }

        void Register()
        {
            OnEffectPeriodic += AuraEffectPeriodicFn(script_impl::OnPeriodic, EFFECT_1, SPELL_AURA_PERIODIC_HEAL);
        }
    };

public:
    spell_druid_regrowth()
        : SpellScriptLoader("spell_druid_regrowth")
    {
    }

    AuraScript* GetAuraScript() const
    {
        return new script_impl();
    }
};

// pulverize
class spell_dru_pulverize : public SpellScriptLoader
{
    public:
        spell_dru_pulverize() : SpellScriptLoader("spell_dru_pulverize") { }

        class spell_dru_pulverize_SpellScript : public SpellScript
        {
            PrepareSpellScript(spell_dru_pulverize_SpellScript);

            void OnHit()
            {
                if (Unit* caster=GetCaster())
                {
                    if (Unit* target=GetHitUnit())
                    {
                        if (AuraEffect const* lacerate = target->GetAuraEffect(SPELL_AURA_PERIODIC_DAMAGE,SPELLFAMILY_DRUID,0x00000000,0x00000100,0x00000000,caster->GetGUID()))
                        {
                            int32 bp0 = 3*lacerate->GetBase()->GetStackAmount();
                            caster->CastCustomSpell(caster,80951,&bp0,0,0,true);
                            target->RemoveAura(lacerate->GetBase());
                        }
                    }
                }
            }

            void Register()
            {
                
                AfterHit += SpellHitFn(spell_dru_pulverize_SpellScript::OnHit);
            }
        };

        SpellScript* GetSpellScript() const
        {
            return new spell_dru_pulverize_SpellScript();
        }
};

// lacerate
class spell_dru_lacerate : public SpellScriptLoader
{
    public:
        spell_dru_lacerate() : SpellScriptLoader("spell_dru_lacerate") { }

        class spell_dru_lacerate_AuraScript : public AuraScript
        {
            PrepareAuraScript(spell_dru_lacerate_AuraScript);

            bool Validate(SpellInfo const* /*spell*/)
            {
                if (!sSpellMgr->GetSpellInfo(50334))
                    return false;
                return true;
            }

            void OnPeriodic(AuraEffect const* aurEff)
            {
                if(Unit* caster = GetCaster())
                {
                    if (Player* player = caster->ToPlayer())
                    {   
                        // Berserk
                        if(player->HasTalent(50334, player->GetActiveSpec()))
                        {
                            if(roll_chance_i(50))
                            {
                                player->RemoveSpellCooldown(33878, true);
                                player->CastSpell(player, 93622, true);
                            }
                        }
                    }
                }
            }

            void Register()
            {
                OnEffectPeriodic += AuraEffectPeriodicFn(spell_dru_lacerate_AuraScript::OnPeriodic, EFFECT_0, SPELL_AURA_PERIODIC_DAMAGE);
            }
        };

        AuraScript* GetAuraScript() const
        {
            return new spell_dru_lacerate_AuraScript();
        }
};

class RaidCheck
{
    public:
        explicit RaidCheck(Unit const* caster) : _caster(caster) { }

        bool operator()(WorldObject* obj) const
        {
            if (Unit* target = obj->ToUnit())
                return !_caster->IsInRaidWith(target);

            return true;
        }

    private:
        Unit const* _caster;
};

// 48438 - Wild Growth
class spell_dru_wild_growth : public SpellScriptLoader
{
    public:
        spell_dru_wild_growth() : SpellScriptLoader("spell_dru_wild_growth") { }

        class spell_dru_wild_growth_SpellScript : public SpellScript
        {
            PrepareSpellScript(spell_dru_wild_growth_SpellScript);

            bool Validate(SpellInfo const* spellInfo) 
            {
                if (spellInfo->Effects[EFFECT_2].IsEffect() || spellInfo->Effects[EFFECT_2].CalcValue() <= 0)
                    return false;
                return true;
            }

            void FilterTargets(std::list<WorldObject*>& targets)
            {
                targets.remove_if(RaidCheck(GetCaster()));

                uint32 const maxTargets = uint32(GetSpellInfo()->Effects[EFFECT_2].CalcValue(GetCaster()));

                if (targets.size() > maxTargets)
                {
                    targets.sort(Trinity::HealthPctOrderPred());
                    targets.resize(maxTargets);
                }

                _targets = targets;
            }

            void SetTargets(std::list<WorldObject*>& targets)
            {
                targets = _targets;
            }

            void Register() 
            {
                OnObjectAreaTargetSelect += SpellObjectAreaTargetSelectFn(spell_dru_wild_growth_SpellScript::FilterTargets, EFFECT_0, TARGET_UNIT_DEST_AREA_ALLY);
                OnObjectAreaTargetSelect += SpellObjectAreaTargetSelectFn(spell_dru_wild_growth_SpellScript::SetTargets, EFFECT_1, TARGET_UNIT_DEST_AREA_ALLY);
            }

        private:
            std::list<WorldObject*> _targets;
        };

        SpellScript* GetSpellScript() const 
        {
            return new spell_dru_wild_growth_SpellScript();
        }
};

void AddSC_druid_spell_scripts()
{
    new spell_dru_dash();
    new spell_dru_eclipse_energize();
    new spell_dru_enrage();
    new spell_dru_glyph_of_starfire();
    new spell_dru_idol_lifebloom();
    new spell_dru_insect_swarm();
    new spell_dru_lifebloom();
    new spell_dru_living_seed();
    new spell_dru_living_seed_proc();
    new spell_dru_owlkin_frenzy();
    new spell_dru_predatory_strikes();
    new spell_dru_rip();
    new spell_dru_savage_roar();
    new spell_dru_starfall_dummy();
    new spell_dru_survival_instincts();
    new spell_dru_swift_flight_passive();
    new spell_dru_tiger_s_fury();
    new spell_dru_typhoon();
    new spell_dru_t10_restoration_4p_bonus();
    new spell_dru_rejuv();
    new spell_dru_emp_touch();
    new spell_dru_emp_touch_triggered();
    new spell_sha_efflorescence();
    new spell_sha_efflorescence_heal();
    new spell_dru_harmony();
    new spell_dru_feral_swiftness();
    new spell_dru_feral_aggression();
    new spell_dru_stampede();
    new spell_dru_skull_bash();
    new spell_dru_primal_madness();
    new spell_dru_pulverize();
    new spell_dru_innervate();
    new spell_dru_beam();
    new spell_dru_wild_mushroom();
    new spell_dru_wild_mushroom_detonation();
    new spell_dru_rake();
    //new spell_druid_frenzied_regeneration();
    new spell_dru_barkskin();
    new spell_dru_energize();
    new spell_dru_starsurge();
    new spell_dru_faerie_fire();
    new spell_dru_thorns();
    new spell_druid_ferocious_bite();
    new spell_druid_blood_in_the_water();
    new spell_druid_shred_maul();
    new spell_druid_regrowth();
	new spell_dru_lacerate();
    new spell_dru_wild_growth();
}
