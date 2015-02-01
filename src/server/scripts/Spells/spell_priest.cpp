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
* Scripts for spells with SPELLFAMILY_PRIEST and SPELLFAMILY_GENERIC spells used by priest players.
* Ordered alphabetically using scriptname.
* Scriptnames of files in this file should be prefixed with "spell_pri_".
*/

#include "Player.h"
#include "ScriptMgr.h"
#include "SpellScript.h"
#include "SpellAuraEffects.h"
#include "GridNotifiers.h"

enum PriestSpells
{
	SPELL_PRIEST_GLYPH_OF_CIRCLE_OF_HEALING = 55675,
	SPELL_PRIEST_DIVINE_AEGIS = 47753,
	SPELL_PRIEST_BODY_AND_SOUL_DISPEL = 64136,
	SPELL_PRIEST_BODY_AND_SOUL_SPEED = 65081,
	SPELL_PRIEST_CURE_DISEASE = 528,
	SPELL_PRIEST_GLYPH_OF_LIGHTWELL = 55673,
	SPELL_PRIEST_GLYPH_OF_PRAYER_OF_HEALING_HEAL = 56161,
	SPELL_PRIEST_GLYPH_OF_SHADOW = 107906,
	SPELL_PRIEST_GUARDIAN_SPIRIT_HEAL = 48153,
	SPELL_PRIEST_LEAP_OF_FAITH = 73325,
	SPELL_PRIEST_LEAP_OF_FAITH_EFFECT = 92832,
	SPELL_PRIEST_LEAP_OF_FAITH_EFFECT_TRIGGER = 92833,
	SPELL_PRIEST_LEAP_OF_FAITH_TRIGGERED = 92572,
	SPELL_PRIEST_MANA_LEECH_PROC = 34650,
	SPELL_PRIEST_PENANCE_R1 = 47540,
	SPELL_PRIEST_PENANCE_R1_DAMAGE = 47758,
	SPELL_PRIEST_PENANCE_R1_HEAL = 47757,
	SPELL_PRIEST_REFLECTIVE_SHIELD_R1 = 33201,
	SPELL_PRIEST_REFLECTIVE_SHIELD_TRIGGERED = 33619,
	SPELL_PRIEST_SHADOWFORM_VISUAL_WITHOUT_GLYPH = 107903,
	SPELL_PRIEST_SHADOWFORM_VISUAL_WITH_GLYPH = 107904,
	SPELL_PRIEST_SHADOW_WORD_DEATH = 32409,
	SPELL_PRIEST_T9_HEALING_2P = 67201,
	SPELL_PRIEST_VAMPIRIC_TOUCH_DISPEL = 64085,
	SPELL_PRIEST_HOLY_WORD_SANCTUARY_TRIGGERED = 88686,
	PRIEST_SPELL_SPIRIT_OF_REDEMPTION_IMMUNITY = 62371,
	PRIEST_SPELL_SPIRIT_OF_REDEMPTION_FORM = 27795,
	PRIEST_SPELL_SPIRIT_OF_REDEMPTION_TALENT = 20711,
	PRIEST_SPELL_SPIRIT_OF_REDEMPTION_ROOT = 27792,
	PRIEST_SPELL_SPIRIT_OF_REDEMPTION_SHAPESHIFT = 27827,
	PRIEST_SPELL_REVELATIONS = 88627,
	PRIEST_SPELL_RENEW = 139,
	PRIEST_SPELL_SANCTUARY_4YD_DUMMY = 88667,
	PRIEST_SPELL_SANCTUARY_4YD_HEAL = 88668,
	PRIEST_SPELL_SANCTUARY_8YD_DUMMY = 88685,
	PRIEST_SPELL_SANCTUARY_8YD_HEAL = 88686,
};

enum PriestSpellIcons
{
	PRIEST_ICON_ID_BORROWED_TIME = 2899,
	PRIEST_ICON_ID_PAIN_AND_SUFFERING = 2874,
};

class PowerCheck
{
public:
	explicit PowerCheck(Powers const power) : _power(power) { }

	bool operator()(WorldObject* obj) const
	{
		if (Unit* target = obj->ToUnit())
			return target->getPowerType() != _power;

		return true;
	}

private:
	Powers const _power;
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

// -34861 - Circle of Healing
class spell_pri_circle_of_healing : public SpellScriptLoader
{
public:
	spell_pri_circle_of_healing() : SpellScriptLoader("spell_pri_circle_of_healing") { }

	class spell_pri_circle_of_healing_SpellScript : public SpellScript
	{
		PrepareSpellScript(spell_pri_circle_of_healing_SpellScript);

		bool Validate(SpellInfo const* /*spellInfo*/)
		{
			if (!sSpellMgr->GetSpellInfo(SPELL_PRIEST_GLYPH_OF_CIRCLE_OF_HEALING))
				return false;
			return true;
		}

		void FilterTargets(std::list<WorldObject*>& targets)
		{
			targets.remove_if(RaidCheck(GetCaster()));

			uint32 const maxTargets = GetCaster()->HasAura(SPELL_PRIEST_GLYPH_OF_CIRCLE_OF_HEALING) ? 6 : 5; // Glyph of Circle of Healing

			if (targets.size() > maxTargets)
			{
				targets.sort(Trinity::HealthPctOrderPred());
				targets.resize(maxTargets);
			}
		}

		void Register()
		{
			OnObjectAreaTargetSelect += SpellObjectAreaTargetSelectFn(spell_pri_circle_of_healing_SpellScript::FilterTargets, EFFECT_0, TARGET_UNIT_DEST_AREA_ALLY);
		}
	};

	SpellScript* GetSpellScript() const
	{
		return new spell_pri_circle_of_healing_SpellScript();
	}
};

// -47509 - Divine Aegis
class spell_pri_divine_aegis : public SpellScriptLoader
{
public:
	spell_pri_divine_aegis() : SpellScriptLoader("spell_pri_divine_aegis") { }

	class spell_pri_divine_aegis_AuraScript : public AuraScript
	{
		PrepareAuraScript(spell_pri_divine_aegis_AuraScript);

		bool Validate(SpellInfo const* /*spellInfo*/)
		{
			if (!sSpellMgr->GetSpellInfo(SPELL_PRIEST_DIVINE_AEGIS))
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

			int32 absorb = CalculatePct(int32(eventInfo.GetHealInfo()->GetHeal()), aurEff->GetAmount());

			// Multiple effects stack, so let's try to find this aura.
			if (AuraEffect const* aegis = eventInfo.GetProcTarget()->GetAuraEffect(SPELL_PRIEST_DIVINE_AEGIS, EFFECT_0))
				absorb += aegis->GetAmount();

			absorb = std::min(absorb, eventInfo.GetProcTarget()->getLevel() * 125);

			GetTarget()->CastCustomSpell(SPELL_PRIEST_DIVINE_AEGIS, SPELLVALUE_BASE_POINT0, absorb, eventInfo.GetProcTarget(), true, NULL, aurEff);
		}

		void Register()
		{
			DoCheckProc += AuraCheckProcFn(spell_pri_divine_aegis_AuraScript::CheckProc);
			OnEffectProc += AuraEffectProcFn(spell_pri_divine_aegis_AuraScript::HandleProc, EFFECT_0, SPELL_AURA_DUMMY);
		}
	};

	AuraScript* GetAuraScript() const
	{
		return new spell_pri_divine_aegis_AuraScript();
	}
};

// 64844 - Divine Hymn
class spell_pri_divine_hymn : public SpellScriptLoader
{
public:
	spell_pri_divine_hymn() : SpellScriptLoader("spell_pri_divine_hymn") { }

	class spell_pri_divine_hymn_SpellScript : public SpellScript
	{
		PrepareSpellScript(spell_pri_divine_hymn_SpellScript);

		void FilterTargets(std::list<WorldObject*>& targets)
		{
			targets.remove_if(RaidCheck(GetCaster()));

			uint32 const maxTargets = 3;

			if (targets.size() > maxTargets)
			{
				targets.sort(Trinity::HealthPctOrderPred());
				targets.resize(maxTargets);
			}
		}

