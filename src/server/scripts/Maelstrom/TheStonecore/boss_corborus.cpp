#include "the_stonecore.h"
#include "SpellAuraEffects.h"
#include "SpellScript.h"
#include "ScriptedCreature.h"
#include "ScriptMgr.h"

enum Spells
{
	SPELL_CLEARALLDEBUFFS = 34098,
	SPELL_CRYSTAL_BARRAGE = 86881,
	SPELL_DAMPENING_WAVE = 82415,
	SPELL_SUBMERGE = 81629,
	SPELL_THRASHING_CHARGE_TELEPORT = 81839,
	SPELL_THRASHING_CHARGE = 81801, // dummy / visual
	SPELL_THRASHING_CHARGE_SUMMON = 81816,
	SPELL_THRASHING_CHARGE_DAMAGE = 81828, // casted by Trigger
	SPELL_SUMMON_BEETLE = 82190,
	SPELL_EMERGE = 82185, // Rock Borer
	SPELL_EMERGE_CORBORUS = 81948,
};

enum Phases
{
	PHASE_NORMAL = 1,
	PHASE_SUBMERGED
};

enum Events
{
	EVENT_CRYSTAL_BARRAGE = 1,
	EVENT_DAMPENING_WAVE,
	EVENT_SUBMERGE,
	EVENT_EMERGE,
	EVENT_EMERGE_END,
	EVENT_THRASHING_CHARGE,
	EVENT_THRASHING_CHARGE_CAST,
	EVENT_THRASHING_CHARGE_DMG,
};

enum Actions
{
	ACTION_START_AT = 1
};

class boss_corborus : public CreatureScript
{
	struct boss_corborusAI : public BossAI
	{
		boss_corborusAI(Creature * creature) : BossAI(creature, DATA_CORBORUS) {}

		void Reset()
		{
			ported = false;
			thrashingCharges = 0;
			events.SetPhase(PHASE_NORMAL);
			me->RemoveFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_NON_ATTACKABLE);
			me->SetFloatValue(UNIT_FIELD_COMBATREACH, 12.0f);
			_Reset();
		}

		void DoAction(int32 const action)
		{
			switch (action)
			{
			case ACTION_START_AT:
				me->GetMotionMaster()->MoveCharge(1159.375f, 880.072f, 284.991f, SPEED_CHARGE / 1.0f);
				me->SetReactState(REACT_AGGRESSIVE);
				break;
			default:
				break;
			}
		}

		void EnterCombat(Unit * /*who*/)
		{
			_EnterCombat();
			me->SetSpeed(MOVE_WALK, 1.5f);
			events.SetPhase(PHASE_NORMAL);
			events.ScheduleEvent(EVENT_CRYSTAL_BARRAGE, urand(8000, 10000), 0, PHASE_NORMAL);
			events.ScheduleEvent(EVENT_DAMPENING_WAVE, urand(5000, 8000), 0, PHASE_NORMAL);
			events.ScheduleEvent(EVENT_SUBMERGE, 30000, 0, PHASE_NORMAL);
		}

		void JustDied(Unit* /*killer*/)
		{
			_JustDied();
		}

