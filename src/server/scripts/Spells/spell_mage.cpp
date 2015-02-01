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
 * Scripts for spells with SPELLFAMILY_MAGE and SPELLFAMILY_GENERIC spells used by mage players.
 * Ordered alphabetically using scriptname.
 * Scriptnames of files in this file should be prefixed with "spell_mage_".
 */

#include "Player.h"
#include "ScriptMgr.h"
#include "SpellScript.h"
#include "SpellAuraEffects.h"
#include "Pet.h"

enum MageSpells
{
    SPELL_MAGE_PIERCING_CHILL                    = 83154,
    SPELL_MAGE_EARLY_FROST_R1                    = 83162,
    SPELL_MAGE_EARLY_FROST_R2                    = 83239,
    SPELL_MAGE_BURNOUT                           = 29077,
    SPELL_MAGE_COLD_SNAP                         = 11958,
    SPELL_MAGE_FOCUS_MAGIC_PROC                  = 54648,
    SPELL_MAGE_FROST_WARDING_R1                  = 11189,
    SPELL_MAGE_FROST_WARDING_TRIGGERED           = 57776,
    SPELL_MAGE_INCANTERS_ABSORBTION_R1           = 44394,
    SPELL_MAGE_INCANTERS_ABSORBTION_TRIGGERED    = 44413,
    SPELL_MAGE_IGNITE                            = 12654,
    SPELL_MAGE_MASTER_OF_ELEMENTS_ENERGIZE       = 29077,
    SPELL_MAGE_SQUIRREL_FORM                     = 32813,
    SPELL_MAGE_GIRAFFE_FORM                      = 32816,
    SPELL_MAGE_SERPENT_FORM                      = 32817,
    SPELL_MAGE_DRAGONHAWK_FORM                   = 32818,
    SPELL_MAGE_WORGEN_FORM                       = 32819,
    SPELL_MAGE_SHEEP_FORM                        = 32820,
    SPELL_MAGE_GLYPH_OF_ETERNAL_WATER            = 70937,
    SPELL_MAGE_SUMMON_WATER_ELEMENTAL_PERMANENT  = 70908,
    SPELL_MAGE_SUMMON_WATER_ELEMENTAL_TEMPORARY  = 70907,
    SPELL_MAGE_GLYPH_OF_BLAST_WAVE               = 62126,

    SPELL_MAGE_FLAMESTRIKE                       = 2120,

    SPELL_MAGE_CHILLED_R1                        = 12484,
    SPELL_MAGE_CHILLED_R2                        = 12485,

    SPELL_MAGE_CONE_OF_COLD_AURA_R1              = 11190,
    SPELL_MAGE_CONE_OF_COLD_AURA_R2              = 12489,
    SPELL_MAGE_CONE_OF_COLD_TRIGGER_R1           = 83301,
    SPELL_MAGE_CONE_OF_COLD_TRIGGER_R2           = 83302,

    SPELL_MAGE_SHATTERED_BARRIER_R1              = 44745,
    SPELL_MAGE_SHATTERED_BARRIER_R2              = 54787,
    SPELL_MAGE_SHATTERED_BARRIER_FREEZE_R1       = 55080,
    SPELL_MAGE_SHATTERED_BARRIER_FREEZE_R2       = 83073,

    SPELL_MAGE_IMPROVED_MANA_GEM_TRIGGERED       = 83098,

    SPELL_MAGE_RING_OF_FROST_SUMMON              = 82676,
    SPELL_MAGE_RING_OF_FROST_FREEZE              = 82691,
    SPELL_MAGE_RING_OF_FROST_DUMMY               = 91264,

    SPELL_MAGE_RITUAL_OF_REFRESHMENT_R1          = 74650,
    SPELL_MAGE_RITUAL_OF_REFRESHMENT_R2          = 92824,
    SPELL_MAGE_RITUAL_OF_REFRESHMENT_R3          = 92827,

    SPELL_MAGE_FINGERS_OF_FROST                  = 44544,

	SPELL_MAGE_INVISIBILITY_FADING               = 66,
	SPELL_MAGE_INVISIBILITY_INVISIBLE            = 32612,
};

enum MageIcons
{
    ICON_MAGE_SHATTER                            = 976,
    ICON_MAGE_IMPROVED_FLAMESTRIKE               = 37,
    ICON_MAGE_IMPROVED_FREEZE                    = 94,
    ICON_MAGE_INCANTER_S_ABSORPTION              = 2941,
    ICON_MAGE_IMPROVED_MANA_GEM                  = 1036
};

enum MagePetCalculate
{
    SPELL_MAGE_PET_SCALING_05         = 89764
};

// Incanter's Absorbtion
class spell_mage_incanters_absorbtion_base_AuraScript : public AuraScript
{
    public:
        bool Validate(SpellInfo const* /*spellInfo*/)
        {
            if (!sSpellMgr->GetSpellInfo(SPELL_MAGE_INCANTERS_ABSORBTION_TRIGGERED))
                return false;
            if (!sSpellMgr->GetSpellInfo(SPELL_MAGE_INCANTERS_ABSORBTION_R1))
                return false;
            return true;
        }

        void Trigger(AuraEffect* aurEff, DamageInfo& /*dmgInfo*/, uint32& absorbAmount)
        {
            Unit* target = GetTarget();

            if (AuraEffect* talentAurEff = target->GetAuraEffectOfRankedSpell(SPELL_MAGE_INCANTERS_ABSORBTION_R1, EFFECT_0))
            {
                int32 bp = CalculatePct(absorbAmount, talentAurEff->GetAmount());
                target->CastCustomSpell(target, SPELL_MAGE_INCANTERS_ABSORBTION_TRIGGERED, &bp, NULL, NULL, true, NULL, aurEff);
            }
        }
};

// 11113 - Blast Wave
class spell_mage_blast_wave : public SpellScriptLoader
{
    public:
        spell_mage_blast_wave() : SpellScriptLoader("spell_mage_blast_wave") { }

        class spell_mage_blast_wave_SpellScript : public SpellScript
        {
            PrepareSpellScript(spell_mage_blast_wave_SpellScript);

            bool Validate(SpellInfo const* /*spellInfo*/)
            {
                if (!sSpellMgr->GetSpellInfo(SPELL_MAGE_FLAMESTRIKE))
                    return false;
                return true;
            }

            void CountTargets(std::list<WorldObject*>& targetList)
            {
                _targetCount = targetList.size();
            }

            void HandleImprovedFlamestrike()
            {
                if (_targetCount >= 2)
                    if (AuraEffect* aurEff = GetCaster()->GetAuraEffect(SPELL_AURA_DUMMY, SPELLFAMILY_MAGE, ICON_MAGE_IMPROVED_FLAMESTRIKE, EFFECT_0))
                        if (roll_chance_i(aurEff->GetAmount()))
                        {
                            float x, y, z;
                            WorldLocation const* loc = GetExplTargetDest();
                            if (!loc)
                                return;

                            loc->GetPosition(x, y, z);
                            GetCaster()->CastSpell(x, y, z, SPELL_MAGE_FLAMESTRIKE, true);
                        }
            }

            void Register()
            {
                OnObjectAreaTargetSelect += SpellObjectAreaTargetSelectFn(spell_mage_blast_wave_SpellScript::CountTargets, EFFECT_0, TARGET_UNIT_DEST_AREA_ENEMY);
                AfterCast += SpellCastFn(spell_mage_blast_wave_SpellScript::HandleImprovedFlamestrike);
            }

        private:
            uint32 _targetCount;
        };

        SpellScript* GetSpellScript() const
        {
            return new spell_mage_blast_wave_SpellScript();
        }
};

// 82676 - Ring of Frost
class spell_mage_ring_of_frost : public SpellScriptLoader
{
    public:
        spell_mage_ring_of_frost() : SpellScriptLoader("spell_mage_ring_of_frost") { }

        class spell_mage_ring_of_frost_AuraScript : public AuraScript
        {
            PrepareAuraScript(spell_mage_ring_of_frost_AuraScript);

            bool Validate(SpellInfo const* /*spellInfo*/)
            {
                if (!sSpellMgr->GetSpellInfo(SPELL_MAGE_RING_OF_FROST_SUMMON))
                    return false;
                if (!sSpellMgr->GetSpellInfo(SPELL_MAGE_RING_OF_FROST_FREEZE))
                    return false;
                if (!sSpellMgr->GetSpellInfo(SPELL_MAGE_RING_OF_FROST_DUMMY))
                    return false;
                return true;
            }

            void HandleEffectPeriodic(AuraEffect const* aurEff)
            {
                std::list<Creature*> MinionList;
                GetTarget()->GetAllMinionsByEntry(MinionList, GetSpellInfo()->Effects[EFFECT_0].MiscValue);
                for (std::list<Creature*>::iterator itr = MinionList.begin(); itr != MinionList.end(); itr++)
                {
                    TempSummon* ringOfFrost = (*itr)->ToTempSummon();
                    if (GetMaxDuration() - (int32)ringOfFrost->GetTimer() >= sSpellMgr->GetSpellInfo(SPELL_MAGE_RING_OF_FROST_DUMMY)->GetDuration())
                        GetTarget()->CastSpell(ringOfFrost->GetPositionX(), ringOfFrost->GetPositionY(), ringOfFrost->GetPositionZ(), SPELL_MAGE_RING_OF_FROST_FREEZE, true);
                }
            }