		void Register()
		{
			OnObjectAreaTargetSelect += SpellObjectAreaTargetSelectFn(spell_pri_divine_hymn_SpellScript::FilterTargets, EFFECT_ALL, TARGET_UNIT_SRC_AREA_ALLY);
		}
	};

	SpellScript* GetSpellScript() const
	{
		return new spell_pri_divine_hymn_SpellScript();
	}
};

class spell_pri_body_and_soul : public SpellScriptLoader
{
public:
	spell_pri_body_and_soul() : SpellScriptLoader("spell_pri_body_and_soul") { }

	class spell_pri_body_and_soul_AuraScript : public AuraScript
	{
		PrepareAuraScript(spell_pri_body_and_soul_AuraScript);

		bool Validate(SpellInfo const* /*spellInfo*/)
		{
			if (!sSpellMgr->GetSpellInfo(SPELL_PRIEST_CURE_DISEASE) ||
				!sSpellMgr->GetSpellInfo(SPELL_PRIEST_BODY_AND_SOUL_DISPEL))
				return false;
			return true;
		}

		void HandleEffectSpeedProc(AuraEffect const* aurEff, ProcEventInfo& eventInfo)
		{
			PreventDefaultAction();
			// Proc only with Power Word: Shield or Leap of Faith
			if (!(eventInfo.GetDamageInfo()->GetSpellInfo()->SpellFamilyFlags[0] & 0x1 || eventInfo.GetDamageInfo()->GetSpellInfo()->SpellFamilyFlags[2] & 0x80000))
				return;

			GetTarget()->CastCustomSpell(SPELL_PRIEST_BODY_AND_SOUL_SPEED, SPELLVALUE_BASE_POINT0, aurEff->GetAmount(), eventInfo.GetProcTarget(), true, NULL, aurEff);
		}

		void HandleEffectDispelProc(AuraEffect const* aurEff, ProcEventInfo& eventInfo)
		{
			PreventDefaultAction();
			// Proc only with Cure Disease
			if (eventInfo.GetDamageInfo()->GetSpellInfo()->Id != SPELL_PRIEST_CURE_DISEASE || eventInfo.GetProcTarget() != GetTarget())
				return;

			if (roll_chance_i(aurEff->GetAmount()))
				GetTarget()->CastSpell(eventInfo.GetProcTarget(), SPELL_PRIEST_BODY_AND_SOUL_DISPEL, true, NULL, aurEff);
		}

		void Register()
		{
			OnEffectProc += AuraEffectProcFn(spell_pri_body_and_soul_AuraScript::HandleEffectSpeedProc, EFFECT_0, SPELL_AURA_DUMMY);
			OnEffectProc += AuraEffectProcFn(spell_pri_body_and_soul_AuraScript::HandleEffectDispelProc, EFFECT_1, SPELL_AURA_DUMMY);
		}
	};

	AuraScript* GetAuraScript() const
	{
		return new spell_pri_body_and_soul_AuraScript();
	}
};

// 55680 - Glyph of Prayer of Healing
class spell_pri_glyph_of_prayer_of_healing : public SpellScriptLoader
{
public:
	spell_pri_glyph_of_prayer_of_healing() : SpellScriptLoader("spell_pri_glyph_of_prayer_of_healing") { }

	class spell_pri_glyph_of_prayer_of_healing_AuraScript : public AuraScript
	{
		PrepareAuraScript(spell_pri_glyph_of_prayer_of_healing_AuraScript);

		bool Validate(SpellInfo const* /*spellInfo*/)
		{
			if (!sSpellMgr->GetSpellInfo(SPELL_PRIEST_GLYPH_OF_PRAYER_OF_HEALING_HEAL))
				return false;
			return true;
		}

		void HandleProc(AuraEffect const* aurEff, ProcEventInfo& eventInfo)
		{
			PreventDefaultAction();

			SpellInfo const* triggeredSpellInfo = sSpellMgr->GetSpellInfo(SPELL_PRIEST_GLYPH_OF_PRAYER_OF_HEALING_HEAL);
			int32 heal = int32(CalculatePct(int32(eventInfo.GetHealInfo()->GetHeal()), aurEff->GetAmount()) / triggeredSpellInfo->GetMaxTicks());
			GetTarget()->CastCustomSpell(SPELL_PRIEST_GLYPH_OF_PRAYER_OF_HEALING_HEAL, SPELLVALUE_BASE_POINT0, heal, eventInfo.GetProcTarget(), true, NULL, aurEff);
		}

		void Register()
		{
			OnEffectProc += AuraEffectProcFn(spell_pri_glyph_of_prayer_of_healing_AuraScript::HandleProc, EFFECT_0, SPELL_AURA_DUMMY);
		}
	};

	AuraScript* GetAuraScript() const
	{
		return new spell_pri_glyph_of_prayer_of_healing_AuraScript();
	}
};

// 47788 - Guardian Spirit
class spell_pri_guardian_spirit : public SpellScriptLoader
{
public:
	spell_pri_guardian_spirit() : SpellScriptLoader("spell_pri_guardian_spirit") { }

	class spell_pri_guardian_spirit_AuraScript : public AuraScript
	{
		PrepareAuraScript(spell_pri_guardian_spirit_AuraScript);

		uint32 healPct;

		bool Validate(SpellInfo const* /*spellInfo*/)
		{
			if (!sSpellMgr->GetSpellInfo(SPELL_PRIEST_GUARDIAN_SPIRIT_HEAL))
				return false;
			return true;
		}

		bool Load()
		{
			healPct = GetSpellInfo()->Effects[EFFECT_1].CalcValue();
			return true;
		}

		void CalculateAmount(AuraEffect const* /*aurEff*/, int32 & amount, bool & /*canBeRecalculated*/)
		{
			// Set absorbtion amount to unlimited
			amount = -1;
		}

		void Absorb(AuraEffect* /*aurEff*/, DamageInfo & dmgInfo, uint32 & absorbAmount)
		{
			Unit* target = GetTarget();
			if (dmgInfo.GetDamage() < target->GetHealth())
				return;

			int32 healAmount = int32(target->CountPctFromMaxHealth(healPct));
			// remove the aura now, we don't want 40% healing bonus
			Remove(AURA_REMOVE_BY_ENEMY_SPELL);
			target->CastCustomSpell(target, SPELL_PRIEST_GUARDIAN_SPIRIT_HEAL, &healAmount, NULL, NULL, true);
			absorbAmount = dmgInfo.GetDamage();
		}

		void Register()
		{
			DoEffectCalcAmount += AuraEffectCalcAmountFn(spell_pri_guardian_spirit_AuraScript::CalculateAmount, EFFECT_1, SPELL_AURA_SCHOOL_ABSORB);
			OnEffectAbsorb += AuraEffectAbsorbFn(spell_pri_guardian_spirit_AuraScript::Absorb, EFFECT_1);
		}
	};

	AuraScript* GetAuraScript() const
	{
		return new spell_pri_guardian_spirit_AuraScript();
	}
};

// Cure Disease
class spell_pri_cure_disease : public SpellScriptLoader
{
public:
	spell_pri_cure_disease() : SpellScriptLoader("spell_pri_cure_disease") { }

	class spell_pri_cure_disease_SpellScript : public SpellScript
	{
		PrepareSpellScript(spell_pri_cure_disease_SpellScript);

		void HandleOnHit()
		{
			Unit* caster = GetCaster();
			if (GetHitUnit() != caster)
				return;

			// Body and soul
			if (AuraEffect* bodyAndSoul = caster->GetDummyAuraEffect(SPELLFAMILY_PRIEST, 2218, EFFECT_1))
				if (roll_chance_i(bodyAndSoul->GetAmount()))
					caster->CastSpell(caster, 64136, true);
		}

		void Register()
		{
			OnHit += SpellHitFn(spell_pri_cure_disease_SpellScript::HandleOnHit);
		}
	};

	SpellScript* GetSpellScript() const
	{
		return new spell_pri_cure_disease_SpellScript();
	}
};

// 92833 - Leap of Faith
class spell_pri_leap_of_faith_effect_trigger : public SpellScriptLoader
{
public:
	spell_pri_leap_of_faith_effect_trigger() : SpellScriptLoader("spell_pri_leap_of_faith_effect_trigger") { }

