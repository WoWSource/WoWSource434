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
 * Scripts for spells with SPELLFAMILY_PALADIN and SPELLFAMILY_GENERIC spells used by paladin players.
 * Ordered alphabetically using scriptname.
 * Scriptnames of files in this file should be prefixed with "spell_pal_".
 */

#include "Player.h"
#include "ScriptMgr.h"
#include "SpellScript.h"
#include "SpellAuraEffects.h"
#include "Group.h"

enum PaladinSpells
{
    SPELL_PALADIN_DIVINE_PLEA                    = 54428,
    SPELL_PALADIN_BLESSING_OF_SANCTUARY_BUFF     = 67480,
    SPELL_PALADIN_BLESSING_OF_SANCTUARY_ENERGIZE = 57319,

    SPELL_PALADIN_HOLY_SHOCK_R1                  = 20473,
    SPELL_PALADIN_HOLY_SHOCK_R1_DAMAGE           = 25912,
    SPELL_PALADIN_HOLY_SHOCK_R1_HEALING          = 25914,

    SPELL_PALADIN_BLESSING_OF_LOWER_CITY_DRUID   = 37878,
    SPELL_PALADIN_BLESSING_OF_LOWER_CITY_PALADIN = 37879,
    SPELL_PALADIN_BLESSING_OF_LOWER_CITY_PRIEST  = 37880,
    SPELL_PALADIN_BLESSING_OF_LOWER_CITY_SHAMAN  = 37881,

    SPELL_PALADIN_DIVINE_STORM                   = 53385,
    SPELL_PALADIN_DIVINE_STORM_DUMMY             = 54171,
    SPELL_PALADIN_DIVINE_STORM_HEAL              = 54172,

    SPELL_PALADIN_EYE_FOR_AN_EYE_DAMAGE          = 25997,

    SPELL_PALADIN_FORBEARANCE                    = 25771,
    SPELL_PALADIN_IMMUNE_SHIELD_MARKER           = 61988,

    SPELL_PALADIN_HAND_OF_SACRIFICE              = 6940,
    SPELL_PALADIN_DIVINE_SACRIFICE               = 64205,

    SPELL_PALADIN_DIVINE_PURPOSE_PROC            = 90174,
    
    SPELL_PALADIN_GLYPH_OF_SALVATION             = 63225,

    SPELL_PALADIN_RIGHTEOUS_DEFENSE_TAUNT        = 31790,

    SPELL_PALADIN_SEAL_OF_RIGHTEOUSNESS          = 25742,

    SPELL_GENERIC_ARENA_DAMPENING                = 74410,
    SPELL_GENERIC_BATTLEGROUND_DAMPENING         = 74411
};

enum PaladinGuardianOfAncientKingsSpells

{

	SPELL_PALADIN_GOAK_HOLY_SUMMON = 86669,
	SPELL_PALADIN_GOAK_ANCIENT_HEALER = 86674,
	SPELL_PALADIN_GOAK_PROTECTION_SUMMON = 86659,
	SPELL_PALADIN_GOAK_RETRIBUTION_SUMMON = 86698,
	SPELL_PALADIN_GOAK_ANCIENT_POWER = 86700,
	SPELL_PALADIN_GOAK_ANCIENT_CRUSADER = 86701,
	SPELL_PALADIN_GOAK_ANCIENT_CRUSADER_GUARDIAN = 86703,
	SPELL_PALADIN_GOAK_ANCIENT_FURY = 86704,

};

// 86704 - Ancient Fury
class spell_pal_ancient_fury : public SpellScriptLoader
{
public:
	spell_pal_ancient_fury() : SpellScriptLoader("spell_pal_ancient_fury") { }

	class spell_pal_ancient_fury_SpellScript : public SpellScript
	{
		PrepareSpellScript(spell_pal_ancient_fury_SpellScript);

		void CountTargets(std::list<WorldObject*>& targetList)
		{
			_targetCount = targetList.size();
		}

		void ChangeDamage(SpellEffIndex /*effIndex*/)
		{
			Unit* caster = GetCaster();
			Unit* target = GetHitUnit();

			if (caster && target && _targetCount)
			{
				int32 damage = GetHitDamage();

				if (Aura* aura = caster->GetAura(SPELL_PALADIN_GOAK_ANCIENT_POWER))
				{
					damage = caster->SpellDamageBonusDone(target, GetSpellInfo(), damage, SPELL_DIRECT_DAMAGE);
					damage = (damage * aura->GetStackAmount());

					// "divided evenly among all targets"
					damage /= _targetCount;
				}

				SetHitDamage(damage);
			}
		}
	private:
		uint32 _targetCount;

		void Register()
		{
			OnObjectAreaTargetSelect += SpellObjectAreaTargetSelectFn(spell_pal_ancient_fury_SpellScript::CountTargets, EFFECT_0, TARGET_UNIT_SRC_AREA_ENEMY);
			OnEffectHitTarget += SpellEffectFn(spell_pal_ancient_fury_SpellScript::ChangeDamage, EFFECT_0, SPELL_EFFECT_SCHOOL_DAMAGE);
		}
	};

	SpellScript* GetSpellScript() const
	{
		return new spell_pal_ancient_fury_SpellScript();
	}
};

// 86698  - Guardian of Ancient Kings Retribution
class spell_pal_guardian_of_ancient_kings_retri : public SpellScriptLoader
{
public:
	spell_pal_guardian_of_ancient_kings_retri() : SpellScriptLoader("spell_pal_guardian_of_ancient_kings_retri") { }

	class spell_pal_guardian_of_ancient_kings_retri_AuraScript : public AuraScript
	{
		PrepareAuraScript(spell_pal_guardian_of_ancient_kings_retri_AuraScript);

		void HandleRemove(AuraEffect const* /*aurEff*/, AuraEffectHandleModes /*mode*/)
		{
			Unit* caster = GetCaster();
			Unit* target = GetTarget();

			if (caster && target)
			{
				if (GetStackAmount())
				{
					caster->CastSpell(target, SPELL_PALADIN_GOAK_ANCIENT_FURY, true);
				}
			}
		}

		void Register()
		{
			OnEffectRemove += AuraEffectRemoveFn(spell_pal_guardian_of_ancient_kings_retri_AuraScript::HandleRemove, EFFECT_2, SPELL_AURA_DUMMY, AURA_EFFECT_HANDLE_REAL);
		}
	};

	AuraScript* GetAuraScript() const
	{
		return new spell_pal_guardian_of_ancient_kings_retri_AuraScript();
	}
};

// 86150 - Guardian of Ancient Kings action bar spell
class spell_pal_guardian_of_ancient_kings : public SpellScriptLoader
{
public:
	spell_pal_guardian_of_ancient_kings() : SpellScriptLoader("spell_pal_guardian_of_ancient_kings") { }

	class spell_pal_guardian_of_ancient_kings_SpellScript : public SpellScript
	{
		PrepareSpellScript(spell_pal_guardian_of_ancient_kings_SpellScript);

		void HandleDummy(SpellEffIndex /*effIndex*/)
		{
			if (Unit* caster = GetCaster())
			{
				if (Player* player = caster->ToPlayer())
				{
					switch (player->GetPrimaryTalentTree(player->GetActiveSpec()))
					{
						// Holy Guardian
					case TALENT_TREE_PALADIN_HOLY:
						caster->CastSpell(caster, SPELL_PALADIN_GOAK_HOLY_SUMMON, true);

						// 5 stack buff
						caster->CastSpell(caster, SPELL_PALADIN_GOAK_ANCIENT_HEALER, true);
						break;
						// Protection Guardian
					case TALENT_TREE_PALADIN_PROTECTION:
						caster->CastSpell(caster, SPELL_PALADIN_GOAK_PROTECTION_SUMMON, true);
						break;
						// Retribution Guardian
					case TALENT_TREE_PALADIN_RETRIBUTION:
						caster->CastSpell(caster, SPELL_PALADIN_GOAK_RETRIBUTION_SUMMON, true);

						// Ancient Power proc buff
						caster->CastSpell(caster, SPELL_PALADIN_GOAK_ANCIENT_CRUSADER, true);
						break;
					}
				}
			}
		}

		void Register()
		{
			OnEffectHitTarget += SpellEffectFn(spell_pal_guardian_of_ancient_kings_SpellScript::HandleDummy, EFFECT_0, SPELL_EFFECT_DUMMY);
		}
	};

	SpellScript* GetSpellScript() const
	{
		return new spell_pal_guardian_of_ancient_kings_SpellScript();
	}
};


// 31850 - Ardent Defender
class spell_pal_ardent_defender : public SpellScriptLoader
{
    public:
        spell_pal_ardent_defender() : SpellScriptLoader("spell_pal_ardent_defender") { }

        class spell_pal_ardent_defender_AuraScript : public AuraScript
        {
            PrepareAuraScript(spell_pal_ardent_defender_AuraScript);

            uint32 absorbPct, healPct;

            enum Spell
            {
                PAL_SPELL_ARDENT_DEFENDER_HEAL = 66235,
            };