            void Apply(AuraEffect const* /*aurEff*/, AuraEffectHandleModes /*mode*/)
            {
                std::list<Creature*> MinionList;
                GetTarget()->GetAllMinionsByEntry(MinionList, GetSpellInfo()->Effects[EFFECT_0].MiscValue);
                TempSummon* ringOfFrost = NULL;

                // Get the last summoned RoF, save it and despawn older ones
                for (std::list<Creature*>::iterator itr = MinionList.begin(); itr != MinionList.end(); itr++)
                {
                    TempSummon* summon = (*itr)->ToTempSummon();

                    if (ringOfFrost && summon)
                    {
                        if (summon->GetTimer() > ringOfFrost->GetTimer())
                        {
                            ringOfFrost->DespawnOrUnsummon();
                            ringOfFrost = summon;
                        }
                        else
                            summon->DespawnOrUnsummon();
                    }
                    else if (summon)
                        ringOfFrost = summon;
                }
            }

            void Register()
            {
                 OnEffectPeriodic += AuraEffectPeriodicFn(spell_mage_ring_of_frost_AuraScript::HandleEffectPeriodic, EFFECT_1, SPELL_AURA_PERIODIC_TRIGGER_SPELL);
                 OnEffectApply += AuraEffectApplyFn(spell_mage_ring_of_frost_AuraScript::Apply, EFFECT_1, SPELL_AURA_PERIODIC_TRIGGER_SPELL, AURA_EFFECT_HANDLE_REAL_OR_REAPPLY_MASK);
            }
        };

        AuraScript* GetAuraScript() const
        {
            return new spell_mage_ring_of_frost_AuraScript();
        }
};

// 82691 - Ring of Frost (freeze effect)
class spell_mage_ring_of_frost_freeze : public SpellScriptLoader
{
    public:
        spell_mage_ring_of_frost_freeze() : SpellScriptLoader("spell_mage_ring_of_frost_freeze") { }

        class spell_mage_ring_of_frost_freeze_SpellScript : public SpellScript
        {
            PrepareSpellScript(spell_mage_ring_of_frost_freeze_SpellScript);

            bool Validate(SpellInfo const* /*spellInfo*/)
            {
                if (!sSpellMgr->GetSpellInfo(SPELL_MAGE_RING_OF_FROST_SUMMON))
                    return false;
                if (!sSpellMgr->GetSpellInfo(SPELL_MAGE_RING_OF_FROST_FREEZE))
                    return false;
                return true;
            }

            void FilterTargets(std::list<WorldObject*>& targets)
            {
                float outRadius = sSpellMgr->GetSpellInfo(SPELL_MAGE_RING_OF_FROST_SUMMON)->Effects[EFFECT_0].CalcRadius();
                float inRadius = 4.7f;

                for (std::list<WorldObject*>::iterator itr = targets.begin(); itr != targets.end();)
                    if (Unit* unit = (*itr)->ToUnit())
                    {
                        if (unit->HasAura(SPELL_MAGE_RING_OF_FROST_DUMMY) || unit->HasAura(SPELL_MAGE_RING_OF_FROST_FREEZE) || unit->GetExactDist(GetExplTargetDest()) > outRadius || unit->GetExactDist(GetExplTargetDest()) < inRadius)
                        {
                            WorldObject* temp = (*itr);
                            itr++;
                            targets.remove(temp);
                        }
                        else
                            itr++;
                    }
                    else
                        itr++;
            }

            void Register()
            {
                OnObjectAreaTargetSelect += SpellObjectAreaTargetSelectFn(spell_mage_ring_of_frost_freeze_SpellScript::FilterTargets, EFFECT_0, TARGET_UNIT_DEST_AREA_ENEMY);
            }
        };

        SpellScript* GetSpellScript() const
        {
            return new spell_mage_ring_of_frost_freeze_SpellScript();
        }

        class spell_mage_ring_of_frost_freeze_AuraScript : public AuraScript
        {
            PrepareAuraScript(spell_mage_ring_of_frost_freeze_AuraScript);

            bool Validate(SpellInfo const* /*spellInfo*/)
            {
                if (!sSpellMgr->GetSpellInfo(SPELL_MAGE_RING_OF_FROST_DUMMY))
                    return false;
                return true;
            }

            void OnRemove(AuraEffect const* aurEff, AuraEffectHandleModes /*mode*/)
            {
                if (GetCaster())
                    GetCaster()->CastSpell(GetTarget(), SPELL_MAGE_RING_OF_FROST_DUMMY, true);
            }

            void Register()
            {
                AfterEffectRemove += AuraEffectRemoveFn(spell_mage_ring_of_frost_freeze_AuraScript::OnRemove, EFFECT_0, SPELL_AURA_MOD_STUN, AURA_EFFECT_HANDLE_REAL);
            }
        };

        AuraScript* GetAuraScript() const
        {
            return new spell_mage_ring_of_frost_freeze_AuraScript();
        }
};

// Flame orb target filter
class spell_mage_orb_filter : public SpellScriptLoader
{
    public:
        spell_mage_orb_filter() : SpellScriptLoader("spell_mage_orb_filter") { }

        class spell_mage_orb_filter_SpellScript : public SpellScript
        {
            PrepareSpellScript(spell_mage_orb_filter_SpellScript);

            void FilterTargets(std::list<WorldObject*>& targets)
            {
                Position casterPos;
                GetCaster()->GetPosition(&casterPos);
                WorldObject* closestTarget = NULL;
                float closestDistance = 100.0f;
                for (std::list<WorldObject*>::const_iterator itr = targets.begin(); itr != targets.end(); itr++)
                {
                    Position comparePos;
                    (*itr)->GetPosition(&comparePos);
                    float exactDist = casterPos.GetExactDist(&comparePos);
                    if (exactDist < closestDistance && !((*itr)->ToUnit() && (*itr)->ToUnit()->HasBreakableByDamageCrowdControlAura()))
                    {
                        closestDistance = exactDist;
                        closestTarget = (*itr);
                    }
                }
                targets.clear();
                if (closestTarget)
                    targets.push_back(closestTarget);
            }

            void HandleDummy(SpellEffIndex /*effIndex*/)
            {
                Unit* caster = GetCaster();
                if (Unit* target = GetHitUnit())
                {
                    // Flame Orb
                    if (caster->GetEntry() == 44214)
                        caster->CastSpell(target, 82739, true, NULL, NULL, caster->GetOwnerGUID());
                    else
                    {
                        // Cast 40% slow if second rank of talent is learned
                        if (caster->GetOwner()->HasAura(84727))
                            caster->CastSpell(target, 84721, true, NULL, NULL, caster->GetOwnerGUID());
                        else
                            caster->CastSpell(target, 95969, true, NULL, NULL, caster->GetOwnerGUID());
                    }
                    // (Hack Fix) Checking if target is in range and camoufled, then remove camoufled aura.
                    if (target->HasAura(1784))
                        target->RemoveAura(1784);
                    if (target->HasAura(5215))
                        target->RemoveAura(5215);
                    if(target->HasAura(32612))
                        target->RemoveAura(32612);

                    caster->AddSpellCooldown(GetSpellInfo()->Id, 0, time(NULL) + GetSpellInfo()->Effects[EFFECT_0].BasePoints / 1000);
                    caster->CastSpell(target, 86719, true);
                    if (!caster->HasAura(82736))
                        caster->CastSpell(caster, 82736);
                }
            }

            void Register()
            {
                OnObjectAreaTargetSelect += SpellObjectAreaTargetSelectFn(spell_mage_orb_filter_SpellScript::FilterTargets, EFFECT_0, TARGET_UNIT_DEST_AREA_ENEMY);
                OnEffectHitTarget += SpellEffectFn(spell_mage_orb_filter_SpellScript::HandleDummy, EFFECT_0, SPELL_EFFECT_DUMMY);
            }
        };

        SpellScript* GetSpellScript() const
        {
            return new spell_mage_orb_filter_SpellScript();
        }
};

// -44449 - Burnout
class spell_mage_burnout : public SpellScriptLoader
{
    public:
        spell_mage_burnout() : SpellScriptLoader("spell_mage_burnout") { }

        class spell_mage_burnout_AuraScript : public AuraScript
        {
            PrepareAuraScript(spell_mage_burnout_AuraScript);

            bool Validate(SpellInfo const* /*spellInfo*/)
            {
                if (!sSpellMgr->GetSpellInfo(SPELL_MAGE_BURNOUT))
                    return false;
                return true;
            }

            bool CheckProc(ProcEventInfo& eventInfo)
            {
                return eventInfo.GetDamageInfo()->GetSpellInfo(); // eventInfo.GetSpellInfo()
            }