	class spell_pri_leap_of_faith_effect_trigger_SpellScript : public SpellScript
	{
		PrepareSpellScript(spell_pri_leap_of_faith_effect_trigger_SpellScript);

		bool Validate(SpellInfo const* /*spellInfo*/)
		{
			if (!sSpellMgr->GetSpellInfo(SPELL_PRIEST_LEAP_OF_FAITH_EFFECT))
				return false;
			return true;
		}

		void HandleEffectDummy(SpellEffIndex /*effIndex*/)
		{
			Unit* caster = GetCaster();
			Position destPos;
			GetHitDest()->GetPosition(&destPos);

			// Body and soul
			if (AuraEffect* bodyAndSoul = caster->GetDummyAuraEffect(SPELLFAMILY_PRIEST, 2218, EFFECT_0))
			{
				switch (bodyAndSoul->GetAmount())
				{
				case 30:
					caster->CastSpell(GetHitUnit(), 64128, true);
					break;
				case 60:
					caster->CastSpell(GetHitUnit(), 65081, true);
					break;
				}
			}

			SpellCastTargets targets;
			targets.SetDst(destPos);
			targets.SetUnitTarget(caster);
			GetHitUnit()->CastSpell(targets, sSpellMgr->GetSpellInfo(GetEffectValue()), NULL);
		}

		void Register()
		{
			OnEffectHitTarget += SpellEffectFn(spell_pri_leap_of_faith_effect_trigger_SpellScript::HandleEffectDummy, EFFECT_0, SPELL_EFFECT_DUMMY);
		}
	};

	SpellScript* GetSpellScript() const
	{
		return new spell_pri_leap_of_faith_effect_trigger_SpellScript();
	}
};

// -7001 - Lightwell Renew
class spell_pri_lightwell_renew : public SpellScriptLoader
{
public:
	spell_pri_lightwell_renew() : SpellScriptLoader("spell_pri_lightwell_renew") { }

	class spell_pri_lightwell_renew_AuraScript : public AuraScript
	{
		PrepareAuraScript(spell_pri_lightwell_renew_AuraScript);

		void CalculateAmount(AuraEffect const* /*aurEff*/, int32& amount, bool& /*canBeRecalculated*/)
		{
			if (Unit* caster = GetCaster())
			{
				// Bonus from Glyph of Lightwell
				if (AuraEffect* modHealing = caster->GetAuraEffect(SPELL_PRIEST_GLYPH_OF_LIGHTWELL, EFFECT_0))
					AddPct(amount, modHealing->GetAmount());
			}
		}

		void Register()
		{
			DoEffectCalcAmount += AuraEffectCalcAmountFn(spell_pri_lightwell_renew_AuraScript::CalculateAmount, EFFECT_0, SPELL_AURA_PERIODIC_HEAL);
		}
	};

	AuraScript* GetAuraScript() const
	{
		return new spell_pri_lightwell_renew_AuraScript();
	}
};

// 8129 - Mana Burn
class spell_pri_mana_burn : public SpellScriptLoader
{
public:
	spell_pri_mana_burn() : SpellScriptLoader("spell_pri_mana_burn") { }

	class spell_pri_mana_burn_SpellScript : public SpellScript
	{
		PrepareSpellScript(spell_pri_mana_burn_SpellScript);

		void HandleAfterHit()
		{
			if (Unit* unitTarget = GetHitUnit())
				unitTarget->RemoveAurasWithMechanic((1 << MECHANIC_FEAR) | (1 << MECHANIC_POLYMORPH));
		}

		void Register()
		{
			AfterHit += SpellHitFn(spell_pri_mana_burn_SpellScript::HandleAfterHit);
		}
	};

	SpellScript* GetSpellScript() const
	{
		return new spell_pri_mana_burn_SpellScript;
	}
};

// 28305 - Mana Leech (Passive) (Priest Pet Aura)
class spell_pri_mana_leech : public SpellScriptLoader
{
public:
	spell_pri_mana_leech() : SpellScriptLoader("spell_pri_mana_leech") { }

	class spell_pri_mana_leech_AuraScript : public AuraScript
	{
		PrepareAuraScript(spell_pri_mana_leech_AuraScript);

		bool Validate(SpellInfo const* /*spellInfo*/)
		{
			if (!sSpellMgr->GetSpellInfo(SPELL_PRIEST_MANA_LEECH_PROC))
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

		void HandleProc(AuraEffect const* aurEff, ProcEventInfo& /*eventInfo*/)
		{
			PreventDefaultAction();
			GetTarget()->CastSpell(_procTarget, SPELL_PRIEST_MANA_LEECH_PROC, true, NULL, aurEff);
		}

		void Register()
		{
			DoCheckProc += AuraCheckProcFn(spell_pri_mana_leech_AuraScript::CheckProc);
			OnEffectProc += AuraEffectProcFn(spell_pri_mana_leech_AuraScript::HandleProc, EFFECT_0, SPELL_AURA_DUMMY);
		}

	private:
		Unit* _procTarget;
	};

	AuraScript* GetAuraScript() const
	{
		return new spell_pri_mana_leech_AuraScript();
	}
};

// 48045 Mind Sear
class spell_pri_mind_sear_spell : public SpellScriptLoader
{
public:
	spell_pri_mind_sear_spell() : SpellScriptLoader("spell_pri_mind_sear_spell") { }

	class spell_pri_mind_sear_spell_SpellScript : public SpellScript
	{
		PrepareSpellScript(spell_pri_mind_sear_spell_SpellScript);

		SpellCastResult CheckCast()
		{
			if (GetCaster() == GetExplTargetUnit())
				return SPELL_FAILED_BAD_TARGETS;

			return SPELL_CAST_OK;
		}

		void Register()
		{
			OnCheckCast += SpellCheckCastFn(spell_pri_mind_sear_spell_SpellScript::CheckCast);
		}
	};

	SpellScript* GetSpellScript() const
	{
		return new spell_pri_mind_sear_spell_SpellScript();
	}
};

// 49821 - Mind Sear
class spell_pri_mind_sear : public SpellScriptLoader
{
public:
	spell_pri_mind_sear() : SpellScriptLoader("spell_pri_mind_sear") { }

	class spell_pri_mind_sear_SpellScript : public SpellScript
	{
		PrepareSpellScript(spell_pri_mind_sear_SpellScript);

		void FilterTargets(std::list<WorldObject*>& unitList)
		{
			unitList.remove_if(Trinity::ObjectGUIDCheck(GetCaster()->GetUInt64Value(UNIT_FIELD_CHANNEL_OBJECT)));
		}

		void Register()
		{
			OnObjectAreaTargetSelect += SpellObjectAreaTargetSelectFn(spell_pri_mind_sear_SpellScript::FilterTargets, EFFECT_0, TARGET_UNIT_DEST_AREA_ENEMY);
		}
	};

	SpellScript* GetSpellScript() const
	{
		return new spell_pri_mind_sear_SpellScript();
	}
};

// 47948 - Pain and Suffering (Proc)
class spell_pri_pain_and_suffering_proc : public SpellScriptLoader
{
public:
	spell_pri_pain_and_suffering_proc() : SpellScriptLoader("spell_pri_pain_and_suffering_proc") { }

	class spell_pri_pain_and_suffering_proc_SpellScript : public SpellScript
	{
		PrepareSpellScript(spell_pri_pain_and_suffering_proc_SpellScript);

		void HandleEffectScriptEffect(SpellEffIndex /*effIndex*/)
		{
			// Refresh Shadow Word: Pain on target
			if (Unit* unitTarget = GetHitUnit())
				if (AuraEffect* aur = unitTarget->GetAuraEffect(SPELL_AURA_PERIODIC_DAMAGE, SPELLFAMILY_PRIEST, 0x8000, 0, 0, GetCaster()->GetGUID()))
					aur->GetBase()->RefreshDuration();
		}

		void Register()
		{
			OnEffectHitTarget += SpellEffectFn(spell_pri_pain_and_suffering_proc_SpellScript::HandleEffectScriptEffect, EFFECT_0, SPELL_EFFECT_SCRIPT_EFFECT);
		}
	};