            bool Load()
            {
                healPct = GetSpellInfo()->Effects[EFFECT_1].CalcValue();
                absorbPct = GetSpellInfo()->Effects[EFFECT_0].CalcValue();
                return GetUnitOwner()->GetTypeId() == TYPEID_PLAYER;
            }

            void CalculateAmount(AuraEffect const* aurEff, int32 & amount, bool & canBeRecalculated)
            {
                // Set absorbtion amount to unlimited
                amount = -1;
            }

            void Absorb(AuraEffect* aurEff, DamageInfo & dmgInfo, uint32 & absorbAmount)
            {
                Unit* victim = GetTarget();
                int32 remainingHealth = victim->GetHealth() - dmgInfo.GetDamage();
                uint32 allowedHealth = victim->CountPctFromMaxHealth(35);
                // If damage kills us
                if (remainingHealth <= 0 && !victim->ToPlayer()->HasSpellCooldown(PAL_SPELL_ARDENT_DEFENDER_HEAL))
                {
                    // Cast healing spell, completely avoid damage
                    absorbAmount = dmgInfo.GetDamage();

                    int32 healAmount = int32(victim->CountPctFromMaxHealth(15));
                    victim->CastCustomSpell(victim, PAL_SPELL_ARDENT_DEFENDER_HEAL, &healAmount, NULL, NULL, true, NULL, aurEff);
                    victim->ToPlayer()->AddSpellCooldown(PAL_SPELL_ARDENT_DEFENDER_HEAL, 0, time(NULL) + 120);
                }
                else if (remainingHealth < int32(allowedHealth))
                {
                    // Reduce damage that brings us under 35% (or full damage if we are already under 35%) by x%
                    uint32 damageToReduce = (victim->GetHealth() < allowedHealth)
                        ? dmgInfo.GetDamage()
                        : allowedHealth - remainingHealth;
                    absorbAmount = CalculatePct(damageToReduce, absorbPct);
                }
            }

            void Register()
            {
                 DoEffectCalcAmount += AuraEffectCalcAmountFn(spell_pal_ardent_defender_AuraScript::CalculateAmount, EFFECT_0, SPELL_AURA_SCHOOL_ABSORB);
                 OnEffectAbsorb += AuraEffectAbsorbFn(spell_pal_ardent_defender_AuraScript::Absorb, EFFECT_0);
            }
        };

        AuraScript* GetAuraScript() const
        {
            return new spell_pal_ardent_defender_AuraScript();
        }
};

// 37877 - Blessing of Faith
class spell_pal_blessing_of_faith : public SpellScriptLoader
{
    public:
        spell_pal_blessing_of_faith() : SpellScriptLoader("spell_pal_blessing_of_faith") { }

        class spell_pal_blessing_of_faith_SpellScript : public SpellScript
        {
            PrepareSpellScript(spell_pal_blessing_of_faith_SpellScript);

            bool Validate(SpellInfo const* /*spellInfo*/)
            {
                if (!sSpellMgr->GetSpellInfo(SPELL_PALADIN_BLESSING_OF_LOWER_CITY_DRUID) || !sSpellMgr->GetSpellInfo(SPELL_PALADIN_BLESSING_OF_LOWER_CITY_PALADIN) || !sSpellMgr->GetSpellInfo(SPELL_PALADIN_BLESSING_OF_LOWER_CITY_PRIEST) || !sSpellMgr->GetSpellInfo(SPELL_PALADIN_BLESSING_OF_LOWER_CITY_SHAMAN))
                    return false;
                return true;
            }

            void HandleDummy(SpellEffIndex /*effIndex*/)
            {
                if (Unit* unitTarget = GetHitUnit())
                {
                    uint32 spell_id = 0;
                    switch (unitTarget->getClass())
                    {
                        case CLASS_DRUID:
                            spell_id = SPELL_PALADIN_BLESSING_OF_LOWER_CITY_DRUID;
                            break;
                        case CLASS_PALADIN:
                            spell_id = SPELL_PALADIN_BLESSING_OF_LOWER_CITY_PALADIN;
                            break;
                        case CLASS_PRIEST:
                            spell_id = SPELL_PALADIN_BLESSING_OF_LOWER_CITY_PRIEST;
                            break;
                        case CLASS_SHAMAN:
                            spell_id = SPELL_PALADIN_BLESSING_OF_LOWER_CITY_SHAMAN;
                            break;
                        default:
                            return; // ignore for non-healing classes
                    }
                    Unit* caster = GetCaster();
                    caster->CastSpell(caster, spell_id, true);
                }
            }

            void Register()
            {
                OnEffectHitTarget += SpellEffectFn(spell_pal_blessing_of_faith_SpellScript::HandleDummy, EFFECT_0, SPELL_EFFECT_DUMMY);
            }
        };

        SpellScript* GetSpellScript() const
        {
            return new spell_pal_blessing_of_faith_SpellScript();
        }
};

// Judgement - 54158
class spell_pal_judgement : public SpellScriptLoader
{
    public:
        spell_pal_judgement() : SpellScriptLoader("spell_pal_judgement") { }

        class spell_pal_judgement_SpellScript : public SpellScript
        {
            PrepareSpellScript(spell_pal_judgement_SpellScript);

            void HandleScriptEffect(SpellEffIndex /*effIndex*/)
            {
                Player* caster = GetCaster()->ToPlayer();
                int32 spellPower = caster->SpellBaseDamageBonusDone(GetSpellInfo()->GetSchoolMask());
                int32 attackPower = caster->GetTotalAttackPowerValue(BASE_ATTACK);
                int32 basepoints0 = 0;
                Unit::AuraApplicationMap & sealAuras = caster->GetAppliedAuras();
                for (Unit::AuraApplicationMap::iterator iter = sealAuras.begin(); iter != sealAuras.end(); iter++)
                {
                    Aura* aura = iter->second->GetBase();
                    if (aura->GetSpellInfo()->GetSpellSpecific() == SPELL_SPECIFIC_SEAL)
                    {
                        switch (aura->GetSpellInfo()->Id)
                        {
                            case 20165: // Seal of Insight
                                basepoints0 = 0.25f * spellPower + 0.16f * attackPower;
                                break;
                            case 20154: // Seal of Righteousness
                                basepoints0 = 0.32f * spellPower + 0.2f * attackPower;
                                break;
                            case 20164: // Seal of Justice
                                basepoints0 = 0.25f * spellPower + 0.16f * attackPower;
                                break;
                            case 31801: // Seal of Truth
                            {
                                basepoints0 = 0.223f * spellPower + 0.142f * attackPower;
                                // Damage is increased by 20% per stack
                                if (Aura* censure = GetHitUnit()->GetAura(31803, caster->GetGUID()))
                                    AddPct(basepoints0, censure->GetStackAmount() * 20);
                                break;
                            }
                        }
                        break;
                    }
                }
                caster->CastCustomSpell(GetHitUnit(), 54158, &basepoints0, NULL, NULL, true);
                // Long arm of the law
                if (AuraEffect* aurEff = caster->GetDummyAuraEffect(SPELLFAMILY_PALADIN, 3013, EFFECT_0))
                    if (roll_chance_i(aurEff->GetAmount()))
                        if (caster->GetDistance(GetHitUnit()) > 15.0f)
                            caster->CastSpell(caster, 87173, true);

                // Communion
                if (AuraEffect* communion = caster->GetAuraEffect(31876, EFFECT_1, caster->GetGUID()))
                    caster->CastSpell(caster, 57669, true);
            }

            void Register()
            {
                OnEffectHitTarget += SpellEffectFn(spell_pal_judgement_SpellScript::HandleScriptEffect, EFFECT_0, SPELL_EFFECT_SCRIPT_EFFECT);
            }
        };

        SpellScript* GetSpellScript() const
        {
            return new spell_pal_judgement_SpellScript();
        }
};

// 20911 - Blessing of Sanctuary
// 25899 - Greater Blessing of Sanctuary
class spell_pal_blessing_of_sanctuary : public SpellScriptLoader
{
    public:
        spell_pal_blessing_of_sanctuary() : SpellScriptLoader("spell_pal_blessing_of_sanctuary") { }

        class spell_pal_blessing_of_sanctuary_AuraScript : public AuraScript
        {
            PrepareAuraScript(spell_pal_blessing_of_sanctuary_AuraScript);

            bool Validate(SpellInfo const* /*spellInfo*/)
            {
                if (!sSpellMgr->GetSpellInfo(SPELL_PALADIN_BLESSING_OF_SANCTUARY_BUFF))
                    return false;
                if (!sSpellMgr->GetSpellInfo(SPELL_PALADIN_BLESSING_OF_SANCTUARY_ENERGIZE))
                    return false;
                return true;
            }

            void HandleEffectApply(AuraEffect const* /*aurEff*/, AuraEffectHandleModes /*mode*/)
            {
                Unit* target = GetTarget();
                if (Unit* caster = GetCaster())
                    caster->CastSpell(target, SPELL_PALADIN_BLESSING_OF_SANCTUARY_BUFF, true);
            }