            void HandleProc(AuraEffect const* aurEff, ProcEventInfo& eventInfo)
            {
                PreventDefaultAction();

                int32 mana = int32(eventInfo.GetDamageInfo()->GetSpellInfo()->CalcPowerCost(GetTarget(), eventInfo.GetDamageInfo()->GetSchoolMask()));
                mana = CalculatePct(mana, aurEff->GetAmount());

                GetTarget()->CastCustomSpell(SPELL_MAGE_BURNOUT, SPELLVALUE_BASE_POINT0, mana, GetTarget(), true, NULL, aurEff);
            }

            void Register()
            {
                DoCheckProc += AuraCheckProcFn(spell_mage_burnout_AuraScript::CheckProc);
                OnEffectProc += AuraEffectProcFn(spell_mage_burnout_AuraScript::HandleProc, EFFECT_1, SPELL_AURA_DUMMY);
            }
        };

        AuraScript* GetAuraScript() const
        {
            return new spell_mage_burnout_AuraScript();
        }
};

// Pyromaniac
class spell_mage_pyromaniac : public SpellScriptLoader
{
    public:
        spell_mage_pyromaniac() : SpellScriptLoader("spell_mage_pyromaniac") { }

        class spell_mage_pyromaniac_AuraScript : public AuraScript
        {
            PrepareAuraScript(spell_mage_pyromaniac_AuraScript);

            bool isFireDot(uint32 auraId)
            {
                return auraId == 44457
                || auraId == 12654
                || auraId == 92315; // Pyroblast + Hot Strake Effect.
            }

            void AfterRemove(AuraEffect const* aurEff, AuraEffectHandleModes mode)
            {
                if (Unit* caster = GetCaster())
                {
                    uint32 dotCount = 0;
                    std::map<uint64, uint32> usedGuids;
                    Player::appliedAurasList const& auras = caster->ToPlayer()->appliedAuras;
                    ACE_Guard<ACE_Recursive_Thread_Mutex> g(caster->m_appliedAuraMutex);
                    for (Player::appliedAurasList::const_iterator itr = auras.begin(); itr != auras.end(); itr++)
                    {
                        Aura* aura = (*itr);
                        if (isFireDot(aura->GetId()))
                        {
                            uint64 ownerGuid = aura->GetUnitOwner()->GetGUID();
                            if (usedGuids.find(ownerGuid) == usedGuids.end())
                            {
                                dotCount++;
                                usedGuids[ownerGuid] = aura->GetId();
                            }
                        }
                    }
                    if (dotCount < 3)
                        caster->RemoveAurasDueToSpell(83582);
                }
            }

            void AfterApply(AuraEffect const* aurEff, AuraEffectHandleModes mode)
            {
                if (Unit* caster = GetCaster())
                {
                    if (AuraEffect* pyroManiac = caster->GetDummyAuraEffect(SPELLFAMILY_MAGE, 2128, EFFECT_0))
                    {
                        uint32 dotCount = 0;
                        std::map<uint64, uint32> usedGuids;
                        Player::appliedAurasList const& auras = caster->ToPlayer()->appliedAuras;
                        ACE_Guard<ACE_Recursive_Thread_Mutex> g(caster->m_appliedAuraMutex);
                        for (Player::appliedAurasList::const_iterator itr = auras.begin(); itr != auras.end(); itr++)
                        {
                            Aura* aura = (*itr);
                            if (isFireDot(aura->GetId()))
                            {
                                uint64 ownerGuid = aura->GetUnitOwner()->GetGUID();
                                if (usedGuids.find(ownerGuid) == usedGuids.end())
                                {
                                    dotCount++;
                                    usedGuids[ownerGuid] = aura->GetId();
                                }
                            }
                        }
                        if (dotCount >= 3)
                        {
                            int32 bp0 = pyroManiac->GetAmount();
                            caster->CastCustomSpell(caster, 83582, &bp0, NULL, NULL, true);
                        }
                    }
                }
            }

            void Register()
            {
                AfterEffectApply += AuraEffectApplyFn(spell_mage_pyromaniac_AuraScript::AfterApply, EFFECT_0, SPELL_AURA_PERIODIC_DAMAGE, AURA_EFFECT_HANDLE_REAL);
                AfterEffectRemove += AuraEffectRemoveFn(spell_mage_pyromaniac_AuraScript::AfterRemove, EFFECT_0, SPELL_AURA_PERIODIC_DAMAGE, AURA_EFFECT_HANDLE_REAL);
            }
        };

        AuraScript* GetAuraScript() const
        {
            return new spell_mage_pyromaniac_AuraScript();
        }
};

// Frostfire Bolt
class spell_mage_ffb: public SpellScriptLoader
{
    public:
        spell_mage_ffb() : SpellScriptLoader("spell_mage_ffb") { }

        class spell_mage_ffb_SpellScript : public SpellScript
        {
            PrepareSpellScript(spell_mage_ffb_SpellScript);

            void HandleOnHit()
            {
                if (!GetHitUnit())
                    return;

                AuraEffect* glyph = GetCaster()->GetAuraEffect(61205, EFFECT_0);
                if (Aura* debuff = GetHitAura())
                {
                    if (debuff->GetApplicationOfTarget(GetHitUnit()->GetGUID()))
                    {
                        // Modify slow based on glyph
                        if (AuraEffect* slow = debuff->GetEffect(EFFECT_0))
                        {
                            if (glyph)
                            {
                                slow->SetAmount(0);
                                slow->HandleEffect(GetHitUnit(), AURA_REMOVE_BY_DEFAULT, false);
                            }
                        }

                        // Modify dot based on glyph
                        if (AuraEffect* dot = debuff->GetEffect(EFFECT_2))
                        {
                            if (!glyph)
                            {
                                dot->SetAmount(0);
                                dot->HandleEffect(GetHitUnit(), AURA_REMOVE_BY_DEFAULT, false);
                                dot->SetPeriodic(false);
                            }
                        }
                    }
                }
            }

            void Register()
            {
                OnHit += SpellHitFn(spell_mage_ffb_SpellScript::HandleOnHit);
            }
        };

        SpellScript* GetSpellScript() const
        {
            return new spell_mage_ffb_SpellScript();
        }
};

// Icy veins
class spell_mage_icy_veins: public SpellScriptLoader
{
    public:
        spell_mage_icy_veins() : SpellScriptLoader("spell_mage_icy_veins") { }

        class spell_mage_icy_veins_SpellScript : public SpellScript
        {
            PrepareSpellScript(spell_mage_icy_veins_SpellScript);

            void HandleOnHit()
            {
                if (Unit* target = GetHitUnit())
                    if (target->HasAura(56374))
                    {
                        target->RemoveAurasByType(SPELL_AURA_HASTE_SPELLS, 0, 0, true, false);
                        target->RemoveAurasByType(SPELL_AURA_MOD_DECREASE_SPEED);
                    }
            }

            void Register()
            {
                OnHit += SpellHitFn(spell_mage_icy_veins_SpellScript::HandleOnHit);
            }
        };

        SpellScript* GetSpellScript() const
        {
            return new spell_mage_icy_veins_SpellScript();
        }
};

// Polymorph
class spell_mage_polymorph : public SpellScriptLoader
{
    public:
        spell_mage_polymorph() : SpellScriptLoader("spell_mage_polymorph") { }

        class spell_mage_polymorph_AuraScript : public AuraScript
        {
            PrepareAuraScript(spell_mage_polymorph_AuraScript);

            void AfterRemove(AuraEffect const* aurEff, AuraEffectHandleModes /*mode*/)
            {
                AuraRemoveMode removeMode = GetTargetApplication()->GetRemoveMode();
                if (removeMode != AURA_REMOVE_BY_DAMAGE)
                    return;

                if (Unit* caster = GetCaster())
                    if (AuraEffect* aur = caster->GetDummyAuraEffect(SPELLFAMILY_MAGE, 82, EFFECT_0))
                    {
                        uint32 triggeredId = 0;
                        switch (aur->GetSpellInfo()->Id)
                        {
                            case 11210:
                                triggeredId = 83046;
                                break;
                            case 12592:
                                triggeredId = 83047;
                                break;
                        }
                        GetTarget()->CastSpell(GetTarget(), triggeredId, true);
                    }
            }

            void Register()
            {
                AfterEffectRemove += AuraEffectRemoveFn(spell_mage_polymorph_AuraScript::AfterRemove, EFFECT_0, SPELL_AURA_MOD_CONFUSE, AURA_EFFECT_HANDLE_REAL);
            }
        };

        AuraScript* GetAuraScript() const
        {
            return new spell_mage_polymorph_AuraScript();
        }
};

// 42208 - Blizzard
/// Updated 4.3.4
class spell_mage_blizzard : public SpellScriptLoader
{
   public:
       spell_mage_blizzard() : SpellScriptLoader("spell_mage_blizzard") { }

       class spell_mage_blizzard_SpellScript : public SpellScript
       {
           PrepareSpellScript(spell_mage_blizzard_SpellScript);

           bool Validate(SpellInfo const* /*spellInfo*/)
           {
               if (!sSpellMgr->GetSpellInfo(SPELL_MAGE_CHILLED_R1))
                   return false;
               if (!sSpellMgr->GetSpellInfo(SPELL_MAGE_CHILLED_R2))
                   return false;
               return true;
           }