	SpellScript* GetSpellScript() const
	{
		return new spell_pri_pain_and_suffering_proc_SpellScript;
	}
};

// 4.3.4 updated
class spell_pri_mind_blast : public SpellScriptLoader
{
public:
	spell_pri_mind_blast() : SpellScriptLoader("spell_pri_mind_blast") { }

	class spell_pri_mind_blast_SpellScript : public SpellScript
	{
		PrepareSpellScript(spell_pri_mind_blast_SpellScript);

		void HandleOnHit()
		{
			Unit* caster = GetCaster();
			if (Unit* unitTarget = GetHitUnit())
			{
				if (Aura* aur = caster->GetAuraOfRankedSpell(15273, caster->GetGUID()))
				{
					int32 chance = aur->GetSpellInfo()->Effects[EFFECT_1].BasePoints;
					if (caster->GetShapeshiftForm() == FORM_SHADOW && roll_chance_i(chance))
						caster->CastSpell(unitTarget, 48301, true);
				}
				// Remove Mind spike debuff
				unitTarget->RemoveAurasDueToSpell(87178, caster->GetGUID());
			}

			// Shadow orbs
			if (AuraEffect* aurEff = caster->GetDummyAuraEffect(SPELLFAMILY_GENERIC, 4941, EFFECT_0))
			{
				uint8 stacks = aurEff->GetBase()->GetStackAmount();
				int32 amount = aurEff->GetAmount() / (2.0f * stacks);
				if (AuraEffect* mastery = caster->GetAuraEffect(77486, EFFECT_0))
					amount += mastery->GetAmount();
				caster->CastCustomSpell(caster, 95799, &amount, &amount, NULL, true);
				aurEff->GetBase()->Remove();
			}
		}

		void Register()
		{
			OnHit += SpellHitFn(spell_pri_mind_blast_SpellScript::HandleOnHit);
		}
	};

	SpellScript* GetSpellScript() const
	{
		return new spell_pri_mind_blast_SpellScript;
	}
};

// 4.3.4 updated
class spell_pri_mind_spike : public SpellScriptLoader
{
public:
	spell_pri_mind_spike() : SpellScriptLoader("spell_pri_mind_spike") { }

	class spell_pri_mind_spike_SpellScript : public SpellScript
	{
		PrepareSpellScript(spell_pri_mind_spike_SpellScript);

		void HandleDummy(SpellEffIndex /*effIndex*/)
		{
			Unit* caster = GetCaster();
			if (Unit* unitTarget = GetHitUnit())
			{
				unitTarget->RemoveAurasByType(SPELL_AURA_PERIODIC_DAMAGE, caster->GetGUID(), NULL, true, false, SPELLFAMILY_PRIEST, SPELL_SCHOOL_MASK_SHADOW);
				unitTarget->RemoveAurasByType(SPELL_AURA_PERIODIC_LEECH, caster->GetGUID(), NULL, true, false, SPELLFAMILY_PRIEST, SPELL_SCHOOL_MASK_SHADOW);
			}
		}

		void HandleOnHit()
		{
			Unit* caster = GetCaster();
			// Shadow orbs
			if (AuraEffect* aurEff = caster->GetDummyAuraEffect(SPELLFAMILY_GENERIC, 4941, EFFECT_0))
			{
				uint8 stacks = aurEff->GetBase()->GetStackAmount();
				int32 amount = aurEff->GetAmount() / (2.0f * stacks);
				if (AuraEffect* mastery = caster->GetAuraEffect(77486, EFFECT_0))
					amount += mastery->GetAmount();
				caster->CastCustomSpell(caster, 95799, &amount, &amount, NULL, true);
				aurEff->GetBase()->Remove();
			}
		}

		void Register()
		{
			OnEffectHitTarget += SpellEffectFn(spell_pri_mind_spike_SpellScript::HandleDummy, EFFECT_0, SPELL_EFFECT_SCHOOL_DAMAGE);
			OnHit += SpellHitFn(spell_pri_mind_spike_SpellScript::HandleOnHit);
		}
	};

	SpellScript* GetSpellScript() const
	{
		return new spell_pri_mind_spike_SpellScript;
	}
};

// 4.3.4 updated
class spell_pri_fade : public SpellScriptLoader
{
public:
	spell_pri_fade() : SpellScriptLoader("spell_pri_fade") { }

	class spell_pri_fade_SpellScript : public SpellScript
	{
		PrepareSpellScript(spell_pri_fade_SpellScript);

		void HandleOnHit()
		{
			Unit* caster = GetCaster();
			if (AuraEffect* aur = caster->GetDummyAuraEffect(SPELLFAMILY_PRIEST, 2901, EFFECT_0))
				if (roll_chance_i(aur->GetBaseAmount()))
					caster->RemoveMovementImpairingAuras();
		}

		void Register()
		{
			OnHit += SpellHitFn(spell_pri_fade_SpellScript::HandleOnHit);
		}
	};

	SpellScript* GetSpellScript() const
	{
		return new spell_pri_fade_SpellScript;
	}
};


// 47540 - Penance
class spell_pri_penance : public SpellScriptLoader
{
public:
	spell_pri_penance() : SpellScriptLoader("spell_pri_penance") { }

	class spell_pri_penance_SpellScript : public SpellScript
	{
		PrepareSpellScript(spell_pri_penance_SpellScript);

		bool Load()
		{
			return GetCaster()->GetTypeId() == TYPEID_PLAYER;
		}

		bool Validate(SpellInfo const* spellInfo)
		{
			if (!sSpellMgr->GetSpellInfo(SPELL_PRIEST_PENANCE_R1))
				return false;
			// can't use other spell than this penance due to spell_ranks dependency
			if (sSpellMgr->GetFirstSpellInChain(SPELL_PRIEST_PENANCE_R1) != sSpellMgr->GetFirstSpellInChain(spellInfo->Id))
				return false;

			uint8 rank = sSpellMgr->GetSpellRank(spellInfo->Id);
			if (!sSpellMgr->GetSpellWithRank(SPELL_PRIEST_PENANCE_R1_DAMAGE, rank, true))
				return false;
			if (!sSpellMgr->GetSpellWithRank(SPELL_PRIEST_PENANCE_R1_HEAL, rank, true))
				return false;

			return true;
		}

		void HandleDummy(SpellEffIndex /*effIndex*/)
		{
			Unit* caster = GetCaster();
			if (Unit* unitTarget = GetHitUnit())
			{
				if (!unitTarget->isAlive())
					return;

				uint8 rank = sSpellMgr->GetSpellRank(GetSpellInfo()->Id);

				if (caster->IsFriendlyTo(unitTarget))
					caster->CastSpell(unitTarget, sSpellMgr->GetSpellWithRank(SPELL_PRIEST_PENANCE_R1_HEAL, rank), false, 0);
				else
					caster->CastSpell(unitTarget, sSpellMgr->GetSpellWithRank(SPELL_PRIEST_PENANCE_R1_DAMAGE, rank), false, 0);
			}
		}

		SpellCastResult CheckCast()
		{
			Player* caster = GetCaster()->ToPlayer();
			if (Unit* target = GetExplTargetUnit())
				if (!caster->IsFriendlyTo(target) && !caster->IsValidAttackTarget(target))
					return SPELL_FAILED_BAD_TARGETS;
			return SPELL_CAST_OK;
		}

		void Register()
		{
			OnEffectHitTarget += SpellEffectFn(spell_pri_penance_SpellScript::HandleDummy, EFFECT_0, SPELL_EFFECT_DUMMY);
			OnCheckCast += SpellCheckCastFn(spell_pri_penance_SpellScript::CheckCast);
		}
	};

	SpellScript* GetSpellScript() const
	{
		return new spell_pri_penance_SpellScript;
	}
};

// -17 - Power Word: Shield
class spell_pri_power_word_shield : public SpellScriptLoader
{
public:
	spell_pri_power_word_shield() : SpellScriptLoader("spell_pri_power_word_shield") { }

	class spell_pri_power_word_shield_AuraScript : public AuraScript
	{
		PrepareAuraScript(spell_pri_power_word_shield_AuraScript);

