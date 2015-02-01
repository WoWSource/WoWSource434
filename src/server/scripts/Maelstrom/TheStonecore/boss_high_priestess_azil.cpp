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

/*
* TODO: grip and seismic shard part is unfinished...
*/
#include "the_stonecore.h"
#include "ScriptPCH.h"
#include "MoveSplineInit.h"
#include "Vehicle.h"
#include "ScriptedCreature.h"

enum Spells
{
	SPELL_CURSE_OF_BLOOD = 79345,
	SPELL_FORCE_GRIP = 79351,
	SPELL_FORCE_GRIP_DOWN = 79359,
	SPELL_FORCE_GRIP_UP = 79358,
	SPELL_SUMMON_GRAVITY_WELL = 79340,
	SPELL_SEISMIC_SHARD = 79002, // visual
	SPELL_SEISMIC_SHARD_CHARGE = 79014, // damage + leap
	SPELL_SEISMIC_SHARD_PULL = 86862, // pulls the shard -> makes it enter vehicle
	SPELL_SEISMIC_SHARD_TAR = 80511, // target visual
	SPELL_SEISMIC_SHARD_THROW = 79015, // throw visual
	SPELL_SEISMIC_SHARD_SUMM_1 = 86856, // summons shards
	SPELL_SEISMIC_SHARD_SUMM_2 = 86858,
	SPELL_SEISMIC_SHARD_SUMM_3 = 86860,
	SPELL_SEISMIC_SHARD_VISUAL = 79009,
	SPELL_ENERGY_SHIELD = 82858,

	SPELL_GRAVITY_WELL_VIS_1 = 79245, // after 8 sec - removed
	SPELL_GRAVITY_WELL_PERIODIC = 79244,
	SPELL_GRAVITY_WELL_SCRIPT = 79251,
	SPELL_GRAVITY_WELL_DMG = 79249,
	SPELL_GRAVITY_WELL_PULL = 79333,
	SPELL_GRAVITY_WELL_SCALE = 92475, // hc only

	SPELL_RIDE_VEHICLE = 46598,
};

enum Events
{
	EVENT_CURSE_OF_BLOOD = 1,
	EVENT_FORCE_GRIP = 2,
	EVENT_SEISMIC_SHARD = 3,
	EVENT_SEISMIC_SHARD_THROW = 4,
	EVENT_SHIELD_PHASE_END = 5,
	EVENT_GRAVITY_WELL = 6,
	EVENT_ENERGY_SHIELD = 7,
	EVENT_ENERGY_SHIELD_END = 8,
	EVENT_ADDS_SUMMON = 9,
};

enum Phases
{
	PHASE_NORMAL = 1,
	PHASE_SHIELD = 2
};

enum Misc
{
	VEHICLE_GRIP = 892,
	VEHICLE_NORMAL = 903,
	POINT_FLY = 1,
	POINT_PLATFORM = 2,
};

enum Quotes
{
	SAY_AGGRO = 1,
	SAY_DEATH = 2,
	SAY_SLAY = 3,
	SAY_SHIELD = 4
};

static const Position summonPos[2] =
{
	{ 1271.93f, 1042.73f, 210.0f, 0.0f }, // W
	{ 1250.99f, 949.48f, 205.5f, 0.0f }   // E
};

class boss_azil : public CreatureScript
{
public:
	boss_azil() : CreatureScript("boss_priestess_azil") {}

	struct boss_azilAI : public BossAI
	{
		boss_azilAI(Creature * creature) : BossAI(creature, DATA_HIGH_PRIESTESS_AZIL), vehicle(creature->GetVehicleKit())
		{
			ASSERT(vehicle);
		}

		void Reset()
		{
			seismicShards = 0;
			me->SetReactState(REACT_AGGRESSIVE);
			me->RemoveFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_DISABLE_MOVE);
			_Reset();
		}

		void EnterCombat(Unit * /*victim*/)
		{
			Talk(SAY_AGGRO);
			me->GetMotionMaster()->MoveJump(1332.59f, 983.41f, 207.62f, 10.0f, 10.0f);
			events.SetPhase(PHASE_NORMAL);
			events.ScheduleEvent(EVENT_FORCE_GRIP, 10000);
			events.ScheduleEvent(EVENT_ENERGY_SHIELD, 45000);
			events.ScheduleEvent(EVENT_CURSE_OF_BLOOD, urand(5000, 8000));
			events.ScheduleEvent(EVENT_GRAVITY_WELL, urand(3000, 5000));
			events.ScheduleEvent(EVENT_ADDS_SUMMON, urand(10000, 15000));
			_EnterCombat();
		}