           void AddChillEffect(SpellEffIndex /*effIndex*/)
           {
               Unit* caster = GetCaster();
               if (Unit* unitTarget = GetHitUnit())
               {
                   if (caster->IsScriptOverriden(GetSpellInfo(), 836))
                       caster->CastSpell(unitTarget, SPELL_MAGE_CHILLED_R1, true);
                   else if (caster->IsScriptOverriden(GetSpellInfo(), 988))
                       caster->CastSpell(unitTarget, SPELL_MAGE_CHILLED_R2, true);
               }
           }

           void Register()
           {
               OnEffectHitTarget += SpellEffectFn(spell_mage_blizzard_SpellScript::AddChillEffect, EFFECT_0, SPELL_EFFECT_SCHOOL_DAMAGE);
           }
       };

       SpellScript* GetSpellScript() const
       {
           return new spell_mage_blizzard_SpellScript();
       }
};

// 11958 - Cold Snap
class spell_mage_cold_snap : public SpellScriptLoader
{
    public:
        spell_mage_cold_snap() : SpellScriptLoader("spell_mage_cold_snap") { }

        class spell_mage_cold_snap_SpellScript : public SpellScript
        {
            PrepareSpellScript(spell_mage_cold_snap_SpellScript);

            bool Load()
            {
                return GetCaster()->GetTypeId() == TYPEID_PLAYER;
            }

            void HandleDummy(SpellEffIndex /*effIndex*/)
            {
                Player* caster = GetCaster()->ToPlayer();
                // immediately finishes the cooldown on Frost spells
                const SpellCooldowns& cm = caster->GetSpellCooldownMap();
                for (SpellCooldowns::const_iterator itr = cm.begin(); itr != cm.end();)
                {
                    SpellInfo const* spellInfo = sSpellMgr->GetSpellInfo(itr->first);

                    if (spellInfo->SpellFamilyName == SPELLFAMILY_MAGE &&
                        ((spellInfo->GetSchoolMask() & SPELL_SCHOOL_MASK_FROST) || spellInfo->Id == 92283) &&
                        spellInfo->Id != SPELL_MAGE_COLD_SNAP && spellInfo->GetRecoveryTime() > 0)
                    {
                        caster->RemoveSpellCooldown((itr++)->first, true);
                    }
                    else
                        ++itr;
                }
            }

            void Register()
            {
                OnEffectHit += SpellEffectFn(spell_mage_cold_snap_SpellScript::HandleDummy, EFFECT_0, SPELL_EFFECT_DUMMY);
            }
        };

        SpellScript* GetSpellScript() const
        {
            return new spell_mage_cold_snap_SpellScript();
        }
};

// 120 - Cone of Cold
/// Updated 4.3.4
class spell_mage_cone_of_cold : public SpellScriptLoader
{
    public:
        spell_mage_cone_of_cold() : SpellScriptLoader("spell_mage_cone_of_cold") { }

        class spell_mage_cone_of_cold_SpellScript : public SpellScript
        {
            PrepareSpellScript(spell_mage_cone_of_cold_SpellScript);

            void HandleConeOfColdScript()
            {
                Unit* caster = GetCaster();
                if (Unit* unitTarget = GetHitUnit())
                {
                    if (caster->HasAura(SPELL_MAGE_CONE_OF_COLD_AURA_R1)) // Improved Cone of Cold Rank 1
                        unitTarget->CastSpell(unitTarget, SPELL_MAGE_CONE_OF_COLD_TRIGGER_R1, true);
                    else if (caster->HasAura(SPELL_MAGE_CONE_OF_COLD_AURA_R2)) // Improved Cone of Cold Rank 2
                        unitTarget->CastSpell(unitTarget, SPELL_MAGE_CONE_OF_COLD_TRIGGER_R2, true);
                }
            }

            void Register()
            {
                AfterHit += SpellHitFn(spell_mage_cone_of_cold_SpellScript::HandleConeOfColdScript);
            }
        };

        SpellScript* GetSpellScript() const
        {
            return new spell_mage_cone_of_cold_SpellScript();
        }
};

// 42955 Conjure Refreshment
/// Updated 4.3.4
struct ConjureRefreshmentData
{
    uint32 minLevel;
    uint32 maxLevel;
    uint32 spellId;
};

uint8 const MAX_CONJURE_REFRESHMENT_SPELLS = 7;
ConjureRefreshmentData const _conjureData[MAX_CONJURE_REFRESHMENT_SPELLS] =
{
    { 33, 43, 92739 },
    { 44, 53, 92799 },
    { 54, 63, 92802 },
    { 64, 73, 92805 },
    { 74, 79, 74625 },
    { 80, 84, 92822 },
    { 85, 85, 92727 }
};

// 42955 - Conjure Refreshment
class spell_mage_conjure_refreshment : public SpellScriptLoader
{
    public:
        spell_mage_conjure_refreshment() : SpellScriptLoader("spell_mage_conjure_refreshment") { }

        class spell_mage_conjure_refreshment_SpellScript : public SpellScript
        {
            PrepareSpellScript(spell_mage_conjure_refreshment_SpellScript);

            bool Validate(SpellInfo const* /*spellInfo*/)
            {
                for (uint8 i = 0; i < MAX_CONJURE_REFRESHMENT_SPELLS; ++i)
                    if (!sSpellMgr->GetSpellInfo(_conjureData[i].spellId))
                        return false;
                return true;
            }

            bool Load()
            {
                if (GetCaster()->GetTypeId() != TYPEID_PLAYER)
                    return false;
                return true;
            }

            void HandleDummy(SpellEffIndex /*effIndex*/)
            {
                uint8 level = GetHitUnit()->getLevel();
                for (uint8 i = 0; i < MAX_CONJURE_REFRESHMENT_SPELLS; ++i)
                {
                    ConjureRefreshmentData const& spellData = _conjureData[i];
                    if (level < spellData.minLevel || level > spellData.maxLevel)
                        continue;
                    GetHitUnit()->CastSpell(GetHitUnit(), spellData.spellId, true);
                    break;
                }
            }

            void Register()
            {
                OnEffectHitTarget += SpellEffectFn(spell_mage_conjure_refreshment_SpellScript::HandleDummy, EFFECT_0, SPELL_EFFECT_DUMMY);
            }
        };

        SpellScript* GetSpellScript() const
        {
            return new spell_mage_conjure_refreshment_SpellScript();
        }
};

// 54646 - Focus Magic
class spell_mage_focus_magic : public SpellScriptLoader
{
    public:
        spell_mage_focus_magic() : SpellScriptLoader("spell_mage_focus_magic") { }

        class spell_mage_focus_magic_AuraScript : public AuraScript
        {
            PrepareAuraScript(spell_mage_focus_magic_AuraScript);

            bool Validate(SpellInfo const* /*spellInfo*/)
            {
                if (!sSpellMgr->GetSpellInfo(SPELL_MAGE_FOCUS_MAGIC_PROC))
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
                _procTarget = GetCaster();
                return _procTarget && _procTarget->isAlive();
            }

            void HandleProc(AuraEffect const* aurEff, ProcEventInfo& /*eventInfo*/)
            {
                PreventDefaultAction();
                GetTarget()->CastSpell(_procTarget, SPELL_MAGE_FOCUS_MAGIC_PROC, true, NULL, aurEff);
            }

            void Register()
            {
                DoCheckProc += AuraCheckProcFn(spell_mage_focus_magic_AuraScript::CheckProc);
                OnEffectProc += AuraEffectProcFn(spell_mage_focus_magic_AuraScript::HandleProc, EFFECT_0, SPELL_AURA_MOD_SPELL_CRIT_CHANCE);
            }

        private:
            Unit* _procTarget;
        };

        AuraScript* GetAuraScript() const
        {
            return new spell_mage_focus_magic_AuraScript();
        }
};

// 116 - Frostbolt
/// Updated 4.3.4
class spell_mage_frostbolt : public SpellScriptLoader
{
   public:
       spell_mage_frostbolt() : SpellScriptLoader("spell_mage_frostbolt") { }

       class spell_mage_frostbolt_SpellScript : public SpellScript
       {
           PrepareSpellScript(spell_mage_frostbolt_SpellScript);

           void RecalculateDamage(SpellEffIndex /*effIndex*/)
           {
               if (GetHitUnit() && GetHitUnit()->HasAuraState(AURA_STATE_FROZEN, GetSpellInfo(), GetCaster()))
               {
                   if (AuraEffect* aurEff = GetCaster()->GetAuraEffect(SPELL_AURA_DUMMY, SPELLFAMILY_MAGE, ICON_MAGE_SHATTER, EFFECT_1))
                   {
                       int32 damage = GetHitDamage();
                       AddPct(damage, aurEff->GetAmount());
                       SetHitDamage(damage);
                   }
               }
           }