            void HandleEffectRemove(AuraEffect const* /*aurEff*/, AuraEffectHandleModes /*mode*/)
            {
                Unit* target = GetTarget();
                target->RemoveAura(SPELL_PALADIN_BLESSING_OF_SANCTUARY_BUFF, GetCasterGUID());
            }

            bool CheckProc(ProcEventInfo& /*eventInfo*/)
            {
                return GetTarget()->getPowerType() == POWER_MANA;
            }

            void HandleProc(AuraEffect const* aurEff, ProcEventInfo& /*eventInfo*/)
            {
                PreventDefaultAction();
                GetTarget()->CastSpell(GetTarget(), SPELL_PALADIN_BLESSING_OF_SANCTUARY_ENERGIZE, true, NULL, aurEff);
            }

            void Register()
            {
                AfterEffectApply += AuraEffectApplyFn(spell_pal_blessing_of_sanctuary_AuraScript::HandleEffectApply, EFFECT_0, SPELL_AURA_DUMMY, AURA_EFFECT_HANDLE_REAL_OR_REAPPLY_MASK);
                AfterEffectRemove += AuraEffectRemoveFn(spell_pal_blessing_of_sanctuary_AuraScript::HandleEffectRemove, EFFECT_0, SPELL_AURA_DUMMY, AURA_EFFECT_HANDLE_REAL_OR_REAPPLY_MASK);
                DoCheckProc += AuraCheckProcFn(spell_pal_blessing_of_sanctuary_AuraScript::CheckProc);
                OnEffectProc += AuraEffectProcFn(spell_pal_blessing_of_sanctuary_AuraScript::HandleProc, EFFECT_0, SPELL_AURA_DUMMY);
            }
        };

        AuraScript* GetAuraScript() const
        {
            return new spell_pal_blessing_of_sanctuary_AuraScript();
        }
};

// 64205 - Divine Sacrifice
class spell_pal_divine_sacrifice : public SpellScriptLoader
{
    public:
        spell_pal_divine_sacrifice() : SpellScriptLoader("spell_pal_divine_sacrifice") { }

        class spell_pal_divine_sacrifice_AuraScript : public AuraScript
        {
            PrepareAuraScript(spell_pal_divine_sacrifice_AuraScript);

            uint32 groupSize, minHpPct;
            int32 remainingAmount;

            bool Load()
            {

                if (Unit* caster = GetCaster())
                {
                    if (caster->GetTypeId() == TYPEID_PLAYER)
                    {
                        if (caster->ToPlayer()->GetGroup())
                            groupSize = caster->ToPlayer()->GetGroup()->GetMembersCount();
                        else
                            groupSize = 1;
                    }
                    else
                        return false;

                    remainingAmount = (caster->CountPctFromMaxHealth(GetSpellInfo()->Effects[EFFECT_2].CalcValue(caster)) * groupSize);
                    minHpPct = GetSpellInfo()->Effects[EFFECT_1].CalcValue(caster);
                    return true;
                }
                return false;
            }

            void Split(AuraEffect* /*aurEff*/, DamageInfo & /*dmgInfo*/, uint32 & splitAmount)
            {
                remainingAmount -= splitAmount;
                // break when absorbed everything it could, or if the casters hp drops below 20%
                if (Unit* caster = GetCaster())
                    if (remainingAmount <= 0 || (caster->GetHealthPct() < minHpPct))
                        caster->RemoveAura(SPELL_PALADIN_DIVINE_SACRIFICE);
            }

            void Register()
            {
                OnEffectSplit += AuraEffectSplitFn(spell_pal_divine_sacrifice_AuraScript::Split, EFFECT_0);
            }
        };

        AuraScript* GetAuraScript() const
        {
            return new spell_pal_divine_sacrifice_AuraScript();
        }
};

// 53385 - Divine Storm
class spell_pal_divine_storm : public SpellScriptLoader
{
    public:
        spell_pal_divine_storm() : SpellScriptLoader("spell_pal_divine_storm") { }

        class spell_pal_divine_storm_SpellScript : public SpellScript
        {
            PrepareSpellScript(spell_pal_divine_storm_SpellScript);

            uint32 healPct;
            uint8 targetCount;

            bool Validate(SpellInfo const* /*spellInfo*/)
            {
                if (!sSpellMgr->GetSpellInfo(SPELL_PALADIN_DIVINE_STORM_DUMMY))
                    return false;
                return true;
            }

            bool Load()
            {
                healPct = GetSpellInfo()->Effects[EFFECT_1].CalcValue(GetCaster());
                targetCount = 0;
                return true;
            }
            
            void CheckTargetCount(std::list<WorldObject*>& targets)
            {
                targetCount = targets.size();
            }
            
            void HandleRewardPoints(SpellEffIndex index)
            {
                if (targetCount < 4)
                    PreventHitDefaultEffect(index);
            }

            void TriggerHeal()
            {
                Unit* caster = GetCaster();
                caster->CastCustomSpell(SPELL_PALADIN_DIVINE_STORM_DUMMY, SPELLVALUE_BASE_POINT0, (GetHitDamage() * healPct) / 100, caster, true);
            }

            void Register()
            {
                AfterHit += SpellHitFn(spell_pal_divine_storm_SpellScript::TriggerHeal);
                OnObjectAreaTargetSelect += SpellObjectAreaTargetSelectFn(spell_pal_divine_storm_SpellScript::CheckTargetCount, EFFECT_2, TARGET_UNIT_SRC_AREA_ENEMY);
                OnEffectHitTarget += SpellEffectFn(spell_pal_divine_storm_SpellScript::HandleRewardPoints, EFFECT_0, SPELL_EFFECT_ENERGIZE);
            }
        };

        SpellScript* GetSpellScript() const
        {
            return new spell_pal_divine_storm_SpellScript();
        }
};

// 54171 - Divine Storm (Dummy)
class spell_pal_divine_storm_dummy : public SpellScriptLoader
{
    public:
        spell_pal_divine_storm_dummy() : SpellScriptLoader("spell_pal_divine_storm_dummy") { }

        class spell_pal_divine_storm_dummy_SpellScript : public SpellScript
        {
            PrepareSpellScript(spell_pal_divine_storm_dummy_SpellScript);

            bool Validate(SpellInfo const* /*spellInfo*/)
            {
                if (!sSpellMgr->GetSpellInfo(SPELL_PALADIN_DIVINE_STORM_HEAL))
                    return false;
                return true;
            }

            void CountTargets(std::list<WorldObject*>& targetList)
            {
                _targetCount = targetList.size();
            }

            void HandleDummy(SpellEffIndex /*effIndex*/)
            {
                if (!_targetCount || ! GetHitUnit())
                    return;

                int32 heal = GetEffectValue() / _targetCount;
                GetCaster()->CastCustomSpell(GetHitUnit(), SPELL_PALADIN_DIVINE_STORM_HEAL, &heal, NULL, NULL, true);
            }
        private:
            uint32 _targetCount;

            void Register()
            {
                OnEffectHitTarget += SpellEffectFn(spell_pal_divine_storm_dummy_SpellScript::HandleDummy, EFFECT_0, SPELL_EFFECT_DUMMY);
                OnObjectAreaTargetSelect += SpellObjectAreaTargetSelectFn(spell_pal_divine_storm_dummy_SpellScript::CountTargets, EFFECT_0, TARGET_UNIT_CASTER_AREA_RAID);
            }
        };

        SpellScript* GetSpellScript() const
        {
            return new spell_pal_divine_storm_dummy_SpellScript();
        }
};

// 33695 - Exorcism and Holy Wrath Damage
class spell_pal_exorcism_and_holy_wrath_damage : public SpellScriptLoader
{
    public:
        spell_pal_exorcism_and_holy_wrath_damage() : SpellScriptLoader("spell_pal_exorcism_and_holy_wrath_damage") { }

        class spell_pal_exorcism_and_holy_wrath_damage_AuraScript : public AuraScript
        {
            PrepareAuraScript(spell_pal_exorcism_and_holy_wrath_damage_AuraScript);

            void HandleEffectCalcSpellMod(AuraEffect const* aurEff, SpellModifier*& spellMod)
            {
                if (!spellMod)
                {
                    spellMod = new SpellModifier(aurEff->GetBase());
                    spellMod->op = SPELLMOD_DAMAGE;
                    spellMod->type = SPELLMOD_FLAT;
                    spellMod->spellId = GetId();
                    spellMod->mask[1] = 0x200002;
                }

                spellMod->value = aurEff->GetAmount();
            }

            void Register()
            {
                DoEffectCalcSpellMod += AuraEffectCalcSpellModFn(spell_pal_exorcism_and_holy_wrath_damage_AuraScript::HandleEffectCalcSpellMod, EFFECT_0, SPELL_AURA_DUMMY);
            }
        };

        AuraScript* GetAuraScript() const
        {
            return new spell_pal_exorcism_and_holy_wrath_damage_AuraScript();
        }
};

// -9799 - Eye for an Eye
class spell_pal_eye_for_an_eye : public SpellScriptLoader
{
    public:
        spell_pal_eye_for_an_eye() : SpellScriptLoader("spell_pal_eye_for_an_eye") { }