		void SpellHit(Unit * /*caster*/, const SpellInfo *spell)
		{
			Spell * curSpell = me->GetCurrentSpell(CURRENT_GENERIC_SPELL);
			if (curSpell && curSpell->m_spellInfo->Id == SPELL_FORCE_GRIP)
			for (uint8 i = 0; i < 3; ++i)
			if (spell->Effects[i].Effect == SPELL_EFFECT_INTERRUPT_CAST)
				me->InterruptSpell(CURRENT_GENERIC_SPELL, false);
		}

		void JustSummoned(Creature * summon)
		{
			BossAI::JustSummoned(summon);
		}

		void KilledUnit(Unit * victim)
		{
			if (victim->GetTypeId() == TYPEID_PLAYER)
				Talk(SAY_SLAY);
		}

		void JustDied(Unit * /*killer*/)
		{
			Talk(SAY_DEATH);
			_JustDied();
		}

		void MovementInform(uint32 type, uint32 id)
		{
			switch (id)
			{
			case POINT_PLATFORM:
				events.ScheduleEvent(EVENT_SHIELD_PHASE_END, 30000);
				Movement::MoveSplineInit init(me);
				init.SetFacing(me->GetHomePosition().GetOrientation());
				DoCast(SPELL_SEISMIC_SHARD);
				events.ScheduleEvent(EVENT_SEISMIC_SHARD, 4000);
				if (vehicle)
				{
					first = true;
					vehicle->InstallAccessory(NPC_SEISMIC_SHARD, 0, true, TEMPSUMMON_DEAD_DESPAWN, 0);
					vehicle->InstallAccessory(NPC_SEISMIC_SHARD, 1, true, TEMPSUMMON_DEAD_DESPAWN, 0);
				}
				break;
			}
		}

		void OnInstallAccessory(Vehicle* veh, Creature* accessory)
		{
			accessory->CastSpell(accessory, SPELL_SEISMIC_SHARD_VISUAL);
			accessory->setActive(true);
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
				case EVENT_CURSE_OF_BLOOD:
					DoCastVictim(SPELL_CURSE_OF_BLOOD);
					events.ScheduleEvent(EVENT_CURSE_OF_BLOOD, urand(8000, 10000), 0, PHASE_NORMAL);
					break;
				case EVENT_FORCE_GRIP:
					vehicle->SetVehicleId(VEHICLE_GRIP);
					DoCastVictim(SPELL_FORCE_GRIP);
					events.ScheduleEvent(EVENT_FORCE_GRIP, urand(15000, 20000), 0, PHASE_NORMAL);
					break;
				case EVENT_GRAVITY_WELL:
					DoCastRandom(SPELL_SUMMON_GRAVITY_WELL, 100.0f, false);
					events.ScheduleEvent(EVENT_GRAVITY_WELL, urand(15000, 20000), 0, PHASE_NORMAL);
					break;
				case EVENT_SEISMIC_SHARD:
					if (Unit * target = SelectTarget(SELECT_TARGET_RANDOM, 0, 0.0f, true))
					{
						target->GetPosition(&shardPos);
						DoCast(SPELL_SEISMIC_SHARD_PULL);
						me->CastSpell(shardPos.GetPositionX(), shardPos.GetPositionY(), shardPos.GetPositionZ(), SPELL_SEISMIC_SHARD_TAR, false);
						DoCast(SPELL_SEISMIC_SHARD_THROW);
						events.ScheduleEvent(EVENT_SEISMIC_SHARD_THROW, 3000);
					}
					if (seismicShards < 2)
					{
						++seismicShards;
						events.ScheduleEvent(EVENT_SEISMIC_SHARD, 7000);
					}
					break;
				case EVENT_SEISMIC_SHARD_THROW:
					if (Unit* passenger = vehicle->GetPassenger((first ? 0 : 1)))
					{
						if (passenger)
						{
							passenger->ExitVehicle();
							passenger->CastSpell(shardPos.GetPositionX(), shardPos.GetPositionY(), shardPos.GetPositionZ(), SPELL_SEISMIC_SHARD_CHARGE, false);
							passenger->ToCreature()->DespawnOrUnsummon(3000);
							first = false;
						}
					}
					break;
				case EVENT_ENERGY_SHIELD:
					Talk(SAY_SHIELD);
					me->SetFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_DISABLE_MOVE);
					me->GetMotionMaster()->Clear();
					me->GetMotionMaster()->MoveIdle();
					vehicle->SetVehicleId(VEHICLE_NORMAL);
					me->SetReactState(REACT_PASSIVE);
					me->SetUInt64Value(UNIT_FIELD_TARGET, 0);
					events.SetPhase(PHASE_SHIELD);
					DoCast(SPELL_ENERGY_SHIELD);
					seismicShards = 0;
					events.ScheduleEvent(EVENT_ENERGY_SHIELD_END, 2000);
					break;
				case EVENT_ENERGY_SHIELD_END: // fly up
					me->GetMotionMaster()->MoveJump(me->GetHomePosition(), 30.0f, 30.0f, POINT_PLATFORM);
					break;
				case EVENT_SHIELD_PHASE_END:
					me->RemoveFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_DISABLE_MOVE);
					me->RemoveAurasDueToSpell(SPELL_SEISMIC_SHARD);
					me->RemoveAurasDueToSpell(SPELL_ENERGY_SHIELD);
					if (Unit * victim = me->GetVictim())
					{
						me->SetReactState(REACT_AGGRESSIVE);
						DoStartMovement(victim);
					}
					events.SetPhase(PHASE_NORMAL);
					events.RescheduleEvent(EVENT_CURSE_OF_BLOOD, urand(8000, 10000), 0, PHASE_NORMAL);
					events.RescheduleEvent(EVENT_GRAVITY_WELL, urand(15000, 20000), 0, PHASE_NORMAL);
					events.RescheduleEvent(EVENT_FORCE_GRIP, urand(10000, 12000), 0, PHASE_NORMAL);
					events.ScheduleEvent(EVENT_ENERGY_SHIELD, urand(40000, 45000), 0, PHASE_NORMAL);
					break;
				case EVENT_ADDS_SUMMON:
				{
										  if (Unit* target = SelectTarget(SELECT_TARGET_RANDOM))
										  {
											  uint8 amount = 3;
											  uint8 pos = urand(0, 1);
											  if (events.IsInPhase(PHASE_SHIELD))
												  amount = urand(8, 10);
											  for (int i = 0; i < amount; ++i)
											  {
												  Position tarPos;
												  me->GetRandomPoint(summonPos[pos], 5.0f, tarPos);
												  if (Creature * summon = me->SummonCreature(NPC_FOLLOWER, tarPos, TEMPSUMMON_DEAD_DESPAWN, 1000))
												  {
													  summon->AI()->AttackStart(target);
													  summon->AI()->DoZoneInCombat();
												  }
											  }
										  }
										  events.ScheduleEvent(EVENT_ADDS_SUMMON, urand(10000, 12000));
				}
					break;
				default:
					break;
				}
			}

			DoMeleeAttackIfReady();
		}
	private:
		bool first;
		Vehicle* vehicle;
		uint8 seismicShards;
		Position shardPos;
	};

	CreatureAI * GetAI(Creature * creature) const
	{
		return new boss_azilAI(creature);
	}
};

