/*
 * Copyright (C) 2011-2012 Project SkyFire <http://www.projectskyfire.org/>
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License as published by the
 * Free Software Foundation; either version 3 of the License, or (at your
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

enum eCorlaYells
{
    SAY_AGGRO                   = 0,
    SAY_EVOLVE                  = 1,
    SAY_KILL                    = 2,
    SAY_DEATH                   = 3,
};

enum eCorlaSpells
{
    NPC_CHANNEL_TRIGGER         = 39842,
    SPELL_DRAIN_ESSENCE         = 75645, // Cast from Corla on trigger.

    SPELL_KNEELING              = 75608,

    SPELL_DARK_COMMAND          = 75823,
    SPELL_AURA_OF_ACCELERATION  = 75817,

    SPELL_EVOLUTION_TRIGGER     = 75610,

    SPELL_EVOLUTION             = 75697, // Stacks on player / NPC to 100 and transfers them
    SPELL_EVOLUTION_HEROIC      = 87378,

    SPELL_TWILIGHT_EVOLUTION    = 75732, // Cast from Corla on player after 100 stack (transforms to enemy)

    SPELL_BEAM_VISUAL           = 75677,
    SPELL_BEAM_PERIODIC         = 75706, // Triggers channel and periodic
};

class boss_corla_herald_of_twilight: public CreatureScript
{
public:
    boss_corla_herald_of_twilight() : CreatureScript("boss_corla_herald_of_twilight") { }

    CreatureAI* GetAI(Creature* creature) const
    {
        return new boss_corlaAI (creature);
    }

    struct boss_corlaAI : public ScriptedAI
    {
        boss_corlaAI(Creature* creature) : ScriptedAI(creature)
        {
            instance = creature->GetInstanceScript();
        }

        InstanceScript* instance;
        uint32 DarkCommandTimer;

        void Reset()
        {
            if (Creature * trigger = GetClosestCreatureWithEntry(me, NPC_CHANNEL_TRIGGER, 100.0f))
            {
                me->SetUInt64Value(UNIT_FIELD_CHANNEL_OBJECT, trigger->GetGUID());
                me->SetUInt32Value(UNIT_CHANNEL_SPELL, 75645);
            }

            SetAddsChanneling(false);

            DarkCommandTimer = urand(10000, 20000);
        }

        void KilledUnit(Unit* /*victim*/)
        {
            Talk(SAY_KILL);
        }

        void JustDied(Unit* /*Kill*/)
        {
            Talk(SAY_DEATH);

            std::list<Creature*> toRespawn;

            GetCreatureListWithEntryInGrid(toRespawn, me, NPC_TWILIGHT_ZEALOT_BOSS, 50.0f);
            if (!toRespawn.empty())
                for(std::list<Creature*>::iterator itr = toRespawn.begin(); itr != toRespawn.end(); ++itr)
                    (*itr)->DespawnOrUnsummon();

            toRespawn.clear();

            GetCreatureListWithEntryInGrid(toRespawn, me, NPC_EVOLVED_TWILIGHT_ZEALOT, 50.0f);
            if (!toRespawn.empty())
                for(std::list<Creature*>::iterator itr2 = toRespawn.begin(); itr2 != toRespawn.end(); ++itr2)
                        (*itr2)->DespawnOrUnsummon();

            if (instance)
                instance->SetData(BOSS_CORLA, DONE);
        }

        void EnterCombat(Unit* /*Ent*/)
        {
            Talk(SAY_AGGRO);

            if (instance)
                instance->SetData(BOSS_CORLA, IN_PROGRESS);

            me->SetUInt64Value(UNIT_FIELD_CHANNEL_OBJECT, 0);
            me->SetUInt32Value(UNIT_CHANNEL_SPELL, 0);

            me->CastSpell(me, SPELL_AURA_OF_ACCELERATION, true);

            SetAddsChanneling(true);
        }

        void SetAddsChanneling(bool apply)
        {
            std::list<Creature*> zaelots;
            GetCreatureListWithEntryInGrid(zaelots, me, NPC_TWILIGHT_ZEALOT_BOSS, 50.0f);
            if (!zaelots.empty())
            {
                for(std::list<Creature*>::iterator itr = zaelots.begin(); itr != zaelots.end(); ++itr)
                {
                    if (apply)
                        (*itr)->CastSpell((*itr), SPELL_BEAM_PERIODIC, true);
                    else
                        (*itr)->RemoveAurasDueToSpell(SPELL_BEAM_PERIODIC);
                }
            }

            if(!apply)
            {
                std::list<Creature*> evolvedZaelots;
                GetCreatureListWithEntryInGrid(evolvedZaelots, me, NPC_EVOLVED_TWILIGHT_ZEALOT, 100.0f);
                if (!evolvedZaelots.empty())
                {
                    for(std::list<Creature*>::iterator itr2 = evolvedZaelots.begin(); itr2 != evolvedZaelots.end(); ++itr2)
                    {
                        (*itr2)->Respawn(true);
                        (*itr2)->CastSpell(*itr2, SPELL_KNEELING, true);
                    }
                }
            }
        }

        void UpdateAI(uint32 const Diff)
        {
            if (!UpdateVictim())
                return;

            if (DarkCommandTimer <= Diff)
            {
                if (me->IsNonMeleeSpellCasted(false))
                    return;

                if (Unit * pTarget = SelectTarget(SELECT_TARGET_RANDOM, 0, 100.0f, true))
                    me->CastSpell(pTarget, SPELL_DARK_COMMAND, false);

                DarkCommandTimer = urand(15000, 25000);
            }
            else
                DarkCommandTimer -= Diff;

            DoMeleeAttackIfReady();
        }
    };
};