        class spell_pal_eye_for_an_eye_AuraScript : public AuraScript
        {
            PrepareAuraScript(spell_pal_eye_for_an_eye_AuraScript);

            bool Validate(SpellInfo const* /*spellInfo*/)
            {
                if (!sSpellMgr->GetSpellInfo(SPELL_PALADIN_EYE_FOR_AN_EYE_DAMAGE))
                    return false;
                return true;
            }

            void OnProc(AuraEffect const* aurEff, ProcEventInfo& eventInfo)
            {
                PreventDefaultAction();
                // return damage % to attacker but < 50% own total health
                int32 damage = int32(std::min(CalculatePct(eventInfo.GetDamageInfo()->GetDamage(), aurEff->GetAmount()), GetTarget()->GetMaxHealth() / 2));
                GetTarget()->CastCustomSpell(SPELL_PALADIN_EYE_FOR_AN_EYE_DAMAGE, SPELLVALUE_BASE_POINT0, damage, eventInfo.GetProcTarget(), true, NULL, aurEff);
            }

            void Register()
            {
                OnEffectProc += AuraEffectProcFn(spell_pal_eye_for_an_eye_AuraScript::OnProc, EFFECT_0, SPELL_AURA_DUMMY);
            }
        };

        AuraScript* GetAuraScript() const
        {
            return new spell_pal_eye_for_an_eye_AuraScript();
        }
};

// 54968 - Glyph of Holy Light
class spell_pal_glyph_of_holy_light : public SpellScriptLoader
{
    public:
        spell_pal_glyph_of_holy_light() : SpellScriptLoader("spell_pal_glyph_of_holy_light") { }

        class spell_pal_glyph_of_holy_light_SpellScript : public SpellScript
        {
            PrepareSpellScript(spell_pal_glyph_of_holy_light_SpellScript);

            void FilterTargets(std::list<WorldObject*>& targets)
            {
                uint32 const maxTargets = GetSpellInfo()->MaxAffectedTargets;

                if (targets.size() > maxTargets)
                {
                    targets.sort(Trinity::HealthPctOrderPred());
                    targets.resize(maxTargets);
                }
            }

            void Register() 
            {
                OnObjectAreaTargetSelect += SpellObjectAreaTargetSelectFn(spell_pal_glyph_of_holy_light_SpellScript::FilterTargets, EFFECT_0, TARGET_UNIT_DEST_AREA_ALLY);
            }
        };

        SpellScript* GetSpellScript() const 
        {
            return new spell_pal_glyph_of_holy_light_SpellScript();
        }
};

// 63521 - Guarded by The Light
class spell_pal_guarded_by_the_light : public SpellScriptLoader
{
    public:
        spell_pal_guarded_by_the_light() : SpellScriptLoader("spell_pal_guarded_by_the_light") { }

        class spell_pal_guarded_by_the_light_SpellScript : public SpellScript
        {
            PrepareSpellScript(spell_pal_guarded_by_the_light_SpellScript);

            bool Validate(SpellInfo const* /*spellInfo*/)
            {
                if (!sSpellMgr->GetSpellInfo(SPELL_PALADIN_DIVINE_PLEA))
                    return false;
                return true;
            }

            void HandleScriptEffect(SpellEffIndex /*effIndex*/)
            {
                // Divine Plea
                if (Aura* aura = GetCaster()->GetAura(SPELL_PALADIN_DIVINE_PLEA))
                    aura->RefreshDuration();
            }

            void Register()
            {
                OnEffectHitTarget += SpellEffectFn(spell_pal_guarded_by_the_light_SpellScript::HandleScriptEffect, EFFECT_0, SPELL_EFFECT_SCRIPT_EFFECT);
            }
        };

        SpellScript* GetSpellScript() const
        {
            return new spell_pal_guarded_by_the_light_SpellScript();
        }
};

// 6940 - Hand of Sacrifice
class spell_pal_hand_of_sacrifice : public SpellScriptLoader
{
    public:
        spell_pal_hand_of_sacrifice() : SpellScriptLoader("spell_pal_hand_of_sacrifice") { }

        class spell_pal_hand_of_sacrifice_AuraScript : public AuraScript
        {
            PrepareAuraScript(spell_pal_hand_of_sacrifice_AuraScript);

            int32 remainingAmount;

            bool Load()
            {
                if (Unit* caster = GetCaster())
                {
                    remainingAmount = caster->GetMaxHealth();
                    return true;
                }
                return false;
            }

            void Split(AuraEffect* /*aurEff*/, DamageInfo & /*dmgInfo*/, uint32 & splitAmount)
            {
                remainingAmount -= splitAmount;

                if (remainingAmount <= 0)
                {
                    GetTarget()->RemoveAura(SPELL_PALADIN_HAND_OF_SACRIFICE);
                }
            }

            void Register()
            {
                OnEffectSplit += AuraEffectSplitFn(spell_pal_hand_of_sacrifice_AuraScript::Split, EFFECT_0);
            }
        };

        AuraScript* GetAuraScript() const
        {
            return new spell_pal_hand_of_sacrifice_AuraScript();
        }
};

// 1038 - Hand of Salvation
class spell_pal_hand_of_salvation : public SpellScriptLoader
{
    public:
        spell_pal_hand_of_salvation() : SpellScriptLoader("spell_pal_hand_of_salvation") { }

        class spell_pal_hand_of_salvation_AuraScript : public AuraScript
        {
            PrepareAuraScript(spell_pal_hand_of_salvation_AuraScript);

            void CalculateAmount(AuraEffect const* /*aurEff*/, int32& amount, bool& /*canBeRecalculated*/)
            {
                if (Unit* caster = GetCaster())
                {
                    // Glyph of Salvation
                    if (caster->GetGUID() == GetUnitOwner()->GetGUID())
                        if (AuraEffect const* aurEff = caster->GetAuraEffect(SPELL_PALADIN_GLYPH_OF_SALVATION, EFFECT_0))
                            amount -= aurEff->GetAmount();
                }
            }

            void Register()
            {
                DoEffectCalcAmount += AuraEffectCalcAmountFn(spell_pal_hand_of_salvation_AuraScript::CalculateAmount, EFFECT_1, SPELL_AURA_MOD_DAMAGE_PERCENT_TAKEN);
            }
        };

        AuraScript* GetAuraScript() const
        {
            return new spell_pal_hand_of_salvation_AuraScript();
        }
};

// -20473 - Holy Shock
class spell_pal_holy_shock : public SpellScriptLoader
{
    public:
        spell_pal_holy_shock() : SpellScriptLoader("spell_pal_holy_shock") { }

        class spell_pal_holy_shock_SpellScript : public SpellScript
        {
            PrepareSpellScript(spell_pal_holy_shock_SpellScript);

            bool Validate(SpellInfo const* spell)
            {
                if (!sSpellMgr->GetSpellInfo(SPELL_PALADIN_HOLY_SHOCK_R1))
                    return false;

                // can't use other spell than holy shock due to spell_ranks dependency
                if (sSpellMgr->GetFirstSpellInChain(SPELL_PALADIN_HOLY_SHOCK_R1) != sSpellMgr->GetFirstSpellInChain(spell->Id))
                    return false;

                uint8 rank = sSpellMgr->GetSpellRank(spell->Id);
                if (!sSpellMgr->GetSpellWithRank(SPELL_PALADIN_HOLY_SHOCK_R1_DAMAGE, rank, true) || !sSpellMgr->GetSpellWithRank(SPELL_PALADIN_HOLY_SHOCK_R1_HEALING, rank, true))
                    return false;

                return true;
            }

            void HandleDummy(SpellEffIndex /*effIndex*/)
            {
                Unit* caster = GetCaster();
                if (Unit* unitTarget = GetHitUnit())
                {
                    uint8 rank = sSpellMgr->GetSpellRank(GetSpellInfo()->Id);
                    if (caster->IsFriendlyTo(unitTarget))
                        caster->CastSpell(unitTarget, sSpellMgr->GetSpellWithRank(SPELL_PALADIN_HOLY_SHOCK_R1_HEALING, rank), true, 0);
                    else
                        caster->CastSpell(unitTarget, sSpellMgr->GetSpellWithRank(SPELL_PALADIN_HOLY_SHOCK_R1_DAMAGE, rank), true, 0);
                }
            }

            SpellCastResult CheckCast()
            {
                Unit* caster = GetCaster();
                if (Unit* target = GetExplTargetUnit())
                {
                    if (!caster->IsFriendlyTo(target))
                    {
                        if (!caster->IsValidAttackTarget(target))
                            return SPELL_FAILED_BAD_TARGETS;

                        if (!caster->isInFront(target))
                            return SPELL_FAILED_UNIT_NOT_INFRONT;
                    }
                }
                else
                    return SPELL_FAILED_BAD_TARGETS;
                return SPELL_CAST_OK;
            }