		bool Validate(SpellInfo const* /*spellInfo*/)
		{
			if (!sSpellMgr->GetSpellInfo(SPELL_PRIEST_REFLECTIVE_SHIELD_TRIGGERED))
				return false;
			if (!sSpellMgr->GetSpellInfo(SPELL_PRIEST_REFLECTIVE_SHIELD_R1))
				return false;
			return true;
		}

		void CalculateAmount(AuraEffect const* aurEff, int32& amount, bool& canBeRecalculated)
		{
			canBeRecalculated = false;
			if (Unit* caster = GetCaster())
			{
				// +80.68% from sp bonus
				float bonus = 0.87f;
				bonus *= caster->SpellBaseHealingBonusDone(GetSpellInfo()->GetSchoolMask());

				// Improved PW: Shield: its weird having a SPELLMOD_ALL_EFFECTS here but its blizzards doing :)
				// Improved PW: Shield is only applied at the spell healing bonus because it was already applied to the base value in CalculateSpellDamage
				bonus = caster->ApplyEffectModifiers(GetSpellInfo(), aurEff->GetEffIndex(), bonus);
				bonus *= caster->CalculateLevelPenalty(GetSpellInfo());

				amount += int32(bonus);

				// Improved Power Word: Shield
				if (AuraEffect const* impShield = caster->GetDummyAuraEffect(SPELLFAMILY_PRIEST, 566, EFFECT_0))
					AddPct(amount, impShield->GetAmount());

				// Twin Disciplines
				if (AuraEffect const* twinDisciplines = caster->GetAuraEffect(SPELL_AURA_ADD_PCT_MODIFIER, SPELLFAMILY_PRIEST, 0x400000, 0, 0, GetCasterGUID()))
					AddPct(amount, twinDisciplines->GetAmount());

				// Shield discipline
				if (AuraEffect const* mastery = caster->GetDummyAuraEffect(SPELLFAMILY_HUNTER, 566, EFFECT_0))
					AddPct(amount, mastery->GetAmount());

				// Focused Power
				amount *= caster->GetTotalAuraMultiplier(SPELL_AURA_MOD_HEALING_DONE_PERCENT);
			}
		}

		void ReflectDamage(AuraEffect* aurEff, DamageInfo& dmgInfo, uint32& absorbAmount)
		{
			Unit* target = GetTarget();
			if (dmgInfo.GetAttacker() == target)
				return;

			if (Unit* caster = GetCaster())
				if (AuraEffect* talentAurEff = caster->GetDummyAuraEffect(SPELLFAMILY_PRIEST, 4880, EFFECT_0))
				{
				int32 bp = CalculatePct(absorbAmount, talentAurEff->GetAmount());
				target->CastCustomSpell(dmgInfo.GetAttacker(), SPELL_PRIEST_REFLECTIVE_SHIELD_TRIGGERED, &bp, NULL, NULL, true, NULL, aurEff);
				}
		}

		void HandleApplyEffect(AuraEffect const* aurEff, AuraEffectHandleModes /*mode*/)
		{
			if (Unit* caster = GetCaster())
			{
				// Body and soul
				if (AuraEffect* bodyAndSoul = caster->GetDummyAuraEffect(SPELLFAMILY_PRIEST, 2218, EFFECT_0))
				{
					switch (bodyAndSoul->GetAmount())
					{
					case 30:
						caster->CastSpell(GetTarget(), 64128, true);
						break;
					case 60:
						caster->CastSpell(GetTarget(), 65081, true);
						break;
					}
				}

				if (caster == GetTarget())
					// PvP 4 piece - Holy walk
					if (AuraEffect const* aurEff = caster->GetDummyAuraEffect(SPELLFAMILY_PRIEST, 5338, EFFECT_0))
						caster->CastSpell(caster, 96219, true);
			}
		}

		void Register()
		{
			OnEffectApply += AuraEffectApplyFn(spell_pri_power_word_shield_AuraScript::HandleApplyEffect, EFFECT_0, SPELL_AURA_SCHOOL_ABSORB, AURA_EFFECT_HANDLE_REAL_OR_REAPPLY_MASK);
			DoEffectCalcAmount += AuraEffectCalcAmountFn(spell_pri_power_word_shield_AuraScript::CalculateAmount, EFFECT_0, SPELL_AURA_SCHOOL_ABSORB);
			AfterEffectAbsorb += AuraEffectAbsorbFn(spell_pri_power_word_shield_AuraScript::ReflectDamage, EFFECT_0);
		}
	};

	AuraScript* GetAuraScript() const
	{
		return new spell_pri_power_word_shield_AuraScript();
	}
};

// 33110 - Prayer of Mending Heal
class spell_pri_prayer_of_mending_heal : public SpellScriptLoader
{
public:
	spell_pri_prayer_of_mending_heal() : SpellScriptLoader("spell_pri_prayer_of_mending_heal") { }

	class spell_pri_prayer_of_mending_heal_SpellScript : public SpellScript
	{
		PrepareSpellScript(spell_pri_prayer_of_mending_heal_SpellScript);

		void HandleHeal(SpellEffIndex /*effIndex*/)
		{
			if (Unit* caster = GetOriginalCaster())
			{
				if (Aura* prayer = GetHitUnit()->GetAura(41635, caster->GetGUID()))
					if (prayer->GetCharges() == prayer->CalcMaxCharges())
					{
					// Glyph of prayer of mending
					if (AuraEffect* glyph = caster->GetDummyAuraEffect(SPELLFAMILY_PRIEST, 2219, EFFECT_0))
					{
						int32 heal = GetHitHeal();
						AddPct(heal, glyph->GetAmount());
						SetHitHeal(heal);
					}
					}

				if (AuraEffect* aurEff = caster->GetAuraEffect(SPELL_PRIEST_T9_HEALING_2P, EFFECT_0))
				{
					int32 heal = GetHitHeal();
					AddPct(heal, aurEff->GetAmount());
					SetHitHeal(heal);
				}
			}
		}

		void Register()
		{
			OnEffectHitTarget += SpellEffectFn(spell_pri_prayer_of_mending_heal_SpellScript::HandleHeal, EFFECT_0, SPELL_EFFECT_HEAL);
		}
	};

	SpellScript* GetSpellScript() const
	{
		return new spell_pri_prayer_of_mending_heal_SpellScript();
	}
};

// 139 - Renew
class spell_pri_renew : public SpellScriptLoader
{
public:
	spell_pri_renew() : SpellScriptLoader("spell_pri_renew") { }

	class spell_pri_renew_AuraScript : public AuraScript
	{
		PrepareAuraScript(spell_pri_renew_AuraScript);

		bool Load()
		{
			return GetCaster() && GetCaster()->GetTypeId() == TYPEID_PLAYER;
		}

		void HandleApplyEffect(AuraEffect const* aurEff, AuraEffectHandleModes /*mode*/)
		{
			if (Unit* caster = GetCaster())
			{
				// Divine touch
				if (AuraEffect const* aurEff = caster->GetDummyAuraEffect(SPELLFAMILY_PRIEST, 3021, EFFECT_0))
				{
					uint32 heal = caster->SpellHealingBonusDone(GetTarget(), GetSpellInfo(), GetEffect(EFFECT_0)->GetAmount(), DOT);
					heal = GetTarget()->SpellHealingBonusTaken(GetSpellInfo(), heal, DOT, 1, caster->GetGUID());

					int32 basepoints0 = aurEff->GetAmount() * GetEffect(EFFECT_0)->GetTotalTicks() * int32(heal) / 100;
					caster->CastCustomSpell(GetTarget(), 63544, &basepoints0, NULL, NULL, true, NULL, aurEff);
				}
			}
		}

		void Register()
		{
			OnEffectApply += AuraEffectApplyFn(spell_pri_renew_AuraScript::HandleApplyEffect, EFFECT_0, SPELL_AURA_PERIODIC_HEAL, AURA_EFFECT_HANDLE_REAL_OR_REAPPLY_MASK);
		}
	};

	AuraScript* GetAuraScript() const
	{
		return new spell_pri_renew_AuraScript();
	}
};

// Inner Fire
class spell_pri_inner_fire : public SpellScriptLoader
{
public:
	spell_pri_inner_fire() : SpellScriptLoader("spell_pri_inner_fire") { }

