/*
* Copyright (C) 2008-2013 TrinityCore <http://www.trinitycore.org/>
*
* This program is free software; you can redistribute it and/or modify
* it under the terms of the GNU General Public License as published by
* the Free Software Foundation; either version 2 of the License, or
* (at your option) any later version.
*
* This program is distributed in the hope that it will be useful,
* but WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
* GNU General Public License for more details.
*
* You should have received a copy of the GNU General Public License
* along with this program; if not, write to the Free Software
* Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA
*/

#include "the_stonecore.h"
#include "ScriptPCH.h"

enum Spells
{
	SPELL_RUPTURE_PERIODIC = 92393,
	SPELL_SPIKE_SHIELD = 78835,
	SPELL_ENRAGE = 80467,
	SPELL_SHATTER = 78807,
	SPELL_GROUND_SLAM = 78903,
	SPELL_BULWARK = 78939,
	SPELL_PARALYZE = 92426,

	SPELL_RUPTURE_DAM = 92381,
};

enum Events
{
	EVENT_BULWARK = 1,
	EVENT_GROUND_SLAM = 2,
	EVENT_GROUND_SLAM_END = 3,
	EVENT_SHATTER = 4,
	EVENT_PARALYZE = 5,
	EVENT_SPIKE_SHIELD = 6
};

enum Quotes
{
	SAY_AGGRO = 1,
	SAY_SHIELD = 2,
	SAY_SLAY = 3,
	SAY_DEATH = 4

};

class boss_ozruk : public CreatureScript
{
public:
	boss_ozruk() : CreatureScript("boss_ozruk") {}

	struct boss_ozrukAI : public BossAI
	{
		boss_ozrukAI(Creature * creature) : BossAI(creature, DATA_OZRUK) {}

		void Reset()
		{
			enraged = false;
			_Reset();
		}

		void EnterCombat(Unit * /*who*/)
		{
			Talk(SAY_AGGRO);
			events.ScheduleEvent(EVENT_BULWARK, 10000);
			events.ScheduleEvent(EVENT_GROUND_SLAM, 30000);
			events.ScheduleEvent(EVENT_SPIKE_SHIELD, 15000);
			if (!IsHeroic())
				events.ScheduleEvent(EVENT_SHATTER, 10000);
			_EnterCombat();
		}

		void JustSummoned(Creature * summon)
		{
			if (summon->GetEntry() == NPC_RUPTURE_CONTROLLER)
			{
				summon->CastSpell(summon, SPELL_RUPTURE_PERIODIC, false);
				summon->DespawnOrUnsummon(5000);
			}
		}

		void JustDied(Unit * /*killer*/)
		{
			Talk(SAY_DEATH);
			_JustDied();
		}

		void KilledUnit(Unit * victim)
		{
			if (victim->GetTypeId() == TYPEID_PLAYER)
				Talk(SAY_SLAY);
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
				case EVENT_BULWARK:
					DoCast(SPELL_BULWARK);
					events.ScheduleEvent(EVENT_BULWARK, urand(30000, 35000));
					break;
				case EVENT_GROUND_SLAM:
					me->SetReactState(REACT_PASSIVE);
					me->GetMotionMaster()->Clear();
					me->GetMotionMaster()->MoveIdle();
					me->SetUInt64Value(UNIT_FIELD_TARGET, 0);
					me->SetFacingToObject(me->GetVictim());
					DoCast(SPELL_GROUND_SLAM);
					events.ScheduleEvent(EVENT_GROUND_SLAM, 30000);
					events.ScheduleEvent(EVENT_GROUND_SLAM_END, 4500);
					break;
				case EVENT_GROUND_SLAM_END:
					me->SetReactState(REACT_AGGRESSIVE);
					if (Unit * victim = me->GetVictim())
					{
						me->SetUInt64Value(UNIT_FIELD_TARGET, victim->GetGUID());
						DoStartMovement(victim);
					}
					break;
				case EVENT_SHATTER:
					DoCast(SPELL_SHATTER);
					if (!IsHeroic())
						events.ScheduleEvent(EVENT_SHATTER, urand(20000, 25000));
					break;
				case EVENT_PARALYZE:
					DoCast(SPELL_PARALYZE);
					events.ScheduleEvent(EVENT_SHATTER, 3000);
					break;
				case EVENT_SPIKE_SHIELD:
					Talk(SAY_SHIELD);
					DoCast(SPELL_SPIKE_SHIELD);
					events.ScheduleEvent(EVENT_SPIKE_SHIELD, 45000);
					if (IsHeroic())
						events.ScheduleEvent(EVENT_PARALYZE, urand(7000, 8000));
					break;
				default:
					break;
				}
			}

			if (!enraged && me->HealthBelowPct(30))
			{
				enraged = true;
				DoCast(SPELL_ENRAGE);
			}

			DoMeleeAttackIfReady();
		}
	private:
		bool enraged;
	};

	CreatureAI * GetAI(Creature * creature) const
	{
		return new boss_ozrukAI(creature);
	}
};

class spell_rupture_periodic : public SpellScriptLoader
{
public:
	spell_rupture_periodic() : SpellScriptLoader("spell_rupture_periodic") { }

	class spell_rupture_periodic_AuraScript : public AuraScript
	{
		PrepareAuraScript(spell_rupture_periodic_AuraScript)

		void HandleEffectPeriodic(AuraEffect const* aurEff)
		{
			Unit * caster = GetCaster();
			if (!caster)
				return;

			Position pos;
			float dist = aurEff->GetTickNumber() * 5.0f;
			caster->GetNearPosition(pos, dist, 0.0f);

			for (int i = 0; i<3; ++i)
			{
				if (i == 1)
					caster->MovePosition(pos, 3.0f, M_PI / 2);
				else if (i == 2)
					caster->MovePosition(pos, 6.0f, -M_PI / 2);

				if (Creature * creature = caster->SummonCreature(NPC_RUPTURE, pos, TEMPSUMMON_TIMED_DESPAWN, 1000))
					creature->CastSpell(creature, SPELL_RUPTURE_DAM, false);
			}

		}

		void Register()
		{
			OnEffectPeriodic += AuraEffectPeriodicFn(spell_rupture_periodic_AuraScript::HandleEffectPeriodic, EFFECT_0, SPELL_AURA_PERIODIC_TRIGGER_SPELL);
		}
	};

	AuraScript* GetAuraScript() const
	{
		return new spell_rupture_periodic_AuraScript();
	}
};

void AddSC_boss_ozruk()
{
	new boss_ozruk();
	new spell_rupture_periodic();
};