		void JustSummoned(Creature * summon)
		{
			if (summon->GetEntry() == NPC_THRASHING_CHARGE)
			{
				ported = false;
				me->SetUInt64Value(UNIT_FIELD_TARGET, summon->GetGUID());
				me->SetFacingToObject(summon);
				me->SendMovementFlagUpdate();
				events.ScheduleEvent(EVENT_THRASHING_CHARGE_CAST, 500, 0, PHASE_SUBMERGED);
			}
			else if (summon->GetEntry() == NPC_ROCK_BORER)
				summon->CastSpell(summon, SPELL_EMERGE, false);

			BossAI::JustSummoned(summon);
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
				case EVENT_CRYSTAL_BARRAGE:
					DoCastRandom(SPELL_CRYSTAL_BARRAGE, 50.0f);
					events.ScheduleEvent(EVENT_CRYSTAL_BARRAGE, urand(10000, 12000), 0, PHASE_NORMAL);
					break;
				case EVENT_DAMPENING_WAVE:
					DoCast(SPELL_DAMPENING_WAVE);
					events.ScheduleEvent(EVENT_DAMPENING_WAVE, urand(10000, 12000), 0, PHASE_NORMAL);
					break;
				case EVENT_SUBMERGE:
					me->SetFloatValue(UNIT_FIELD_COMBATREACH, 3.0f);
					events.SetPhase(PHASE_SUBMERGED);
					me->SetFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_NON_ATTACKABLE);
					me->SetReactState(REACT_PASSIVE);
					me->SetUInt64Value(UNIT_FIELD_TARGET, 0);
					me->RemoveAllAuras();
					me->StopMoving();
					me->GetMotionMaster()->Clear();
					me->GetMotionMaster()->MoveIdle();
					DoCast(SPELL_SUBMERGE);
					events.RescheduleEvent(EVENT_THRASHING_CHARGE, 4000, 0, PHASE_SUBMERGED);
					break;
				case EVENT_EMERGE:
					ported = false;
					events.SetPhase(PHASE_NORMAL);
					events.RescheduleEvent(EVENT_DAMPENING_WAVE, 3000, 0, PHASE_NORMAL);
					events.RescheduleEvent(EVENT_CRYSTAL_BARRAGE, 4500, 0, PHASE_NORMAL);
					events.ScheduleEvent(EVENT_EMERGE_END, 2500, 0, PHASE_NORMAL);
					DoCast(SPELL_EMERGE_CORBORUS);
					break;
				case EVENT_EMERGE_END:
					me->RemoveAllAuras();
					me->SetReactState(REACT_AGGRESSIVE);
					me->SetFloatValue(UNIT_FIELD_COMBATREACH, 12.0f);
					me->RemoveFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_NON_ATTACKABLE | UNIT_FLAG_NOT_SELECTABLE);
					if (Unit * victim = me->GetVictim())
						DoStartMovement(victim);
					events.ScheduleEvent(EVENT_SUBMERGE, 60000, 0, PHASE_NORMAL);
					break;
				case EVENT_THRASHING_CHARGE:
					if (!ported)
					{
						ported = true;
						if (Unit * target = SelectTarget(SELECT_TARGET_RANDOM, 0, 0.0f, true))
						{
							me->GetMotionMaster()->Clear();
							me->NearTeleportTo(target->GetPositionX(), target->GetPositionY(), target->GetPositionZ() + 2.0f, 0.0f);
						}
						events.ScheduleEvent(EVENT_THRASHING_CHARGE, 500, 0, PHASE_SUBMERGED);
					}
					else
						DoCast(me, SPELL_THRASHING_CHARGE_SUMMON, true);
					break;
				case EVENT_THRASHING_CHARGE_CAST:
					DoCast(me, SPELL_THRASHING_CHARGE, false);
					me->ClearUnitState(UNIT_STATE_CASTING);
					if (thrashingCharges >= 3)
						events.ScheduleEvent(EVENT_EMERGE, 7000, 0, PHASE_SUBMERGED);
					else
					{
						++thrashingCharges;
						events.ScheduleEvent(EVENT_THRASHING_CHARGE, urand(8000, 10000), 0, PHASE_SUBMERGED);
					}
					events.ScheduleEvent(EVENT_THRASHING_CHARGE_DMG, 2900, 0, PHASE_SUBMERGED);
					break;
				case EVENT_THRASHING_CHARGE_DMG:
					DoCast(me, SPELL_THRASHING_CHARGE_DAMAGE, true);
					break;
				default:
					break;
				}
			}

			if (events.GetPhaseMask() & (1 << PHASE_NORMAL))
				DoMeleeAttackIfReady();

			EnterEvadeIfOutOfCombatArea(diff);
		}
	private:
		bool ported;
		uint8 thrashingCharges;
	};