           void HandleEarlyFrost()
           {
               Unit* caster = GetCaster();
               if (AuraEffect* aurEff = caster->GetAuraEffect(SPELL_AURA_ADD_FLAT_MODIFIER, SPELLFAMILY_MAGE, 189, 0))
               {
                   uint32 spellId = 0;
                   switch (aurEff->GetId())
                   {
                       case 83049:
                           spellId = SPELL_MAGE_EARLY_FROST_R1;
                           break;
                       case 83050:
                           spellId = SPELL_MAGE_EARLY_FROST_R2;
                           break;
                   }

                   // Check if frostbolt was affected by the modifier
                   if (Spell* spell = GetSpell())
                       if (spell->m_appliedAuras.find(spellId) != spell->m_appliedAuras.end())
                           return;

                   if (spellId && !caster->HasAura(spellId))
                   {
                       caster->CastSpell(caster, spellId, true);
                       caster->RemoveAura(94315);
                   }
               }
           }

           void Register()
           {
               AfterCast += SpellCastFn(spell_mage_frostbolt_SpellScript::HandleEarlyFrost);
               OnEffectHitTarget += SpellEffectFn(spell_mage_frostbolt_SpellScript::RecalculateDamage, EFFECT_1, SPELL_EFFECT_SCHOOL_DAMAGE);
           }
       };

       SpellScript* GetSpellScript() const
       {
           return new spell_mage_frostbolt_SpellScript();
       }
};

// -44457 - Living Bomb
class spell_mage_living_bomb : public SpellScriptLoader
{
    public:
        spell_mage_living_bomb() : SpellScriptLoader("spell_mage_living_bomb") { }

        class spell_mage_living_bomb_AuraScript : public AuraScript
        {
            PrepareAuraScript(spell_mage_living_bomb_AuraScript);

            bool Validate(SpellInfo const* spellInfo)
            {
                if (!sSpellMgr->GetSpellInfo(uint32(spellInfo->Effects[EFFECT_1].CalcValue())))
                    return false;
                return true;
            }

            void AfterRemove(AuraEffect const* aurEff, AuraEffectHandleModes /*mode*/)
            {
                AuraRemoveMode removeMode = GetTargetApplication()->GetRemoveMode();
                if (removeMode != AURA_REMOVE_BY_ENEMY_SPELL && removeMode != AURA_REMOVE_BY_EXPIRE)
                    return;

                if (Unit* caster = GetCaster())
                    caster->CastSpell(GetTarget(), uint32(aurEff->GetAmount()), true, NULL, aurEff);
            }

            void Register()
            {
                AfterEffectRemove += AuraEffectRemoveFn(spell_mage_living_bomb_AuraScript::AfterRemove, EFFECT_1, SPELL_AURA_DUMMY, AURA_EFFECT_HANDLE_REAL);
            }
        };

        AuraScript* GetAuraScript() const
        {
            return new spell_mage_living_bomb_AuraScript();
        }
};

// 11426 - Ice Barrier
/// Updated 4.3.4
class spell_mage_ice_barrier : public SpellScriptLoader
{
   public:
       spell_mage_ice_barrier() : SpellScriptLoader("spell_mage_ice_barrier") { }

       class spell_mage_ice_barrier_AuraScript : public AuraScript
       {
           PrepareAuraScript(spell_mage_ice_barrier_AuraScript);

           void AfterRemove(AuraEffect const* aurEff, AuraEffectHandleModes /*mode*/)
           {
               if (GetTargetApplication()->GetRemoveMode() != AURA_REMOVE_BY_ENEMY_SPELL || aurEff->GetAmount() > 0)
                   return;

               if (GetTarget()->HasAura(SPELL_MAGE_SHATTERED_BARRIER_R1))
                   GetTarget()->CastSpell(GetTarget(), SPELL_MAGE_SHATTERED_BARRIER_FREEZE_R1, true);
               else if (GetTarget()->HasAura(SPELL_MAGE_SHATTERED_BARRIER_R2))
                   GetTarget()->CastSpell(GetTarget(), SPELL_MAGE_SHATTERED_BARRIER_FREEZE_R2, true);
           }

           void CalculateAmount(AuraEffect const* aurEff, int32& amount, bool& canBeRecalculated)
           {
               canBeRecalculated = false;
               if (Unit* caster = GetCaster())
               {
                   // +87.00% from sp bonus
                   amount += floor(0.87f * caster->SpellBaseDamageBonusDone(GetSpellInfo()->GetSchoolMask()) + 0.5f);
                   
                   // Glyph of Ice barrier
                   if (AuraEffect const* glyph = caster->GetDummyAuraEffect(SPELLFAMILY_MAGE, 32, EFFECT_0))
                       AddPct(amount, glyph->GetAmount());
               }
           }


           void Register()
           {
               DoEffectCalcAmount += AuraEffectCalcAmountFn(spell_mage_ice_barrier_AuraScript::CalculateAmount, EFFECT_0, SPELL_AURA_SCHOOL_ABSORB);
               AfterEffectRemove += AuraEffectRemoveFn(spell_mage_ice_barrier_AuraScript::AfterRemove, EFFECT_0, SPELL_AURA_SCHOOL_ABSORB, AURA_EFFECT_HANDLE_REAL);
           }
       };

       AuraScript* GetAuraScript() const
       {
           return new spell_mage_ice_barrier_AuraScript();
       }
};

// 543 - Mage Ward
/// Updated 4.3.4
class spell_mage_mage_ward : public SpellScriptLoader
{
   public:
       spell_mage_mage_ward() : SpellScriptLoader("spell_mage_mage_ward") { }

       class spell_mage_mage_ward_AuraScript : public AuraScript
       {
           PrepareAuraScript(spell_mage_mage_ward_AuraScript);

           void HandleAbsorb(AuraEffect* /*aurEff*/, DamageInfo & /*dmgInfo*/, uint32 & absorbAmount)
           {
               if (AuraEffect* aurEff = GetTarget()->GetAuraEffect(SPELL_AURA_DUMMY, SPELLFAMILY_GENERIC, ICON_MAGE_INCANTER_S_ABSORPTION, EFFECT_0))
               {
                   int32 bp = CalculatePct(absorbAmount, aurEff->GetAmount());
                   GetTarget()->CastCustomSpell(GetTarget(), SPELL_MAGE_INCANTERS_ABSORBTION_TRIGGERED, &bp, NULL, NULL, true);
               }
           }

           void CalculateAmount(AuraEffect const* aurEff, int32& amount, bool& canBeRecalculated)
           {
               canBeRecalculated = false;
               if (Unit* caster = GetCaster())
                   // +80.70% from sp bonus
                   amount += floor(0.807f * caster->SpellBaseDamageBonusDone(GetSpellInfo()->GetSchoolMask()) + 0.5f);
           }

           void AfterRemove(AuraEffect const* /*aurEff*/, AuraEffectHandleModes /*mode*/)
           {
               if (GetTargetApplication()->GetRemoveMode() == AURA_REMOVE_BY_ENEMY_SPELL)
                   // Molten Shields
                   if (GetTarget()->HasAura(11094))
                       GetTarget()->CastSpell(GetTarget(), 31643, true);
           }

           void Register()
           {
               DoEffectCalcAmount += AuraEffectCalcAmountFn(spell_mage_mage_ward_AuraScript::CalculateAmount, EFFECT_0, SPELL_AURA_SCHOOL_ABSORB);
               AfterEffectAbsorb += AuraEffectAbsorbFn(spell_mage_mage_ward_AuraScript::HandleAbsorb, EFFECT_0);
               AfterEffectRemove += AuraEffectRemoveFn(spell_mage_mage_ward_AuraScript::AfterRemove, EFFECT_0, SPELL_AURA_SCHOOL_ABSORB, AURA_EFFECT_HANDLE_REAL);
           }
       };

       AuraScript* GetAuraScript() const
       {
           return new spell_mage_mage_ward_AuraScript();
       }
};

// 1463 - Mana Shield
/// Updated 4.3.4
class spell_mage_mana_shield : public SpellScriptLoader
{
    public:
       spell_mage_mana_shield() : SpellScriptLoader("spell_mage_mana_shield") { }

       class spell_mage_mana_shield_AuraScript : public AuraScript
       {
           PrepareAuraScript(spell_mage_mana_shield_AuraScript);

           void HandleAbsorb(AuraEffect* /*aurEff*/, DamageInfo & /*dmgInfo*/, uint32 & absorbAmount)
           {
               if (AuraEffect* aurEff = GetTarget()->GetAuraEffect(SPELL_AURA_DUMMY, SPELLFAMILY_GENERIC, ICON_MAGE_INCANTER_S_ABSORPTION, EFFECT_0))
               {
                   int32 bp = CalculatePct(absorbAmount, aurEff->GetAmount());
                   GetTarget()->CastCustomSpell(GetTarget(), SPELL_MAGE_INCANTERS_ABSORBTION_TRIGGERED, &bp, NULL, NULL, true);
               }
           }

           void AfterRemove(AuraEffect const* /*aurEff*/, AuraEffectHandleModes /*mode*/)
           {
               if (GetTargetApplication()->GetRemoveMode() == AURA_REMOVE_BY_ENEMY_SPELL)
                   if (AuraEffect* aurEff = GetTarget()->GetAuraEffect(SPELL_AURA_DUMMY, SPELLFAMILY_GENERIC, ICON_MAGE_INCANTER_S_ABSORPTION, EFFECT_0))
                       GetTarget()->CastSpell(GetTarget(), 86261, true);
           }