class npc_gravity_well_azil : public CreatureScript
{
public:
	npc_gravity_well_azil() : CreatureScript("npc_gravity_well_azil") {}

	struct npc_gravity_well_azilAI : public ScriptedAI
	{
		npc_gravity_well_azilAI(Creature * creature) : ScriptedAI(creature) {}

		void Reset()
		{
			DoCast(SPELL_GRAVITY_WELL_VIS_1);
			active = false;
			activeTimer = 8000;
			if (!IsHeroic())
				me->DespawnOrUnsummon(20000);
			killCount = 0;
		}

		void SpellHitTarget(Unit * target, const SpellInfo * spell)
		{
			if (target->isAlive() && spell->Id == SPELL_GRAVITY_WELL_SCRIPT)
			{
				int bp = IsHeroic() ? 20000 : 10000; // evtl needs to be increased / lowered
				uint32 distFkt = uint32(me->GetDistance(target)) * 5;
				bp -= (bp * distFkt) / 100;

				me->CastCustomSpell(target, SPELL_GRAVITY_WELL_DMG, &bp, NULL, NULL, true);
			}
		}

		void KilledUnit(Unit * victim)
		{
			if (IsHeroic() && victim->GetEntry() == NPC_FOLLOWER)
			{
				if (killCount == 3)
					me->DespawnOrUnsummon();
				else
				{
					DoCast(me, SPELL_GRAVITY_WELL_SCALE, true);
					++killCount;
				}
			}
		}

		void UpdateAI(uint32 const diff)
		{
			if (!active)
			{
				if (activeTimer <= diff)
				{
					active = true;
					me->RemoveAurasDueToSpell(SPELL_GRAVITY_WELL_VIS_1);
					DoCast(me, SPELL_GRAVITY_WELL_PERIODIC, true);
					DoCast(me, SPELL_GRAVITY_WELL_PULL, true);
				}
				else activeTimer -= diff;
			}
		}

	private:
		bool active;
		uint32 activeTimer;
		uint8 killCount;
	};

	CreatureAI * GetAI(Creature * creature) const
	{
		return new npc_gravity_well_azilAI(creature);
	}
};

void AddSC_boss_azil()
{
	new boss_azil();
	new npc_gravity_well_azil();
}