	class spell_pri_inner_fire_AuraScript : public AuraScript
	{
		PrepareAuraScript(spell_pri_inner_fire_AuraScript);

		void HandleEffectApply(AuraEffect const* /*aurEff*/, AuraEffectHandleModes /*mode*/)
		{
			Unit* target = GetTarget();
			if (AuraEffect* aurEff = target->GetDummyAuraEffect(SPELLFAMILY_PRIEST, 51, EFFECT_0))
				target->CastCustomSpell(91724, SPELLVALUE_BASE_POINT0, aurEff->GetAmount(), target, true);
		}

		void HandleEffectRemove(AuraEffect const* /*aurEff*/, AuraEffectHandleModes /*mode*/)
		{
			GetTarget()->RemoveAurasDueToSpell(91724);
		}

		void Register()
		{
			AfterEffectApply += AuraEffectApplyFn(spell_pri_inner_fire_AuraScript::HandleEffectApply, EFFECT_0, SPELL_AURA_MOD_RESISTANCE_PCT, AURA_EFFECT_HANDLE_REAL_OR_REAPPLY_MASK);
			AfterEffectRemove += AuraEffectRemoveFn(spell_pri_inner_fire_AuraScript::HandleEffectRemove, EFFECT_0, SPELL_AURA_MOD_RESISTANCE_PCT, AURA_EFFECT_HANDLE_REAL_OR_REAPPLY_MASK);
		}
	};

	AuraScript* GetAuraScript() const
	{
		return new spell_pri_inner_fire_AuraScript();
	}
};

// 32379 - Shadow Word Death
class spell_pri_shadow_word_death : public SpellScriptLoader
{
public:
	spell_pri_shadow_word_death() : SpellScriptLoader("spell_pri_shadow_word_death") { }

	class spell_pri_shadow_word_death_SpellScript : public SpellScript
	{
		PrepareSpellScript(spell_pri_shadow_word_death_SpellScript);

		void HandleDamage()
		{
			Unit* caster = GetCaster();
			if (caster->GetTypeId() != TYPEID_PLAYER)
				return;

			Unit* target = GetHitUnit();
			int32 damage = GetHitDamage();
			// Glyph cooldown reset when target was failed to kill
			if (AuraEffect* glyph = caster->GetDummyAuraEffect(SPELLFAMILY_PRIEST, 1980, EFFECT_0))
				if (target && target->GetHealthPct() <= glyph->GetAmount() && !caster->HasAura(95652))
					if (int32(target->GetHealth()) > damage)
					{
				caster->ToPlayer()->RemoveSpellCooldown(32379, true);
				caster->CastSpell(caster, 95652, true);
					}

			// Pain and Suffering reduces damage
			if (AuraEffect* aurEff = caster->GetDummyAuraEffect(SPELLFAMILY_PRIEST, PRIEST_ICON_ID_PAIN_AND_SUFFERING, EFFECT_1))
				AddPct(damage, aurEff->GetAmount());

			caster->CastCustomSpell(caster, SPELL_PRIEST_SHADOW_WORD_DEATH, &damage, 0, 0, true);
		}

		void Register()
		{
			OnHit += SpellHitFn(spell_pri_shadow_word_death_SpellScript::HandleDamage);
		}
	};

	SpellScript* GetSpellScript() const
	{
		return new spell_pri_shadow_word_death_SpellScript();
	}
};

// 15473 - Shadowform
class spell_pri_shadowform : public SpellScriptLoader
{
public:
	spell_pri_shadowform() : SpellScriptLoader("spell_pri_shadowform") { }

	class spell_pri_shadowform_AuraScript : public AuraScript
	{
		PrepareAuraScript(spell_pri_shadowform_AuraScript);

		bool Validate(SpellInfo const* /*spellInfo*/)
		{
			if (!sSpellMgr->GetSpellInfo(SPELL_PRIEST_SHADOWFORM_VISUAL_WITHOUT_GLYPH) ||
				!sSpellMgr->GetSpellInfo(SPELL_PRIEST_SHADOWFORM_VISUAL_WITH_GLYPH))
				return false;
			return true;
		}

		void HandleEffectApply(AuraEffect const* /*aurEff*/, AuraEffectHandleModes /*mode*/)
		{
			GetTarget()->CastSpell(GetTarget(), GetTarget()->HasAura(SPELL_PRIEST_GLYPH_OF_SHADOW) ? SPELL_PRIEST_SHADOWFORM_VISUAL_WITH_GLYPH : SPELL_PRIEST_SHADOWFORM_VISUAL_WITHOUT_GLYPH, true);
		}

		void HandleEffectRemove(AuraEffect const* /*aurEff*/, AuraEffectHandleModes /*mode*/)
		{
			GetTarget()->RemoveAurasDueToSpell(SPELL_PRIEST_SHADOWFORM_VISUAL_WITHOUT_GLYPH);
			GetTarget()->RemoveAurasDueToSpell(SPELL_PRIEST_SHADOWFORM_VISUAL_WITH_GLYPH);
		}

		void Register()
		{
			AfterEffectApply += AuraEffectApplyFn(spell_pri_shadowform_AuraScript::HandleEffectApply, EFFECT_0, SPELL_AURA_MOD_SHAPESHIFT, AURA_EFFECT_HANDLE_REAL_OR_REAPPLY_MASK);
			AfterEffectRemove += AuraEffectRemoveFn(spell_pri_shadowform_AuraScript::HandleEffectRemove, EFFECT_0, SPELL_AURA_MOD_SHAPESHIFT, AURA_EFFECT_HANDLE_REAL_OR_REAPPLY_MASK);
		}
	};

	AuraScript* GetAuraScript() const
	{
		return new spell_pri_shadowform_AuraScript();
	}
};

// 34914 - Vampiric Touch
class spell_pri_vampiric_touch : public SpellScriptLoader
{
public:
	spell_pri_vampiric_touch() : SpellScriptLoader("spell_pri_vampiric_touch") { }

	class spell_pri_vampiric_touch_AuraScript : public AuraScript
	{
		PrepareAuraScript(spell_pri_vampiric_touch_AuraScript);

		void HandleDispel(DispelInfo* dispelInfo)
		{
			if (Unit* caster = GetCaster())
				if (Unit* target = GetUnitOwner())
					if (AuraEffect const* aurEff = caster->GetDummyAuraEffect(SPELLFAMILY_PRIEST, 1869, EFFECT_1))
						if (roll_chance_i(aurEff->GetSpellInfo()->Effects[EFFECT_0].BasePoints))
							if (Unit* dispeller = dispelInfo->GetDispeller())
								dispeller->CastSpell(dispeller, 87204, true);
		}

		void Register()
		{
			AfterDispel += AuraDispelFn(spell_pri_vampiric_touch_AuraScript::HandleDispel);
		}
	};

	AuraScript* GetAuraScript() const
	{
		return new spell_pri_vampiric_touch_AuraScript();
	}
};

// Shadow orbs
class spell_pri_shadow_orbs : public SpellScriptLoader
{
public:
	spell_pri_shadow_orbs() : SpellScriptLoader("spell_pri_shadow_orbs") { }

	class spell_pri_shadow_orbs_AuraScript : public AuraScript
	{
		PrepareAuraScript(spell_pri_shadow_orbs_AuraScript);

		void HandleEffectApply(AuraEffect const* /*aurEff*/, AuraEffectHandleModes /*mode*/)
		{
			if (GetStackAmount() == GetSpellInfo()->StackAmount)
				GetTarget()->CastSpell(GetTarget(), 93683, true);
		}

		void HandleEffectRemove(AuraEffect const* /*aurEff*/, AuraEffectHandleModes /*mode*/)
		{
			GetTarget()->RemoveAurasDueToSpell(93683);
		}

		void Register()
		{
			AfterEffectApply += AuraEffectApplyFn(spell_pri_shadow_orbs_AuraScript::HandleEffectApply, EFFECT_0, SPELL_AURA_DUMMY, AURA_EFFECT_HANDLE_REAL_OR_REAPPLY_MASK);
			AfterEffectRemove += AuraEffectRemoveFn(spell_pri_shadow_orbs_AuraScript::HandleEffectRemove, EFFECT_0, SPELL_AURA_DUMMY, AURA_EFFECT_HANDLE_REAL_OR_REAPPLY_MASK);
		}
	};