           void CalculateAmount(AuraEffect const* aurEff, int32& amount, bool& canBeRecalculated)
           {
               canBeRecalculated = false;
               if (Unit* caster = GetCaster())
                   // +80.7% from sp bonus
                   amount += floor(0.807f * caster->SpellBaseDamageBonusDone(GetSpellInfo()->GetSchoolMask()) + 0.5f);
           }

           void Register()
           {
               DoEffectCalcAmount += AuraEffectCalcAmountFn(spell_mage_mana_shield_AuraScript::CalculateAmount, EFFECT_0, SPELL_AURA_MANA_SHIELD);
               AfterEffectManaShield += AuraEffectManaShieldFn(spell_mage_mana_shield_AuraScript::HandleAbsorb, EFFECT_0);
               AfterEffectRemove += AuraEffectRemoveFn(spell_mage_mana_shield_AuraScript::AfterRemove, EFFECT_0, SPELL_AURA_MANA_SHIELD, AURA_EFFECT_HANDLE_REAL);
           }
       };

       AuraScript* GetAuraScript() const
       {
           return new spell_mage_mana_shield_AuraScript();
       }
};

enum SilvermoonPolymorph
{
    NPC_AUROSALIA       = 18744
};

// TODO: move out of here and rename - not a mage spell
class spell_mage_polymorph_cast_visual : public SpellScriptLoader
{
    public:
        spell_mage_polymorph_cast_visual() : SpellScriptLoader("spell_mage_polymorph_visual") { }

        class spell_mage_polymorph_cast_visual_SpellScript : public SpellScript
        {
            PrepareSpellScript(spell_mage_polymorph_cast_visual_SpellScript);

            static const uint32 PolymorhForms[6];

            bool Validate(SpellInfo const* /*spellInfo*/)
            {
                // check if spell ids exist in dbc
                for (uint32 i = 0; i < 6; i++)
                    if (!sSpellMgr->GetSpellInfo(PolymorhForms[i]))
                        return false;
                return true;
            }

            void HandleDummy(SpellEffIndex /*effIndex*/)
            {
                if (Unit* target = GetCaster()->FindNearestCreature(NPC_AUROSALIA, 30.0f))
                    if (target->GetTypeId() == TYPEID_UNIT)
                        target->CastSpell(target, PolymorhForms[urand(0, 5)], true);
            }

            void Register()
            {
                // add dummy effect spell handler to Polymorph visual
                OnEffectHitTarget += SpellEffectFn(spell_mage_polymorph_cast_visual_SpellScript::HandleDummy, EFFECT_0, SPELL_EFFECT_DUMMY);
            }
        };

        SpellScript* GetSpellScript() const
        {
            return new spell_mage_polymorph_cast_visual_SpellScript();
        }
};

uint32 const spell_mage_polymorph_cast_visual::spell_mage_polymorph_cast_visual_SpellScript::PolymorhForms[6] =
{
    SPELL_MAGE_SQUIRREL_FORM,
    SPELL_MAGE_GIRAFFE_FORM,
    SPELL_MAGE_SERPENT_FORM,
    SPELL_MAGE_DRAGONHAWK_FORM,
    SPELL_MAGE_WORGEN_FORM,
    SPELL_MAGE_SHEEP_FORM
};

// 5405  - Replenish Mana (Mana Gem)
/// Updated 4.3.4
class spell_mage_replenish_mana : public SpellScriptLoader
{
   public:
       spell_mage_replenish_mana() : SpellScriptLoader("spell_mage_replenish_mana") { }

       class spell_mage_replenish_mana_SpellScript : public SpellScript
       {
           PrepareSpellScript(spell_mage_replenish_mana_SpellScript);

           bool Validate(SpellInfo const* /*spellInfo*/)
           {
               if (!sSpellMgr->GetSpellInfo(SPELL_MAGE_IMPROVED_MANA_GEM_TRIGGERED))
                   return false;
               return true;
           }

           void HandleImprovedManaGem()
           {
               if (AuraEffect* aurEff = GetCaster()->GetAuraEffect(SPELL_AURA_DUMMY, SPELLFAMILY_MAGE, ICON_MAGE_IMPROVED_MANA_GEM, EFFECT_0))
               {
                   int32 bp = CalculatePct(GetCaster()->GetMaxPower(POWER_MANA), aurEff->GetAmount());
                   GetCaster()->CastCustomSpell(GetCaster(), SPELL_MAGE_IMPROVED_MANA_GEM_TRIGGERED, &bp, &bp, NULL, true);
               }
           }

           void Register()
           {
               AfterCast += SpellCastFn(spell_mage_replenish_mana_SpellScript::HandleImprovedManaGem);
           }
       };

       SpellScript* GetSpellScript() const
       {
           return new spell_mage_replenish_mana_SpellScript();
       }
};

// 31687 - Summon Water Elemental
class spell_mage_summon_water_elemental : public SpellScriptLoader
{
    public:
        spell_mage_summon_water_elemental() : SpellScriptLoader("spell_mage_summon_water_elemental") { }

        class spell_mage_summon_water_elemental_SpellScript : public SpellScript
        {
            PrepareSpellScript(spell_mage_summon_water_elemental_SpellScript);

            bool Validate(SpellInfo const* /*spellInfo*/)
            {
                if (!sSpellMgr->GetSpellInfo(SPELL_MAGE_GLYPH_OF_ETERNAL_WATER) || !sSpellMgr->GetSpellInfo(SPELL_MAGE_SUMMON_WATER_ELEMENTAL_TEMPORARY) || !sSpellMgr->GetSpellInfo(SPELL_MAGE_SUMMON_WATER_ELEMENTAL_PERMANENT))
                    return false;
                return true;
            }

            void HandleDummy(SpellEffIndex /*effIndex*/)
            {
                Unit* caster = GetCaster();

                if (Player* player = caster->ToPlayer())
                    if (Guardian* elemental = player->GetGuardianPet())
                        // Check if the pet we are going to unsummon is the mage's water elemental
                        if (elemental->GetEntry() == uint32(sSpellMgr->GetSpellInfo(SPELL_MAGE_SUMMON_WATER_ELEMENTAL_TEMPORARY)->Effects[EFFECT_0].MiscValue) ||
                            elemental->GetEntry() == uint32(sSpellMgr->GetSpellInfo(SPELL_MAGE_SUMMON_WATER_ELEMENTAL_PERMANENT)->Effects[EFFECT_0].MiscValue))
                            elemental->UnSummon();

                // Glyph of Eternal Water
                if (caster->HasAura(SPELL_MAGE_GLYPH_OF_ETERNAL_WATER))
                    caster->CastSpell(caster, SPELL_MAGE_SUMMON_WATER_ELEMENTAL_PERMANENT, true);
                else
                    caster->CastSpell(caster, SPELL_MAGE_SUMMON_WATER_ELEMENTAL_TEMPORARY, true);
            }

            void Register()
            {
                // add dummy effect spell handler to Summon Water Elemental
                OnEffectHit += SpellEffectFn(spell_mage_summon_water_elemental_SpellScript::HandleDummy, EFFECT_0, SPELL_EFFECT_DUMMY);
            }
        };

        SpellScript* GetSpellScript() const
        {
            return new spell_mage_summon_water_elemental_SpellScript();
        }
};

// 33395 Water Elemental's Freeze
/// Updated 4.3.4
class spell_mage_water_elemental_freeze : public SpellScriptLoader
{
   public:
       spell_mage_water_elemental_freeze() : SpellScriptLoader("spell_mage_water_elemental_freeze") { }

       class spell_mage_water_elemental_freeze_SpellScript : public SpellScript
       {
           PrepareSpellScript(spell_mage_water_elemental_freeze_SpellScript);

           bool Validate(SpellInfo const* /*spellInfo*/)
           {
               if (!sSpellMgr->GetSpellInfo(SPELL_MAGE_FINGERS_OF_FROST))
                   return false;
               return true;
           }

           void CountTargets(std::list<WorldObject*>& targetList)
           {
               _didHit = !targetList.empty();
           }

           void HandleImprovedFreeze()
           {
               if (!_didHit)
                   return;

               Unit* owner = GetCaster()->GetOwner();
               if (!owner)
                   return;

               if (AuraEffect* aurEff = owner->GetAuraEffect(SPELL_AURA_DUMMY, SPELLFAMILY_MAGE, ICON_MAGE_IMPROVED_FREEZE, EFFECT_0))
               {
                   if (roll_chance_i(aurEff->GetAmount()))
                       owner->CastCustomSpell(SPELL_MAGE_FINGERS_OF_FROST, SPELLVALUE_AURA_STACK, 2, owner, true);
               }
           }

           void Register()
           {
               OnObjectAreaTargetSelect += SpellObjectAreaTargetSelectFn(spell_mage_water_elemental_freeze_SpellScript::CountTargets, EFFECT_0, TARGET_UNIT_DEST_AREA_ENEMY);
               AfterCast += SpellCastFn(spell_mage_water_elemental_freeze_SpellScript::HandleImprovedFreeze);
           }

       private:
           bool _didHit;
       };