            void Register()
            {
                OnCheckCast += SpellCheckCastFn(spell_pal_holy_shock_SpellScript::CheckCast);
                OnEffectHitTarget += SpellEffectFn(spell_pal_holy_shock_SpellScript::HandleDummy, EFFECT_0, SPELL_EFFECT_DUMMY);
            }
        };

        SpellScript* GetSpellScript() const
        {
            return new spell_pal_holy_shock_SpellScript();
        }
};

// -633 - Lay on Hands
class spell_pal_lay_on_hands : public SpellScriptLoader
{
    class script_impl : public SpellScript
    {
        PrepareSpellScript(script_impl);

        enum
        {
            SPELL_FORBEARANCE               = 25771,
            SPELL_AVENGING_WRATH_MARKER     = 61987,
            SPELL_IMMUNE_SHIELD_MARKER      = 61988,
            SPELL_GLYPH_OF_DIVINITY         = 54939,
            SPELL_GLYPH_OF_DIVINITY_MANA    = 54986,
        };

        bool Load()
        {
            return GetCaster()->GetTypeId() == TYPEID_PLAYER;
        }

        SpellCastResult CheckCast()
        {
            Unit* const caster = GetCaster();
            if (Unit* const target = GetExplTargetUnit())
                if (caster == target)
                    if (target->HasAura(SPELL_FORBEARANCE)
                        || target->HasAura(SPELL_AVENGING_WRATH_MARKER)
                        || target->HasAura(SPELL_IMMUNE_SHIELD_MARKER))
                        return SPELL_FAILED_TARGET_AURASTATE;

            return SPELL_CAST_OK;
        }

        void HandleScript()
        {
            // Really only on caster?
            Player* const caster = GetCaster()->ToPlayer();
            if (caster == GetHitUnit())
            {
                caster->CastSpell(caster, SPELL_FORBEARANCE, true);
                caster->CastSpell(caster, SPELL_AVENGING_WRATH_MARKER, true);
                caster->CastSpell(caster, SPELL_IMMUNE_SHIELD_MARKER, true);
            }

            // Glyph of Divinity
            if (caster->HasAura(SPELL_GLYPH_OF_DIVINITY))
                caster->CastSpell(caster, SPELL_GLYPH_OF_DIVINITY_MANA, true);
        }

        void Register()
        {
            OnCheckCast += SpellCheckCastFn(script_impl::CheckCast);
            AfterHit += SpellHitFn(script_impl::HandleScript);
        }
    };

public:
    spell_pal_lay_on_hands()
        : SpellScriptLoader("spell_pal_lay_on_hands")
    {
    }

    SpellScript* GetSpellScript() const
    {
        return new script_impl();
    }
};

// 31789 - Righteous Defense
class spell_pal_righteous_defense : public SpellScriptLoader
{
    public:
        spell_pal_righteous_defense() : SpellScriptLoader("spell_pal_righteous_defense") { }

        class spell_pal_righteous_defense_SpellScript : public SpellScript
        {
            PrepareSpellScript(spell_pal_righteous_defense_SpellScript);

            bool Validate(SpellInfo const* /*spellInfo*/)
            {
                if (!sSpellMgr->GetSpellInfo(SPELL_PALADIN_RIGHTEOUS_DEFENSE_TAUNT))
                    return false;
                return true;
            }

            SpellCastResult CheckCast()
            {
                Unit* caster = GetCaster();
                if (caster->GetTypeId() != TYPEID_PLAYER)
                    return SPELL_FAILED_DONT_REPORT;

                if (Unit* target = GetExplTargetUnit())
                {
                    if (!target->IsFriendlyTo(caster) || target->getAttackers().empty())
                        return SPELL_FAILED_BAD_TARGETS;
                }
                else
                    return SPELL_FAILED_BAD_TARGETS;

                return SPELL_CAST_OK;
            }

            void HandleTriggerSpellLaunch(SpellEffIndex effIndex)
            {
                PreventHitDefaultEffect(effIndex);
            }

            void HandleTriggerSpellHit(SpellEffIndex effIndex)
            {
                PreventHitDefaultEffect(effIndex);
                if (Unit* target = GetHitUnit())
                    GetCaster()->CastSpell(target, SPELL_PALADIN_RIGHTEOUS_DEFENSE_TAUNT, true);
            }

            void Register()
            {
                OnCheckCast += SpellCheckCastFn(spell_pal_righteous_defense_SpellScript::CheckCast);
                //! WORKAROUND
                //! target select will be executed in hitphase of effect 0
                //! so we must handle trigger spell also in hit phase (default execution in launch phase)
                //! see issue #3718
                OnEffectLaunchTarget += SpellEffectFn(spell_pal_righteous_defense_SpellScript::HandleTriggerSpellLaunch, EFFECT_1, SPELL_EFFECT_TRIGGER_SPELL);
                OnEffectHitTarget += SpellEffectFn(spell_pal_righteous_defense_SpellScript::HandleTriggerSpellHit, EFFECT_1, SPELL_EFFECT_TRIGGER_SPELL);
            }
        };

        SpellScript* GetSpellScript() const
        {
            return new spell_pal_righteous_defense_SpellScript();
        }
};

// 85256 - Templar's Verdict
class spell_pal_templar_s_verdict : public SpellScriptLoader
{
public:
    spell_pal_templar_s_verdict() : SpellScriptLoader("spell_pal_templar_s_verdict") { }

    class spell_pal_templar_s_verdict_SpellScript : public SpellScript
    {
        PrepareSpellScript(spell_pal_templar_s_verdict_SpellScript);

        bool Validate(SpellInfo const* /*spellEntry*/)
        {
            if (!sSpellMgr->GetSpellInfo(SPELL_PALADIN_DIVINE_PURPOSE_PROC))
                return false;

            return true;
        }

        bool Load()
        {
            if (GetCaster()->GetTypeId() != TYPEID_PLAYER)
                return false;

            if (GetCaster()->ToPlayer()->getClass() != CLASS_PALADIN)
                return false;

            return true;
        }

        void ChangeDamage(SpellEffIndex /*effIndex*/)
        {
            Unit* caster = GetCaster();
            int32 damage = GetHitDamage();
            if (Unit* target = GetHitUnit())
            {
                caster->SpellDamageBonusDone(GetHitUnit(), GetSpellInfo(), damage, SPELL_DIRECT_DAMAGE);
                damage = target->SpellDamageBonusTaken(GetSpellInfo(), damage, SPELL_DIRECT_DAMAGE, 1, caster->GetGUID());;

                if (caster->HasAura(SPELL_PALADIN_DIVINE_PURPOSE_PROC))
                    damage *= 7.5;  // 7.5*30% = 225%
                else
                {
                    switch (caster->GetPower(POWER_HOLY_POWER))
                    {
                    case 1: // 1 Holy Power
                        damage = damage;
                        break;
                    case 2: // 2 Holy Power
                        damage *= 3;    // 3*30 = 90%
                        break;
                    case 3: // 3 Holy Power
                        damage *= 7.5;  // 7.5*30% = 225%
                        break;
                    }
                }
            }
            
            SetHitDamage(damage);
        }

        void Register()
        {
            OnEffectHitTarget += SpellEffectFn(spell_pal_templar_s_verdict_SpellScript::ChangeDamage, EFFECT_0, SPELL_EFFECT_WEAPON_PERCENT_DAMAGE);
        }
    };

    SpellScript* GetSpellScript() const
    {
        return new spell_pal_templar_s_verdict_SpellScript();
    }
};


class spell_pal_shield_of_the_righteous : public SpellScriptLoader
{
    public:
        spell_pal_shield_of_the_righteous() : SpellScriptLoader("spell_pal_shield_of_the_righteous") { }

        class spell_pal_shield_of_the_righteous_SpellScript : public SpellScript
        {
            PrepareSpellScript(spell_pal_shield_of_the_righteous_SpellScript);

            bool Load()
            {
                if (GetCaster()->GetTypeId() != TYPEID_PLAYER)
                    return false;

                if (GetCaster()->ToPlayer()->getClass() != CLASS_PALADIN)
                    return false;

                return true;
            }

            void ChangeDamage(SpellEffIndex /*effIndex*/)
            {                
                Unit* caster = GetCaster();
                int32 damage = GetHitDamage();

                    switch (caster->GetHolyPoints())
                    {
                        case 1: // 1 Holy Power
                            //damage = damage;
                            break;
                        case 2: // 2 Holy Power
                            damage *= 3;
                            break;
                        case 3: // 3 Holy Power
                            damage *= 6; 
                            break;
                }

                SetHitDamage(damage);
            }

            void Register()
            {
                OnEffectHitTarget += SpellEffectFn(spell_pal_shield_of_the_righteous_SpellScript::ChangeDamage, EFFECT_0, SPELL_EFFECT_SCHOOL_DAMAGE);
            }
        };

        SpellScript* GetSpellScript() const
        {
            return new spell_pal_shield_of_the_righteous_SpellScript();
        }
};

// Light of Dawn
class spell_pal_lod : public SpellScriptLoader
{
    class HealthPctOrderPred
    {
    public:
        HealthPctOrderPred()
        {
        }