	AuraScript* GetAuraScript() const
	{
		return new spell_pri_shadow_orbs_AuraScript();
	}
};


// 527 - Dispel Magic
class spell_pri_dispel_magic : public SpellScriptLoader
{
public:
	spell_pri_dispel_magic() : SpellScriptLoader("spell_pri_dispel_magic") { }

	class spell_pri_dispel_magic_SpellScript : public SpellScript
	{
		PrepareSpellScript(spell_pri_dispel_magic_SpellScript);

		void HandleDummy(SpellEffIndex /*effIndex*/)
		{
			Unit* caster = GetCaster();
			if (Unit* unitTarget = GetHitUnit())
			{
				if (caster->IsFriendlyTo(unitTarget))
					caster->CastSpell(unitTarget, 97690, true);
				else
					caster->CastSpell(unitTarget, 97691, true);
			}
		}

		SpellCastResult CheckCast()
		{
			Unit* caster = GetCaster();
			if (Unit* target = GetExplTargetUnit())
				if (caster->IsFriendlyTo(target) && caster != target)
					if (!caster->HasAura(33167))
						return SPELL_FAILED_BAD_TARGETS;

			return SPELL_CAST_OK;
		}

		void Register()
		{
			OnCheckCast += SpellCheckCastFn(spell_pri_dispel_magic_SpellScript::CheckCast);
			OnEffectHitTarget += SpellEffectFn(spell_pri_dispel_magic_SpellScript::HandleDummy, EFFECT_0, SPELL_EFFECT_DUMMY);
		}
	};

	SpellScript* GetSpellScript() const
	{
		return new spell_pri_dispel_magic_SpellScript();
	}
};

// 89485 - Inner Focus
class spell_pri_inner_focus : public SpellScriptLoader
{
public:
	spell_pri_inner_focus() : SpellScriptLoader("spell_pri_inner_focus") { }

	class spell_pri_inner_focus_SpellScript : public SpellScript
	{
		PrepareSpellScript(spell_pri_inner_focus_SpellScript);

		void HandleOnCast()
		{
			Unit* caster = GetCaster();
			if (AuraEffect* aurEff = caster->GetAuraEffectOfRankedSpell(89488, EFFECT_0))
			{
				uint32 spellId = 0;
				switch (aurEff->GetId())
				{
				case 89488:
					spellId = 96266;
					break;
				case 89489:
					spellId = 96267;
					break;
				}
				caster->CastSpell(caster, spellId, true);
			}
		}

		void Register()
		{
			OnCast += SpellCastFn(spell_pri_inner_focus_SpellScript::HandleOnCast);
		}
	};

	SpellScript* GetSpellScript() const
	{
		return new spell_pri_inner_focus_SpellScript();
	}
};

// 94472, 81751 - Atonement heal
class spell_pri_atonement_heal : public SpellScriptLoader
{
public:
	spell_pri_atonement_heal() : SpellScriptLoader("spell_pri_atonement_heal") { }

	class spell_pri_atonement_heal_SpellScript : public SpellScript
	{
		PrepareSpellScript(spell_pri_atonement_heal_SpellScript);

		void HandleHeal(SpellEffIndex /*effIndex*/)
		{
			if (Unit* caster = GetOriginalCaster())
				if (GetHitUnit()->GetGUID() == caster->GetGUID())
				{
				int32 heal = GetHitHeal() / 2.0f;
				SetHitHeal(heal);
				}
		}

		void Register()
		{
			OnEffectHitTarget += SpellEffectFn(spell_pri_atonement_heal_SpellScript::HandleHeal, EFFECT_0, SPELL_EFFECT_HEAL);
		}
	};

	SpellScript* GetSpellScript() const
	{
		return new spell_pri_atonement_heal_SpellScript();
	}
};

// 81781 Power Word: Barrier
class spell_pri_power_word_barrier : public SpellScriptLoader
{
public:
	spell_pri_power_word_barrier() : SpellScriptLoader("spell_pri_power_word_barrier") {}

	class spell_pri_power_word_barrier_AuraScript : public AuraScript
	{
		PrepareAuraScript(spell_pri_power_word_barrier_AuraScript);

		bool Load()
		{
			if (Unit* caster = GetCaster())
				if (Unit* owner = caster->GetOwner())
					if (owner->GetTypeId() == TYPEID_PLAYER && owner->HasAura(55689))
						return false;

			return true;
		}

		void HandlePeriodicTriggerSpell(AuraEffect const* aurEff)
		{
			PreventDefaultAction();
		}

		void Register()
		{
			OnEffectPeriodic += AuraEffectPeriodicFn(spell_pri_power_word_barrier_AuraScript::HandlePeriodicTriggerSpell, EFFECT_1, SPELL_AURA_PERIODIC_TRIGGER_SPELL);
		}
	};

	AuraScript* GetAuraScript() const
	{
		return new spell_pri_power_word_barrier_AuraScript();
	}
};

class spell_pri_holyword_sanctuary_heal : public SpellScriptLoader
{
public:
	spell_pri_holyword_sanctuary_heal() : SpellScriptLoader("spell_pri_holyword_sanctuary_heal") { }

	class spell_pri_holyword_sanctuary_heal_SpellScript : public SpellScript
	{
		PrepareSpellScript(spell_pri_holyword_sanctuary_heal_SpellScript);

		void FilterTargets(std::list<WorldObject*>& unitList)
		{
			hitCount = unitList.size();
		}

		void HandleHeal(SpellEffIndex /*effIndex*/)
		{
			if (hitCount <= 6)
				return;

			SetHitHeal(GetHitHeal() - GetHitHeal()*(hitCount - 6) / hitCount);

		}

		void Register()
		{
			OnObjectAreaTargetSelect += SpellObjectAreaTargetSelectFn(spell_pri_holyword_sanctuary_heal_SpellScript::FilterTargets, EFFECT_0, TARGET_UNIT_DEST_AREA_ALLY);
			OnEffectHitTarget += SpellEffectFn(spell_pri_holyword_sanctuary_heal_SpellScript::HandleHeal, EFFECT_0, SPELL_EFFECT_HEAL);
		}

		uint8 hitCount;
	};

	SpellScript* GetSpellScript() const
	{
		return new spell_pri_holyword_sanctuary_heal_SpellScript();
	}
};

// Psychic scream
class spell_pri_fear : public SpellScriptLoader
{
public:
	spell_pri_fear() : SpellScriptLoader("spell_pri_fear") { }

	class spell_pri_fear_SpellScript : public SpellScript
	{
		PrepareSpellScript(spell_pri_fear_SpellScript);

		void HandleOnHit()
		{
			if (!GetHitUnit())
				return;

			AuraEffect* glyph = GetCaster()->GetAuraEffect(55676, EFFECT_0);
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
		}

		void Register()
		{
			OnHit += SpellHitFn(spell_pri_fear_SpellScript::HandleOnHit);
		}
	};

	SpellScript* GetSpellScript() const
	{
		return new spell_pri_fear_SpellScript();
	}
};


// Friendly Dispel magic
class spell_pri_friendly_dispel : public SpellScriptLoader
{
public:
	spell_pri_friendly_dispel() : SpellScriptLoader("spell_pri_friendly_dispel") { }

	class spell_pri_friendly_dispel_SpellScript : public SpellScript
	{
		PrepareSpellScript(spell_pri_friendly_dispel_SpellScript);

		void HandleOnDispel()
		{
			// Glyph of dispel magic
			if (AuraEffect* aura = GetCaster()->GetAuraEffect(55677, EFFECT_0))
			{
				int32 bp0 = int32(GetHitUnit()->CountPctFromMaxHealth(aura->GetAmount()));
				GetCaster()->CastCustomSpell(GetHitUnit(), 56131, &bp0, NULL, NULL, true);
			}
		}

		void Register()
		{
			OnSuccessfulDispel += SpellDispelFn(spell_pri_friendly_dispel_SpellScript::HandleOnDispel);
		}
	};