       SpellScript* GetSpellScript() const
       {
           return new spell_mage_water_elemental_freeze_SpellScript();
       }
};

//  83154 - Piercing Chill
class spell_mage_piercing_chill : public SpellScriptLoader
{
    public:
        spell_mage_piercing_chill() : SpellScriptLoader("spell_mage_piercing_chill") { }

        class spell_mage_piercing_chill_SpellScript : public SpellScript
        {
            PrepareSpellScript(spell_mage_piercing_chill_SpellScript);

            void FilterTargets(std::list<WorldObject*>& targets)
            {
                if (_targets.size())
                {
                    targets = _targets;
                    return;
                }

                if (AuraEffect* aurEff = GetCaster()->GetAuraEffect(SPELL_AURA_DUMMY, SPELLFAMILY_MAGE, 4625, EFFECT_0))
                {
                    targets.remove(GetExplTargetUnit());
                    Trinity::Containers::RandomResizeList(targets, aurEff->GetAmount());
                    targets.push_back(GetExplTargetUnit());
                    _targets = targets;
                }
            }

            void Register()
            {
                OnObjectAreaTargetSelect += SpellObjectAreaTargetSelectFn(spell_mage_piercing_chill_SpellScript::FilterTargets, EFFECT_0, TARGET_UNIT_DEST_AREA_ENEMY);
                OnObjectAreaTargetSelect += SpellObjectAreaTargetSelectFn(spell_mage_piercing_chill_SpellScript::FilterTargets, EFFECT_2, TARGET_UNIT_DEST_AREA_ENEMY);
            }

        private:
           std::list<WorldObject*> _targets;
        };

        SpellScript* GetSpellScript() const
        {
            return new spell_mage_piercing_chill_SpellScript();
        }
};

class spell_mage_cauterize : public SpellScriptLoader
{
    public:
        spell_mage_cauterize() : SpellScriptLoader("spell_mage_cauterize") { }

        class spell_mage_cauterize_AuraScript : public AuraScript
        {
            PrepareAuraScript(spell_mage_cauterize_AuraScript);

            uint32 absorbChance, healPct;

            enum Spell
            {
                MAGE_SPELL_CAUTERIZE_HEAL = 87023,
            };

            bool Load()
            {
                healPct = GetSpellInfo()->Effects[EFFECT_1].CalcValue();
                absorbChance = GetSpellInfo()->Effects[EFFECT_0].CalcValue();
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
                // If damage kills us
                if (remainingHealth <= 0 && !victim->ToPlayer()->HasSpellCooldown(MAGE_SPELL_CAUTERIZE_HEAL))
                {
                    if (roll_chance_i(absorbChance))
                    {
                        // Cast healing spell, completely avoid damage
                        absorbAmount = dmgInfo.GetDamage();
                        int32 healAmount = int32(victim->CountPctFromMaxHealth(healPct));
                        victim->CastCustomSpell(victim, MAGE_SPELL_CAUTERIZE_HEAL, NULL, &healAmount, NULL, true, NULL, aurEff);
                        victim->ToPlayer()->AddSpellCooldown(MAGE_SPELL_CAUTERIZE_HEAL, 0, time(NULL) + 60);
                    }
                }
            }

            void Register()
            {
                 DoEffectCalcAmount += AuraEffectCalcAmountFn(spell_mage_cauterize_AuraScript::CalculateAmount, EFFECT_0, SPELL_AURA_SCHOOL_ABSORB);
                 OnEffectAbsorb += AuraEffectAbsorbFn(spell_mage_cauterize_AuraScript::Absorb, EFFECT_0);
            }
        };

        AuraScript* GetAuraScript() const
        {
            return new spell_mage_cauterize_AuraScript();
        }
};

class spell_mage_combustion : public SpellScriptLoader
{
    public:
        spell_mage_combustion() : SpellScriptLoader("spell_mage_combustion") { }

        class spell_mage_combustion_SpellScript : public SpellScript
        {
            PrepareSpellScript(spell_mage_combustion_SpellScript);

            void HandleScriptEffect(SpellEffIndex /*effIndex*/)
            {
                Unit* caster = GetCaster();
                int32 damageSum = 0;
                if (Unit* unitTarget = GetHitUnit())
                {
                    Unit::AuraEffectList const & aurasA = unitTarget->GetAuraEffectsByType(SPELL_AURA_PERIODIC_DAMAGE);
                    for (Unit::AuraEffectList::const_iterator itr = aurasA.begin(); itr != aurasA.end(); ++itr)
                    {
                        if (((*itr)->GetCasterGUID() != caster->GetGUID())
                            || (*itr)->GetSpellInfo()->SpellFamilyName != GetSpellInfo()->SpellFamilyName)
                            continue;

                        // Info taken from Elitist jerks
                        if ((*itr)->GetId() == 44457 || (*itr)->GetId() == 11366)
                            damageSum += (*itr)->GetAmount() / 3;
                        else
                            damageSum += (*itr)->GetAmount() / 2;
                    }
                    if (damageSum)
                        caster->CastCustomSpell(unitTarget, 83853, &damageSum, NULL, NULL, true);
                }
            }

            void Register()
            {
                OnEffectHitTarget += SpellEffectFn(spell_mage_combustion_SpellScript::HandleScriptEffect, EFFECT_0, SPELL_EFFECT_SCRIPT_EFFECT);
            }
        };

        SpellScript* GetSpellScript() const
        {
            return new spell_mage_combustion_SpellScript();
        }
};

class spell_mage_impact : public SpellScriptLoader
{
    public:
        spell_mage_impact() : SpellScriptLoader("spell_mage_impact") { }

        class spell_mage_impact_SpellScript : public SpellScript
        {
            PrepareSpellScript(spell_mage_impact_SpellScript);

            void FilterTargets(std::list<WorldObject*>& targets)
            {
                if (GetExplTargetUnit())
                    targets.remove(GetExplTargetUnit());
            }

            void HandleScriptEffect(SpellEffIndex /*effIndex*/)
            {
                Unit* caster = GetCaster();
                if (Unit* unitTarget = GetExplTargetUnit())
                {
                    Unit::AuraEffectList const & aurasA = unitTarget->GetAuraEffectsByType(SPELL_AURA_PERIODIC_DAMAGE);
                    for (Unit::AuraEffectList::const_iterator itr = aurasA.begin(); itr != aurasA.end(); ++itr)
                    {
                        if (((*itr)->GetCasterGUID() != caster->GetGUID())
                            || (*itr)->GetSpellInfo()->SpellFamilyName != SPELLFAMILY_MAGE)
                            continue;

                        caster->AddAuraForTarget((*itr)->GetBase(), GetHitUnit());
                    }
                }
            }

            void Register()
            {
                OnObjectAreaTargetSelect += SpellObjectAreaTargetSelectFn(spell_mage_impact_SpellScript::FilterTargets, EFFECT_1, TARGET_UNIT_DEST_AREA_ENEMY);
                OnEffectHitTarget += SpellEffectFn(spell_mage_impact_SpellScript::HandleScriptEffect, EFFECT_1, SPELL_EFFECT_SCRIPT_EFFECT);
            }
        };

        SpellScript* GetSpellScript() const
        {
            return new spell_mage_impact_SpellScript();
        }
};

// Arcane potency
class spell_mage_arcane_potency : public SpellScriptLoader
{
    public:
        spell_mage_arcane_potency() : SpellScriptLoader("spell_mage_arcane_potency") { }

        class spell_mage_arcane_potency_SpellScript : public SpellScript
        {
            PrepareSpellScript(spell_mage_arcane_potency_SpellScript);

            void HandleOnHit()
            {
                if (AuraEffect* aPotency = GetCaster()->GetDummyAuraEffect(SPELLFAMILY_MAGE, 2120, EFFECT_0))
                {
                    uint32 triggerId = 0;
                    switch (aPotency->GetId())
                    {
                        case 31571:
                            triggerId = 57529;
                            break;
                        case 31572:
                            triggerId = 57531;
                            break;
                    }
                    if (triggerId)
                        GetCaster()->CastSpell(GetCaster(), triggerId, true);
                }
            }

            void Register()
            {
                OnHit += SpellHitFn(spell_mage_arcane_potency_SpellScript::HandleOnHit);
            }
        };

        SpellScript* GetSpellScript() const
        {
            return new spell_mage_arcane_potency_SpellScript();
        }
};

// 43987 Ritual of Refreshment
/// Updated 4.3.4
class spell_mage_ritual_of_refreshment : public SpellScriptLoader
{
    public:
        spell_mage_ritual_of_refreshment() : SpellScriptLoader("spell_mage_ritual_of_refreshment") { }

        class spell_mage_ritual_of_refreshment_SpellScript : public SpellScript
        {
            PrepareSpellScript(spell_mage_ritual_of_refreshment_SpellScript);

            bool Validate(SpellInfo const* /*spellInfo*/)
            {
                if (!sSpellMgr->GetSpellInfo(SPELL_MAGE_RITUAL_OF_REFRESHMENT_R1))
                    return false;
                if (!sSpellMgr->GetSpellInfo(SPELL_MAGE_RITUAL_OF_REFRESHMENT_R2))
                    return false;
                if (!sSpellMgr->GetSpellInfo(SPELL_MAGE_RITUAL_OF_REFRESHMENT_R3))
                    return false;
                return true;
            }