        bool operator() (const WorldObject* a, const WorldObject* b) const
        {
            Unit const* const first = a->ToUnit();
            Unit const* const second = b->ToUnit();

            float rA = first->GetMaxHealth() ? float(first->GetHealth()) / float(first->GetMaxHealth()) : 0.0f;
            float rB = second->GetMaxHealth() ? float(second->GetHealth()) / float(second->GetMaxHealth()) : 0.0f;
            return rA < rB;
        }
    };

    class LightOfDawnTargetSelect
    {
        Unit const* const m_caster;

    public:
        LightOfDawnTargetSelect(Unit* caster) : m_caster(caster)
        {
        }

        bool operator() (WorldObject* target)
        {
            if (target->GetTypeId() != TYPEID_PLAYER)
                return true;

            return target->ToPlayer()->GetGroup() != m_caster->ToPlayer()->GetGroup();
        }
    };

    class script_impl : public SpellScript
    {
        PrepareSpellScript(script_impl);

        enum { SPELL_GLYPH_OF_LIGHT_OF_DAWN = 54940 };

        bool m_glyph;

        bool Load()
        {
            Unit const* const caster = GetCaster();
            m_glyph = caster->HasAura(SPELL_GLYPH_OF_LIGHT_OF_DAWN);
            return caster->GetTypeId() == TYPEID_PLAYER;
        }

        void FilterTargets(std::list<WorldObject*>& targets)
        {
            targets.remove_if(LightOfDawnTargetSelect(GetCaster()));
            targets.sort(HealthPctOrderPred());
            if (targets.size() >> (m_glyph ? 4 : 6))
                targets.resize(m_glyph ? 4 : 6);
        }

        void HandleHeal(SpellEffIndex)
        {
            int32 const heal = GetHitHeal() * GetCaster()->GetHolyPoints();
            SetHitHeal(heal);
        }

        void Register()
        {
            OnObjectAreaTargetSelect += SpellObjectAreaTargetSelectFn(script_impl::FilterTargets, EFFECT_0, TARGET_UNIT_CONE_ALLY);
            OnEffectHitTarget += SpellEffectFn(script_impl::HandleHeal, EFFECT_0, SPELL_EFFECT_HEAL);
        }
    };

public:
    spell_pal_lod()
        : SpellScriptLoader("spell_pal_lod")
    {
    }

    SpellScript* GetSpellScript() const
    {
        return new script_impl();
    }
};

// Consecration
class spell_pal_consecration : public SpellScriptLoader
{
    public:
        spell_pal_consecration() : SpellScriptLoader("spell_pal_consecration") { }

        class spell_pal_consecration_AuraScript : public AuraScript
        {
            PrepareAuraScript(spell_pal_consecration_AuraScript);

            bool Validate(SpellInfo const* /*spellInfo*/) { return true; }

            void HandleEffectPeriodic(AuraEffect const* aurEff)
            {
                Unit* caster = GetCaster();
                // Dummy aura is applied to enemys around us - Why?
                if (GetTarget() != GetCaster())
                    return;

                std::list<Creature*> MinionList;
                GetTarget()->GetAllMinionsByEntry(MinionList, 43499);
                for (std::list<Creature*>::iterator itr = MinionList.begin(); itr != MinionList.end(); itr++)                
                    GetTarget()->CastSpell((*itr)->GetPositionX(), (*itr)->GetPositionY(), (*itr)->GetPositionZ(), 81297, true);                    
            }

            void Apply(AuraEffect const* /*aurEff*/, AuraEffectHandleModes /*mode*/)
            {
                if (GetTarget() != GetCaster())
                    return;

                std::list<Creature*> MinionList;
                GetTarget()->GetAllMinionsByEntry(MinionList, 43499);
                TempSummon* consecration = NULL;
                // Get the last summoned Consecration, save it and despawn older ones
                for (std::list<Creature*>::iterator itr = MinionList.begin(); itr != MinionList.end(); itr++)
                {
                    TempSummon* summon = (*itr)->ToTempSummon();

                    if (consecration && summon)
                    {
                        if (summon->GetTimer() > consecration->GetTimer())
                        {
                            consecration->DespawnOrUnsummon();
                            consecration = summon;
                        }
                        else
                            summon->DespawnOrUnsummon();
                    }
                    else if (summon)
                        consecration = summon;
                }
            }

            void Register()
            {
                OnEffectApply += AuraEffectApplyFn(spell_pal_consecration_AuraScript::Apply, EFFECT_1, SPELL_AURA_PERIODIC_DUMMY, AURA_EFFECT_HANDLE_REAL_OR_REAPPLY_MASK);
                OnEffectPeriodic += AuraEffectPeriodicFn(spell_pal_consecration_AuraScript::HandleEffectPeriodic, EFFECT_1, SPELL_AURA_PERIODIC_DUMMY);
            }
        };

        AuraScript* GetAuraScript() const
        {
            return new spell_pal_consecration_AuraScript();
        }
};

class spell_pal_word_of_glory: public SpellScriptLoader
{
    class script_impl : public SpellScript
    {
        PrepareSpellScript(script_impl);

        enum
        {
            SPELL_ETERNAL_GLORY             = 87163,
            SPELL_ETERNAL_GLORY_ENERGIZE    = 88676,
            SPELL_GUARDED_BY_THE_LIGHT      = 85639,
            SPELL_SELFLESS_HEALER           = 85803,
            SPELL_GLYPH_OF_LONG_WORD        = 93466,
        };

        int32 m_holyPower;

        bool Load()
        {
            m_holyPower = 0;
            return true;
        }

        void HandleHeal(SpellEffIndex)
        {
            Unit const* const caster = GetCaster();
            Unit const* const target = GetHitUnit();
            if (!target)
                return;

            int32 heal = GetHitHeal();
            heal *= caster->GetHolyPoints();

            if (caster == target)
            {
                // Guarded by the Light
                if (AuraEffect const* const aurEff = caster->GetAuraEffectOfRankedSpell(SPELL_GUARDED_BY_THE_LIGHT, EFFECT_0))
                    AddPct(heal, aurEff->GetAmount());
            }
            else
            {
                // Selfless Healer
                if (AuraEffect const* const aurEff = caster->GetAuraEffectOfRankedSpell(SPELL_SELFLESS_HEALER, EFFECT_0))
                    AddPct(heal, aurEff->GetAmount());
            }

            SetHitHeal(heal);
        }

        void HandleHoT(SpellEffIndex)
        {
            if (!GetCaster()->HasAura(SPELL_GLYPH_OF_LONG_WORD))
                PreventHitAura();
        }

        void GetHolyPower()
        {
            m_holyPower = GetCaster()->GetHolyPoints();
        }

        void HandleEnergize()
        {
            // Eternal Glory
            Unit* const caster = GetCaster();
            if (AuraEffect const* const aurEff = caster->GetAuraEffectOfRankedSpell(SPELL_ETERNAL_GLORY, EFFECT_0))
                if (roll_chance_i(aurEff->GetAmount()) && m_holyPower > 0)
                    caster->CastCustomSpell(caster, SPELL_ETERNAL_GLORY_ENERGIZE, &m_holyPower, NULL, NULL, true);
        }

        void Register()
        {
            OnEffectHitTarget += SpellEffectFn(script_impl::HandleHeal, EFFECT_0, SPELL_EFFECT_HEAL);
            OnEffectHitTarget += SpellEffectFn(script_impl::HandleHoT, EFFECT_1, SPELL_EFFECT_APPLY_AURA);
            BeforeCast += SpellCastFn(script_impl::GetHolyPower);
            AfterCast += SpellCastFn(script_impl::HandleEnergize);
        }
    };

public:
    spell_pal_word_of_glory()
        : SpellScriptLoader("spell_pal_word_of_glory")
    {
    }

    SpellScript* GetSpellScript() const
    {
        return new script_impl();
    }
};

class spell_paladin_divine_purpose : public SpellScriptLoader
{
    class script_impl : public AuraScript
    {
        PrepareAuraScript(script_impl);

        enum { SPELL_DIVINE_PURPOSE = 90174 };

        void HandleProc(AuraEffect const* aurEff, ProcEventInfo&)
        {
            if (roll_chance_i(aurEff->GetAmount()))
            {
                Unit* const target = GetTarget();
                target->CastSpell(target, SPELL_DIVINE_PURPOSE, true);
            }
        }

        void Register()
        {
            OnEffectProc += AuraEffectProcFn(script_impl::HandleProc, EFFECT_0, SPELL_AURA_DUMMY);
        }
    };

public:
    spell_paladin_divine_purpose()
        : SpellScriptLoader("spell_paladin_divine_purpose")
    {
    }

    AuraScript* GetAuraScript() const
    {
        return new script_impl();
    }
};

// Cleanse
class spell_pal_cleanse : public SpellScriptLoader
{
    public:
        spell_pal_cleanse() : SpellScriptLoader("spell_pal_cleanse") { }

        class spell_pal_cleanse_SpellScript : public SpellScript
        {
            PrepareSpellScript(spell_pal_cleanse_SpellScript);