public:
	boss_corborus() : CreatureScript("boss_corborus") {}

	CreatureAI * GetAI(Creature * creature) const
	{
		return new boss_corborusAI(creature);
	}
};

class spell_crystal_barrage : public SpellScriptLoader
{
	enum
	{
		SPELL_CRYSTAL_CHARGE_HC = 92012
	};

	class spell_crystal_barrageAuraScript : public AuraScript
	{
		PrepareAuraScript(spell_crystal_barrageAuraScript);

		void HandleTriggerSpell(AuraEffect const* aurEff)
		{
			if (Unit * caster = GetCaster())
			{
				if (caster->GetMap()->IsHeroic() && !(aurEff->GetTickNumber() % 3))
					caster->CastSpell(caster, SPELL_CRYSTAL_CHARGE_HC, true);
			}
		}

		void Register()
		{
			OnEffectPeriodic += AuraEffectPeriodicFn(spell_crystal_barrageAuraScript::HandleTriggerSpell, EFFECT_0, SPELL_AURA_PERIODIC_TRIGGER_SPELL);
		}
	};

public:
	spell_crystal_barrage() : SpellScriptLoader("spell_crystal_barrage") { }

	AuraScript* GetAuraScript() const
	{
		return new spell_crystal_barrageAuraScript();
	}
};

class npc_crystal_shard : public CreatureScript
{
	enum
	{
		SPELL_CRYSTAL_SHARDS_AURA = 80895,
		SPELL_CRYSTAL_SHARDS_TARGET = 80912,
		SPELL_CRYSTAL_SHARDS_DAMAGE = 80913
	};

	struct npc_crystal_shardAI : public BossAI
	{
		npc_crystal_shardAI(Creature * creature) : BossAI(creature, DATA_CORBORUS)
		{
			me->SetReactState(REACT_PASSIVE);
		}

		void Reset()
		{
			spawnTimer = 5000;
			spawned = false;
		}

		void SpellHitTarget(Unit * /*target*/, const SpellInfo * spell)
		{
			if (spell->Id == SPELL_CRYSTAL_SHARDS_TARGET)
			{
				DoCast(SPELL_CRYSTAL_SHARDS_DAMAGE);
				me->DespawnOrUnsummon(250);
			}
		}

		void UpdateAI(uint32 const diff)
		{
			if (!spawned)
			{
				if (spawnTimer <= diff)
				{
					spawned = true;
					me->SetReactState(REACT_AGGRESSIVE);
					DoCast(SPELL_CRYSTAL_SHARDS_AURA);
					DoZoneInCombat();
				}
				else spawnTimer -= diff;
			}
		}

	private:
		uint32 spawnTimer;
		bool spawned;
	};
public:
	npc_crystal_shard() : CreatureScript("npc_crystal_shard") {}

	CreatureAI * GetAI(Creature * creature) const
	{
		return new npc_crystal_shardAI(creature);
	}
};

class AreaTrigger_at_rockdoor_break : public AreaTriggerScript
{
public:
	AreaTrigger_at_rockdoor_break() : AreaTriggerScript("at_rockdoor_break") {}

	bool OnTrigger(Player* player, AreaTriggerEntry const* /*trigger*/)
	{
		if (InstanceScript* instance = player->GetInstanceScript())
		{
			if (GameObject* go = ObjectAccessor::GetGameObject(*player, instance->GetData64(DATA_ROCKDOOR)))
			if (go->getLootState() == GO_READY)
				go->UseDoorOrButton();

			if (Creature* corborus = ObjectAccessor::GetCreature(*player, instance->GetData64(DATA_CORBORUS)))
			if (corborus->HasFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_IMMUNE_TO_PC))
				corborus->AI()->DoAction(ACTION_START_AT);
		}

		return false;
	}
};

void AddSC_boss_corborus()
{
	new boss_corborus;
	new spell_crystal_barrage();
	new npc_crystal_shard();
	new AreaTrigger_at_rockdoor_break();
};