            void HandleDummy(SpellEffIndex /*effIndex*/)
            {
                if (Unit* caster = GetCaster())
                    if (caster->GetTypeId() == TYPEID_PLAYER)
                    {
                        caster->ToPlayer()->RemoveSpellCooldown(SPELL_MAGE_RITUAL_OF_REFRESHMENT_R1, true); // Rank 1
                        caster->ToPlayer()->RemoveSpellCooldown(SPELL_MAGE_RITUAL_OF_REFRESHMENT_R2, true); // Rank 2
                        caster->ToPlayer()->RemoveSpellCooldown(SPELL_MAGE_RITUAL_OF_REFRESHMENT_R3, true); // Rank 3

                        if (caster->getLevel() >= 85)
                            caster->CastSpell(caster, SPELL_MAGE_RITUAL_OF_REFRESHMENT_R3, true);
                        else if (caster->getLevel() > 80)
                            caster->CastSpell(caster, SPELL_MAGE_RITUAL_OF_REFRESHMENT_R2, true);
                        else if (caster->getLevel() > 75)
                            caster->CastSpell(caster, SPELL_MAGE_RITUAL_OF_REFRESHMENT_R1, true);
                    }
            }

            void Register()
            {
                OnEffectLaunch += SpellEffectFn(spell_mage_ritual_of_refreshment_SpellScript::HandleDummy, EFFECT_0, SPELL_EFFECT_DUMMY);
            }
        };

        SpellScript* GetSpellScript() const
        {
            return new spell_mage_ritual_of_refreshment_SpellScript();
        }
};

class spell_mage_pet_scaling_05 : public SpellScriptLoader
{
public:
    spell_mage_pet_scaling_05() : SpellScriptLoader("spell_mage_pet_scaling_05") { }

    class spell_mage_pet_scaling_05_AuraScript : public AuraScript
    {
        PrepareAuraScript(spell_mage_pet_scaling_05_AuraScript);

        bool Load()
        {
            if (!GetCaster() || !GetCaster()->ToTempSummon() || !GetCaster()->ToTempSummon()->GetSummoner() || GetCaster()->ToTempSummon()->GetSummoner()->GetTypeId() != TYPEID_PLAYER)
                return false;
            return true;
        }

        void CalculateAmountCritPct(AuraEffect const* /* aurEff */, int32& amount, bool& /*canBeRecalculated*/)
        {
            if (Player* owner = GetCaster()->ToTempSummon()->GetSummoner()->ToPlayer())
            {
                float CritSpell = owner->GetSpellCritFromIntellect();
                CritSpell += owner->GetTotalAuraModifier(SPELL_AURA_MOD_SPELL_CRIT_CHANCE);
                CritSpell += owner->GetTotalAuraModifier(SPELL_AURA_MOD_CRIT_PCT);
                CritSpell += owner->GetRatingBonusValue(CR_CRIT_SPELL);
                amount += int32(CritSpell);
            }
        }

        void CalculateAmountHaste(AuraEffect const* /* aurEff */, int32& amount, bool& /*canBeRecalculated*/)
        {
            if (!GetCaster() || !GetCaster()->GetOwner())
                return;
            if (Player* owner = GetCaster()->ToTempSummon()->GetSummoner()->ToPlayer())
            {
                float meleeHaste = (1.0f - owner->m_modAttackSpeedPct[BASE_ATTACK]) * 100.0f;
                amount += int32(meleeHaste);
            }
        }

        void CalculateAmountResistance(AuraEffect const* /* aurEff */, int32& /*amount*/, bool& /*canBeRecalculated*/)
        {

        }

        void Register()
        {
            DoEffectCalcAmount += AuraEffectCalcAmountFn(spell_mage_pet_scaling_05_AuraScript::CalculateAmountCritPct, EFFECT_0, SPELL_AURA_MOD_CRIT_PCT);
            DoEffectCalcAmount += AuraEffectCalcAmountFn(spell_mage_pet_scaling_05_AuraScript::CalculateAmountHaste, EFFECT_1, SPELL_AURA_MELEE_SLOW);
            DoEffectCalcAmount += AuraEffectCalcAmountFn(spell_mage_pet_scaling_05_AuraScript::CalculateAmountResistance, EFFECT_2, SPELL_AURA_MOD_TARGET_RESISTANCE);
        }
    };

    AuraScript* GetAuraScript() const
    {
        return new spell_mage_pet_scaling_05_AuraScript();
    }
};

// 66 - Invisibility (Fading)
class spell_mage_invisibility_fading : public SpellScriptLoader
{
public:
	spell_mage_invisibility_fading() : SpellScriptLoader("spell_mage_invisibility_fading") { }

	class spell_mage_invisibility_fading_AuraScript : public AuraScript
	{
		PrepareAuraScript(spell_mage_invisibility_fading_AuraScript);

		bool Validate(SpellInfo const* /*spellInfo*/)
		{
			if (!sSpellMgr->GetSpellInfo(SPELL_MAGE_INVISIBILITY_FADING))
				return false;
			return true;
		}

		void HandleApply(AuraEffect const* /*aurEff*/, AuraEffectHandleModes /*mode*/)
		{
			// Cast Invisibility on Elemental
			if (Unit* player = GetTarget()->ToPlayer())
			{
				if (Guardian* elemental = player->ToPlayer()->GetGuardianPet())
				{
					elemental->AttackStop();
					elemental->InterruptNonMeleeSpells(false);
					elemental->SendMeleeAttackStop();
					elemental->AddAura(SPELL_MAGE_INVISIBILITY_FADING, elemental);
				}
			}
		}

		void Register()
		{
			OnEffectApply += AuraEffectRemoveFn(spell_mage_invisibility_fading_AuraScript::HandleApply, EFFECT_1, SPELL_AURA_PERIODIC_TRIGGER_SPELL, AURA_EFFECT_HANDLE_REAL);
		}
	};

	AuraScript* GetAuraScript() const
	{
		return new spell_mage_invisibility_fading_AuraScript();
	}
};

// 32612 - Invisibility (Invisible)
class spell_mage_invisibility_invisible : public SpellScriptLoader
{
public:
	spell_mage_invisibility_invisible() : SpellScriptLoader("spell_mage_invisibility_invisible") { }

	class spell_mage_invisibility_invisible_AuraScript : public AuraScript
	{
		PrepareAuraScript(spell_mage_invisibility_invisible_AuraScript);

		bool Validate(SpellInfo const* /*spellInfo*/)
		{
			if (!sSpellMgr->GetSpellInfo(SPELL_MAGE_INVISIBILITY_INVISIBLE))
				return false;
			return true;
		}

		void RemoveEffect(AuraEffect const* /*aurEff*/, AuraEffectHandleModes /*mode*/)
		{
			if (Unit* target = GetTarget())
			{
				if (target->isGuardian())
				{
					// Remove Invisibility from mage when elemental does an action
					if (Unit* owner = target->GetOwner())
						owner->RemoveAura(SPELL_MAGE_INVISIBILITY_INVISIBLE);
				}
				else
				{
					// Remove Invisibility from elemental when mage does an action
					if (Guardian* elemental = target->ToPlayer()->GetGuardianPet())
						elemental->RemoveAura(SPELL_MAGE_INVISIBILITY_INVISIBLE);
				}
			}
		}

		void Register()
		{
			OnEffectRemove += AuraEffectRemoveFn(spell_mage_invisibility_invisible_AuraScript::RemoveEffect, EFFECT_1, SPELL_AURA_MOD_INVISIBILITY, AURA_EFFECT_HANDLE_REAL);
		}
	};

	AuraScript* GetAuraScript() const
	{
		return new spell_mage_invisibility_invisible_AuraScript();
	}
};


void AddSC_mage_spell_scripts()
{
    new spell_mage_blast_wave();
    new spell_mage_blizzard();
    new spell_mage_burnout();
    new spell_mage_cold_snap();
    new spell_mage_cone_of_cold();
    new spell_mage_conjure_refreshment();
    new spell_mage_focus_magic();
    new spell_mage_frostbolt();
    new spell_mage_ice_barrier();
    new spell_mage_living_bomb();
    new spell_mage_mage_ward();
    new spell_mage_mana_shield();
    new spell_mage_polymorph_cast_visual();
    new spell_mage_replenish_mana();
    new spell_mage_summon_water_elemental();
    new spell_mage_water_elemental_freeze();
    new spell_mage_piercing_chill();
    new spell_mage_cauterize();
    new spell_mage_ring_of_frost();
    new spell_mage_ring_of_frost_freeze();
    new spell_mage_polymorph();
    new spell_mage_arcane_potency();
    new spell_mage_combustion();
    new spell_mage_pyromaniac();
    new spell_mage_impact();
    new spell_mage_orb_filter();
    new spell_mage_ffb();
    new spell_mage_ritual_of_refreshment();
    new spell_mage_pet_scaling_05();
    new spell_mage_icy_veins();
	new spell_mage_invisibility_fading();
	new spell_mage_invisibility_invisible();
}