            void HandleOnHit()
            {
                Unit* caster = GetCaster();
                Unit* target = GetHitUnit();
                if (caster != target)
                    return;
                
                if (AuraEffect* sacrifice = caster->GetAuraEffect(SPELL_AURA_ADD_PCT_MODIFIER, SPELLFAMILY_PALADIN, 3022, EFFECT_0))
                {
                    for (Unit::AuraApplicationMap::iterator iter = caster->GetAppliedAuras().begin(); iter != caster->GetAppliedAuras().end();)
                    {
                        Aura const* aura = iter->second->GetBase();
                        if (aura->GetSpellInfo()->GetAllEffectsMechanicMask() & (1 << MECHANIC_SNARE | 1 << MECHANIC_ROOT))
                        {
                            caster->RemoveAurasDueToSpellByDispel(aura->GetId(), GetSpellInfo()->Id, aura->GetCasterGUID(), caster, aura->GetStackAmount());
                            break;
                        }
                        ++iter;
                    }
                }
            }

            void Register()
            {
                OnHit += SpellHitFn(spell_pal_cleanse_SpellScript::HandleOnHit);
            }
        };

        SpellScript* GetSpellScript() const
        {
            return new spell_pal_cleanse_SpellScript();
        }
};

// Inquisition
class spell_pal_inquisition : public SpellScriptLoader
{
    class script_impl : public SpellScript
    {
        PrepareSpellScript(script_impl);

        enum { SPELL_ITEM_SET_T11_4P_BONUS = 90299 };

        void HandleApply(SpellEffIndex)
        {
            Unit* const caster = GetCaster();
            int32 holyPower = caster->GetHolyPoints();
            if (caster->HasAura(SPELL_ITEM_SET_T11_4P_BONUS))
                holyPower += 1;
            Aura* const aura = GetHitAura();
            aura->SetDuration(aura->GetMaxDuration() * holyPower);
        }

        void Register()
        {
            OnEffectHitTarget += SpellEffectFn(script_impl::HandleApply, EFFECT_0, SPELL_EFFECT_APPLY_AURA);
        }
    };

public:
    spell_pal_inquisition()
        : SpellScriptLoader("spell_pal_inquisition")
    {
    }

    SpellScript* GetSpellScript() const
    {
        return new script_impl();
    }
};

// 20154 - Seal of Righteousness
class spell_pal_seal_of_righteousness : public SpellScriptLoader
{
    public:
        spell_pal_seal_of_righteousness() : SpellScriptLoader("spell_pal_seal_of_righteousness") { }

        class spell_pal_seal_of_righteousness_AuraScript : public AuraScript
        {
            PrepareAuraScript(spell_pal_seal_of_righteousness_AuraScript);

            bool Validate(SpellInfo const* /*spellInfo*/)
            {
                if (!sSpellMgr->GetSpellInfo(SPELL_PALADIN_SEAL_OF_RIGHTEOUSNESS))
                    return false;
                return true;
            }

            bool CheckProc(ProcEventInfo& eventInfo)
            {
                return eventInfo.GetProcTarget();
            }

            void HandleProc(AuraEffect const* aurEff, ProcEventInfo& eventInfo)
            {
                PreventDefaultAction();

                float ap = GetTarget()->GetTotalAttackPowerValue(BASE_ATTACK);
                int32 holy = GetTarget()->SpellBaseDamageBonusDone(SPELL_SCHOOL_MASK_HOLY);
                holy += eventInfo.GetProcTarget()->SpellBaseDamageBonusTaken(SPELL_SCHOOL_MASK_HOLY);
                int32 bp = int32((ap * 0.011f + 0.022f * holy) * GetTarget()->GetAttackTime(BASE_ATTACK) / 1000);
                GetTarget()->CastCustomSpell(SPELL_PALADIN_SEAL_OF_RIGHTEOUSNESS, SPELLVALUE_BASE_POINT0, bp, eventInfo.GetProcTarget(), true, NULL, aurEff);
            }

            void Register()
            {
                DoCheckProc += AuraCheckProcFn(spell_pal_seal_of_righteousness_AuraScript::CheckProc);
                OnEffectProc += AuraEffectProcFn(spell_pal_seal_of_righteousness_AuraScript::HandleProc, EFFECT_0, SPELL_AURA_DUMMY);
            }
        };

        AuraScript* GetAuraScript() const
        {
            return new spell_pal_seal_of_righteousness_AuraScript();
        }
};

class spell_pal_avenging_wrath: public SpellScriptLoader
{
    public:
        spell_pal_avenging_wrath() : SpellScriptLoader("spell_pal_avenging_wrath") { }

        class spell_pal_avenging_wrath_AuraScript : public AuraScript
        {
            PrepareAuraScript(spell_pal_avenging_wrath_AuraScript);

            void HandleEffectApply(AuraEffect const* /*aurEff*/, AuraEffectHandleModes /*mode*/)
            {
                Unit* target = GetTarget();
                if (Unit* caster = GetCaster())
                    if (AuraEffect* aurEff = caster->GetAuraEffect(SPELL_AURA_ADD_FLAT_MODIFIER, SPELLFAMILY_PALADIN, 3029, EFFECT_0))
                        caster->CastSpell(target, 57318, true);
            }

            void HandleEffectRemove(AuraEffect const* /*aurEff*/, AuraEffectHandleModes /*mode*/)
            {
                GetTarget()->RemoveAurasDueToSpell(57318);
            }

            void Register()
            {
                AfterEffectApply += AuraEffectApplyFn(spell_pal_avenging_wrath_AuraScript::HandleEffectApply, EFFECT_2, SPELL_AURA_ADD_PCT_MODIFIER, AURA_EFFECT_HANDLE_REAL);
                AfterEffectRemove += AuraEffectRemoveFn(spell_pal_avenging_wrath_AuraScript::HandleEffectRemove, EFFECT_2, SPELL_AURA_ADD_PCT_MODIFIER, AURA_EFFECT_HANDLE_REAL);
            }
        };

        AuraScript* GetAuraScript() const
        {
            return new spell_pal_avenging_wrath_AuraScript();
        }
};

// Exorcism
class spell_pal_exorcism : public SpellScriptLoader
{
   public:
       spell_pal_exorcism() : SpellScriptLoader("spell_pal_exorcism") { }

       class spell_pal_exorcism_SpellScript : public SpellScript
       {
           PrepareSpellScript(spell_pal_exorcism_SpellScript);

           void RecalculateDamage(SpellEffIndex /*effIndex*/)
           {
               Unit* caster = GetCaster();
               float coeff = 0.344f;
               int32 SpellPowerBonus = caster->SpellBaseDamageBonusDone(GetSpellInfo()->GetSchoolMask());
               int32 APbonus = caster->GetTotalAttackPowerValue(BASE_ATTACK);
               int32 bonus = std::max(SpellPowerBonus, APbonus);
               SetEffectDamage(GetEffectValue() + (bonus * coeff));;
           }

           void HandleDot(SpellEffIndex effIndex)
           {
               AuraEffect* glyph = GetCaster()->GetAuraEffect(54934, EFFECT_0);
               if (!glyph)
               {
                   PreventHitAura();
                   return;
               }
               uint32 dotDamage = CalculatePct(GetHitDamage(), glyph->GetAmount());
               if (GetHitAura())
                   if (AuraEffect* periodic = GetHitAura()->GetEffect(EFFECT_1))
                       periodic->SetAmount(dotDamage / periodic->GetTotalTicks());
           }

           void Register()
           {
               OnEffectLaunchTarget += SpellEffectFn(spell_pal_exorcism_SpellScript::RecalculateDamage, EFFECT_0, SPELL_EFFECT_SCHOOL_DAMAGE);
               OnEffectHitTarget += SpellEffectFn(spell_pal_exorcism_SpellScript::HandleDot, EFFECT_1, SPELL_EFFECT_APPLY_AURA);
           }
       };

       SpellScript* GetSpellScript() const
       {
           return new spell_pal_exorcism_SpellScript();
       }
};

// Holy wrath
class spell_pal_holy_wrath : public SpellScriptLoader
{
    class HolyWrathTargetSelector
    {
        enum { SPELL_GLYPH_OF_HOLY_WRATH = 56420 };

        Unit const* const m_caster;

    public:
        HolyWrathTargetSelector(Unit const* caster)
            : m_caster(caster)
        {
        }

        bool operator() (WorldObject* target)
        {
            Unit* const unit = target->ToUnit();
            if (!unit)
                return true;

            // Demon or Undead
            if (unit->GetCreatureTypeMask() & CREATURE_TYPEMASK_DEMON_OR_UNDEAD)
                return false;

            // With glyph and Dragonkin or Elemental
            if ((unit->GetCreatureType() == CREATURE_TYPE_ELEMENTAL || unit->GetCreatureType() == CREATURE_TYPE_DRAGONKIN) && m_caster->HasAura(SPELL_GLYPH_OF_HOLY_WRATH))
                return false;

            return true;
        }
    };

    class script_impl : public SpellScript
    {
        PrepareSpellScript(script_impl);