enum eCorlaTriggerSpells
{
    SPELL_NETHER_DRAGON_ESSENCE     = 75649,
    SPELL_NETHER_DRAGON_ESSENCE_1   = 75650,
    SPELL_NETHER_DRAGON_ESSENCE_2   = 75653,
    SPELL_NETHER_DRAGON_ESSENCE_3   = 75654,
};

static uint32 SpellNetherDrakeVisual[3]  = {
    SPELL_NETHER_DRAGON_ESSENCE_1,
    SPELL_NETHER_DRAGON_ESSENCE_2,
    SPELL_NETHER_DRAGON_ESSENCE_3
};

class mob_zaelot_trigger: public CreatureScript
{
public:
    mob_zaelot_trigger() : CreatureScript("mob_zaelot_trigger") { }

    CreatureAI* GetAI(Creature* creature) const
    {
        return new mob_zaelot_triggerAI (creature);
    }

    struct mob_zaelot_triggerAI : public ScriptedAI
    {
        mob_zaelot_triggerAI(Creature* creature) : ScriptedAI(creature)
        {
        }

        uint8 VisualCounter;
        uint32 VisualCastTimer;

        void Reset()
        {
            VisualCounter = 0;
            VisualCastTimer = 1000;
        }

        void UpdateAI(uint32 const Diff)
        {
            if (VisualCastTimer <= Diff)
            {
                me->CastSpell(me, SpellNetherDrakeVisual[VisualCounter], true);
                if (++VisualCounter == 3)
                    VisualCounter = 0;

                VisualCastTimer = 3000;
            }
            else VisualCastTimer -= Diff;

            if (!UpdateVictim())
                return;
        }
    };
};

class spell_corla_beam_target_check : public SpellScriptLoader
{
    public:
        spell_corla_beam_target_check() : SpellScriptLoader("spell_corla_beam_target_check") { }

        class spell_corla_beam_target_check_SpellScript : public SpellScript
        {
            PrepareSpellScript(spell_corla_beam_target_check_SpellScript);

            void CheckTarget()
            {
                if (Unit * target = GetHitUnit())
                {
                    if (Creature * trigger = GetClosestCreatureWithEntry(GetHitUnit(), NPC_CHANNEL_TRIGGER, 100.0f))
                    {
                        target->CastSpell(trigger, SPELL_BEAM_VISUAL, true);
                        if (Aura * aura = target->GetAura(target->GetMap()->IsHeroic() ? SPELL_EVOLUTION_HEROIC : SPELL_EVOLUTION))
                        {
                            aura->ModStackAmount(2); // Aura mods stack twice in time of 400 ms
                            if (aura->GetStackAmount() >= 100) // Convert player / creature on 100 stacks
                                if (Creature * corla = GetClosestCreatureWithEntry(target, BOSS_CORLA, 100.0f))
                                {
                                    if (target->GetTypeId() == TYPEID_PLAYER)
                                        corla->CastSpell(target, SPELL_TWILIGHT_EVOLUTION, true);
                                    else
                                    {
                                        ((Creature*)target)->RemoveAurasDueToSpell(SPELL_BEAM_PERIODIC);
                                        ((Creature*)target)->RemoveAurasDueToSpell(SPELL_KNEELING);
                                        ((Creature*)target)->UpdateEntry(NPC_EVOLVED_TWILIGHT_ZEALOT);
                                    }
                                }
                        }
                        else
                            target->CastSpell(target, SPELL_EVOLUTION, true);
                    }
                }
            }

            void FilterTargets(std::list<WorldObject*>& unitList)
            {
                if (Unit * caster = GetCaster())
                {
                    if (unitList.empty()) // Push caster if no targets found
                    {
                        unitList.push_back(caster);
                    }
                    else
                    {
                        unitList.sort(Trinity::ObjectDistanceOrderPred(caster, false));
                        unitList.resize(1);
                    }
                }
            }

            void Register()
            {
                OnObjectAreaTargetSelect += SpellObjectAreaTargetSelectFn(spell_corla_beam_target_check_SpellScript::FilterTargets, EFFECT_0, TARGET_UNIT_CONE_ENTRY);
                OnHit += SpellHitFn(spell_corla_beam_target_check_SpellScript::CheckTarget);
            }
        };

        SpellScript* GetSpellScript() const
        {
            return new spell_corla_beam_target_check_SpellScript();
        }
};

void AddSC_boss_corla_herald_of_twilight()
{
    new boss_corla_herald_of_twilight();
    new mob_zaelot_trigger();
    new spell_corla_beam_target_check();
}