	SpellScript* GetSpellScript() const
	{
		return new spell_pri_friendly_dispel_SpellScript();
	}
};


// Spirit of Redemption (Shapeshift) - 27827
class spell_pri_spirit_of_redemption_form : public SpellScriptLoader
{
public:
	spell_pri_spirit_of_redemption_form() : SpellScriptLoader("spell_pri_spirit_of_redemption_form") { }

	class spell_pri_spirit_of_redemption_form_AuraScript : public AuraScript
	{
		PrepareAuraScript(spell_pri_spirit_of_redemption_form_AuraScript);

		void OnRemove(AuraEffect const* aurEff, AuraEffectHandleModes /*mode*/)
		{
			if (Unit* caster = GetCaster())
			{
				caster->RemoveAura(PRIEST_SPELL_SPIRIT_OF_REDEMPTION_FORM);
				caster->setDeathState(JUST_DIED);
			}
		}

		void Register()
		{
			OnEffectRemove += AuraEffectRemoveFn(spell_pri_spirit_of_redemption_form_AuraScript::OnRemove, EFFECT_0, SPELL_AURA_WATER_BREATHING, AURA_EFFECT_HANDLE_REAL);
		}
	};

	AuraScript* GetAuraScript() const
	{
		return new spell_pri_spirit_of_redemption_form_AuraScript();
	}
};

// Spirit of Redemption - 20711
class spell_pri_spirit_of_redemption : public SpellScriptLoader
{
public:
	spell_pri_spirit_of_redemption() : SpellScriptLoader("spell_pri_spirit_of_redemption") { }

	class spell_pri_spirit_of_redemption_AuraScript : public AuraScript
	{
		PrepareAuraScript(spell_pri_spirit_of_redemption_AuraScript);

		void CalculateAmount(AuraEffect const* /*auraEffect*/, int32& amount, bool& /*canBeRecalculated*/)
		{
			amount = -1;
		}

		void Absorb(AuraEffect* /*auraEffect*/, DamageInfo& dmgInfo, uint32& absorbAmount)
		{
			if (Unit* caster = GetCaster())
			{
				if (dmgInfo.GetDamage() < caster->GetHealth())
					return;

				if (caster->HasAura(PRIEST_SPELL_SPIRIT_OF_REDEMPTION_SHAPESHIFT))
					return;

				caster->CastSpell(caster, PRIEST_SPELL_SPIRIT_OF_REDEMPTION_FORM, true);
				caster->CastSpell(caster, PRIEST_SPELL_SPIRIT_OF_REDEMPTION_SHAPESHIFT, true);

				absorbAmount = caster->GetHealth() - 1;
			}
		}

		void Register()
		{
			DoEffectCalcAmount += AuraEffectCalcAmountFn(spell_pri_spirit_of_redemption_AuraScript::CalculateAmount, EFFECT_0, SPELL_AURA_SCHOOL_ABSORB);
			OnEffectAbsorb += AuraEffectAbsorbFn(spell_pri_spirit_of_redemption_AuraScript::Absorb, EFFECT_0);
		}
	};

	AuraScript* GetAuraScript() const
	{
		return new spell_pri_spirit_of_redemption_AuraScript();
	}
};


// 81208,81206 Chakra: Serenity and Chakra: Sanctuary spell swap supressor
class spell_pri_chakra_swap_supressor : public SpellScriptLoader
{
public:
	spell_pri_chakra_swap_supressor() : SpellScriptLoader("spell_pri_chakra_swap_supressor") { }

	class spell_pri_chakra_swap_supressor_AuraScript : public AuraScript
	{
		PrepareAuraScript(spell_pri_chakra_swap_supressor_AuraScript);

		void CalculateAmount(AuraEffect const* /*aurEff*/, int32 & amount, bool & /*canBeRecalculated*/)
		{
			if (GetCaster())
			{
				// Revelations
				if (!GetCaster()->HasAura(88627))
					amount = 88625;
			}
		}

		void Register()
		{
			DoEffectCalcAmount += AuraEffectCalcAmountFn(spell_pri_chakra_swap_supressor_AuraScript::CalculateAmount, EFFECT_2, SPELL_AURA_OVERRIDE_ACTIONBAR_SPELLS);
		}
	};

	AuraScript* GetAuraScript() const
	{
		return new spell_pri_chakra_swap_supressor_AuraScript();
	}
};

// 81585 Chakra: Serenity
class spell_pri_chakra_serenity_proc : public SpellScriptLoader
{
public:
	spell_pri_chakra_serenity_proc() : SpellScriptLoader("spell_pri_chakra_serenity_proc") {}

	class spell_pri_chakra_serenity_proc_SpellScript : public SpellScript
	{
		PrepareSpellScript(spell_pri_chakra_serenity_proc_SpellScript);

		void HandleScriptEffect(SpellEffIndex /*effIndex*/)
		{
			Unit* target = GetHitUnit();

			if (!target)
				return;

			if (Aura* renew = target->GetAura(PRIEST_SPELL_RENEW, GetCaster()->GetGUID()))
				renew->RefreshDuration();
		}

		void Register()
		{
			OnEffectHitTarget += SpellEffectFn(spell_pri_chakra_serenity_proc_SpellScript::HandleScriptEffect, EFFECT_0, SPELL_EFFECT_SCRIPT_EFFECT);
		}
	};

	SpellScript* GetSpellScript() const
	{
		return new spell_pri_chakra_serenity_proc_SpellScript();
	}
};

// 88685 Chakra: Sanctuary
class spell_pri_chakra_sanctuary_heal : public SpellScriptLoader
{
public:
	spell_pri_chakra_sanctuary_heal() : SpellScriptLoader("spell_pri_chakra_sanctuary_heal") { }

	class spell_pri_chakra_sanctuary_heal_AuraScript : public AuraScript
	{
		PrepareAuraScript(spell_pri_chakra_sanctuary_heal_AuraScript);

		void OnTick(AuraEffect const* aurEff)
		{
			if (DynamicObject* dynObj = GetCaster()->GetDynObject(88685))
				if (GetCaster()->GetMapId() == dynObj->GetMapId())
					GetCaster()->CastSpell(dynObj->GetPositionX(), dynObj->GetPositionY(), dynObj->GetPositionZ(), 88686, true);
		}

		void Register()
		{
			OnEffectPeriodic += AuraEffectPeriodicFn(spell_pri_chakra_sanctuary_heal_AuraScript::OnTick, EFFECT_1, SPELL_AURA_PERIODIC_DUMMY);
		}
	};

	AuraScript* GetAuraScript() const
	{
		return new spell_pri_chakra_sanctuary_heal_AuraScript();
	}
};

void AddSC_priest_spell_scripts()
{
	new spell_pri_circle_of_healing();
	new spell_pri_divine_aegis();
	new spell_pri_divine_hymn();
	new spell_pri_body_and_soul();
	new spell_pri_glyph_of_prayer_of_healing();
	new spell_pri_guardian_spirit();
	new spell_pri_leap_of_faith_effect_trigger();
	new spell_pri_lightwell_renew();
	new spell_pri_mana_burn();
	new spell_pri_mana_leech();
	new spell_pri_mind_sear();
	new spell_pri_mind_sear_spell();
	new spell_pri_pain_and_suffering_proc();
	new spell_pri_penance();
	new spell_pri_power_word_shield();
	new spell_pri_prayer_of_mending_heal();
	new spell_pri_renew();
	new spell_pri_shadow_word_death();
	new spell_pri_shadowform();
	new spell_pri_vampiric_touch();
	new spell_pri_inner_focus();
	new spell_pri_atonement_heal();
	new spell_pri_mind_blast();
	new spell_pri_fade();
	new spell_pri_dispel_magic();
	new spell_pri_shadow_orbs();
	new spell_pri_mind_spike();
	new spell_pri_power_word_barrier();
	new spell_pri_holyword_sanctuary_heal();
	new spell_pri_cure_disease();
	new spell_pri_inner_fire();
	new spell_pri_fear();
	new spell_pri_friendly_dispel();
	new spell_pri_spirit_of_redemption_form();
	new spell_pri_spirit_of_redemption();
	new spell_pri_chakra_swap_supressor();
	new spell_pri_chakra_serenity_proc();
	new spell_pri_chakra_sanctuary_heal();
}