        uint8 m_targetsCount;

        bool Load()
        {
            m_targetsCount = 0;
            return GetCaster()->GetTypeId() == TYPEID_PLAYER;
        }

        void FilterStun(std::list<WorldObject*>& unitList)
        {
            unitList.remove_if(HolyWrathTargetSelector(GetCaster()));
        }

        void FilterDamage(std::list<WorldObject*>& unitList)
        {
            m_targetsCount = unitList.size();
        }

        void HandleDamage(SpellEffIndex)
        {
            // Divide damage to all targets
            int32 const value = GetHitDamage() / m_targetsCount;
            SetHitDamage(value);
        }

        void Register()
        {
            OnEffectHitTarget += SpellEffectFn(script_impl::HandleDamage, EFFECT_0, SPELL_EFFECT_SCHOOL_DAMAGE);
            OnObjectAreaTargetSelect += SpellObjectAreaTargetSelectFn(script_impl::FilterDamage, EFFECT_0, TARGET_UNIT_SRC_AREA_ENEMY);
            OnObjectAreaTargetSelect += SpellObjectAreaTargetSelectFn(script_impl::FilterStun, EFFECT_1, TARGET_UNIT_SRC_AREA_ENEMY);
        }
    };

public:
    spell_pal_holy_wrath()
        : SpellScriptLoader("spell_pal_holy_wrath")
    {
    }

    SpellScript* GetSpellScript() const
    {
        return new script_impl();
    }
};

// Sacred shield
class spell_pal_sacred_shield : public SpellScriptLoader
{
    public:
        spell_pal_sacred_shield() : SpellScriptLoader("spell_pal_sacred_shield") { }

        class spell_pal_sacred_shield_AuraScript : public AuraScript
        {
            PrepareAuraScript(spell_pal_sacred_shield_AuraScript);

            void CalculateAmount(AuraEffect const* aurEff, int32& amount, bool& canBeRecalculated)
            {
                canBeRecalculated = false;
                if (Unit* caster = GetCaster())
                {
                    float bonus = 2.8f * caster->GetTotalAttackPowerValue(BASE_ATTACK);
                    bonus = caster->ApplyEffectModifiers(GetSpellInfo(), aurEff->GetEffIndex(), bonus);
                    amount += int32(bonus);
                }
            }

            void Register()
            {
                DoEffectCalcAmount += AuraEffectCalcAmountFn(spell_pal_sacred_shield_AuraScript::CalculateAmount, EFFECT_0, SPELL_AURA_SCHOOL_ABSORB);
            }
        };

        AuraScript* GetAuraScript() const
        {
            return new spell_pal_sacred_shield_AuraScript();
        }
};

class spell_paladin_pursuit_of_justice : public SpellScriptLoader
{
    class script_impl : public AuraScript
    {
        PrepareAuraScript(script_impl);

        enum { SPELL_PURSUIT_OF_JUSTICE_ENERGIZE = 89024 };

        bool Load()
        {
            Unit const* const caster = GetCaster();
            return caster && caster->GetTypeId() == TYPEID_PLAYER;
        }

        bool CheckProc(ProcEventInfo& procInfo)
        {
            SpellInfo const* const spellInfo = procInfo.GetSpellInfo();
            if (!spellInfo)
                return false;

            switch (spellInfo->Mechanic)
            {
                case MECHANIC_STUN:
                case MECHANIC_FEAR:
                case MECHANIC_ROOT:
                    return true;
                default:
                    return false;
            }
        }

        void HandleProc(ProcEventInfo& eventInfo)
        {
            Unit* const caster = GetCaster();
            caster->CastSpell(caster, SPELL_PURSUIT_OF_JUSTICE_ENERGIZE, true);
        }

        void Register()
        {
            DoCheckProc += AuraCheckProcFn(script_impl::CheckProc);
            OnProc += AuraProcFn(script_impl::HandleProc);
        }
    };

public:
    spell_paladin_pursuit_of_justice()
        : SpellScriptLoader("spell_paladin_pursuit_of_justice")
    {
    }

    AuraScript* GetAuraScript() const
    {
        return new script_impl();
    }
};

class spell_paladin_holy_radiance : public SpellScriptLoader
{
    class script_impl : public AuraScript
    {
        PrepareAuraScript(script_impl);

        enum { SPELL_HOLY_RADIANCE = 86452 };

        void OnPeriodic(AuraEffect const* const aurEff)
        {
            PreventDefaultAction();
            if (Unit* const caster = GetCaster())
                caster->CastSpell(caster, SPELL_HOLY_RADIANCE, true);
        }

        void Register()
        {
            OnEffectPeriodic += AuraEffectPeriodicFn(script_impl::OnPeriodic, EFFECT_0, SPELL_AURA_PERIODIC_DUMMY);
        }
    };

public:
    spell_paladin_holy_radiance()
        : SpellScriptLoader("spell_paladin_holy_radiance")
    {
    }

    AuraScript* GetAuraScript() const
    {
        return new script_impl();
    }
};

class spell_pal_judgements_of_the_wise : public SpellScriptLoader
{
    public:
        spell_pal_judgements_of_the_wise() : SpellScriptLoader("spell_pal_judgements_of_the_wise") { }

        class spell_pal_judgements_of_the_wise_AuraScript : public AuraScript
        {
            PrepareAuraScript(spell_pal_judgements_of_the_wise_AuraScript);

            bool Load()
            {
                if (GetCaster()->GetTypeId() != TYPEID_PLAYER)
                    return false;
                return true;
            }

            void CalculateMana(AuraEffect const* /*aurEff*/, int32& amount, bool& canBeRecalculated)
            {
                if (Unit* caster = GetCaster())
                {
                    canBeRecalculated = true;
                    int32 basemana = caster->ToPlayer()->GetCreateMana();
                    amount = basemana * 0.03f;
                }
            }

            void Register()
            {
                DoEffectCalcAmount += AuraEffectCalcAmountFn(spell_pal_judgements_of_the_wise_AuraScript::CalculateMana, EFFECT_0, SPELL_AURA_PERIODIC_ENERGIZE);
            }
        };

        AuraScript* GetAuraScript() const
        {
            return new spell_pal_judgements_of_the_wise_AuraScript();
        }
};

class spell_pal_judgements_of_the_bold : public SpellScriptLoader
{
    public:
        spell_pal_judgements_of_the_bold() : SpellScriptLoader("spell_pal_judgements_of_the_bold") { }

        class spell_pal_judgements_of_the_bold_AuraScript : public AuraScript
        {
            PrepareAuraScript(spell_pal_judgements_of_the_bold_AuraScript);

            bool Load()
            {
                if (GetCaster()->GetTypeId() != TYPEID_PLAYER)
                    return false;
                return true;
            }

            void CalculateMana(AuraEffect const* /*aurEff*/, int32& amount, bool& canBeRecalculated)
            {
                if (Unit* caster = GetCaster())
                {
                    canBeRecalculated = true;
                    int32 basemana = caster->ToPlayer()->GetCreateMana();
                    amount = basemana * 0.025f;
                }
            }

            void Register()
            {
                DoEffectCalcAmount += AuraEffectCalcAmountFn(spell_pal_judgements_of_the_bold_AuraScript::CalculateMana, EFFECT_0, SPELL_AURA_PERIODIC_ENERGIZE);
            }
        };

        AuraScript* GetAuraScript() const
        {
            return new spell_pal_judgements_of_the_bold_AuraScript();
        }
};

void AddSC_paladin_spell_scripts()
{
	new spell_pal_guardian_of_ancient_kings_retri();
	new spell_pal_guardian_of_ancient_kings();
	new spell_pal_ancient_fury();
    new spell_pal_ardent_defender();
    new spell_pal_blessing_of_faith();
    new spell_pal_blessing_of_sanctuary();
    new spell_pal_divine_sacrifice();
    new spell_pal_divine_storm();
    new spell_pal_divine_storm_dummy();
    new spell_pal_exorcism_and_holy_wrath_damage();
    new spell_pal_eye_for_an_eye();
    new spell_pal_glyph_of_holy_light();
    new spell_pal_guarded_by_the_light();
    new spell_pal_hand_of_sacrifice();
    new spell_pal_hand_of_salvation();
    new spell_pal_holy_shock();
    new spell_pal_lay_on_hands();
    new spell_pal_righteous_defense();
    new spell_pal_seal_of_righteousness();
    new spell_pal_templar_s_verdict();
    new spell_pal_shield_of_the_righteous();
    new spell_pal_judgement();
    new spell_pal_lod();
    new spell_pal_avenging_wrath();
    new spell_pal_inquisition();
    new spell_pal_word_of_glory();
    new spell_pal_sacred_shield();
    new spell_pal_consecration();
    new spell_paladin_divine_purpose();
    new spell_pal_cleanse();
    new spell_pal_exorcism();
    new spell_pal_holy_wrath();
    new spell_paladin_pursuit_of_justice();
    new spell_paladin_holy_radiance();
    new spell_pal_judgements_of_the_bold();
    new spell_pal_judgements_of_the_wise();
}
