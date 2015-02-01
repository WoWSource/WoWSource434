#include "ScriptPCH.h"
#include "Unit.h"
#include "gilneas.h"
#include "ScriptedEscortAI.h"
#include "Vehicle.h"
#include "GameObjectAI.h"
#include "CreatureGroups.h"
#include "ScriptMgr.h"
#include "ScriptedCreature.h"
#include "ScriptedFollowerAI.h"
#include "ObjectMgr.h"
#include "Creature.h"
#include "CreatureAI.h"
#include "SpellScript.h"
#include "Object.h"
#include "SpellInfo.h"
#include "Language.h"
#include <math.h>
#include "Player.h"

/*######
## npc_gilnean_crow
######*/

class npc_gilnean_crow : public CreatureScript
{
public:
	npc_gilnean_crow() : CreatureScript("npc_gilnean_crow") {}

	CreatureAI* GetAI(Creature* creature) const
	{
		return new npc_gilnean_crowAI(creature);
	}

	struct npc_gilnean_crowAI : public ScriptedAI
	{
		npc_gilnean_crowAI(Creature* creature) : ScriptedAI(creature) {}

		uint32 tSpawn;
		bool Move;

		void Reset()
		{
			Move = false;
			tSpawn = 0;
			me->SetPosition(me->GetCreatureData()->posX, me->GetCreatureData()->posY, me->GetCreatureData()->posZ, me->GetCreatureData()->orientation);
		}

		void SpellHit(Unit* /*caster*/, const SpellInfo* spell)
		{
			if (spell->Id == SPELL_PING_GILNEAN_CROW)
			{
				if (!Move)
				{
					me->SetUInt32Value(UNIT_NPC_EMOTESTATE, EMOTE_ONESHOT_NONE); // Change our emote state to allow flight
					me->SetCanFly(true);
					Move = true;
				}
			}
		}

		void UpdateAI(const uint32 diff)
		{
			if (!Move)
				return;

			if (tSpawn <= diff)
			{
				me->GetMotionMaster()->MovePoint(0, (me->GetPositionX() + irand(-15, 15)), (me->GetPositionY() + irand(-15, 15)), (me->GetPositionZ() + irand(5, 15)), true);
				tSpawn = urand(500, 1000);
			}
			else tSpawn -= diff;

			if ((me->GetPositionZ() - me->GetCreatureData()->posZ) >= 20.0f)
			{
				me->DisappearAndDie();
				me->RemoveCorpse(true);
				Move = false;
			}
		}
	};
};

// Phase 2
/*######
## npc_prince_liam_greymane_phase2
######*/
class npc_prince_liam_greymane_phase2 : public CreatureScript
{
public:
	npc_prince_liam_greymane_phase2() : CreatureScript("npc_prince_liam_greymane_phase2") { }

	CreatureAI* GetAI(Creature* creature) const
	{
		return new npc_prince_liam_greymane_phase2AI(creature);
	}

	struct npc_prince_liam_greymane_phase2AI : public ScriptedAI
	{
		npc_prince_liam_greymane_phase2AI(Creature* c) : ScriptedAI(c) {}

		uint32 tAnimate, tSound, dmgCount, tYell, tSeek;
		bool playSnd, doYell;

		void Reset()
		{
			tAnimate = DELAY_ANIMATE;
			dmgCount = 0;
			tSound = DELAY_SOUND;
			playSnd = false;
			tSeek = urand(1000, 2000);
			doYell = true;
			tYell = DELAY_YELL_PRINCE_LIAM_GREYMANE;
		}

		// There is NO phase shift here!!!!
		void DamageTaken(Unit* who, uint32 & /*uiDamage*/)
		{
			bool isPlayer = who->GetTypeId() == TYPEID_PLAYER;

			if (isPlayer || who->isPet())
			{
				me->getThreatManager().resetAllAggro();
				if (isPlayer)
					who->AddThreat(me, 1.0f);
				me->AddThreat(who, 1.0f);
				me->AI()->AttackStart(who);
				dmgCount = 0;
			}
		}

		void DamageDealt(Unit* target, uint32& /*damage*/, DamageEffectType /*damaGetPositionYpe*/)
		{
			if (target->GetEntry() == NPC_RAMPAGING_WORGEN_1)
				++dmgCount;
		}

		void UpdateAI(uint32 const diff)
		{
			//If creature has no target
			if (!UpdateVictim())
			{
				if (tSeek <= diff)
				{
					//Find worgen nearby
					if (me->isAlive() && !me->isInCombat() && (me->GetDistance2d(me->GetHomePosition().GetPositionX(), me->GetHomePosition().GetPositionY()) <= 1.0f))
					if (Creature* enemy = me->FindNearestCreature(NPC_RAMPAGING_WORGEN_1, 16.0f, true))
						me->AI()->AttackStart(enemy);
					tSeek = urand(1000, 2000);//optimize cpu load
				}
				else tSeek -= diff;

				//Yell only once after Reset()
				if (doYell)
				{
					//Yell Timer
					if (tYell <= diff)
					{
						//Random yell
						Talk(YELL_PRINCE_LIAM_GREYMANE);
						tYell = urand(10000, 20000);
						doYell = false;
					}
					else
						tYell -= diff;
				}
			}
			else
			{
				//Play sword attack sound
				if (tSound <= diff)
				{
					me->PlayDistanceSound(SOUND_SWORD_FLESH);
					tSound = DELAY_SOUND;
					playSnd = false;
				}

				if (playSnd == true) tSound -= diff;

				//Attack
				if (dmgCount < 2)
					DoMeleeAttackIfReady();
				else if (me->GetVictim()->GetTypeId() == TYPEID_PLAYER) dmgCount = 0;
				else if (me->GetVictim()->isPet()) dmgCount = 0;
				else
				{
					if (tAnimate <= diff)
					{
						me->HandleEmoteCommand(EMOTE_ONESHOT_ATTACK1H);
						playSnd = true;
						tAnimate = DELAY_ANIMATE;
					}
					else
						tAnimate -= diff;
				}

				//Stop yell timer on combat
				doYell = false;
			}
		}

	};
};

/*######
## go_merchant_square_door
######*/
class go_merchant_square_door : public GameObjectScript
{
public:
	go_merchant_square_door() : GameObjectScript("go_merchant_square_door"), aPlayer(NULL) {}

	float x, y, z, wx, wy, angle, tQuestCredit;
	bool opened;
	uint8 spawnKind;
	Player* aPlayer;
	GameObject* go;
	uint32 DoorTimer;

	bool OnGossipHello(Player* player, GameObject* go)
	{
		if (player->GetQuestStatus(QUEST_EVAC_MERC_SQUA) == QUEST_STATUS_INCOMPLETE && go->GetGoState() == GO_STATE_READY)
		{
			aPlayer = player;
			opened = 1;
			tQuestCredit = 2500;
			go->SetGoState(GO_STATE_ACTIVE);
			DoorTimer = DOOR_TIMER;
			spawnKind = urand(1, 3); // 1, 2=citizen, 3=citizen&worgen (66%, 33%)
			angle = go->GetOrientation();
			x = go->GetPositionX() - cos(angle) * 2;
			y = go->GetPositionY() - sin(angle) * 2;
			z = go->GetPositionZ();
			wx = x - cos(angle) * 2;
			wy = y - sin(angle) * 2;

			if (spawnKind < 3)
			{
				if (Creature* spawnedCreature = go->SummonCreature(NPC_FRIGHTENED_CITIZEN_1, x, y, z, angle, TEMPSUMMON_TIMED_DESPAWN, SUMMON1_TTL))
				{
					spawnedCreature->SetPhaseMask(2, 1);
					if (Player* target = spawnedCreature->FindNearestPlayer(10.0f))
					{
						spawnedCreature->AddUnitState(UNIT_STATE_IGNORE_PATHFINDING);
						spawnedCreature->SetReactState(REACT_PASSIVE);
						spawnedCreature->GetMotionMaster()->MovePoint(42, player->GetPositionX(), player->GetPositionY(), player->GetPositionZ());
					}
					spawnedCreature->DespawnOrUnsummon(8000);
				}
			}
			else
			{
				if (Creature* spawnedCreature = go->SummonCreature(NPC_FRIGHTENED_CITIZEN_2, x, y, z, angle, TEMPSUMMON_TIMED_DESPAWN, SUMMON1_TTL))
				{
					spawnedCreature->SetPhaseMask(2, 1);
					if (Player* target = spawnedCreature->FindNearestPlayer(10.0f))
					{
						spawnedCreature->AddUnitState(UNIT_STATE_IGNORE_PATHFINDING);
						spawnedCreature->SetReactState(REACT_PASSIVE);
						spawnedCreature->GetMotionMaster()->MovePoint(42, player->GetPositionX(), player->GetPositionY(), player->GetPositionZ());
					}
					spawnedCreature->DespawnOrUnsummon(8000);
				}
			}
			return true;
		}
		return false;
	}

	void OnUpdate(GameObject* go, uint32 diff)
	{
		if (opened == 1)
		{
			if (tQuestCredit <= ((float)diff / 8))
			{
				opened = 0;

				if (aPlayer)
					aPlayer->KilledMonsterCredit(35830, 0);

				if (spawnKind == 3)
				{
					if (Creature* spawnedCreature = go->SummonCreature(NPC_RAMPAGING_WORGEN_2, wx, wy, z, angle, TEMPSUMMON_TIMED_DESPAWN, SUMMON1_TTL))
					{
						spawnedCreature->SetPhaseMask(6, 1);
						spawnedCreature->SetReactState(REACT_AGGRESSIVE);
					}
				}
			}
			else tQuestCredit -= ((float)diff / 8);
		}

		if (DoorTimer <= diff)
		{
			if (go->GetGoState() == GO_STATE_ACTIVE)
				go->SetGoState(GO_STATE_READY);

			DoorTimer = DOOR_TIMER;
		}
		else
			DoorTimer -= diff;
	}
};

/*######
## npc_gilneas_city_guard_phase2
######*/
class npc_gilneas_city_guard_phase2 : public CreatureScript
{
public:
	npc_gilneas_city_guard_phase2() : CreatureScript("npc_gilneas_city_guard_phase2") {}

	CreatureAI* GetAI(Creature* creature) const
	{
		return new npc_gilneas_city_guard_phase2AI(creature);
	}

	struct npc_gilneas_city_guard_phase2AI : public ScriptedAI
	{
		npc_gilneas_city_guard_phase2AI(Creature* creature) : ScriptedAI(creature) {}

		void Reset()
		{
			tAnimate = DELAY_ANIMATE;
			dmgCount = 0;
			tSound = DELAY_SOUND;
			playSound = false;
			tSeek = urand(1000, 2000);
		}

		void DamageTaken(Unit* who, uint32 &Damage)
		{
			if (who->GetTypeId() == TYPEID_PLAYER)
			{
				me->getThreatManager().resetAllAggro();
				who->AddThreat(me, 1.0f);
				me->AddThreat(who, 1.0f);
				me->AI()->AttackStart(who);
				dmgCount = 0;
			}
			else if (who->isPet())
			{
				me->getThreatManager().resetAllAggro();
				me->AddThreat(who, 1.0f);
				me->AI()->AttackStart(who);
				dmgCount = 0;
			}
		}

		void DamageDealt(Unit* target, uint32& damage, DamageEffectType damageType)
		{
			if (target->GetEntry() == NPC_RAMPAGING_WORGEN_1)
				dmgCount++;
		}

		void UpdateAI(const uint32 diff)
		{
			if (tSeek <= diff)
			{
				if ((me->isAlive()) && (!me->isInCombat() && (me->GetDistance2d(me->GetHomePosition().GetPositionX(), me->GetHomePosition().GetPositionY()) <= 1.0f)))
				if (Creature* enemy = me->FindNearestCreature(NPC_RAMPAGING_WORGEN_1, 16.0f, true))
					me->AI()->AttackStart(enemy);
				tSeek = urand(1000, 2000);
			}
			else tSeek -= diff;

			if (!UpdateVictim())
				return;

			if (tSound <= diff)
			{
				me->PlayDistanceSound(SOUND_SWORD_FLESH);
				tSound = DELAY_SOUND;
				playSound = false;
			}

			if (playSound == true)
				tSound -= diff;

			if (dmgCount < 2)
				DoMeleeAttackIfReady();
			else
			if (me->GetVictim()->GetTypeId() == TYPEID_PLAYER) dmgCount = 0;
			else
			if (me->GetVictim()->isPet()) dmgCount = 0;
			else
			{
				if (tAnimate <= diff)
				{
					me->HandleEmoteCommand(EMOTE_ONESHOT_ATTACK1H);
					playSound = true;
					tAnimate = DELAY_ANIMATE;
				}
				else
					tAnimate -= diff;
			}
		}

	private:
		uint32 tAnimate;
		uint32 tSound;
		uint32 dmgCount;
		uint32 tSeek;
		bool playSound;
	};
};

/*######
## npc_rampaging_worgen_phase2
######*/
class npc_rampaging_worgen : public CreatureScript
{
public:
	npc_rampaging_worgen() : CreatureScript("npc_rampaging_worgen") {}

	CreatureAI* GetAI(Creature* creature) const
	{
		return new npc_rampaging_worgenAI(creature);
	}

	struct npc_rampaging_worgenAI : public ScriptedAI
	{
		npc_rampaging_worgenAI(Creature* creature) : ScriptedAI(creature) {}

		uint32 tEnrage;
		uint32 dmgCount;
		uint32 tAnimate;
		uint32 tSound;
		bool playSound, willCastEnrage;

		void Reset()
		{
			tEnrage = 0;
			dmgCount = 0;
			tAnimate = DELAY_ANIMATE;
			tSound = DELAY_SOUND;
			playSound = false;
			willCastEnrage = urand(0, 1);
		}

		void DamageDealt(Unit* target, uint32& damage, DamageEffectType damageType)
		{
			if (target->GetEntry() == NPC_GILNEAS_CITY_GUARD || target->GetEntry() == NPC_PRINCE_LIAM_GREYMANE || target->GetEntry() == NPC_NORTHGATE_REBEL)
				dmgCount++;
		}

		void DamageTaken(Unit* who, uint32 &Damage)
		{
			if (who->GetTypeId() == TYPEID_PLAYER)
			{
				me->getThreatManager().resetAllAggro();
				who->AddThreat(me, 1.0f);
				me->AddThreat(who, 1.0f);
				me->AI()->AttackStart(who);
				dmgCount = 0;
			}
			else if (who->isPet())
			{
				me->getThreatManager().resetAllAggro();
				me->AddThreat(who, 1.0f);
				me->AI()->AttackStart(who);
				dmgCount = 0;
			}
		}

		void UpdateAI(const uint32 diff)
		{
			if (!UpdateVictim())
				return;

			me->SetVisible(true);
			//me->AI()->AttackStart(player);

			if (tEnrage <= diff && willCastEnrage)
			{
				if (me->GetHealthPct() <= 30)
				{
					Talk(0);
					DoCast(me, SPELL_ENRAGE);
					tEnrage = CD_ENRAGE;
				}
			}
			else tEnrage -= diff;

			if (playSound == true)
				tSound -= diff;

			if (tSound <= diff)
			{
				me->PlayDistanceSound(SOUND_SWORD_PLATE);
				tSound = DELAY_SOUND;
				playSound = false;
			}

			if (dmgCount < 2)
				DoMeleeAttackIfReady();
			else
			if (me->GetVictim()->GetTypeId() == TYPEID_PLAYER)
				dmgCount = 0;
			else
			if (me->GetVictim()->isPet())
				dmgCount = 0;
			else
			{
				if (tAnimate <= diff)
				{
					me->HandleEmoteCommand(EMOTE_ONESHOT_ATTACK_UNARMED);
					tAnimate = DELAY_ANIMATE;
					playSound = true;
				}
				else
					tAnimate -= diff;
			}
		}
	};
};

/*######
## npc_mariam_spellwalker
######*/
class npc_mariam_spellwalker : public CreatureScript
{
public:
	npc_mariam_spellwalker() : CreatureScript("npc_mariam_spellwalker") {}

	CreatureAI* GetAI(Creature* creature) const
	{
		return new npc_mariam_spellwalkerAI(creature);
	}

	struct npc_mariam_spellwalkerAI : public ScriptedAI
	{
		npc_mariam_spellwalkerAI(Creature* creature) : ScriptedAI(creature) {}

		uint32 tSeek;

		void Reset()
		{
			tSeek = urand(1000, 2000);
		}

		void DamageTaken(Unit* who, uint32& damage)
		{
			if (me->HealthBelowPct(AI_MIN_HP) && who->GetEntry() == NPC_BLOODFANG_WORGEN)
				damage = 0;
		}

		void UpdateAI(const uint32 diff)
		{
			if (tSeek <= diff)
			{
				if ((me->isAlive()) && (!me->isInCombat() && (me->GetDistance2d(me->GetHomePosition().GetPositionX(), me->GetHomePosition().GetPositionY()) <= 1.0f)))
				if (Creature* enemy = me->FindNearestCreature(NPC_BLOODFANG_WORGEN, 5.0f, true))
					me->AI()->AttackStart(enemy); // She should really only grab agro when npc Cleese is not there, so we will keep this range small
				tSeek = urand(1000, 2000); // optimize cpu load, seeking only sometime between 1 and 2 seconds
			}
			else tSeek -= diff;

			if (!UpdateVictim())
				return;

			if (me->GetVictim()->GetEntry() == NPC_BLOODFANG_WORGEN)
				DoSpellAttackIfReady(SPELL_FROSTBOLT_VISUAL_ONLY); //Dummy spell, visual only to prevent getting agro (Blizz-like)
			else
				DoMeleeAttackIfReady();
		}
	};
};

/*######
## npc_rampaging_worgen2
######*/
class npc_rampaging_worgen2 : public CreatureScript
{
public:
	npc_rampaging_worgen2() : CreatureScript("npc_rampaging_worgen2") {}

	CreatureAI* GetAI(Creature* creature) const
	{
		return new npc_rampaging_worgen2AI(creature);
	}

	struct npc_rampaging_worgen2AI : public ScriptedAI
	{
		npc_rampaging_worgen2AI(Creature* creature) : ScriptedAI(creature) {}

		void JustRespawned()
		{
			tEnrage = 0;
			tRun = 500;
			onceRun = true;
			x = me->m_positionX + cos(me->m_orientation) * 8;
			y = me->m_positionY + sin(me->m_orientation) * 8;
			z = me->m_positionZ;
			willCastEnrage = urand(0, 1);
		}

		void UpdateAI(const uint32 diff)
		{
			if (tRun <= diff && onceRun)
			{
				me->GetMotionMaster()->MoveCharge(x, y, z, 8);
				onceRun = false;
			}
			else
				tRun -= diff;

			if (!UpdateVictim())
				return;

			if (tEnrage <= diff)
			{
				if (me->GetHealthPct() <= 30 && willCastEnrage)
				{
					me->MonsterTextEmote(-106, 0);
					DoCast(me, SPELL_ENRAGE);
					tEnrage = CD_ENRAGE;
				}
			}
			else
				tEnrage -= diff;

			DoMeleeAttackIfReady();
		}

	private:
		uint16 tRun;
		uint16 tEnrage;
		bool onceRun;
		bool willCastEnrage;
		float x, y, z;
	};
};

/*######
## npc_gilnean_royal_guard
######*/
class npc_gilnean_royal_guard : public CreatureScript
{
public:
	npc_gilnean_royal_guard() : CreatureScript("npc_gilnean_royal_guard") {}

	CreatureAI* GetAI(Creature* creature) const
	{
		return new npc_gilnean_royal_guardAI(creature);
	}

	struct npc_gilnean_royal_guardAI : public ScriptedAI
	{
		npc_gilnean_royal_guardAI(Creature* creature) : ScriptedAI(creature) {}

		void Reset()
		{
			tAnimate = DELAY_ANIMATE;
			dmgCount = 0;
			tSound = DELAY_SOUND;
			playSound = false;
			tSeek = urand(1000, 2000);
		}

		void DamageTaken(Unit* who, uint32 &Damage)
		{
			if (who->GetTypeId() == TYPEID_PLAYER)
			{
				me->getThreatManager().resetAllAggro();
				who->AddThreat(me, 1.0f);
				me->AddThreat(who, 1.0f);
				me->AI()->AttackStart(who);
				dmgCount = 0;
			}
			else if (who->isPet())
			{
				me->getThreatManager().resetAllAggro();
				me->AddThreat(who, 1.0f);
				me->AI()->AttackStart(who);
				dmgCount = 0;
			}
		}

		void DamageDealt(Unit* target, uint32 &damage, DamageEffectType damageType)
		{
			if (target->GetEntry() == NPC_BLOODFANG_WORGEN)
				dmgCount++;
		}

		void UpdateAI(const uint32 diff)
		{
			if (tSeek <= diff)
			{
				if ((me->isAlive()) && (!me->isInCombat() && (me->GetDistance2d(me->GetHomePosition().GetPositionX(), me->GetHomePosition().GetPositionY()) <= 1.0f)))
				if (Creature* enemy = me->FindNearestCreature(NPC_BLOODFANG_WORGEN, 16.0f, true))
					me->AI()->AttackStart(enemy);
				tSeek = urand(1000, 2000);
			}
			else tSeek -= diff;

			if (!UpdateVictim())
				return;

			if (tSound <= diff)
			{
				me->PlayDistanceSound(SOUND_SWORD_FLESH);
				tSound = DELAY_SOUND;
				playSound = false;
			}

			if (playSound == true)
				tSound -= diff;

			if (dmgCount < 2)
				DoMeleeAttackIfReady();
			else
			if (me->GetVictim()->GetTypeId() == TYPEID_PLAYER) dmgCount = 0;
			else
			if (me->GetVictim()->isPet()) dmgCount = 0;
			else
			{
				if (tAnimate <= diff)
				{
					me->HandleEmoteCommand(EMOTE_ONESHOT_ATTACK1H);
					playSound = true;
					tAnimate = DELAY_ANIMATE;
				}
				else
					tAnimate -= diff;
			}
		}

	private:
		uint32 tAnimate;
		uint32 tSound;
		uint32 dmgCount;
		uint32 tSeek;
		bool playSound;

	};
};

/*######
## npc_frightened_citizen
######*/
class npc_frightened_citizen : public CreatureScript
{
public:
	npc_frightened_citizen() : CreatureScript("npc_frightened_citizen") {}

	CreatureAI* GetAI(Creature* creature) const
	{
		return new npc_frightened_citizenAI(creature);
	}

	struct npc_frightened_citizenAI : public ScriptedAI
	{
		npc_frightened_citizenAI(Creature* creature) : ScriptedAI(creature) {}

		void Reset()
		{
			startPath = false;
			mui_moveTimer = 1000;
		}

		void MovementInform(uint32 type, uint32 point)
		{
			if (point == 42)
			{
				startPath = true;
				Talk(0, 0);
			}
			else if (point == 5)
				me->DespawnOrUnsummon();
		}

		void UpdateAI(const uint32 diff)
		{
			if (!startPath)
				return;

			if (mui_moveTimer <= diff)
			{
				me->ClearUnitState(UNIT_STATE_IGNORE_PATHFINDING);
				me->GetMotionMaster()->MovePoint(5, FrightenedWay[urand(0, 1)]);
				mui_moveTimer = 10000;
			}
			else
				mui_moveTimer -= diff;
		}

	private:
		bool startPath;
		uint32 mui_moveTimer;
	};
};

/*######
## npc_sergeant_cleese
######*/
class npc_sergeant_cleese : public CreatureScript
{
public:
	npc_sergeant_cleese() : CreatureScript("npc_sergeant_cleese") {}

	CreatureAI* GetAI(Creature* creature) const
	{
		return new npc_sergeant_cleeseAI(creature);
	}

	struct npc_sergeant_cleeseAI : public ScriptedAI
	{
		npc_sergeant_cleeseAI(Creature* creature) : ScriptedAI(creature) {}

		uint32 tSeek;

		void Reset()
		{
			tSeek = urand(1000, 2000);
		}

		void DamageTaken(Unit* who, uint32& damage)
		{
			if (who->GetTypeId() == TYPEID_PLAYER)
			{
				me->getThreatManager().resetAllAggro();
				who->AddThreat(me, 1.0f);
				me->AddThreat(who, 1.0f);
				me->AI()->AttackStart(who);
			}
			else if (who->isPet())
			{
				me->getThreatManager().resetAllAggro();
				me->AddThreat(who, 1.0f);
				me->AI()->AttackStart(who);
			}
			else if (me->HealthBelowPct(AI_MIN_HP) && who->GetEntry() == NPC_BLOODFANG_WORGEN)
				damage = 0;
		}

		void UpdateAI(const uint32 diff)
		{
			if (tSeek <= diff)
			{
				if ((me->isAlive()) && (!me->isInCombat() && (me->GetDistance2d(me->GetHomePosition().GetPositionX(), me->GetHomePosition().GetPositionY()) <= 1.0f)))
				if (Creature* enemy = me->FindNearestCreature(NPC_BLOODFANG_WORGEN, 10.0f, true))
					me->AI()->AttackStart(enemy);
				tSeek = urand(1000, 2000); // optimize cpu load, seeking only sometime between 1 and 2 seconds
			}
			else tSeek -= diff;

			if (!UpdateVictim())
				return;
			else
				DoMeleeAttackIfReady();
		}
	};
};
static const Position spawnPos[3][3] =
{
	{
		{ -1718.195f, 1526.525879f, 55.918981f, 4.698965f }, // Spawn Left
		{ -1718.152f, 1486.315918f, 57.232832f, 5.437238f }, // MoveTo Left
		{ -1699.458f, 1468.477783f, 52.297140f, 5.523634f } // JumpTo Left
	},
	{
		{ -1625.497f, 1492.348755f, 73.716627f, 4.070646f }, // Spawn Mid
		{ -1632.688f, 1485.533447f, 74.958799f, 3.874443f }, // MoveTo Mid
		{ -1676.319f, 1445.140747f, 52.297140f, 0.748411f } // JumpTo Mid - CENTER
	},
	{
		{ -1562.104f, 1409.827148f, 71.676458f, 3.218636f }, // Spawn Right
		{ -1594.044f, 1408.207397f, 72.850088f, 3.120459f }, // MoveTo Right
		{ -1655.406f, 1425.001953f, 52.297109f, 2.291864f } //JumpTo Right
	}
};

/* QUEST - 14154 - By The Skin of His Teeth - START */
class npc_lord_darius_crowley_c1 : public CreatureScript
{
	enum
	{
		SPELL_DEMORALIZING_SHOUT = 61044,
		SPELL_BY_THE_SKIN = 66914,
		SPELL_LEFT_HOOK = 67825,
		SPELL_SNAP_KICK = 67827,
		SPELL_PHASING_AURA = 59073,

		QUEST_BY_THE_SKIN = 14154,
		ACTION_START_EVENT = 1,

		NPC_WORGEN_RUNT_C2 = 35456,
		NPC_WORGEN_RUNT_C1 = 35188,
		NPC_WORGEN_ALPHA_C2 = 35167,
		NPC_WORGEN_ALPHA_C1 = 35170,

		EVENT_DEMORALIZING_SHOUT = 1,
		EVENT_LEFT_HOOK = 2,
		EVENT_SNAP_KICK = 3,
		EVENT_NEXT_WAVE = 4,
	};

	struct npc_lord_darius_crowleyAI : public Scripted_NoMovementAI
	{
		npc_lord_darius_crowleyAI(Creature* c) : Scripted_NoMovementAI(c), Summons(me) {}

		Player* m_player;

		void Reset()
		{
			EventInProgress = false;
			stage = 1;
			summonPos = 0;
			cnt = 0;
			toSummon = 0;
			phaseTimer = 15000;
			m_player = NULL;

			events.Reset();
			SetCombatMovement(false);
			Summons.DespawnAll();
		}

		void EnterEvadeMode()
		{
			if (!EventInProgress)
				ScriptedAI::EnterEvadeMode();
		}

		void JustSummoned(Creature* summoned)
		{
			Summons.Summon(summoned);
			summoned->AI()->SetData(0, summonPos);
		}

		void DoAction(const int32 /*action*/)
		{
			if (!EventInProgress)
			{
				EventInProgress = true;
				stage = 1;
				events.ScheduleEvent(EVENT_NEXT_WAVE, 5000);
				events.ScheduleEvent(EVENT_DEMORALIZING_SHOUT, 20000);
				events.ScheduleEvent(EVENT_LEFT_HOOK, 25000);
				events.ScheduleEvent(EVENT_SNAP_KICK, 28000);
			}
		}

		void SetGUID(uint64 guid, int32)
		{
			if (!EventInProgress)
				playerGUID = guid;
		}

		void SummonedCreatureDespawn(Creature* summoned)
		{
			Summons.Despawn(summoned);
		}

		void JustDied(Unit* /*killer*/)
		{
			if (Player * player = me->GetPlayer(*me, playerGUID))
			{
				player->RemoveAurasDueToSpell(SPELL_PHASING_AURA);
				player->FailQuest(QUEST_BY_THE_SKIN);
			}
		}

		void CastVictim(uint32 spellId)
		{
			if (me->GetVictim())
				DoCastVictim(spellId);
		}

		void UpdateAI(const uint32 diff)
		{
			if (me->isInCombat())
			if (Unit* victim = me->SelectVictim())
				AttackStart(victim);

			if (!EventInProgress)
				return;

			events.Update(diff);

			if (uint32 eventId = events.ExecuteEvent())
			{
				switch (eventId)
				{
				case EVENT_DEMORALIZING_SHOUT:
					DoCast(SPELL_DEMORALIZING_SHOUT);
					events.ScheduleEvent(EVENT_DEMORALIZING_SHOUT, 15000);
					break;
				case EVENT_LEFT_HOOK:
					CastVictim(SPELL_LEFT_HOOK);
					events.ScheduleEvent(EVENT_LEFT_HOOK, 5000);
					break;
				case EVENT_SNAP_KICK:
					CastVictim(SPELL_SNAP_KICK);
					events.ScheduleEvent(EVENT_SNAP_KICK, urand(3000, 8000));
					break;
				case EVENT_NEXT_WAVE:
				{
										cnt = 0;
										toSummon = 0;

										switch (urand(1, 5)) // After intial wave, wave spawns should be random
										{
										case 1: // One Alpha on SW Roof and One Alpha on NW Roof
											me->SummonCreature(NPC_WORGEN_ALPHA_C2, SW_ROOF_SPAWN_LOC_1, TEMPSUMMON_TIMED_OR_CORPSE_DESPAWN, WORGEN_EVENT_SPAWNTIME);
											me->SummonCreature(NPC_WORGEN_ALPHA_C1, NW_ROOF_SPAWN_LOC_1, TEMPSUMMON_TIMED_OR_CORPSE_DESPAWN, WORGEN_EVENT_SPAWNTIME);
											break;

										case 2: // 8 Runts on NW Roof
											for (int i = 0; i < 5; i++)
												me->SummonCreature(NPC_WORGEN_RUNT_C1, NW_ROOF_SPAWN_LOC_1, TEMPSUMMON_TIMED_OR_CORPSE_DESPAWN, WORGEN_EVENT_SPAWNTIME);
											me->SummonCreature(NPC_WORGEN_RUNT_C1, NW_ROOF_SPAWN_LOC_2, TEMPSUMMON_TIMED_OR_CORPSE_DESPAWN, WORGEN_EVENT_SPAWNTIME);
											break;

										case 3: // 8 Runts on SW Roof
											for (int i = 0; i < 5; i++)
												me->SummonCreature(NPC_WORGEN_RUNT_C2, SW_ROOF_SPAWN_LOC_1, TEMPSUMMON_TIMED_OR_CORPSE_DESPAWN, WORGEN_EVENT_SPAWNTIME);
											me->SummonCreature(NPC_WORGEN_RUNT_C2, SW_ROOF_SPAWN_LOC_2, TEMPSUMMON_TIMED_OR_CORPSE_DESPAWN, WORGEN_EVENT_SPAWNTIME);
											break;

										case 4: // One Alpha on SW Roof and One Alpha on N Roof
											me->SummonCreature(NPC_WORGEN_ALPHA_C2, SW_ROOF_SPAWN_LOC_1, TEMPSUMMON_TIMED_OR_CORPSE_DESPAWN, WORGEN_EVENT_SPAWNTIME);
											me->SummonCreature(NPC_WORGEN_ALPHA_C1, N_ROOF_SPAWN_LOC, TEMPSUMMON_TIMED_OR_CORPSE_DESPAWN, WORGEN_EVENT_SPAWNTIME);
											break;
										case 5: // 8 Runts - Half NW and Half SW
											for (int i = 0; i < 5; i++)
												me->SummonCreature(NPC_WORGEN_RUNT_C2, SW_ROOF_SPAWN_LOC_1, TEMPSUMMON_TIMED_OR_CORPSE_DESPAWN, WORGEN_EVENT_SPAWNTIME);
											me->SummonCreature(NPC_WORGEN_RUNT_C1, NW_ROOF_SPAWN_LOC_2, TEMPSUMMON_TIMED_OR_CORPSE_DESPAWN, WORGEN_EVENT_SPAWNTIME);
											break;
										default:
											return;
										}
										for (int i = 0; i < cnt; ++i)
											me->SummonCreature(NPC_WORGEN_RUNT_C2, spawnPos[summonPos][0], TEMPSUMMON_CORPSE_DESPAWN, 10000);
										me->SummonCreature(toSummon, spawnPos[summonPos][0], TEMPSUMMON_CORPSE_DESPAWN, 10000);

										++stage;

										if (summonPos > 1)
											summonPos = 0;
										else
											++summonPos;

										if (stage > 7)
										{
											stage = 1;
											EventInProgress = false;
											events.Reset();
										}
										else
											events.ScheduleEvent(EVENT_NEXT_WAVE, 15000);
				}
					break;
				default:
					break;
				}
			}

			DoMeleeAttackIfReady();
		}

	private:
		bool EventInProgress;
		uint8 stage;
		uint8 summonPos;
		int cnt;
		uint32 toSummon;
		uint32 phaseTimer;
		uint64 playerGUID;

		EventMap events;
		SummonList Summons;
	};

public:
	bool OnQuestAccept(Player* player, Creature* creature, Quest const* quest)
	{
		if (quest->GetQuestId() == QUEST_BY_THE_SKIN)
		{
			creature->AI()->DoAction(ACTION_START_EVENT);
			creature->AI()->SetGUID(player->GetGUID());
			creature->CastSpell(player, SPELL_BY_THE_SKIN, true);
		}
		return true;
	}

	npc_lord_darius_crowley_c1() : CreatureScript("npc_lord_darius_crowley_c1") { }

	CreatureAI* GetAI(Creature* creature) const
	{
		return new npc_lord_darius_crowleyAI(creature);
	}
};


/*######
## npc_worgen_runt_c1
######*/
class npc_worgen_runt_c1 : public CreatureScript
{
public:
	npc_worgen_runt_c1() : CreatureScript("npc_worgen_runt_c1") {}

	CreatureAI* GetAI(Creature* creature) const
	{
		return new npc_worgen_runt_c1AI(creature);
	}

	struct npc_worgen_runt_c1AI : public ScriptedAI
	{
		npc_worgen_runt_c1AI(Creature* creature) : ScriptedAI(creature) {}

		uint32 WaypointId, willCastEnrage, tEnrage, CommonWPCount;
		bool Run, Loc1, Loc2, Jump, Combat;

		void Reset()
		{
			Run = Loc1 = Loc2 = Combat = Jump = false;
			WaypointId = 0;
			tEnrage = 0;
			willCastEnrage = urand(0, 1);
		}

		void UpdateAI(const uint32 diff)
		{
			if (me->GetPositionX() == -1611.40f && me->GetPositionY() == 1498.49f) // I was spawned in location 1
			{
				Run = true; // Start running across roof
				Loc1 = true;
			}
			else if (me->GetPositionX() == -1618.86f && me->GetPositionY() == 1505.68f) // I was spawned in location 2
			{
				Run = true; // Start running across roof
				Loc2 = true;
			}

			if (Run && !Jump && !Combat)
			{
				if (Loc1) // If I was spawned in Location 1
				{
					if (WaypointId < 2)
						me->GetMotionMaster()->MovePoint(WaypointId, NW_WAYPOINT_LOC1[WaypointId].X, NW_WAYPOINT_LOC1[WaypointId].Y, NW_WAYPOINT_LOC1[WaypointId].Z);
				}
				else if (Loc2)// If I was spawned in Location 2
				{
					if (WaypointId < 2)
						me->GetMotionMaster()->MovePoint(WaypointId, NW_WAYPOINT_LOC2[WaypointId].X, NW_WAYPOINT_LOC2[WaypointId].Y, NW_WAYPOINT_LOC2[WaypointId].Z);
				}
			}

			if (!Run && Jump && !Combat) // After Jump
			{
				if (me->GetPositionZ() == PLATFORM_Z) // Check that we made it to the platform
				{
					me->GetMotionMaster()->Clear(); // Stop Movement
					// Set our new home position so we don't try and run back to the rooftop on reset
					me->SetHomePosition(me->GetPositionX(), me->GetPositionY(), me->GetPositionZ(), me->GetOrientation());
					Combat = true; // Start Combat
					Jump = false; // We have already Jumped
				}
			}

			if (Combat && !Run && !Jump) // Our Combat AI
			{
				if (Player* player = me->SelectNearestPlayer(40.0f)) // Try to attack nearest player 1st (Blizz-Like)
					AttackStart(player);
				else
					AttackStart(me->FindNearestCreature(NPC_LORD_DARIUS_CROWLEY_C1, 40.0f)); // Attack Darius 2nd - After that, doesn't matter

				if (!UpdateVictim())
					return;

				if (tEnrage <= diff) // Our Enrage trigger
				{
					if (me->GetHealthPct() <= 30 && willCastEnrage)
					{
						me->MonsterTextEmote(-106, 0);
						DoCast(me, SPELL_ENRAGE);
						tEnrage = CD_ENRAGE;
					}
				}
				else
					tEnrage -= diff;

				DoMeleeAttackIfReady();
			}
		}

		void MovementInform(uint32 Type, uint32 PointId)
		{
			if (Type != POINT_MOTION_TYPE)
				return;

			if (Loc1)
			{
				CommonWPCount = sizeof(NW_WAYPOINT_LOC1) / sizeof(Waypoint); // Count our waypoints
			}
			else if (Loc2)
			{
				CommonWPCount = sizeof(NW_WAYPOINT_LOC2) / sizeof(Waypoint); // Count our waypoints
			}

			WaypointId = PointId + 1; // Increase to next waypoint

			if (WaypointId >= CommonWPCount) // If we have reached the last waypoint
			{
				if (Loc1)
				{
					me->GetMotionMaster()->MoveJump(-1668.52f + irand(-3, 3), 1439.69f + irand(-3, 3), PLATFORM_Z, 20.0f, 22.0f);
					Loc1 = false;
				}
				else if (Loc2)
				{
					me->GetMotionMaster()->MoveJump(-1678.04f + irand(-3, 3), 1450.88f + irand(-3, 3), PLATFORM_Z, 20.0f, 22.0f);
					Loc2 = false;
				}

				Run = false; // Stop running - Regardless of spawn location
				Jump = true; // Time to Jump - Regardless of spawn location
			}
		}
	};
};

/*######
## npc_worgen_runt_c2
######*/
class npc_worgen_runt_c2 : public CreatureScript
{
public:
	npc_worgen_runt_c2() : CreatureScript("npc_worgen_runt_c2") {}

	CreatureAI* GetAI(Creature* creature) const
	{
		return new npc_worgen_runt_c2AI(creature);
	}

	struct npc_worgen_runt_c2AI : public ScriptedAI
	{
		npc_worgen_runt_c2AI(Creature* creature) : ScriptedAI(creature) {}

		uint32 WaypointId, willCastEnrage, tEnrage, CommonWPCount;
		bool Run, Loc1, Loc2, Jump, Combat;

		void Reset()
		{
			Run = Loc1 = Loc2 = Combat = Jump = false;
			WaypointId = 0;
			tEnrage = 0;
			willCastEnrage = urand(0, 1);
		}

		void UpdateAI(const uint32 diff)
		{
			if (me->GetPositionX() == -1732.81f && me->GetPositionY() == 1526.34f) // I was spawned in location 1
			{
				Run = true; // Start running across roof
				Loc1 = true;
			}
			else if (me->GetPositionX() == -1737.49f && me->GetPositionY() == 1526.11f) // I was spawned in location 2
			{
				Run = true; // Start running across roof
				Loc2 = true;
			}

			if (Run && !Jump && !Combat)
			{
				if (Loc1) // If I was spawned in Location 1
				{
					if (WaypointId < 2)
						me->GetMotionMaster()->MovePoint(WaypointId, SW_WAYPOINT_LOC1[WaypointId].X, SW_WAYPOINT_LOC1[WaypointId].Y, SW_WAYPOINT_LOC1[WaypointId].Z);
				}
				else if (Loc2)// If I was spawned in Location 2
				{
					if (WaypointId < 2)
						me->GetMotionMaster()->MovePoint(WaypointId, SW_WAYPOINT_LOC2[WaypointId].X, SW_WAYPOINT_LOC2[WaypointId].Y, SW_WAYPOINT_LOC2[WaypointId].Z);
				}
			}

			if (!Run && Jump && !Combat) // After Jump
			{
				if (me->GetPositionZ() == PLATFORM_Z) // Check that we made it to the platform
				{
					me->GetMotionMaster()->Clear(); // Stop Movement
					// Set our new home position so we don't try and run back to the rooftop on reset
					me->SetHomePosition(me->GetPositionX(), me->GetPositionY(), me->GetPositionZ(), me->GetOrientation());
					Combat = true; // Start Combat
					Jump = false; // We have already Jumped
				}
			}

			if (Combat && !Run && !Jump) // Our Combat AI
			{
				if (Player* player = me->SelectNearestPlayer(50.0f)) // Try to attack nearest player 1st (Blizz-Like)
					AttackStart(player);
				else
					AttackStart(me->FindNearestCreature(NPC_LORD_DARIUS_CROWLEY_C1, 50.0f)); // Attack Darius 2nd - After that, doesn't matter

				if (!UpdateVictim())
					return;

				if (tEnrage <= diff) // Our Enrage trigger
				{
					if (me->GetHealthPct() <= 30 && willCastEnrage)
					{
						me->MonsterTextEmote(-106, 0);
						DoCast(me, SPELL_ENRAGE);
						tEnrage = CD_ENRAGE;
					}
				}
				else
					tEnrage -= diff;

				DoMeleeAttackIfReady();
			}
		}

		void MovementInform(uint32 Type, uint32 PointId)
		{
			if (Type != POINT_MOTION_TYPE)
				return;

			if (Loc1)
			{
				CommonWPCount = sizeof(SW_WAYPOINT_LOC1) / sizeof(Waypoint); // Count our waypoints
			}
			else if (Loc2)
			{
				CommonWPCount = sizeof(SW_WAYPOINT_LOC2) / sizeof(Waypoint); // Count our waypoints
			}

			WaypointId = PointId + 1; // Increase to next waypoint

			if (WaypointId >= CommonWPCount) // If we have reached the last waypoint
			{
				if (Loc1)
				{
					me->GetMotionMaster()->MoveJump(-1685.521f + irand(-3, 3), 1458.48f + irand(-3, 3), PLATFORM_Z, 20.0f, 22.0f);
					Loc1 = false;
				}
				else if (Loc2)
				{
					me->GetMotionMaster()->MoveJump(-1681.81f + irand(-3, 3), 1445.54f + irand(-3, 3), PLATFORM_Z, 20.0f, 22.0f);
					Loc2 = false;
				}

				Run = false; // Stop running - Regardless of spawn location
				Jump = true; // Time to Jump - Regardless of spawn location
			}
		}
	};
};

/*######
## npc_worgen_alpha_c1
######*/
class npc_worgen_alpha_c1 : public CreatureScript
{
public:
	npc_worgen_alpha_c1() : CreatureScript("npc_worgen_alpha_c1") {}

	CreatureAI* GetAI(Creature* creature) const
	{
		return new npc_worgen_alpha_c1AI(creature);
	}

	struct npc_worgen_alpha_c1AI : public ScriptedAI
	{
		npc_worgen_alpha_c1AI(Creature* creature) : ScriptedAI(creature) {}

		uint32 WaypointId, willCastEnrage, tEnrage, CommonWPCount;
		bool Run, Loc1, Loc2, Jump, Combat;

		void Reset()
		{
			Run = Loc1 = Loc2 = Combat = Jump = false;
			WaypointId = 0;
			tEnrage = 0;
			willCastEnrage = urand(0, 1);
		}

		void UpdateAI(const uint32 diff)
		{
			if (me->GetPositionX() == -1618.86f && me->GetPositionY() == 1505.68f) // I was spawned in location 1 on NW Rooftop
			{
				Run = true; // Start running across roof
				Loc1 = true;
			}
			else if (me->GetPositionX() == -1562.59f && me->GetPositionY() == 1409.35f) // I was spawned on the North Rooftop
			{
				Run = true; // Start running across roof
				Loc2 = true;
			}

			if (Run && !Jump && !Combat)
			{
				if (Loc1) // If I was spawned in Location 1
				{
					if (WaypointId < 2)
						me->GetMotionMaster()->MovePoint(WaypointId, NW_WAYPOINT_LOC1[WaypointId].X, NW_WAYPOINT_LOC1[WaypointId].Y, NW_WAYPOINT_LOC1[WaypointId].Z);
				}
				else if (Loc2)// If I was spawned in Location 2
				{
					if (WaypointId < 2)
						me->GetMotionMaster()->MovePoint(WaypointId, N_WAYPOINT_LOC[WaypointId].X, N_WAYPOINT_LOC[WaypointId].Y, N_WAYPOINT_LOC[WaypointId].Z);
				}
			}

			if (!Run && Jump && !Combat) // After Jump
			{
				if (me->GetPositionZ() == PLATFORM_Z) // Check that we made it to the platform
				{
					me->GetMotionMaster()->Clear(); // Stop Movement
					// Set our new home position so we don't try and run back to the rooftop on reset
					me->SetHomePosition(me->GetPositionX(), me->GetPositionY(), me->GetPositionZ(), me->GetOrientation());
					Combat = true; // Start Combat
					Jump = false; // We have already Jumped
				}
			}

			if (Combat && !Run && !Jump) // Our Combat AI
			{
				if (Player* player = me->SelectNearestPlayer(40.0f)) // Try to attack nearest player 1st (Blizz-Like)
					AttackStart(player);
				else
					AttackStart(me->FindNearestCreature(NPC_LORD_DARIUS_CROWLEY_C1, 40.0f)); // Attack Darius 2nd - After that, doesn't matter

				if (!UpdateVictim())
					return;

				if (tEnrage <= diff) // Our Enrage trigger
				{
					if (me->GetHealthPct() <= 30 && willCastEnrage)
					{
						me->MonsterTextEmote(-106, 0);
						DoCast(me, SPELL_ENRAGE);
						tEnrage = CD_ENRAGE;
					}
				}
				else
					tEnrage -= diff;

				DoMeleeAttackIfReady();
			}
		}

		void MovementInform(uint32 Type, uint32 PointId)
		{
			if (Type != POINT_MOTION_TYPE)
				return;

			if (Loc1)
			{
				CommonWPCount = sizeof(NW_WAYPOINT_LOC1) / sizeof(Waypoint); // Count our waypoints
			}
			else if (Loc2)
			{
				CommonWPCount = sizeof(N_WAYPOINT_LOC) / sizeof(Waypoint); // Count our waypoints
			}

			WaypointId = PointId + 1; // Increase to next waypoint

			if (WaypointId >= CommonWPCount) // If we have reached the last waypoint
			{
				if (Loc1)
				{
					me->GetMotionMaster()->MoveJump(-1668.52f + irand(-3, 3), 1439.69f + irand(-3, 3), PLATFORM_Z, 20.0f, 22.0f);
					Loc1 = false;
				}
				else if (Loc2)
				{
					me->GetMotionMaster()->MoveJump(-1660.17f + irand(-3, 3), 1429.55f + irand(-3, 3), PLATFORM_Z, 22.0f, 20.0f);
					Loc2 = false;
				}

				Run = false; // Stop running - Regardless of spawn location
				Jump = true; // Time to Jump - Regardless of spawn location
			}
		}
	};
};

/*######
## npc_worgen_alpha_c2
######*/
class npc_worgen_alpha_c2 : public CreatureScript
{
public:
	npc_worgen_alpha_c2() : CreatureScript("npc_worgen_alpha_c2") {}

	CreatureAI* GetAI(Creature* creature) const
	{
		return new npc_worgen_alpha_c2AI(creature);
	}

	struct npc_worgen_alpha_c2AI : public ScriptedAI
	{
		npc_worgen_alpha_c2AI(Creature* creature) : ScriptedAI(creature) {}

		uint32 WaypointId, willCastEnrage, tEnrage, CommonWPCount;
		bool Run, Jump, Combat;

		void Reset()
		{
			Run = Combat = Jump = false;
			WaypointId = 0;
			tEnrage = 0;
			willCastEnrage = urand(0, 1);
		}

		void UpdateAI(const uint32 diff)
		{
			if (me->GetPositionX() == -1732.81f && me->GetPositionY() == 1526.34f) // I was just spawned
			{
				Run = true; // Start running across roof
			}

			if (Run && !Jump && !Combat)
			{
				if (WaypointId < 2)
					me->GetMotionMaster()->MovePoint(WaypointId, SW_WAYPOINT_LOC1[WaypointId].X, SW_WAYPOINT_LOC1[WaypointId].Y, SW_WAYPOINT_LOC1[WaypointId].Z);
			}

			if (!Run && Jump && !Combat) // After Jump
			{
				if (me->GetPositionZ() == PLATFORM_Z) // Check that we made it to the platform
				{
					me->GetMotionMaster()->Clear(); // Stop Movement
					// Set our new home position so we don't try and run back to the rooftop on reset
					me->SetHomePosition(me->GetPositionX(), me->GetPositionY(), me->GetPositionZ(), me->GetOrientation());
					Combat = true; // Start Combat
					Jump = false; // We have already Jumped
				}
			}

			if (Combat && !Run && !Jump) // Our Combat AI
			{
				if (Player* player = me->SelectNearestPlayer(40.0f)) // Try to attack nearest player 1st (Blizz-Like)
					AttackStart(player);
				else
					AttackStart(me->FindNearestCreature(NPC_LORD_DARIUS_CROWLEY_C1, 40.0f)); // Attack Darius 2nd - After that, doesn't matter

				if (!UpdateVictim())
					return;

				if (tEnrage <= diff) // Our Enrage trigger
				{
					if (me->GetHealthPct() <= 30 && willCastEnrage)
					{
						me->MonsterTextEmote(-106, 0);
						DoCast(me, SPELL_ENRAGE);
						tEnrage = CD_ENRAGE;
					}
				}
				else
					tEnrage -= diff;

				DoMeleeAttackIfReady();
			}
		}

		void MovementInform(uint32 Type, uint32 PointId)
		{
			if (Type != POINT_MOTION_TYPE)
				return;

			CommonWPCount = sizeof(SW_WAYPOINT_LOC1) / sizeof(Waypoint); // Count our waypoints

			WaypointId = PointId + 1; // Increase to next waypoint

			if (WaypointId >= CommonWPCount) // If we have reached the last waypoint
			{
				me->GetMotionMaster()->MoveJump(-1685.52f + irand(-3, 3), 1458.48f + irand(-3, 3), PLATFORM_Z, 20.0f, 22.0f);
				Run = false; // Stop running
				Jump = true; // Time to Jump
			}
		}
	};
};

/* QUEST - 14154 - By The Skin of His Teeth - END */

/* QUEST - 14159 - The Rebel Lord's Arsenal - START */
// Phase 4
/*######
## npc_josiah_avery
######*/
class npc_josiah_avery : public CreatureScript
{
public:
	npc_josiah_avery() : CreatureScript("npc_josiah_avery") {}

	bool OnQuestReward(Player* player, Creature* creature, Quest const* quest, uint32 opt)
	{
		if (quest->GetQuestId() == QUEST_THE_REBEL_LORDS_ARSENAL)
		{
			creature->AddAura(SPELL_WORGEN_BITE, player);
			player->RemoveAura(SPELL_PHASE_QUEST_2);
			creature->SetPhaseMask(4, 1);
			creature->CastSpell(creature, SPELL_SUMMON_JOSIAH_AVERY);
			creature->SetPhaseMask(2, 1);
		}
		return true;
	}

	CreatureAI* GetAI(Creature* creature) const
	{
		return new npc_josiah_avery_p2AI(creature);
	}

	struct npc_josiah_avery_p2AI : public ScriptedAI
	{
		npc_josiah_avery_p2AI(Creature* creature) : ScriptedAI(creature) {}

		uint32 tSay; // Time left for say
		uint32 cSay; // Current Say

		// Evade or Respawn
		void Reset()
		{
			tSay = DELAY_SAY_JOSIAH_AVERY; // Reset timer
			cSay = 1;                              // Start from 1
		}

		//Timed events
		void UpdateAI(uint32 const diff)
		{
			//Out of combat
			if (!me->GetVictim())
			{
				//Timed say
				if (tSay <= diff)
				{
					switch (cSay)
					{
					default:
					case 1:
						Talk(SAY_JOSIAH_AVERY_1);
						cSay++;
						break;
					case 2:
						Talk(SAY_JOSIAH_AVERY_2);
						cSay++;
						break;
					case 3:
						Talk(SAY_JOSIAH_AVERY_3);
						cSay++;
						break;
					case 4:
						Talk(SAY_JOSIAH_AVERY_4);
						cSay++;
						break;
					case 5:
						Talk(SAY_JOSIAH_AVERY_5);
						cSay = 1; // Reset to 1
						break;
					}

					tSay = DELAY_SAY_JOSIAH_AVERY; // Reset the timer
				}
				else
				{
					tSay -= diff;
				}
				return;
			}
		}
	};
};

/*######
## npc_josiah_avery_trigger
######*/
class npc_josiah_avery_trigger : public CreatureScript
{
public:
	npc_josiah_avery_trigger() : CreatureScript("npc_josiah_avery_trigger") {}

	CreatureAI* GetAI(Creature* creature) const
	{
		return new npc_josiah_avery_triggerAI(creature);
	}

	struct npc_josiah_avery_triggerAI : public ScriptedAI
	{
		npc_josiah_avery_triggerAI(Creature* creature) : ScriptedAI(creature) {}

		uint32 Phase, tEvent;
		uint64 PlayerGUID;

		void Reset()
		{
			Phase = 0;
			PlayerGUID = 0;
			tEvent = 0;
		}

		void UpdateAI(uint32 const diff)
		{
			if (Creature* Lorna = me->FindNearestCreature(NPC_LORNA_CROWLEY_P4, 60.0f, true))
			if (Creature* BadAvery = me->FindNearestCreature(NPC_JOSIAH_AVERY_P4, 80.0f, true))
			if (Player* player = me->SelectNearestPlayer(50.0f))
			{
				if (!player->HasAura(SPELL_WORGEN_BITE))
					return;
				else
					PlayerGUID = player->GetGUID();
				if (tEvent <= diff)
				{
					switch (Phase)
					{
					case (0) :
					{
								 me->AI()->Talk(SAY_JOSAIH_AVERY_TRIGGER, PlayerGUID); // Tell Player they have been bitten
								 tEvent = 200;
								 Phase++;
								 break;
					}

					case (1) :
					{
								 BadAvery->SetOrientation(BadAvery->GetAngle(player)); // Face Player
								 BadAvery->CastSpell(player, 69873, true); // Do Cosmetic Attack
								 player->GetMotionMaster()->MoveKnockTo(-1791.94f, 1427.29f, 12.4584f, 22.0f, 8.0f, PlayerGUID);
								 BadAvery->getThreatManager().resetAllAggro();
								 tEvent = 1200;
								 Phase++;
								 break;
					}

					case (2) :
					{
								 BadAvery->GetMotionMaster()->MoveJump(-1791.94f, 1427.29f, 12.4584f, 18.0f, 7.0f);
								 tEvent = 600;
								 Phase++;
								 break;
					}

					case (3) :
					{
								 Lorna->CastSpell(BadAvery, SPELL_SHOOT, true);
								 tEvent = 200;
								 Phase++;
								 break;
					}

					case (4) :
					{
								 BadAvery->CastSpell(BadAvery, SPELL_GET_SHOT, true);
								 BadAvery->setDeathState(JUST_DIED);
								 player->SaveToDB();
								 BadAvery->DespawnOrUnsummon(1000);
								 me->DespawnOrUnsummon(1000);
								 tEvent = 5000;
								 Phase++;
								 break;
					}
					}
				}
				else tEvent -= diff;
			}
		}
	};
};

/*######
## npc_lorna_crowley_p4
######*/
class npc_lorna_crowley_p4 : public CreatureScript
{
public:
	npc_lorna_crowley_p4() : CreatureScript("npc_lorna_crowley_p4") {}

	bool OnQuestAccept(Player* player, Creature* creature, Quest const* quest)
	{
		if (quest->GetQuestId() == QUEST_FROM_THE_SHADOWS)
		{
			player->CastSpell(player, SPELL_SUMMON_GILNEAN_MASTIFF);
			creature->AI()->Talk(SAY_LORNA_CROWLEY_P4);
		}
		return true;
	}
};

/*######
# npc_gilnean_mastiff
######*/
class npc_gilnean_mastiff : public CreatureScript
{
public:
	npc_gilnean_mastiff() : CreatureScript("npc_gilnean_mastiff") { }

	CreatureAI* GetAI(Creature* creature) const
	{
		return new npc_gilnean_mastiffAI(creature);
	}

	struct npc_gilnean_mastiffAI : public ScriptedAI
	{
		npc_gilnean_mastiffAI(Creature* creature) : ScriptedAI(creature) {}

		void Reset()
		{
			me->GetCharmInfo()->InitEmptyActionBar(false);
			me->GetCharmInfo()->SetActionBar(0, SPELL_ATTACK_LURKER, ACT_PASSIVE);
			me->SetReactState(REACT_DEFENSIVE);
			me->GetCharmInfo()->SetIsFollowing(true);
		}

		void UpdateAI(uint32 const diff) /*diff*/
		{
			Player* player = me->GetOwner()->ToPlayer();

			if (player->GetQuestStatus(QUEST_FROM_THE_SHADOWS) == QUEST_STATUS_REWARDED)
			{
				me->DespawnOrUnsummon(1);
			}

			if (!UpdateVictim())
			{
				me->GetCharmInfo()->SetIsFollowing(true);
				me->SetReactState(REACT_DEFENSIVE);
				return;
			}

			DoMeleeAttackIfReady();
		}

		void SpellHitTarget(Unit* Mastiff, SpellInfo const* cSpell)
		{
			if (cSpell->Id == SPELL_ATTACK_LURKER)
			{
				Mastiff->RemoveAura(SPELL_SHADOWSTALKER_STEALTH);
				Mastiff->AddThreat(me, 1.0f);
				me->AddThreat(Mastiff, 1.0f);
				me->AI()->AttackStart(Mastiff);
			}
		}

		void JustDied(Unit* /*killer*/) // Otherwise, player is stuck with pet corpse they cannot remove from world
		{
			me->DespawnOrUnsummon(1);
		}

		void KilledUnit(Unit* /*victim*/)
		{
			Reset();
		}
	};
};

/*######
## npc_bloodfang_lurker
######*/
class npc_bloodfang_lurker : public CreatureScript
{
public:
	npc_bloodfang_lurker() : CreatureScript("npc_bloodfang_lurker") {}

	CreatureAI* GetAI(Creature* creature) const
	{
		return new npc_bloodfang_lurkerAI(creature);
	}

	struct npc_bloodfang_lurkerAI : public ScriptedAI
	{
		npc_bloodfang_lurkerAI(Creature* creature) : ScriptedAI(creature) {}

		uint32 tEnrage, tSeek;
		bool willCastEnrage;

		void Reset()
		{
			tEnrage = 0;
			willCastEnrage = urand(0, 1);
			tSeek = urand(5000, 10000);
			DoCast(me, SPELL_SHADOWSTALKER_STEALTH);
		}

		void UpdateAI(uint32 const diff)
		{
			if (tSeek <= diff)
			{
				if ((me->isAlive()) && (!me->isInCombat() && (me->GetDistance2d(me->GetHomePosition().GetPositionX(), me->GetHomePosition().GetPositionY()) <= 2.0f)))
				if (Player* player = me->SelectNearestPlayer(2.0f))
				{
					if (!player->isInCombat())
					{
						me->AI()->AttackStart(player);
						tSeek = urand(5000, 10000);
					}
				}
			}
			else tSeek -= diff;

			if (!UpdateVictim())
				return;

			if (tEnrage <= diff && willCastEnrage && me->GetHealthPct() <= 30)
			{
				me->MonsterTextEmote(-106, 0);
				DoCast(me, SPELL_ENRAGE);
				tEnrage = CD_ENRAGE;
			}
			else
				tEnrage -= diff;

			DoMeleeAttackIfReady();
		}
	};
};

/*######
## spell_attack_lurker
#####*/
class spell_attack_lurker : public SpellScriptLoader
{
public:
	spell_attack_lurker() : SpellScriptLoader("spell_attack_lurker"){}
	class spell_attack_lurker_SpellScript : public SpellScript
	{
		PrepareSpellScript(spell_attack_lurker_SpellScript);
		void HandleDummy(SpellEffIndex /*index*/)
		{
			Unit * caster = GetCaster();
			caster->CastSpell(caster, SPELL_ATTACK_LURKER, true);
			caster->SummonCreature(35463, caster->GetPositionX(), caster->GetPositionY(), caster->GetPositionZ(), caster->GetOrientation(), TEMPSUMMON_DEAD_DESPAWN);
		}
		void Register()
		{
			OnEffectHit += SpellEffectFn(spell_attack_lurker_SpellScript::HandleDummy, EFFECT_0, SPELL_EFFECT_DUMMY);
		}
	};

	SpellScript * GetSpellScript() const
	{
		return new spell_attack_lurker_SpellScript();
	}

};

/*######
## npc_king_genn_greymane
######*/
class npc_king_genn_greymane : public CreatureScript
{
public:
	npc_king_genn_greymane() : CreatureScript("npc_king_genn_greymane") {}

	CreatureAI* GetAI(Creature* creature) const
	{
		return new npc_king_genn_greymaneAI(creature);
	}

	struct npc_king_genn_greymaneAI : public ScriptedAI
	{
		npc_king_genn_greymaneAI(Creature* creature) : ScriptedAI(creature) {}

		uint32 tSummon, tSay;
		bool EventActive, RunOnce;

		void Reset()
		{
			tSay = urand(10000, 20000);
			tSummon = urand(3000, 5000); // How often we spawn
		}

		void SummonNextWave()
		{
			switch (urand(1, 4))
			{
			case (1) :
				for (int i = 0; i < 5; i++)
					me->SummonCreature(NPC_BLOODFANG_RIPPER_P4, -1781.173f + irand(-15, 15), 1372.90f + irand(-15, 15), 19.7803f, urand(0, 6), TEMPSUMMON_TIMED_OR_DEAD_DESPAWN, 15000);
				break;
			case (2) :
				for (int i = 0; i < 5; i++)
					me->SummonCreature(NPC_BLOODFANG_RIPPER_P4, -1756.30f + irand(-15, 15), 1380.61f + irand(-15, 15), 19.7652f, urand(0, 6), TEMPSUMMON_TIMED_OR_DEAD_DESPAWN, 15000);
				break;
			case (3) :
				for (int i = 0; i < 5; i++)
					me->SummonCreature(NPC_BLOODFANG_RIPPER_P4, -1739.84f + irand(-15, 15), 1384.87f + irand(-15, 15), 19.841f, urand(0, 6), TEMPSUMMON_TIMED_OR_DEAD_DESPAWN, 15000);
				break;
			case (4) :
				for (int i = 0; i < 5; i++)
					me->SummonCreature(NPC_BLOODFANG_RIPPER_P4, -1781.173f + irand(-15, 15), 1372.90f + irand(-15, 15), 19.7803f, urand(0, 6), TEMPSUMMON_TIMED_OR_DEAD_DESPAWN, 15000);
				break;
			}
		}

		void UpdateAI(uint32 const diff)
		{
			if (tSay <= diff) // Time for next spawn wave
			{
				Talk(SAY_KING_GENN_GREYMANE);
				tSay = urand(10000, 20000);
			}
			else tSay -= diff;

			if (tSummon <= diff) // Time for next spawn wave
			{
				SummonNextWave(); // Activate next spawn wave
				tSummon = urand(3000, 5000); // Reset our spawn timer
			}
			else tSummon -= diff;
		}

		void JustSummoned(Creature* summoned)
		{
			summoned->GetDefaultMovementType();
			summoned->SetReactState(REACT_AGGRESSIVE);
		}
	};
};

/*######
## npc_king_greymanes_horse
######*/

class npc_king_greymanes_horse : public CreatureScript
{
public:
	npc_king_greymanes_horse() : CreatureScript("npc_king_greymanes_horse") { }

	CreatureAI* GetAI(Creature* creature) const
	{
		return new npc_king_greymanes_horseAI(creature);
	}

	struct npc_king_greymanes_horseAI : public npc_escortAI
	{
		npc_king_greymanes_horseAI(Creature* creature) : npc_escortAI(creature) {}

		uint32 krennansay;
		bool PlayerOn, KrennanOn;

		void AttackStart(Unit* /*who*/) {}
		void EnterCombat(Unit* /*who*/) {}
		void EnterEvadeMode() {}

		void Reset()
		{
			krennansay = 500;//Check every 500ms initially
			PlayerOn = false;
			KrennanOn = false;
		}

		void PassengerBoarded(Unit* who, int8 /*seatId*/, bool apply)
		{
			if (who->GetTypeId() == TYPEID_PLAYER)
			{
				PlayerOn = true;
				if (apply)
				{
					Start(false, true, who->GetGUID());
				}
			}
			else if (who->GetTypeId() == TYPEID_UNIT)
			{
				KrennanOn = true;
				SetEscortPaused(false);
			}
		}

		void WaypointReached(uint32 i)
		{
			Player* player = GetPlayerForEscort();

			switch (i)
			{
			case 5:
				Talk(SAY_GREYMANE_HORSE, player->GetGUID());
				me->GetMotionMaster()->MoveJump(-1679.089f, 1348.42f, 15.31f, 25.0f, 15.0f);
				if (me->GetVehicleKit()->HasEmptySeat(1))
				{
					SetEscortPaused(true);
					break;
				}
				else
					break;
			case 12:
				player->ExitVehicle();
				player->SetClientControl(me, 1);
				break;
			}
		}

		void JustDied(Unit* /*killer*/)
		{
			if (Player* player = GetPlayerForEscort())
				player->FailQuest(QUEST_SAVE_KRENNAN_ARANAS);
		}

		void OnCharmed(bool /*apply*/)
		{
		}

		void UpdateAI(const uint32 diff)
		{
			npc_escortAI::UpdateAI(diff);
			Player* player = GetPlayerForEscort();

			if (PlayerOn)
			{
				player->SetClientControl(me, 0);
				PlayerOn = false;
			}

			if (KrennanOn) // Do Not yell for help after krennan is on
				return;

			if (krennansay <= diff)
			{
				if (Creature* krennan = me->FindNearestCreature(NPC_KRENNAN_ARANAS_TREE, 70.0f, true))
				{
					krennan->AI()->Talk(SAY_NPC_KRENNAN_ARANAS_TREE, player->GetGUID());
					krennansay = urand(4000, 7000);//Repeat every 4 to 7 seconds
				}
			}
			else
				krennansay -= diff;
		}
	};
};

/*######
## npc_krennan_aranas_c2
######*/

class npc_krennan_aranas_c2 : public CreatureScript
{
public:
	npc_krennan_aranas_c2() : CreatureScript("npc_krennan_aranas_c2") { }

	CreatureAI* GetAI(Creature* creature) const
	{
		return new npc_krennan_aranas_c2AI(creature);
	}

	struct npc_krennan_aranas_c2AI : public ScriptedAI
	{
		npc_krennan_aranas_c2AI(Creature* creature) : ScriptedAI(creature) {}

		bool Say, Move, Cast, KrennanDead;
		uint32 SayTimer;

		void AttackStart(Unit* /*who*/) {}
		void EnterCombat(Unit* /*who*/) {}
		void EnterEvadeMode() {}

		void Reset()
		{
			Say = false;
			Move = true;
			Cast = true;
			KrennanDead = false;
			SayTimer = 500;
		}

		void UpdateAI(const uint32 diff)
		{
			if (Creature* krennan = me->FindNearestCreature(NPC_KRENNAN_ARANAS_TREE, 50.0f))
			{
				if (!KrennanDead)
				{
					krennan->DespawnOrUnsummon(0);
					KrennanDead = true;
				}
			}

			if (Creature* horse = me->FindNearestCreature(NPC_GREYMANE_HORSE_P4, 20.0f))//Jump onto horse in seat 2
			{
				if (Cast)
				{
					DoCast(horse, 84275, true);
				}

				if (me->HasAura(84275))
				{
					Cast = false;
				}
			}

			if (!me->HasAura(84275) && Move)
			{
				me->NearTeleportTo(KRENNAN_END_X, KRENNAN_END_Y, KRENNAN_END_Z, KRENNAN_END_O);
				Say = true;
				Move = false;
				SayTimer = 500;
			}

			if (Say && SayTimer <= diff)
			{
				Talk(SAY_KRENNAN_C2);
				me->DespawnOrUnsummon(6000);
				Say = false;
			}
			else
				SayTimer -= diff;
		}
	};
};

/*######
## npc_commandeered_cannon
######*/
class npc_commandeered_cannon : public CreatureScript
{
public:
	npc_commandeered_cannon() : CreatureScript("npc_commandeered_cannon") { }

	CreatureAI* GetAI(Creature* creature) const
	{
		return new npc_commandeered_cannonAI(creature);
	}

	struct npc_commandeered_cannonAI : public ScriptedAI
	{
		npc_commandeered_cannonAI(Creature* creature) : ScriptedAI(creature) {}

		uint32 tEvent;
		uint8 Count, Phase;
		bool EventStart;

		void Reset()
		{
			tEvent = 1400;
			Phase = 0;
			Count = 0;
			EventStart = false;
		}

		void UpdateAI(const uint32 diff)
		{
			if (!EventStart)
				return;

			if (Count > 2)
			{
				Reset();
				return;
			}

			if (tEvent <= diff)
			{
				switch (Phase)
				{
				case (0) :
					for (int i = 0; i < 12; i++)
					{
						me->SummonCreature(NPC_BLOODFANG_WORGEN, -1757.65f + irand(-6, 6), 1384.01f + irand(-6, 6), 19.872f, urand(0, 6), TEMPSUMMON_TIMED_DESPAWN, 5000);
					}
					tEvent = 400;
					Phase++;
					break;

				case (1) :
					if (Creature* Worgen = me->FindNearestCreature(NPC_BLOODFANG_WORGEN, 50.0f, true))
					{
						me->CastSpell(Worgen, SPELL_CANNON_FIRE, true);
						tEvent = 1700;
						Phase = 0;
						Count++;
					}
					break;
				}
			}
			else tEvent -= diff;
		}

		void JustSummoned(Creature* summon)
		{
			summon->SetFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_DISABLE_MOVE);
		}
	};
};

/*######
## npc_lord_godfrey_p4_8
######*/
class npc_lord_godfrey_p4_8 : public CreatureScript
{
public:
	npc_lord_godfrey_p4_8() : CreatureScript("npc_lord_godfrey_p4_8") { }

	bool OnQuestReward(Player* player, Creature* godfrey, Quest const* quest, uint32 opt)
	{
		if (quest->GetQuestId() == QUEST_SAVE_KRENNAN_ARANAS)
		{
			godfrey->AI()->Talk(SAY_LORD_GODFREY_P4);
			player->RemoveAura(SPELL_WORGEN_BITE);
			godfrey->AddAura(SPELL_INFECTED_BITE, player);
			player->RemoveAura(76642);// Only Infected bite aura should be added
			player->CastSpell(player, SPELL_GILNEAS_CANNON_CAMERA);
			player->SaveToDB();
			if (Creature* cannon = GetClosestCreatureWithEntry(godfrey, NPC_COMMANDEERED_CANNON, 50.0f))
			{
				CAST_AI(npc_commandeered_cannon::npc_commandeered_cannonAI, cannon->AI())->EventStart = true; // Start Event
			}
		}
		return true;
	}
};

/*######
## npc_lord_darius_crowley_c2
######*/
class npc_lord_darius_crowley_c2 : public CreatureScript
{
public:
	npc_lord_darius_crowley_c2() : CreatureScript("npc_lord_darius_crowley_c2") { }

	bool OnQuestAccept(Player* player, Creature* creature, Quest const*_Quest)
	{
		if (_Quest->GetQuestId() == QUEST_SACRIFICES)
		{
			if (Creature *horse = player->SummonCreature(35231, player->GetPositionX(), player->GetPositionY(), player->GetPositionZ(), player->GetOrientation(), TEMPSUMMON_MANUAL_DESPAWN))
			{
				player->CastCustomSpell(VEHICLE_SPELL_RIDE_HARDCODED, SPELLVALUE_BASE_POINT0, 1, horse, false);
				CAST_AI(npc_escortAI, (horse->AI()))->Start(false, true, player->GetGUID(), _Quest);
			}
		}
		return true;
	}
};

/*######
## npc_crowley_horse
######*/

class npc_crowley_horse : public CreatureScript
{
public:
	npc_crowley_horse() : CreatureScript("npc_crowley_horse") {}

	struct npc_crowley_horseAI : public npc_escortAI
	{
		npc_crowley_horseAI(Creature* creature) : npc_escortAI(creature) {}

		bool CrowleyOn;
		bool CrowleySpawn;
		bool Run;

		void AttackStart(Unit* /*who*/) {}
		void EnterCombat(Unit* /*who*/) {}
		void EnterEvadeMode() {}

		void Reset()
		{
			CrowleyOn = false;
			CrowleySpawn = false;
			Run = false;
		}

		void PassengerBoarded(Unit* who, int8 /*seatId*/, bool apply)
		{
			if (who->GetTypeId() == TYPEID_PLAYER)
			{
				if (apply)
				{
					Start(false, true, who->GetGUID());
				}
			}
		}

		void WaypointReached(uint32 i)
		{
			Player* player = GetPlayerForEscort();
			Creature* crowley = me->FindNearestCreature(NPC_DARIUS_CROWLEY, 5, true);

			switch (i)
			{
			case 1:
				player->SetClientControl(me, 0);
				me->ApplySpellImmune(0, IMMUNITY_SCHOOL, SPELL_SCHOOL_MASK_FIRE, true);
				crowley->SetFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_NON_ATTACKABLE);
				me->GetMotionMaster()->MoveJump(-1714.02f, 1666.37f, 20.57f, 25.0f, 15.0f);
				break;
			case 4:
				crowley->AI()->Talk(SAY_CROWLEY_HORSE_1);
				break;
			case 10:
				me->GetMotionMaster()->MoveJump(-1571.32f, 1710.58f, 20.49f, 25.0f, 15.0f);
				break;
			case 11:
				crowley->AI()->Talk(SAY_CROWLEY_HORSE_2);
				break;
			case 16:
				crowley->AI()->Talk(SAY_CROWLEY_HORSE_2);
				break;
			case 20:
				me->SetFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_NON_ATTACKABLE);
				me->getThreatManager().resetAllAggro();
				player->SetFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_NON_ATTACKABLE);
				player->getThreatManager().resetAllAggro();
				break;
			case 21:
				player->SetClientControl(me, 1);
				player->RemoveFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_NON_ATTACKABLE);
				player->RemoveAura(SPELL_THROW_TORCH);
				player->ExitVehicle();
				break;
			}
		}

		void JustDied(Unit* /*killer*/)
		{
			if (Player* player = GetPlayerForEscort())
				player->FailQuest(QUEST_SACRIFICES);
		}

		void OnCharmed(bool /*apply*/)
		{
		}

		void UpdateAI(const uint32 diff)
		{
			npc_escortAI::UpdateAI(diff);

			if (me->HasAura(SPELL_THROW_TORCH))
				me->RemoveAura(SPELL_THROW_TORCH);

			if (!CrowleySpawn)
			{
				DoCast(SPELL_SUMMON_CROWLEY);
				if (Creature* crowley = me->FindNearestCreature(NPC_DARIUS_CROWLEY, 5, true))
				{
					CrowleySpawn = true;
				}
			}

			if (CrowleySpawn && !CrowleyOn)
			{
				Creature* crowley = me->FindNearestCreature(NPC_DARIUS_CROWLEY, 5, true);
				crowley->CastSpell(me, SPELL_RIDE_HORSE, true);//Mount Crowley in seat 1
				CrowleyOn = true;
			}

			if (!Run)
			{
				me->SetSpeed(MOVE_RUN, CROWLEY_SPEED);
				Run = true;
			}
		}
	};

	CreatureAI* GetAI(Creature* creature) const
	{
		return new npc_crowley_horseAI(creature);
	}
};

/*######
## npc_bloodfang_stalker_c1
######*/
class npc_bloodfang_stalker_c1 : public CreatureScript
{
public:
	npc_bloodfang_stalker_c1() : CreatureScript("npc_bloodfang_stalker_c1") {}

	CreatureAI* GetAI(Creature* creature) const
	{
		return new npc_bloodfang_stalker_c1AI(creature);
	}

	struct npc_bloodfang_stalker_c1AI : public ScriptedAI
	{
		npc_bloodfang_stalker_c1AI(Creature* creature) : ScriptedAI(creature) {}

		Player* player;
		uint32 tEnrage;
		uint32 tAnimate;
		uint32 BurningReset;
		bool Miss, willCastEnrage;
		bool spellHit;

		void Reset()
		{
			tEnrage = 0;
			tAnimate = DELAY_ANIMATE;
			Miss = false;
			willCastEnrage = urand(0, 1);
			BurningReset = 3000;
			spellHit = false;
		}

		void SpellHit(Unit* caster, const SpellInfo* spell)
		{

			if (caster->HasAura(SPELL_THROW_TORCH))
				caster->RemoveAura(SPELL_THROW_TORCH);

			if (spell->Id == SPELL_THROW_TORCH && caster->GetTypeId() == TYPEID_PLAYER
				&& CAST_PLR(caster)->GetQuestStatus(QUEST_SACRIFICES) == QUEST_STATUS_INCOMPLETE)
			{
				caster->ToPlayer()->KilledMonsterCredit(me->GetEntry(), me->GetGUID());
				me->DespawnOrUnsummon(5000);
			}
		}

		void UpdateAI(const uint32 diff)
		{
			if (me->HasAura(SPELL_THROW_TORCH))
				spellHit = true;
			else
				spellHit = false;

			if (spellHit && BurningReset <= diff)
			{
				me->RemoveAllAuras();
				BurningReset = 5000;
				spellHit = false;
			}
			else
				BurningReset -= diff;

			if (!UpdateVictim())
				return;

			if (tEnrage <= diff && willCastEnrage)
			{
				if (me->GetHealthPct() <= 30)
				{
					Talk(0);
					DoCast(me, SPELL_ENRAGE);
					tEnrage = CD_ENRAGE;
				}
			}
			else tEnrage -= diff;

			if (me->GetVictim()->GetTypeId() == TYPEID_PLAYER)
				Miss = false;
			else if (me->GetVictim()->isPet())
				Miss = false;
			else if (me->GetVictim()->GetEntry() == NPC_NORTHGATE_REBEL_1)
			if (me->GetVictim()->GetHealthPct() < 90)
				Miss = true;

			if (Miss && tAnimate <= diff)
			{
				me->HandleEmoteCommand(EMOTE_ONESHOT_ATTACK_UNARMED);
				me->PlayDistanceSound(SOUND_WORGEN_ATTACK);
				tAnimate = DELAY_ANIMATE;
			}
			else
				tAnimate -= diff;

			if (!Miss)
				DoMeleeAttackIfReady();
		}

	};
};

/*######
## npc_lord_darius_crowley_c3
######*/
class npc_lord_darius_crowley_c3 : public CreatureScript
{
public:
	npc_lord_darius_crowley_c3() : CreatureScript("npc_lord_darius_crowley_c3") {}

	bool OnQuestAccept(Player* pPlayer, Creature* pCreature, Quest const *pQuest)
	{
		if (pQuest->GetQuestId() == 14222) //Last Stand
		{
			if (!pPlayer->FindNearestCreature(35566, 20.0f, true))

				pPlayer->SummonCreature(NPC_FRENZIED_STALKER, -1600.35f, 1509.94f, 29.2493f, 3.27328f, TEMPSUMMON_TIMED_OR_DEAD_DESPAWN, 90000);
			pPlayer->SummonCreature(NPC_FRENZIED_STALKER, -1585.89f, 1532.81f, 29.2244f, 1.60431f, TEMPSUMMON_TIMED_OR_DEAD_DESPAWN, 90000);
			pPlayer->SummonCreature(NPC_FRENZIED_STALKER, -1608.65f, 1523.72f, 29.2357f, 0.931657f, TEMPSUMMON_TIMED_OR_DEAD_DESPAWN, 90000);
			pPlayer->SummonCreature(NPC_FRENZIED_STALKER, -1594.44f, 1521.0f, 29.2395f, 4.25782f, TEMPSUMMON_TIMED_OR_DEAD_DESPAWN, 90000);
			pPlayer->SummonCreature(NPC_FRENZIED_STALKER, -1577.14f, 1529.68f, 29.2263f, 3.57845f, TEMPSUMMON_TIMED_OR_DEAD_DESPAWN, 90000);
			pPlayer->SummonCreature(NPC_FRENZIED_STALKER, -1587.16f, 1544.03f, 29.7076f, 5.32204f, TEMPSUMMON_TIMED_OR_DEAD_DESPAWN, 90000);
			pPlayer->SummonCreature(NPC_FRENZIED_STALKER, -1607.19f, 1511.9f, 29.2489f, 3.97741f, TEMPSUMMON_TIMED_OR_DEAD_DESPAWN, 90000);
			pPlayer->SummonCreature(NPC_FRENZIED_STALKER, -1587.34f, 1532.76f, 29.2273f, 5.0854f, TEMPSUMMON_TIMED_OR_DEAD_DESPAWN, 90000);
		}
		return true;
	}

	bool OnQuestReward(Player* player, Creature* creature, const Quest *_Quest, uint32)
	{
		if (_Quest->GetQuestId() == 14222)
		{
			WorldLocation loc;
			loc.m_mapId = 654;
			loc.m_positionX = -1818.4f;
			loc.m_positionY = 2294.25f;
			loc.m_positionZ = 42.2135f;
			loc.m_orientation = 3.14f;

			player->SetHomebind(loc, 4786);

			player->CastSpell(player, 94293, true);
			player->CastSpell(player, 68996, true);
			player->CastSpell(player, 69196, true);
			player->CastSpell(player, 72788, true);
			//player->CastSpell(player, 72794, true);
			player->RemoveAura(72872);
			player->TeleportTo(loc);
		}
		return true;
	}
};

class PlayerAlreadyStartWorgenZone2
{
public:
	PlayerAlreadyStartWorgenZone2() { }

	bool operator()(Player* player) const
	{
		return (player->GetQuestStatus(14375) != QUEST_STATUS_NONE || player->HasAura(68630));
	}
};

/*###### Start of DuskHaven ######*/

/*######
## npc_trigger_event_c3
######*/
class npc_trigger_event_c3 : public CreatureScript
{
public:
	npc_trigger_event_c3() : CreatureScript("npc_trigger_event_c3") {}

	CreatureAI* GetAI(Creature* creature) const
	{
		return new npc_trigger_event_c3AI(creature);
	}

	struct npc_trigger_event_c3AI : public ScriptedAI
	{
		npc_trigger_event_c3AI(Creature* creature) : ScriptedAI(creature) {}

		void Reset()
		{
			mui_refreshEvent = 1000;
			canStartEvent = false;
		}

		void UpdateAI(const uint32 diff)
		{
			if (mui_refreshEvent <= diff)
			{
				std::list<Player *> _players = me->GetPlayersInRange(10, true);
				_players.remove_if(PlayerAlreadyStartWorgenZone2());
				bool endLoop = true;
				for (std::list<Player *>::const_iterator itr = _players.begin(); itr != _players.end(); itr++)
				{
					if (!canStartEvent)
					{
						canStartEvent = true;
						break;
					}
					endLoop = false;
					(*itr)->CastSpell((*itr), 68630, true);
					(*itr)->CastSpell((*itr), 72799, true);
				}
				if (!endLoop)
					canStartEvent = false;
				mui_refreshEvent = 3000;
			}
			else
				mui_refreshEvent -= diff;
		}

	private:
		uint32 mui_refreshEvent;
		bool canStartEvent;
	};
};

/*######
## npc_king_genn_greymane_c2
######*/
class npc_king_genn_greymane_c2 : public CreatureScript
{
public:
	npc_king_genn_greymane_c2() : CreatureScript("npc_king_genn_greymane_c2") {}

	bool OnQuestReward(Player* player, Creature* creature, Quest const* _Quest, uint32)
	{
		if (_Quest->GetQuestId() == 14375)
		{
			player->RemoveAurasDueToSpell(68630);
			player->RemoveAurasDueToSpell(69196);
			player->CastSpell(player, SPELL_FADE_BACK, true);
			player->CastSpell(player, 72799, true);
			creature->DespawnOrUnsummon();
		}
		return true;
	}
};

/*######
## npc_trigger_event_first_c3
######*/
class npc_trigger_event_first_c3 : public CreatureScript
{
public:
	npc_trigger_event_first_c3() : CreatureScript("npc_trigger_event_first_c3") {}

	CreatureAI* GetAI(Creature* creature) const
	{
		return new npc_trigger_event_first_c3AI(creature);
	}

	struct npc_trigger_event_first_c3AI : public ScriptedAI
	{
		npc_trigger_event_first_c3AI(Creature* creature) : ScriptedAI(creature) {}

		void Reset()
		{
			godfreyGUID = 0;
			gennGUID = 0;
			if (Creature *godfrey = me->SummonCreature(36330, -1844.29f, 2290.67f, 42.30f, 0.32f, TEMPSUMMON_MANUAL_DESPAWN))
				godfreyGUID = godfrey->GetGUID();
			if (Creature *genn = me->SummonCreature(36332, -1844.29f, 2290.67f, 42.30f, 0.32f, TEMPSUMMON_MANUAL_DESPAWN))
			{
				gennGUID = genn->GetGUID();
				genn->RemoveFlag(UNIT_NPC_FLAGS, UNIT_NPC_FLAG_QUESTGIVER);
			}
			endEvent = false;
			mui_timer_event = 12500;
			step = 0;
			me->GetMotionMaster()->MovePoint(42, -1819.72f, 2289.23f, 42.28f);
			waitingForEndMovement = false;
		}

		void MovementInform(uint32 type, uint32 point)
		{
			Creature* godfrey = Unit::GetCreature(*me, godfreyGUID);
			Creature* genn = Unit::GetCreature(*me, gennGUID);
			switch (point)
			{
			case 0:
				waitingForEndMovement = false;
				Talk(0);
				break;
			case 42:
				me->SetFacingTo(1.75f);
				break;
			default:
				break;
			}
		}

		void UpdateAI(const uint32 diff)
		{
			if (waitingForEndMovement)
				return;
			if (mui_timer_event <= diff)
			{
				Creature* godfrey = Unit::GetCreature(*me, godfreyGUID);
				Creature* genn = Unit::GetCreature(*me, gennGUID);
				if (!genn)
				if (godfrey)
					godfrey->DespawnOrUnsummon();
				if (!godfrey || !genn)
				{
					me->DespawnOrUnsummon();
					return;
				}
				switch (step)
				{
				case 0:
					me->GetMotionMaster()->MovePoint(step, -1819.80f, 2290.40f, 42.22f);
					waitingForEndMovement = true;
					step++;
					break;
				case 1:
					godfrey->GetMotionMaster()->MovePoint(step, -1822.14f, 2295.40f, 42.1f);
					mui_timer_event = 4000;
					step++;
					break;
				case 2:
					godfrey->SetFacingToObject(me);
					godfrey->AI()->Talk(0);
					mui_timer_event = 4000;
					step++;
					break;
				case 3:
					genn->GetMotionMaster()->MovePoint(step, -1821.69f, 2292.76f, 42.08f);
					mui_timer_event = 4000;
					step++;
					break;
				case 4:
					genn->SetFacingToObject(godfrey);
					genn->AI()->Talk(0);
					mui_timer_event = 8000;
					step++;
					break;
				case 5:
					genn->AI()->Talk(1);
					genn->SetFacingToObject(me);
					mui_timer_event = 4000;
					step++;
					break;
				case 6:
					genn->SetFlag(UNIT_NPC_FLAGS, UNIT_NPC_FLAG_QUESTGIVER);
					mui_timer_event = 300000;
					endEvent = true;
					step++;
					break;
				case 7:
					godfrey->DespawnOrUnsummon();
					me->DespawnOrUnsummon();
					step++;
				default:
					break;
				}
			}
			else
				mui_timer_event -= diff;
		}

	private:
		uint64 godfreyGUID;
		uint64 gennGUID;
		bool endEvent;
		uint32 mui_timer_event;
		uint32 step;
		bool waitingForEndMovement;
	};
};

/*######
## go_mandragore
######*/
class go_mandragore : public GameObjectScript
{
public:
	go_mandragore() : GameObjectScript("go_mandragore") {}

	bool OnQuestReward(Player* player, GameObject *, Quest const* _Quest, uint32)
	{
		if (_Quest->GetQuestId() == 14320)
		{
			player->SendCinematicStart(168);
			WorldPacket data(SMSG_PLAY_SOUND, 4);
			data << uint32(23676);
			player->GetSession()->SendPacket(&data);
		}
		return true;
	}
};

/*######
## Quest Invasion 14321
######*/

enum sWatchman
{
	QUEST_INVASION = 14321,
	NPC_FORSAKEN_ASSASSIN = 36207,
	SPELL_BACKSTAB = 75360,
	NPC_FORSAKEN_ASSASSIN_SAY = 0,
};

class npc_slain_watchman : public CreatureScript
{
public:
	npc_slain_watchman() : CreatureScript("npc_slain_watchman") { }

	bool OnQuestAccept(Player* player, Creature* creature, Quest const* quest)
	{
		if (quest->GetQuestId() == QUEST_INVASION)
		{
			if (Creature* forsaken = player->SummonCreature(NPC_FORSAKEN_ASSASSIN, -1918.782104f, 2393.562988f, 30.192066f, 6.265f, TEMPSUMMON_TIMED_DESPAWN_OUT_OF_COMBAT, 6000))
			{
				forsaken->SetPhaseMask(1, true);
				forsaken->SetVisible(true);
				forsaken->setFaction(14);
				forsaken->AI()->Talk(NPC_FORSAKEN_ASSASSIN_SAY);
				forsaken->AI()->AttackStart(player);
				forsaken->CastSpell(player, SPELL_BACKSTAB, false);
			}
		}

		return true;
	}
};

class npc_gwen_armstead_qi : public CreatureScript
{
public:
	npc_gwen_armstead_qi() : CreatureScript("npc_gwen_armstead_qi") { }

	bool OnQuestComplete(Player* player, Creature* armstead, Quest const* quest)
	{
		if (quest->GetQuestId() == QUEST_INVASION)
		{
			player->CastSpell(player, 80224, true);
			player->SaveToDB();
		}

		return true;
	}
};

enum greymane_dh
{
	NPC_FORSAKEN_INVADER = 34511,
	SPELL_THROW_BOTTLE = 68552
};

class npc_prince_liam_greymane_dh : public CreatureScript
{
public:
	npc_prince_liam_greymane_dh() : CreatureScript("npc_prince_liam_greymane_dh") { }

	CreatureAI* GetAI(Creature* creature) const
	{
		return new npc_prince_liam_greymane_dhAI(creature);
	}

	struct npc_prince_liam_greymane_dhAI : public ScriptedAI
	{
		npc_prince_liam_greymane_dhAI(Creature* creature) : ScriptedAI(creature)
		{
			SetCombatMovement(false);
		}

		uint32 uiShootTimer;
		bool miss;

		void Reset()
		{
			uiShootTimer = 1000;
			miss = false;

			if (Unit* target = me->FindNearestCreature(NPC_FORSAKEN_INVADER, 40.0f))
				AttackStart(target);
		}

		void UpdateAI(uint32 const diff)
		{
			if (!UpdateVictim())
				return;

			if (me->GetVictim()->GetTypeId() == TYPEID_UNIT)
			{
				if (me->GetVictim()->GetHealthPct() < 90)
					miss = true;
				else
					miss = false;
			}

			if (uiShootTimer <= diff)
			{
				uiShootTimer = 1500;

				if (!me->IsWithinMeleeRange(me->GetVictim(), 0.0f))
				if (Unit* target = me->FindNearestCreature(NPC_FORSAKEN_INVADER, 40.0f))
				if (target != me->GetVictim())
				{
					me->getThreatManager().modifyThreatPercent(me->GetVictim(), -100);
					me->CastSpell(me->GetVictim(), SPELL_THROW_BOTTLE, false);
					me->CombatStart(target);
					me->AddThreat(target, 1000);
				}

				if (!me->IsWithinMeleeRange(me->GetVictim(), 0.0f))
				{
					if (me->HasUnitState(UNIT_STATE_MELEE_ATTACKING))
					{
						me->SetUInt32Value(UNIT_NPC_EMOTESTATE, 0);
						me->ClearUnitState(UNIT_STATE_MELEE_ATTACKING);
						me->SendMeleeAttackStop(me->GetVictim());
					}

					me->CastSpell(me->GetVictim(), SPELL_SHOOT, false);
				}
				else
				if (!me->HasUnitState(UNIT_STATE_MELEE_ATTACKING))
				{
					me->AddUnitState(UNIT_STATE_MELEE_ATTACKING);
					me->SendMeleeAttackStart(me->GetVictim());
				}
			}
			else
				uiShootTimer -= diff;

			DoMeleeAttackIfReady();
		}
	};
};

/* ######
## You Can't Take 'Em Alone - 14348
###### */

enum eHorrid
{
	NPC_HORRID_ABOMINATION_KILL_CREDIT = 36233,
	NPC_PRINCE_LIAM_GREYMANE_QYCTEA = 36140,

	SAY_BARREL = 0,

	SPELL_KEG_PLACED = 68555,
	SPELL_SHOOT_QYCTEA = 68559,   // 68559
	SPELL_RESTITCHING = 68864,
	SPELL_EXPLOSION = 68560,
	SPELL_EXPLOSION_POISON = 42266,
	SPELL_EXPLOSION_BONE_TYPE_ONE = 42267,
	SPELL_EXPLOSION_BONE_TYPE_TWO = 42274,
};

class npc_horrid_abomination : public CreatureScript
{
public:
	npc_horrid_abomination() : CreatureScript("npc_horrid_abomination") { }

	CreatureAI* GetAI(Creature* creature) const
	{
		return new npc_horrid_abominationAI(creature);
	}

	struct npc_horrid_abominationAI : public ScriptedAI
	{
		npc_horrid_abominationAI(Creature* creature) : ScriptedAI(creature)
		{
			uiRestitchingTimer = 3000;
			uiShootTimer = 3000;
			uiPlayerGUID = 0;
			shoot = false;
			miss = false;
			me->_ReactDistance = 10.0f;
		}

		uint64 uiPlayerGUID;
		uint32 uiShootTimer;
		uint32 uiRestitchingTimer;
		bool shoot;
		bool miss;

		void SpellHit(Unit* caster, const SpellInfo* spell)
		{
			if (spell->Id == SPELL_KEG_PLACED)
			{
				me->AI()->Talk(NPC_FORSAKEN_ASSASSIN_SAY);
				shoot = true;
				uiPlayerGUID = caster->GetGUID();
				me->SetReactState(REACT_PASSIVE);
				me->GetMotionMaster()->MoveRandom(5.0f);
				me->CombatStop();

			}

			if (spell->Id == SPELL_SHOOT_QYCTEA)
				ShootEvent();
		}

		void ShootEvent()
		{
			me->RemoveAura(SPELL_KEG_PLACED);

			for (int i = 0; i < 11; ++i)
				DoCast(SPELL_EXPLOSION_POISON);

			for (int i = 0; i < 6; ++i)
				DoCast(SPELL_EXPLOSION_BONE_TYPE_ONE);

			for (int i = 0; i < 4; ++i)
				DoCast(SPELL_EXPLOSION_BONE_TYPE_TWO);

			DoCast(SPELL_EXPLOSION);

			if (Player* player = Unit::GetPlayer(*me, uiPlayerGUID))
				player->KilledMonsterCredit(NPC_HORRID_ABOMINATION_KILL_CREDIT, 0);

			me->DespawnOrUnsummon(1000);
		}

		void DamageTaken(Unit* attacker, uint32 &/*damage*/)
		{
			if (attacker->GetTypeId() != TYPEID_PLAYER)
				return;

			Unit* victim = NULL;

			if (Unit* victim = me->GetVictim())
			if (victim->GetTypeId() == TYPEID_PLAYER)
				return;

			if (victim)
				me->getThreatManager().modifyThreatPercent(victim, -100);

			AttackStart(attacker);
			me->AddThreat(attacker, 10005000);
		}

		void UpdateAI(uint32 const diff)
		{
			if (shoot)
			{
				if (uiShootTimer <= diff)
				{
					shoot = false;
					uiShootTimer = 3000;
					std::list<Creature*> liamList;
					GetCreatureListWithEntryInGrid(liamList, me, NPC_PRINCE_LIAM_GREYMANE_QYCTEA, 50.0f);

					if (liamList.empty())
						ShootEvent();
					else
						(*liamList.begin())->CastSpell(me, SPELL_SHOOT_QYCTEA, false);
				}
				else
					uiShootTimer -= diff;
			}

			if (!UpdateVictim())
				return;

			if (me->GetVictim() && me->GetVictim()->GetTypeId() == TYPEID_UNIT)
			{
				if (me->GetVictim()->GetHealthPct() < 90)
					miss = true;
				else
					miss = false;
			}

			if (uiRestitchingTimer <= diff)
			{
				uiRestitchingTimer = 8000;
				DoCast(SPELL_RESTITCHING);
			}
			else
				uiRestitchingTimer -= diff;

			DoMeleeAttackIfReady();
		}
	};
};

/*######
## spell_round_up_horse
######*/

class spell_round_up_horse : public SpellScriptLoader
{
public:
	spell_round_up_horse() : SpellScriptLoader("spell_round_up_horse") { }

	class spell_round_up_horse_SpellScript : public SpellScript
	{
		PrepareSpellScript(spell_round_up_horse_SpellScript)

		void Trigger(SpellEffIndex effIndex)
		{
			Unit* target = GetExplTargetUnit();

			if (Creature* horse = target->ToCreature())
			if (horse->GetEntry() == 36540)
			if (Vehicle* vehicle = target->GetVehicleKit())
			if (vehicle->HasEmptySeat(0))
				return;

			PreventHitDefaultEffect(effIndex);
		}

		void Register()
		{
			OnEffectLaunch += SpellEffectFn(spell_round_up_horse_SpellScript::Trigger, EFFECT_0, SPELL_EFFECT_TRIGGER_SPELL);
		}
	};

	SpellScript *GetSpellScript() const
	{
		return new spell_round_up_horse_SpellScript();
	}
};

enum qTHE
{
	NPC_MOUNTAIN_HORSE_VEHICLE = 36540,
	NPC_MOUNTAIN_HORSE_KILL_CREDIT = 36560,
	SPELL_ROPE_CHANNEL = 68940,
	SPELL_ROPE_IN_HORSE = 68908,
};

class npc_round_up_horse : public CreatureScript
{
public:
	npc_round_up_horse() : CreatureScript("npc_round_up_horse") { }

	CreatureAI* GetAI(Creature* creature) const
	{
		return new npc_round_up_horseAI(creature);
	}

	struct npc_round_up_horseAI : public FollowerAI
	{
		npc_round_up_horseAI(Creature* creature) : FollowerAI(creature)
		{
			me->SetReactState(REACT_PASSIVE);

			if (me->isSummon())
			if (Unit* summoner = me->ToTempSummon()->GetSummoner())
			if (Player* player = summoner->ToPlayer())
			{
				StartFollow(player);
				me->SetUInt64Value(UNIT_FIELD_CHANNEL_OBJECT, player->GetGUID());
				me->SetUInt32Value(UNIT_CHANNEL_SPELL, SPELL_ROPE_CHANNEL);

				if (Creature* horse = player->GetVehicleCreatureBase())
					horse->AI()->JustSummoned(me);
			}
		}
	};
};

class npc_mountain_horse_vehicle : public CreatureScript
{
public:
	npc_mountain_horse_vehicle() : CreatureScript("npc_mountain_horse_vehicle") { }

	CreatureAI* GetAI(Creature* creature) const
	{
		return new npc_mountain_horse_vehicleAI(creature);
	}

	struct npc_mountain_horse_vehicleAI : public ScriptedAI
	{
		npc_mountain_horse_vehicleAI(Creature* creature) : ScriptedAI(creature)
		{
			creature->SetReactState(REACT_PASSIVE);
			lSummons.clear();
			uiDespawnTimer = 10000;
			despawn = false;
		}

		std::vector<Creature*> lSummons;
		uint32 uiDespawnTimer;
		uint64 PlayerGUID;
		bool despawn;

		void OnCharmed(bool /*charm*/)
		{

		}

		void SpellHit(Unit* /*caster*/, const SpellInfo* spell)
		{
			if (spell->Id == SPELL_ROPE_IN_HORSE)
				me->DespawnOrUnsummon();
		}

		void PassengerBoarded(Unit* who, int8 /*seatId*/, bool apply)
		{
			if (apply)
			{
				despawn = false;

				if (lSummons.empty())
					return;

				for (std::vector<Creature*>::const_iterator itr = lSummons.begin(); itr != lSummons.end(); ++itr)
				if (*itr)
					(*itr)->DespawnOrUnsummon();

				lSummons.clear();
			}
			else
			{
				uiDespawnTimer = 10000;
				despawn = true;

				if (me->FindNearestCreature(36457, 30.0f))
				{
					if (lSummons.empty())
						return;

					Player* player = who->ToPlayer();

					if (!player)
						return;

					for (std::vector<Creature*>::const_iterator itr = lSummons.begin(); itr != lSummons.end(); ++itr)
					if (*itr)
					{
						player->KilledMonsterCredit(NPC_MOUNTAIN_HORSE_KILL_CREDIT, 0);
						(*itr)->SetUInt64Value(UNIT_FIELD_CHANNEL_OBJECT, 0);
						(*itr)->SetUInt32Value(UNIT_CHANNEL_SPELL, 0);
						(*itr)->DespawnOrUnsummon();
					}

					lSummons.clear();
				}
				else
				{
					if (lSummons.empty())
						return;

					for (std::vector<Creature*>::const_iterator itr = lSummons.begin(); itr != lSummons.end(); ++itr)
					if (*itr)
						(*itr)->DespawnOrUnsummon();

					lSummons.clear();
				}
			}
		}

		void JustSummoned(Creature* summoned)
		{
			if (summoned->GetEntry() == 36540)
				lSummons.push_back(summoned);
		}

		void UpdateAI(uint32 const diff)
		{
			if (despawn)
			{
				if (uiDespawnTimer <= diff)
				{
					despawn = false;
					uiDespawnTimer = 10000;
					me->DespawnOrUnsummon();
				}
				else
					uiDespawnTimer -= diff;
			}
		}
	};
};

/*######
## Quest Two By Sea 14382
######*/

enum qTBS
{
	NPC_FORSACEN_CATAPILT = 36283,

};

class npc_forsaken_catapult_qtbs : public CreatureScript
{
public:
	npc_forsaken_catapult_qtbs() : CreatureScript("npc_forsaken_catapult_qtbs") { }

	CreatureAI* GetAI(Creature* creature) const
	{
		return new npc_forsaken_catapult_qtbsAI(creature);
	}

	struct npc_forsaken_catapult_qtbsAI : public ScriptedAI
	{
		npc_forsaken_catapult_qtbsAI(Creature* creature) : ScriptedAI(creature)
		{
			me->SetFlag(UNIT_NPC_FLAGS, UNIT_NPC_FLAG_SPELLCLICK);
			me->setActive(true);
			speedXY = 10.0f;
			speedZ = 10.0f;
			creature->GetPosition(x, y, z);
			uiCastTimer = urand(5000, 10000);
			uiRespawnTimer = 10000;
			CanCast = true;
			respawn = false;
		}

		float speedXY, speedZ, x, y, z;
		uint32 uiCastTimer;
		uint32 uiRespawnTimer;
		bool CanCast;
		bool respawn;

		void PassengerBoarded(Unit* /*who*/, int8 seatId, bool apply)
		{
			if (!apply)
			{
				respawn = true;
				CanCast = false;

				if (seatId == 2)
					me->setFaction(35);
			}
			else
			{
				respawn = false;

				if (seatId == 2)
					CanCast = true;
			}
		}

		void UpdateAI(uint32 const diff)
		{

			if (respawn)
			{
				if (uiRespawnTimer <= diff)
				{
					respawn = false;
					uiRespawnTimer = 10000;
					me->DespawnOrUnsummon();
				}
				else
					uiRespawnTimer -= diff;
			}

			if (CanCast)
			{
				if (uiCastTimer <= diff)
				{
					uiCastTimer = urand(7000, 20000);
					float x, y, z;
					me->GetNearPoint2D(x, y, urand(100, 150), me->GetOrientation());
					z = me->GetBaseMap()->GetHeight(x, y, MAX_HEIGHT);
					me->CastSpell(x, y, z, 89697, false); // this = 68591 the correct spell but needs scripting
				}
				else
					uiCastTimer -= diff;
			}

			if (!UpdateVictim())
				return;

			DoMeleeAttackIfReady();
		}
	};
};

/*######
## Quest Save the Children! 14368
######*/

enum qSTC
{
	QUEST_SAVE_THE_CHILDREN = 14368,

	NPC_CYNTHIA = 36287,
	NPC_ASHLEY = 36288,
	NPC_JAMES = 36289,

	NPC_CYNTHIA_SAY = 1,
	NPC_ASHLEY_SAY = 1,
	NPC_JAMES_SAY = 1,

	SPELL_SAVE_CYNTHIA = 68597,
	SPELL_SAVE_ASHLEY = 68598,
	SPELL_SAVE_JAMES = 68596,

	GO_DOOR_TO_THE_BASEMENT = 206693,
};

#define    PLAYER_SAY_CYNTHIA    "Its not safe here. Go to the Allens basment."
#define    PLAYER_SAY_ASHLEY     "Join the others inside the basement next door. Hurry!"
#define    PLAYER_SAY_JAMES      "Your mothers in the basement next door. Get to her now!"

class npc_james : public CreatureScript
{
public:
	npc_james() : CreatureScript("npc_james") { }

	CreatureAI* GetAI(Creature* creature) const
	{
		return new npc_jamesAI(creature);
	}

	struct npc_jamesAI : public npc_escortAI
	{
		npc_jamesAI(Creature* creature) : npc_escortAI(creature)
		{
			uiEventTimer = 3500;
			uiPlayerGUID = 0;
			Event = false;
		}

		uint64 uiPlayerGUID;
		uint32 uiEventTimer;
		bool Event;

		void Reset()
		{
			me->SetFlag(UNIT_NPC_FLAGS, UNIT_NPC_FLAG_SPELLCLICK);

		}

		void SpellHit(Unit* caster, const SpellInfo* spell)
		{
			if (spell->Id == SPELL_SAVE_JAMES)
			if (Player* player = caster->ToPlayer())
			{
				Event = true;
				uiPlayerGUID = player->GetGUID();
				player->Say(PLAYER_SAY_JAMES, LANG_UNIVERSAL);
				caster->ToPlayer()->KilledMonsterCredit(me->GetEntry(), me->GetGUID());
			}
		}

		void WaypointReached(uint32 point)
		{
			if (point == 7)
			if (GameObject* door = me->FindNearestGameObject(GO_DOOR_TO_THE_BASEMENT, 10.0f))
				door->UseDoorOrButton();
		}

		void UpdateAI(uint32 const diff)
		{
			npc_escortAI::UpdateAI(diff);

			if (Event)
			{
				if (uiEventTimer <= diff)
				{
					Event = false;
					uiEventTimer = 3500;

					if (Unit::GetPlayer(*me, uiPlayerGUID))
					{
						Start(false, true);
						Talk(NPC_JAMES_SAY);
					}
				}
				else
					uiEventTimer -= diff;
			}
		}
	};
};

class npc_ashley : public CreatureScript
{
public:
	npc_ashley() : CreatureScript("npc_ashley") { }

	CreatureAI* GetAI(Creature* creature) const
	{
		return new npc_ashleyAI(creature);
	}

	struct npc_ashleyAI : public npc_escortAI
	{
		npc_ashleyAI(Creature* creature) : npc_escortAI(creature)
		{
			uiEventTimer = 3500;
			uiPlayerGUID = 0;
			Event = false;
		}

		uint64 uiPlayerGUID;
		uint32 uiEventTimer;
		bool Event;

		void Reset()
		{
			me->SetFlag(UNIT_NPC_FLAGS, UNIT_NPC_FLAG_SPELLCLICK);

		}

		void SpellHit(Unit* caster, const SpellInfo* spell)
		{
			if (spell->Id == SPELL_SAVE_ASHLEY)
			if (Player* player = caster->ToPlayer())
			{
				Event = true;
				uiPlayerGUID = player->GetGUID();
				player->Say(PLAYER_SAY_ASHLEY, LANG_UNIVERSAL);
				caster->ToPlayer()->KilledMonsterCredit(me->GetEntry(), me->GetGUID());
			}
		}

		void WaypointReached(uint32 point)
		{
			if (point == 16)
			if (GameObject* door = me->FindNearestGameObject(GO_DOOR_TO_THE_BASEMENT, 10.0f))
				door->UseDoorOrButton();
		}

		void UpdateAI(uint32 const diff)
		{
			npc_escortAI::UpdateAI(diff);

			if (Event)
			{
				if (uiEventTimer <= diff)
				{
					Event = false;
					uiEventTimer = 3500;

					if (Unit::GetPlayer(*me, uiPlayerGUID))
					{
						Start(false, true);
						Talk(NPC_ASHLEY_SAY);
					}
				}
				else
					uiEventTimer -= diff;
			}
		}
	};
};

class npc_cynthia : public CreatureScript
{
public:
	npc_cynthia() : CreatureScript("npc_cynthia") { }

	CreatureAI* GetAI(Creature* creature) const
	{
		return new npc_cynthiaAI(creature);
	}

	struct npc_cynthiaAI : public npc_escortAI
	{
		npc_cynthiaAI(Creature* creature) : npc_escortAI(creature)
		{
			uiEventTimer = 3500;
			uiPlayerGUID = 0;
			Event = false;
		}

		uint64 uiPlayerGUID;
		uint32 uiEventTimer;
		bool Event;

		void Reset()
		{
			me->SetFlag(UNIT_NPC_FLAGS, UNIT_NPC_FLAG_SPELLCLICK);

		}

		void SpellHit(Unit* caster, const SpellInfo* spell)
		{
			if (spell->Id == SPELL_SAVE_CYNTHIA)
			if (Player* player = caster->ToPlayer())
			{
				Event = true;
				uiPlayerGUID = player->GetGUID();
				player->Say(PLAYER_SAY_CYNTHIA, LANG_UNIVERSAL);
				caster->ToPlayer()->KilledMonsterCredit(me->GetEntry(), me->GetGUID());
			}
		}

		void WaypointReached(uint32 point)
		{
			if (point == 8)
			if (GameObject* door = me->FindNearestGameObject(GO_DOOR_TO_THE_BASEMENT, 10.0f))
				door->UseDoorOrButton();
		}

		void UpdateAI(uint32 const diff)
		{
			npc_escortAI::UpdateAI(diff);

			if (Event)
			{
				if (uiEventTimer <= diff)
				{
					Event = false;
					uiEventTimer = 3500;

					if (/*Player* player = */Unit::GetPlayer(*me, uiPlayerGUID))
					{
						Start(false, true);
						Talk(NPC_CYNTHIA_SAY);
					}
				}
				else
					uiEventTimer -= diff;
			}
		}
	};
};

/*###### Quest Gasping for Breath  ######*/

enum qGFB
{
	QUEST_GASPING_FOR_BREATH = 14395,

	NPC_QGFB_KILL_CREDIT = 36450,
	NPC_DROWNING_WATCHMAN = 36440,

	SPELL_RESCUE_DROWNING_WATCHMAN = 68735,
	SPELL_SUMMON_SPARKLES = 69253,
	SPELL_DROWNING = 68730,

	GO_SPARKLES = 197333,

	DROWNING_WATCHMAN_RANDOM_SAY = 0,
};

class npc_drowning_watchman : public CreatureScript
{
public:
	npc_drowning_watchman() : CreatureScript("npc_drowning_watchman") { }

	CreatureAI* GetAI(Creature* creature) const
	{
		return new npc_drowning_watchmanAI(creature);
	}

	struct npc_drowning_watchmanAI : public ScriptedAI
	{
		npc_drowning_watchmanAI(Creature* creature) : ScriptedAI(creature)
		{
			reset = true;
			despawn = false;
			exit = false;
			uiDespawnTimer = 10000;
		}

		uint32 uiDespawnTimer;
		bool reset;
		bool despawn;
		bool exit;

		void SpellHit(Unit* caster, const SpellInfo* spell)
		{
			if (spell->Id == SPELL_RESCUE_DROWNING_WATCHMAN)
			{
				despawn = false;
				uiDespawnTimer = 10000;
				me->RemoveFlag(UNIT_NPC_FLAGS, UNIT_NPC_FLAG_SPELLCLICK);
				me->RemoveAura(SPELL_DROWNING);
				me->EnterVehicle(caster);

				if (GameObject* go = me->FindNearestGameObject(GO_SPARKLES, 10.0f))
					go->Delete();
			}
		}

		void OnExitVehicle(Unit* /*vehicle*/, uint32 /*seatId*/)
		{
			if (!exit)
			{
				float x, y, z, o;
				me->GetPosition(x, y, z, o);
				me->SetHomePosition(x, y, z, o);
				me->Relocate(x, y, z, o);
				reset = true;
				despawn = true;
				Reset();
			}
		}

		void Reset()
		{
			exit = false;

			if (reset)
			{
				DoCast(SPELL_DROWNING);
				me->SetVisible(true);
				DoCast(SPELL_SUMMON_SPARKLES);
				me->SetFlag(UNIT_NPC_FLAGS, UNIT_NPC_FLAG_SPELLCLICK);
				reset = false;
			}
		}

		void UpdateAI(uint32 const diff)
		{
			if (despawn)
			{
				if (uiDespawnTimer <= diff)
				{
					if (GameObject* go = me->FindNearestGameObject(GO_SPARKLES, 10.0f))
						go->Delete();

					reset = true;
					despawn = false;
					uiDespawnTimer = 10000;
					me->DespawnOrUnsummon();
				}
				else
					uiDespawnTimer -= diff;
			}
		}
	};
};

class npc_prince_liam_greymane : public CreatureScript
{
public:
	npc_prince_liam_greymane() : CreatureScript("npc_prince_liam_greymane") { }

	CreatureAI* GetAI(Creature* creature) const
	{
		return new npc_prince_liam_greymaneAI(creature);
	}

	struct npc_prince_liam_greymaneAI : public ScriptedAI
	{
		npc_prince_liam_greymaneAI(Creature* creature) : ScriptedAI(creature){ }

		void MoveInLineOfSight(Unit* who)
		{
			if (who->GetEntry() == NPC_DROWNING_WATCHMAN)
			{
				if (who->IsInWater() || !who->GetVehicle())
					return;

				if (who->GetDistance(-1897.0f, 2519.97f, 1.50667f) < 5.0f)
				if (Unit* unit = who->GetVehicleBase())
				{
					if (Creature* watchman = who->ToCreature())
					{
						watchman->AI()->Talk(DROWNING_WATCHMAN_RANDOM_SAY);
						watchman->DespawnOrUnsummon(15000);
						watchman->SetStandState(UNIT_STAND_STATE_KNEEL);
						CAST_AI(npc_drowning_watchman::npc_drowning_watchmanAI, watchman->AI())->exit = true;
						CAST_AI(npc_drowning_watchman::npc_drowning_watchmanAI, watchman->AI())->reset = true;
						who->ExitVehicle();
						unit->RemoveAura(SPELL_RESCUE_DROWNING_WATCHMAN);
					}

					if (Player* player = unit->ToPlayer())
						player->KilledMonsterCredit(NPC_QGFB_KILL_CREDIT, 0);
				}
			}
		}

		void UpdateAI(uint32 const diff)
		{
			if (!UpdateVictim())
				return;

			DoMeleeAttackIfReady();
		}
	};
};

/*###### Quest Gasping for Breath END  ######*/

/*######
## Quest Grandma's Cat 14401
######*/

enum qGC
{
	NPC_WAHL = 36458,
	NPC_WAHL_WORGEN = 36852,
	NPC_LUCIUS_THE_CRUEL = 36461,

	SPELL_CATCH_CAT = 68743,

	SAY_THIS_CAT_IS_MINE = 0,
	YELL_DONT_MESS = 0,

	QUEST_GRANDMAS_CAT = 14401,

	ACTION_SUMMON_LUCIUS = 1,
	ACTION_CATCH_CHANCE = 2,
	ACTION_CHANCE_DESPAWN = 3,
	POINT_CATCH_CHANCE = 4,
};

class npc_wahl : public CreatureScript
{
public:
	npc_wahl() : CreatureScript("npc_wahl") { }

	CreatureAI* GetAI(Creature* creature) const
	{
		return new npc_wahlAI(creature);
	}

	struct npc_wahlAI : public npc_escortAI
	{
		npc_wahlAI(Creature* creature) : npc_escortAI(creature)
		{
			me->SetReactState(REACT_PASSIVE);
			me->SetFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_NON_ATTACKABLE);
		}

		void WaypointReached(uint32 point)
		{
			if (point == 1)
			if (me->isSummon())
			if (Unit* summoner = me->ToTempSummon()->GetSummoner())
			{
				me->SetDisplayId(NPC_WAHL_WORGEN);
				me->SetReactState(REACT_AGGRESSIVE);
				me->RemoveFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_NON_ATTACKABLE);
				AttackStart(summoner);
			}
		}

		void UpdateAI(const uint32 diff)
		{
			npc_escortAI::UpdateAI(diff);
		}
	};
};

class npc_lucius_the_cruel : public CreatureScript
{
public:
	npc_lucius_the_cruel() : CreatureScript("npc_lucius_the_cruel") { }

	CreatureAI* GetAI(Creature* creature) const
	{
		return new npc_lucius_the_cruelAI(creature);
	}

	struct npc_lucius_the_cruelAI : public ScriptedAI
	{
		npc_lucius_the_cruelAI(Creature* creature) : ScriptedAI(creature)
		{
			SetCombatMovement(false);
			Catch = false;
			Summon = false;
			uiCatchTimer = 1000;
			uiShootTimer = 500;
			uiPlayerGUID = 0;
			uiSummonTimer = 1500;
			me->SetReactState(REACT_PASSIVE);
			me->SetFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_NON_ATTACKABLE);
		}

		uint64 uiPlayerGUID;
		uint32 uiCatchTimer;
		uint32 uiShootTimer;
		uint32 uiSummonTimer;
		bool Catch;
		bool Summon;

		void EnterEvadeMode()
		{
			me->DespawnOrUnsummon();
		}

		void MovementInform(uint32 type, uint32 id)
		{
			if (type != POINT_MOTION_TYPE)
				return;

			if (id == POINT_CATCH_CHANCE)
			{
				me->HandleEmoteCommand(EMOTE_ONESHOT_KNEEL);
				if (me->isSummon())
				if (Unit* summoner = me->ToTempSummon()->GetSummoner())
				if (Creature* chance = summoner->ToCreature())
				{
					Catch = true;
					Summon = true;
					chance->AI()->DoAction(ACTION_CHANCE_DESPAWN);
				}
			}
		}

		void DoAction(int32 const action)
		{
			if (action == ACTION_SUMMON_LUCIUS)
			{
				me->GetMotionMaster()->MovePoint(POINT_CATCH_CHANCE, -2106.372f, 2331.106f, 7.360674f);
				Talk(SAY_THIS_CAT_IS_MINE);
			}
		}

		void UpdateAI(const uint32 diff)
		{
			if (Catch)
			{
				if (uiCatchTimer <= diff)
				{
					Catch = false;
					uiCatchTimer = 1000;
					me->RemoveFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_NON_ATTACKABLE);
					me->SetReactState(REACT_AGGRESSIVE);

					if (Player* player = Unit::GetPlayer(*me, uiPlayerGUID))
						AttackStart(player);
				}
				else
					uiCatchTimer -= diff;
			}

			if (Summon)
			{
				if (uiSummonTimer <= diff)
				{
					Summon = false;
					uiSummonTimer = 1500;

					if (Creature* wahl = me->SummonCreature(NPC_WAHL, -2098.366f, 2352.075f, 7.160643f))
					{
						wahl->AI()->Talk(YELL_DONT_MESS);

						if (npc_escortAI* npc_escort = CAST_AI(npc_wahl::npc_wahlAI, wahl->AI()))
						{
							npc_escort->AddWaypoint(0, -2106.54f, 2342.69f, 6.93668f);
							npc_escort->AddWaypoint(1, -2106.12f, 2334.90f, 7.36691f);
							npc_escort->AddWaypoint(2, -2117.80f, 2357.15f, 5.88139f);
							npc_escort->AddWaypoint(3, -2111.46f, 2366.22f, 7.17151f);
							npc_escort->Start(false, true);
						}
					}

				}
				else
					uiSummonTimer -= diff;
			}

			if (!UpdateVictim())
				return;

			if (uiShootTimer <= diff)
			{
				uiShootTimer = 1000;

				if (me->GetDistance(me->GetVictim()) > 2.0f)
					DoCastVictim(41440);
			}
			else
				uiShootTimer -= diff;

			DoMeleeAttackIfReady();
		}
	};
};

class npc_chance_the_cat : public CreatureScript
{
public:
	npc_chance_the_cat() : CreatureScript("npc_chance_the_cat") { }

	CreatureAI* GetAI(Creature* creature) const
	{
		return new npc_chance_the_catAI(creature);
	}

	struct npc_chance_the_catAI : public ScriptedAI
	{
		npc_chance_the_catAI(Creature* creature) : ScriptedAI(creature)
		{
			Despawn = false;
			uiDespawnTimer = 500;
		}

		uint32 uiDespawnTimer;
		bool Despawn;

		void DoAction(int32 const action)
		{
			if (action == ACTION_CHANCE_DESPAWN)
				Despawn = true;
		}

		void SpellHit(Unit* caster, const SpellInfo* spell)
		{
			if (spell->Id == SPELL_CATCH_CAT && caster->GetTypeId() == TYPEID_PLAYER)
			{
				me->RemoveFlag(UNIT_NPC_FLAGS, UNIT_NPC_FLAG_SPELLCLICK);

				if (Creature* lucius = me->SummonCreature(NPC_LUCIUS_THE_CRUEL, -2111.533f, 2329.95f, 7.390349f))
				{
					lucius->AI()->DoAction(ACTION_SUMMON_LUCIUS);
					CAST_AI(npc_lucius_the_cruel::npc_lucius_the_cruelAI, lucius->AI())->uiPlayerGUID = caster->GetGUID();
				}
			}
		}

		void UpdateAI(const uint32 diff)
		{
			if (Despawn)
			{
				if (uiDespawnTimer <= diff)
				{
					uiDespawnTimer = 500;
					Despawn = false;
					me->DespawnOrUnsummon();
				}
				else
					uiDespawnTimer -= diff;
			}
		}
	};
};

/*######
## Quest To Greymane Manor 14465
######*/

enum qTGM
{
	QUEST_TO_GREYMANE_MANOR = 14465,

	NPC_SWIFT_MOUNTAIN_HORSE = 36741,

	GO_FIRST_GATE = 196399,
	GO_SECOND_GATE = 196401,

};

class npc_swift_mountain_horse : public CreatureScript
{
public:
	npc_swift_mountain_horse() : CreatureScript("npc_swift_mountain_horse") { }

	CreatureAI* GetAI(Creature* creature) const
	{
		return new npc_swift_mountain_horseAI(creature);
	}

	struct npc_swift_mountain_horseAI : public npc_escortAI
	{
		npc_swift_mountain_horseAI(Creature* creature) : npc_escortAI(creature)
		{
			//me->SetReactState(REACT_PASSIVE);
		}

		bool PlayerOn;

		void Reset()
		{
			PlayerOn = false;
		}

		void PassengerBoarded(Unit* who, int8 /*seatId*/, bool apply)
		{
			if (who->GetTypeId() == TYPEID_PLAYER)
			{
				PlayerOn = true;
				if (apply)
				{
					Start(false, true, who->GetGUID());
				}
			}
		}

		void WaypointReached(uint32 point)
		{
			Player* player = GetPlayerForEscort();

			switch (point)
			{
			case 4:
				if (GameObject* go = me->FindNearestGameObject(GO_FIRST_GATE, 30.0f))
					go->UseDoorOrButton();
				break;
			case 12:
				if (GameObject* go = me->FindNearestGameObject(GO_SECOND_GATE, 30.0f))
					go->UseDoorOrButton();
				break;
			case 13:
				if (me->isSummon())
				if (Unit* summoner = me->ToTempSummon()->GetSummoner())
				if (Player* player = summoner->ToPlayer())
				{
					WorldLocation loc;
					loc.m_mapId = 654;
					loc.m_positionX = -1586.57f;
					loc.m_positionY = 2551.24f;
					loc.m_positionZ = 130.218f;
					player->SetHomebind(loc, 817);
					player->CompleteQuest(14465);
					player->SaveToDB();
					//player->SetPhaseMask(0x01, true);
				}
				me->DespawnOrUnsummon();
				break;
			}
		}

		void OnCharmed(bool /*apply*/)
		{
		}

		void UpdateAI(const uint32 diff)
		{
			npc_escortAI::UpdateAI(diff);
			Player* player = GetPlayerForEscort();

			if (PlayerOn)
			{
				player->SetClientControl(me, 0);
				PlayerOn = false;
			}
		}
	};
};

/*######
## npc_gwen_armsteadt_24616
######*/

class npc_gwen_armstead : public CreatureScript
{
public:
	npc_gwen_armstead() : CreatureScript("npc_gwen_armstead") { }

	bool OnQuestAccept(Player* player, Creature* /*creature*/, Quest const* quest)
	{
		if (quest->GetQuestId() == QUEST_TO_GREYMANE_MANOR)
		{
			if (Creature* horse = player->SummonCreature(NPC_SWIFT_MOUNTAIN_HORSE, player->GetPositionX(), player->GetPositionY(), player->GetPositionZ(), player->GetOrientation(), TEMPSUMMON_MANUAL_DESPAWN))
			{
				player->EnterVehicle(horse, 0);
				//player->SetPhaseMask(0x01, true);
				player->CastCustomSpell(VEHICLE_SPELL_RIDE_HARDCODED, SPELLVALUE_BASE_POINT0, 1, horse, false);
				CAST_AI(npc_escortAI, (horse->AI()))->Start(false, true, player->GetGUID(), quest);
			}
		}
		return true;
	}
};

/*######
## Quest The King's Observatory 14466, Alas, Gilneas! 14467
######*/

enum qTKO_AG
{
	QUEST_THE_KINGS_OBSERVATORY = 14466,
	QUEST_ALAS_GILNEAS = 14467,
	QUEST_EXODUS = 24438,

	SPELL_ZONE_SPECIFIC_19 = 74096,
	SPELL_ZONE_SPECIFIC_11 = 69484,
	SPELL_CATACLYSM_TYPE_1 = 80133,
	SPELL_CATACLYSM_TYPE_2 = 68953,
	SPELL_CATACLYSM_TYPE_3 = 80134,

	CINEMATIC_TELESCOPE = 167,
};

class npc_king_genn_greymane_c3 : public CreatureScript
{
public:
	npc_king_genn_greymane_c3() : CreatureScript("npc_king_genn_greymane_c3") { }

	bool OnQuestComplete(Player* player, Creature* /*creature*/, Quest const* quest)
	{
		if (quest->GetQuestId() == QUEST_THE_KINGS_OBSERVATORY)
		{
			player->CastSpell(player, SPELL_CATACLYSM_TYPE_3, true);
			player->CastSpell(player, SPELL_ZONE_SPECIFIC_11, true);
			player->SaveToDB();
		}

		if (quest->GetQuestId() == QUEST_ALAS_GILNEAS)
			player->SendCinematicStart(CINEMATIC_TELESCOPE);

		return true;
	}

	bool OnQuestAccept(Player* player, Creature* /*creature*/, Quest const* quest)
	{
		if (quest->GetQuestId() == QUEST_EXODUS)
		{
			player->RemoveAura(SPELL_ZONE_SPECIFIC_19);
			player->RemoveAura(99488);
			player->RemoveAura(69484);
			player->RemoveAura(68481);
			player->RemoveAura(68243);
			player->RemoveAura(59087);
			player->RemoveAura(59074);
			player->RemoveAura(59073);
			player->CastSpell(player, SPELL_ZONE_SPECIFIC_11, true);
			player->SaveToDB();
		}

		return true;
	}
};

/*######
## Quest Stranded at the Marsh 24468
######*/

enum qSATM
{
	NPC_SWAMP_CROCOLISK = 37078,
	SURVIVOR_RANDOM_SAY = -1977051,    // -1977051 to -1977054
};

class npc_crash_survivor : public CreatureScript
{
public:
	npc_crash_survivor() : CreatureScript("npc_crash_survivor") { }

	CreatureAI* GetAI(Creature* creature) const
	{
		return new npc_crash_survivorAI(creature);
	}

	struct npc_crash_survivorAI : public ScriptedAI
	{
		npc_crash_survivorAI(Creature* creature) : ScriptedAI(creature)
		{
			summon = false;
			despawn = false;
			uiDespawnTimer = 2000;
		}

		uint32 uiDespawnTimer;
		bool miss;
		bool summon;
		bool despawn;

		void Reset()
		{
			miss = false;

			if (!summon)
			{
				summon = true;
				float x, y;
				me->GetNearPoint2D(x, y, 3.0f, me->GetOrientation());
				me->SummonCreature(NPC_SWAMP_CROCOLISK, x, y, me->GetPositionZ());
			}
		}

		void DamageTaken(Unit* doneBy, uint32& damage)
		{
			if (doneBy->ToCreature())
			if (me->GetHealth() <= damage || me->GetHealthPct() <= 80.0f)
				damage = 0;
		}

		void DamageDealt(Unit* target, uint32& damage, DamageEffectType damageType)
		{
			if (target->ToCreature())
			if (target->GetHealth() <= damage || target->GetHealthPct() <= 70.0f)
				damage = 0;
		}

		void UpdateAI(uint32 const diff)
		{
			if (despawn)
			{
				if (uiDespawnTimer <= diff)
				{
					despawn = false;
					summon = false;
					uiDespawnTimer = 2000;
					me->DespawnOrUnsummon();
				}
				else
					uiDespawnTimer -= diff;
			}

			if (!UpdateVictim())
				return;

			if (me->GetVictim()->GetTypeId() == TYPEID_UNIT)
			{
				if (me->GetVictim()->GetHealthPct() < 90)
					miss = true;
				else
					miss = false;
			}

			DoMeleeAttackIfReady();
		}
	};
};

class npc_swamp_crocolisk : public CreatureScript
{
public:
	npc_swamp_crocolisk() : CreatureScript("npc_swamp_crocolisk") { }

	CreatureAI* GetAI(Creature* creature) const
	{
		return new npc_swamp_crocoliskAI(creature);
	}

	struct npc_swamp_crocoliskAI : public ScriptedAI
	{
		npc_swamp_crocoliskAI(Creature* creature) : ScriptedAI(creature){ }

		bool miss;

		void Reset()
		{
			miss = false;

			if (me->isSummon())
			if (Unit* summoner = me->ToTempSummon()->GetSummoner())
			{
				summoner->getThreatManager().resetAllAggro();
				me->getThreatManager().resetAllAggro();
				me->CombatStart(summoner);
				summoner->CombatStart(me);
			}
		}

		void DamageTaken(Unit* attacker, uint32 &/*damage*/)
		{
			if (attacker->GetTypeId() != TYPEID_PLAYER)
				return;

			Unit* victim = NULL;

			if (Unit* victim = me->GetVictim())
			if (victim->GetTypeId() == TYPEID_PLAYER)
				return;

			if (victim)
				me->getThreatManager().modifyThreatPercent(victim, -100);

			AttackStart(attacker);
			me->AddThreat(attacker, 10005000);
		}

		void JustDied(Unit* /*who*/)
		{
			if (me->isSummon())
			if (Unit* summoner = me->ToTempSummon()->GetSummoner())
			{

				if (Creature* surv = summoner->ToCreature())
					CAST_AI(npc_crash_survivor::npc_crash_survivorAI, surv->AI())->despawn = true;
			}
		}

		void UpdateAI(uint32 const /*diff*/)
		{
			if (!UpdateVictim())
				return;

			if (me->GetVictim()->GetTypeId() == TYPEID_UNIT)
			{
				if (me->GetVictim()->GetHealthPct() < 90)
					miss = true;
				else
					miss = false;
			}

			DoMeleeAttackIfReady();
		}
	};
};

/*######
## Quest Introductions Are in Order 24472
######*/

enum qIAO
{
	QUEST_INTRODUCTIONS_ARE_IN_ORDER = 24472,

	NPC_KOROTH_THE_HILLBREAKER_QIAO = 36294,
	NPC_KOROTH_THE_HILLBREAKER_QIAO_FRIEND = 37808,
	NPC_CAPTAIN_ASTHER_QIAO = 37806,
	NPC_FORSAKEN_SOLDIER_QIAO = 37805,
	NPC_FORSAKEN_CATAPULT_QIAO = 37807,

	KOROTH_YELL_WHO_STEAL_BANNER = 0,
	KOROTH_YELL_FIND_YOU = 1,
	LIAN_SAY_HERE_FORSAKEN = 0,
	LIAM_YELL_YOU_CANT = 1,
	CAPITAN_YELL_WILL_ORDER = -1977089,
	KOROTH_YELL_MY_BANNER = 0,

	SPELL_PUSH_BANNER = 70511,
	SPELL_CLEAVE = 16044,
	SPELL_DEMORALIZING_SHOUT_QIAO = 16244,

	POINT_BANNER = 1,
	ACTION_KOROTH_ATTACK = 2,
};

struct Psc_qiao
{
	uint64 uiPlayerGUID;
	uint64 uiCapitanGUID;
	uint32 uiEventTimer;
	uint8 uiPhase;
};

struct sSoldier
{
	Creature* soldier;
	float follow_angle;
	float follow_dist;
};

const float AstherWP[18][3] =
{
	{ -2129.99f, 1824.12f, 25.234f }, { -2132.93f, 1822.23f, 23.984f }, { -2135.81f, 1820.23f, 22.770f },
	{ -2138.72f, 1818.29f, 21.595f }, { -2141.77f, 1816.57f, 20.445f }, { -2144.88f, 1814.96f, 19.380f },
	{ -2147.19f, 1813.85f, 18.645f }, { -2150.51f, 1812.73f, 17.760f }, { -2153.88f, 1811.80f, 16.954f },
	{ -2157.28f, 1810.95f, 16.194f }, { -2160.69f, 1810.20f, 15.432f }, { -2164.12f, 1809.46f, 14.688f },
	{ -2167.55f, 1808.81f, 13.961f }, { -2171.01f, 1808.27f, 13.316f }, { -2174.32f, 1808.00f, 12.935f },
	{ -2177.11f, 1807.75f, 12.717f }, { -2179.79f, 1807.67f, 12.573f }, { -2183.06f, 1807.59f, 12.504f },
};

const float KorothWP[14][3] =
{
	{ -2213.64f, 1863.51f, 15.404f }, { -2212.69f, 1860.14f, 15.302f }, { -2211.15f, 1853.31f, 15.078f },
	{ -2210.39f, 1849.90f, 15.070f }, { -2209.11f, 1845.65f, 15.377f }, { -2206.19f, 1839.29f, 15.147f },
	{ -2204.92f, 1836.03f, 14.420f }, { -2203.76f, 1832.73f, 13.432f }, { -2201.68f, 1826.04f, 12.296f },
	{ -2200.68f, 1822.69f, 12.194f }, { -2199.22f, 1818.77f, 12.175f }, { -2196.29f, 1813.86f, 12.253f },
	{ -2192.46f, 1811.06f, 12.445f }, { -2186.79f, 1808.90f, 12.513f },
};


class npc_koroth_the_hillbreaker_qiao_friend : public CreatureScript
{
public:
	npc_koroth_the_hillbreaker_qiao_friend() : CreatureScript("npc_koroth_the_hillbreaker_qiao_friend") { }

	CreatureAI* GetAI(Creature* creature) const
	{
		return new npc_koroth_the_hillbreaker_qiao_friendAI(creature);
	}

	struct npc_koroth_the_hillbreaker_qiao_friendAI : public npc_escortAI
	{
		npc_koroth_the_hillbreaker_qiao_friendAI(Creature* creature) : npc_escortAI(creature)
		{
			me->setActive(true);
			me->SetReactState(REACT_PASSIVE);
			me->SetFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_NON_ATTACKABLE);
			uiCleaveTimer = 250;
			uiDemoralizingShoutTimer = 500;

			if (me->isSummon())
			{
				for (int i = 0; i < 14; ++i)
					AddWaypoint(i, KorothWP[i][0], KorothWP[i][1], KorothWP[i][2]);

				Start();
				SetRun(true);
			}
		}

		uint32 uiCleaveTimer;
		uint32 uiDemoralizingShoutTimer;

		void FinishEscort()
		{
			if (me->isSummon())
			if (Unit* summoner = me->ToTempSummon()->GetSummoner())
			{
				me->SetReactState(REACT_AGGRESSIVE);
				me->RemoveFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_NON_ATTACKABLE);

				if (Creature* capitan = summoner->ToCreature())
					capitan->AI()->DoAction(ACTION_KOROTH_ATTACK);

				/*  if (summoner->isSummon())
				if (Unit* _summoner = summoner->ToTempSummon()->GetSummoner())
				if (Player* player = _summoner->ToPlayer())*/
				me->MonsterYell("Corpse-men take Koroth's banner! Corpse-men get smashed to bitses!!!!", 0, 0);
			}
		}
		void WaypointReached(uint32 /*point*/) { }

		void UpdateAI(uint32 const diff)
		{
			npc_escortAI::UpdateAI(diff);

			if (!UpdateVictim())
				return;

			if (uiCleaveTimer <= diff)
			{
				uiCleaveTimer = urand(2500, 15000);
				DoCastVictim(SPELL_CLEAVE);
			}
			else
				uiCleaveTimer -= diff;

			if (uiDemoralizingShoutTimer <= diff)
			{
				uiDemoralizingShoutTimer = 5000;
				DoCast(SPELL_DEMORALIZING_SHOUT_QIAO);
			}
			else
				uiDemoralizingShoutTimer -= diff;

			DoMeleeAttackIfReady();
		}
	};
};

class npc_captain_asther_qiao : public CreatureScript
{
public:
	npc_captain_asther_qiao() : CreatureScript("npc_captain_asther_qiao") { }

	CreatureAI* GetAI(Creature* creature) const
	{
		return new npc_captain_asther_qiaoAI(creature);
	}

	struct npc_captain_asther_qiaoAI : public npc_escortAI
	{
		npc_captain_asther_qiaoAI(Creature* creature) : npc_escortAI(creature)
		{
			me->setActive(true);
			uiKorothGUID = 0;
			lSoldiers.clear();
			me->SetReactState(REACT_PASSIVE);
			me->SetFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_NON_ATTACKABLE);

			if (me->isSummon())
				StartEvent();
		}

		std::list<sSoldier> lSoldiers;
		uint64 uiKorothGUID;

		void JustDied(Unit* /*who*/)
		{
			me->DespawnOrUnsummon(15000);

			if (Creature* koroth = Unit::GetCreature(*me, uiKorothGUID))
				koroth->DespawnOrUnsummon(15000);

			if (lSoldiers.empty())
				return;

			for (std::list<sSoldier>::iterator itr = lSoldiers.begin(); itr != lSoldiers.end(); ++itr)
			{
				Creature* soldier = itr->soldier;
				soldier->DespawnOrUnsummon(15000);
			}
		}

		void DoAction(int32 const action)
		{
			if (action == ACTION_KOROTH_ATTACK)
			if (Creature* koroth = Unit::GetCreature(*me, uiKorothGUID))
			{
				me->SetReactState(REACT_AGGRESSIVE);
				me->RemoveFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_NON_ATTACKABLE);
				AttackStart(koroth);
				me->AddThreat(koroth, 100500);
				koroth->AddThreat(me, 100500);
				float x, y, z;
				koroth->GetNearPoint(koroth, x, y, z, 0.0f, 1.0f, koroth->GetOrientation() + M_PI);
				me->GetMotionMaster()->MoveCharge(x, y, z);

				if (lSoldiers.empty())
					return;

				for (std::list<sSoldier>::iterator itr = lSoldiers.begin(); itr != lSoldiers.end(); ++itr)
				{
					Creature* soldier = itr->soldier;
					soldier->SetReactState(REACT_AGGRESSIVE);
					soldier->RemoveFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_NON_ATTACKABLE);
					soldier->AI()->AttackStart(koroth);
					soldier->AddThreat(koroth, 100500);

					if (soldier->GetEntry() == NPC_FORSAKEN_CATAPULT_QIAO)
					{
						koroth->AddThreat(soldier, 200500);
						koroth->AI()->AttackStart(soldier);
						continue;
					}

					koroth->AddThreat(soldier, 10000);
					soldier->GetMotionMaster()->MoveCharge(x, y, z);
				}
			}
		}

		void StartMoveTo(float x, float y, float z)
		{
			if (lSoldiers.empty())
				return;

			float pathangle = atan2(me->GetPositionY() - y, me->GetPositionX() - x);

			for (std::list<sSoldier>::iterator itr = lSoldiers.begin(); itr != lSoldiers.end(); ++itr)
			{
				Creature* member = itr->soldier;

				if (!member->isAlive() || member->GetVictim())
					continue;

				float angle = itr->follow_angle;
				float dist = itr->follow_dist;

				float dx = x - std::cos(angle + pathangle) * dist;
				float dy = y - std::sin(angle + pathangle) * dist;
				float dz = z;

				Trinity::NormalizeMapCoord(dx);
				Trinity::NormalizeMapCoord(dy);

				member->UpdateGroundPositionZ(dx, dy, dz);

				if (member->IsWithinDist(me, dist + 5.0f))
					member->SetUnitMovementFlags(me->GetUnitMovementFlags());
				else
					member->RemoveUnitMovementFlag(MOVEMENTFLAG_WALKING);

				member->GetMotionMaster()->MovePoint(0, dx, dy, dz);
				member->SetHomePosition(dx, dy, dz, pathangle);
			}
		}

		void SummonSoldier(uint64 guid, uint32 SoldierEntry, float dist, float angle)
		{
			float x, y;
			me->GetNearPoint2D(x, y, dist, me->GetOrientation() + angle);
			float z = me->GetBaseMap()->GetHeight(x, y, MAX_HEIGHT);

			if (Creature* soldier = me->SummonCreature(SoldierEntry, x, y, z))
			{
				soldier->SetReactState(REACT_PASSIVE);
				soldier->SetFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_NON_ATTACKABLE);
				soldier->SetSeerGUID(guid);
				soldier->SetVisible(false);
				soldier->setActive(true);
				sSoldier new_soldier;
				new_soldier.soldier = soldier;
				new_soldier.follow_angle = angle;
				new_soldier.follow_dist = dist;
				lSoldiers.push_back(new_soldier);
			}
		}

		void StartEvent()
		{
			if (Unit* summoner = me->ToTempSummon()->GetSummoner())
			if (Player* player = summoner->ToPlayer())
			{
				uint64 uiPlayerGUID = player->GetGUID();

				for (int i = 1; i < 4; ++i)
				{
					SummonSoldier(uiPlayerGUID, NPC_FORSAKEN_SOLDIER_QIAO, i * 2, M_PI);
					SummonSoldier(uiPlayerGUID, NPC_FORSAKEN_SOLDIER_QIAO, i * 2, M_PI - M_PI / (2 * i));
					SummonSoldier(uiPlayerGUID, NPC_FORSAKEN_SOLDIER_QIAO, i * 2, M_PI + M_PI / (2 * i));
				}

				SummonSoldier(uiPlayerGUID, NPC_FORSAKEN_CATAPULT_QIAO, 8.0f, M_PI);

				for (int i = 0; i < 18; ++i)
					AddWaypoint(i, AstherWP[i][0], AstherWP[i][1], AstherWP[i][2]);

				Start();
			}
		}

		void WaypointReached(uint32 point)
		{
			if (point == 15)
			if (Creature* koroth = me->SummonCreature(NPC_KOROTH_THE_HILLBREAKER_QIAO_FRIEND, -2213.64f, 1863.51f, 15.404f))
				uiKorothGUID = koroth->GetGUID();
		}

		void UpdateAI(uint32 const diff)
		{
			npc_escortAI::UpdateAI(diff);

			if (!UpdateVictim())
				return;

			DoMeleeAttackIfReady();
		}
	};
};

class npc_prince_liam_greymane_qiao : public CreatureScript
{
public:
	npc_prince_liam_greymane_qiao() : CreatureScript("npc_prince_liam_greymane_qiao") { }

	bool OnQuestComplete(Player* player, Creature* creature, Quest const* quest)
	{
		if (quest->GetQuestId() == QUEST_INTRODUCTIONS_ARE_IN_ORDER)
			CAST_AI(npc_prince_liam_greymane_qiaoAI, creature->AI())->StartEvent(player);

		if (quest->GetQuestId() == QUEST_EXODUS)
		{
			WorldLocation loc;
			loc.m_mapId = 654;
			loc.m_positionX = -245.84f;
			loc.m_positionY = 1550.56f;
			loc.m_positionZ = 18.6917f;
			player->SetHomebind(loc, 4731);
			player->SaveToDB();
		}

		return true;
	}

	CreatureAI* GetAI(Creature* creature) const
	{
		return new npc_prince_liam_greymane_qiaoAI(creature);
	}

	struct npc_prince_liam_greymane_qiaoAI : public ScriptedAI
	{
		npc_prince_liam_greymane_qiaoAI(Creature* creature) : ScriptedAI(creature)
		{
			lPlayerList.clear();
		}

		std::list<Psc_qiao> lPlayerList;

		void StartEvent(Player* player)
		{
			if (!player)
				return;

			uint64 uiPlayerGUID = player->GetGUID();
			Psc_qiao new_player;
			new_player.uiPlayerGUID = uiPlayerGUID;
			new_player.uiCapitanGUID = 0;
			new_player.uiEventTimer = 0;
			new_player.uiPhase = 0;
			Creature* capitan = NULL;

			if (Creature* capitan = player->SummonCreature(NPC_CAPTAIN_ASTHER_QIAO, -2120.19f, 1833.06f, 30.1510f, 3.87363f))
			{
				capitan->SetSeerGUID(player->GetGUID());
				capitan->SetVisible(false);
				new_player.uiCapitanGUID = capitan->GetGUID();
			}

			if (!capitan)
				return;

			lPlayerList.push_back(new_player);
		}

		void UpdateAI(uint32 const diff)
		{
			if (!lPlayerList.empty())
			for (std::list<Psc_qiao>::iterator itr = lPlayerList.begin(); itr != lPlayerList.end();)
			{
				if (itr->uiEventTimer <= diff)
				{
					++itr->uiPhase;

					if (/*Player* player = */Unit::GetPlayer(*me, itr->uiPlayerGUID))
						switch (itr->uiPhase)
					{
						case 1:
							itr->uiEventTimer = 8000;
							Talk(LIAN_SAY_HERE_FORSAKEN);
							break;
						case 2:
							itr->uiEventTimer = 5000;
							me->HandleEmoteCommand(EMOTE_ONESHOT_ROAR);
							Talk(LIAM_YELL_YOU_CANT);
							break;
						case 3:
							itr->uiEventTimer = 3000;
							me->CastSpell(me, SPELL_PUSH_BANNER/*, false*/);
							break;
						case 4:
							if (Unit::GetCreature(*me, itr->uiCapitanGUID))
							if (Creature* capitan = me->FindNearestCreature(NPC_CAPTAIN_ASTHER_QIAO, 20.0f, true))
								capitan->MonsterYell("Worthless mongrel. I will order our outhouses cleaned with this rag you call a banner.", 0, 0);
							itr = lPlayerList.erase(itr);
							break;
					}
				}
				else
				{
					itr->uiEventTimer -= diff;
					++itr;
				}
			}

			if (!UpdateVictim())
				return;

			DoMeleeAttackIfReady();
		}
	};
};

class npc_koroth_the_hillbreaker_qiao : public CreatureScript
{
public:
	npc_koroth_the_hillbreaker_qiao() : CreatureScript("npc_koroth_the_hillbreaker_qiao") { }

	CreatureAI* GetAI(Creature* creature) const
	{
		return new npc_koroth_the_hillbreaker_qiaoAI(creature);
	}

	struct npc_koroth_the_hillbreaker_qiaoAI : public ScriptedAI
	{
		npc_koroth_the_hillbreaker_qiaoAI(Creature* creature) : ScriptedAI(creature)
		{
			uiEventTimer = 5000;
			Event = false;
		}

		uint32 uiEventTimer;
		bool Event;

		void Reset()
		{
			me->SetReactState(REACT_PASSIVE);
		}

		void MovementInform(uint32 type, uint32 id)
		{
			if (type != POINT_MOTION_TYPE)
				return;

			if (id == POINT_BANNER)
				me->GetMotionMaster()->MoveTargetedHome();
		}

		void UpdateAI(uint32 const diff)
		{
			if (Event)
			{
				if (uiEventTimer <= diff)
				{
					Event = false;
					uiEventTimer = 8000;
					me->MonsterYell("You puny thief! Koroth find you! Koroth smash your face in!", 0, 0);
				}
				else
					uiEventTimer -= diff;
			}

			if (!UpdateVictim())
				return;

			DoMeleeAttackIfReady();
		}
	};
};

class go_koroth_banner : public GameObjectScript
{
public:
	go_koroth_banner() : GameObjectScript("go_koroth_banner") { }

	bool OnGossipHello(Player* player, GameObject* go)
	{
		if (player->GetQuestStatus(QUEST_INTRODUCTIONS_ARE_IN_ORDER) == QUEST_STATUS_INCOMPLETE)
		if (Creature* koroth = go->FindNearestCreature(NPC_KOROTH_THE_HILLBREAKER_QIAO, 30.0f))
		{
			koroth->MonsterYell("Who dares to touch Koroth's banner!?!", 0, 0);
			float x, y;
			go->GetNearPoint2D(x, y, 5.0f, go->GetOrientation() + M_PI / 2);
			koroth->GetMotionMaster()->MovePoint(POINT_BANNER, x, y, go->GetPositionZ());
			CAST_AI(npc_koroth_the_hillbreaker_qiao::npc_koroth_the_hillbreaker_qiaoAI, koroth->AI())->Event = true;
		}

		return false;
	}
};

class npc_koroth_the_hillbreaker : public CreatureScript
{
public:
	npc_koroth_the_hillbreaker() : CreatureScript("npc_koroth_the_hillbreaker") { }

	CreatureAI* GetAI(Creature* creature) const
	{
		return new npc_koroth_the_hillbreakerAI(creature);
	}

	struct npc_koroth_the_hillbreakerAI : public ScriptedAI
	{
		npc_koroth_the_hillbreakerAI(Creature* creature) : ScriptedAI(creature){ }

		void UpdateAI(const uint32 /*diff*/)
		{
			if (!UpdateVictim())
				return;

			DoMeleeAttackIfReady();
		}
	};
};

/*###### Losing Your Tail ######*/

/*######
## npc_trigger_quest_24616
######*/
class npc_trigger_quest_24616 : public CreatureScript
{
public:
	npc_trigger_quest_24616() : CreatureScript("npc_trigger_quest_24616") { }

	CreatureAI* GetAI(Creature* creature) const
	{
		return new npc_trigger_quest_24616AI(creature);
	}

	struct npc_trigger_quest_24616AI : public ScriptedAI
	{
		npc_trigger_quest_24616AI(Creature* creature) : ScriptedAI(creature)
		{
		}

		void Reset()
		{
			mui_timerAllowSummon = urand(3000, 5000);
			allowSummon = false;
			playerGUID = 0;
		}

		void MoveInLineOfSight(Unit* who)
		{
			if (who->GetTypeId() != TYPEID_PLAYER)
				return;
			if (who->ToPlayer()->GetQuestStatus(24616) != QUEST_STATUS_INCOMPLETE || me->FindNearestCreature(37953, 100, false) != NULL)
				return;
			if (me->IsWithinDistInMap(who, 20.0f))
			{
				allowSummon = true;
				playerGUID = who->GetGUID();
			}
		}

		void UpdateAI(uint32 const diff)
		{
			if (!allowSummon)
				return;
			if (mui_timerAllowSummon <= diff)
			{
				if (Player *player = Unit::GetPlayer(*me, playerGUID))
				if (me->FindNearestCreature(37953, 100) == NULL)
					me->CastSpell(player, 70794, true);
				allowSummon = false;
				playerGUID = 0;
				mui_timerAllowSummon = urand(3000, 5000);
			}
			else mui_timerAllowSummon -= diff;
		}

	private:
		uint32 mui_timerAllowSummon;
		bool allowSummon;
		uint64 playerGUID;
	};
};

/*######
## npc_krennan_aranas
######*/
class npc_krennan_aranas : public CreatureScript
{
public:
	npc_krennan_aranas() : CreatureScript("npc_krennan_aranas") { }

	bool OnGossipSelect(Player* player, Creature* creature, uint32 /*uiSender*/, uint32 uiAction)
	{
		player->PlayerTalkClass->ClearMenus();
		switch (uiAction)
		{
		case GOSSIP_ACTION_INFO_DEF + 1:
			player->CLOSE_GOSSIP_MENU();
			CAST_AI(npc_krennan_aranas::npc_krennan_aranasAI, creature->AI())->StartBattle(player);
			break;

		case GOSSIP_ACTION_INFO_DEF + 2:
			player->CLOSE_GOSSIP_MENU();
			CAST_AI(npc_krennan_aranas::npc_krennan_aranasAI, creature->AI())->EndBattle();
			break;
		}
		return true;
	}

	bool OnGossipHello(Player* player, Creature* creature)
	{
		if (creature->IsQuestGiver())
			player->PrepareQuestMenu(creature->GetGUID());

		if (player->GetQuestStatus(QUEST_THE_BATTLE_FOR_GILNEAS_CITY) == QUEST_STATUS_INCOMPLETE)
			player->ADD_GOSSIP_ITEM(GOSSIP_ICON_CHAT, "start battle !", GOSSIP_SENDER_MAIN, GOSSIP_ACTION_INFO_DEF + 1);

		if (player->isGameMaster())
			player->ADD_GOSSIP_ITEM(GOSSIP_ICON_CHAT, "(GM ONLY) RESET EVENT!", GOSSIP_SENDER_MAIN, GOSSIP_ACTION_INFO_DEF + 2);

		player->SEND_GOSSIP_MENU(2474, creature->GetGUID());

		return true;
	}

	CreatureAI* GetAI(Creature* pCreature) const
	{
		return new npc_krennan_aranasAI(pCreature);
	}

	struct npc_krennan_aranasAI : public ScriptedAI
	{
		npc_krennan_aranasAI(Creature *c) : ScriptedAI(c) {}

		uint64 princeGUID;

		void Reset()
		{
			princeGUID = 0;
			if (Creature* newPrince = me->SummonCreature(NPC_PRINCE_LIAM_GREYMANE_BATTLE, -1408.661f, 1260.017f, 36.51123f, 1.79f, TEMPSUMMON_DEAD_DESPAWN, 180000))
				princeGUID = newPrince->GetGUID();
		}

		void StartBattle(Player* pl)
		{
			if (Creature* prince = Unit::GetCreature(*me, princeGUID))
			{
				if (me->GetDistance2d(prince->GetPositionX(), prince->GetPositionY()) > 50)
					me->MonsterSay("event already started please wait a minute.", LANG_UNIVERSAL, 0);
				else
				{
					prince->AI()->DoAction(ACTION_START_EVENT);
					Talk(0, pl->GetGUID());
				}
			}
			else
			{
				princeGUID = 0;
				if (Creature* newPrince = me->SummonCreature(NPC_PRINCE_LIAM_GREYMANE_BATTLE, -1408.661f, 1260.017f, 36.51123f, 1.79f, TEMPSUMMON_DEAD_DESPAWN, 180000))
				{
					princeGUID = newPrince->GetGUID();
					prince->AI()->DoAction(ACTION_START_EVENT);
					Talk(0, pl->GetGUID());
				}
			}
		}

		void EndBattle()
		{
			if (Creature* prince = Unit::GetCreature(*me, princeGUID))
				prince->DespawnOrUnsummon();
			princeGUID = 0;
			if (Creature* newPrince = me->SummonCreature(NPC_PRINCE_LIAM_GREYMANE_BATTLE, -1408.661f, 1260.017f, 36.51123f, 1.79f, TEMPSUMMON_DEAD_DESPAWN, 180000))
				princeGUID = newPrince->GetGUID();
		}
	};
};

/*######
## npc_prince_liam_greymane_gilneas
######*/
class npc_prince_liam_greymane_gilneas : public CreatureScript
{
public:    npc_prince_liam_greymane_gilneas() : CreatureScript("npc_prince_liam_greymane_gilneas") {}

		   struct npc_prince_liam_greymane_gilneasAI : public npc_escortAI
		   {
			   npc_prince_liam_greymane_gilneasAI(Creature *c) : npc_escortAI(c), summons(me)
			   {
			   }

			   void Reset()
			   {
				   me->CastSpell(me, 72069, true);

				   startEvent = false;
				   intro = true;
				   canFollow = false;
				   phase = 1;
				   intoTimer = 5000;
				   checkTimer = 3000;
				   summons.DespawnAll();
				   float summon_x = me->GetPositionX() - 6;
				   float summon_y = me->GetPositionY() - 4;

				   int j = 0;
				   int row = 0;

				   for (int i = 0; i < 70; i++)
				   {
					   if (j > 6)
					   {
						   j = 0;
						   row++;
						   summon_y -= 3;
						   summon_x = me->GetPositionX() - 6;
					   }

					   if (Creature * milit = me->SummonCreature(38221, (summon_x + cos(5.42f)*row), (summon_y - sin(5.42f)*j), 36.51123f, 1.79f, TEMPSUMMON_MANUAL_DESPAWN))
					   {
						   militiamans[i] = milit->GetGUID();
						   if (i % 2 == 0)
							   milit->AI()->SetData(DATA_GROUP, GROUP_1);
						   else
							   milit->AI()->SetData(DATA_GROUP, GROUP_1); // need to be improved...
					   }
					   summon_x += 2;
					   j++;
				   }
				   mui_moveTimer = 1000;
			   }

			   void JustDied(Unit* /*killer*/)
			   {
				   summons.DespawnAll();
			   }

			   bool CanFollow() { return canFollow; }

			   void EnterCombat(Unit* pWho)
			   {
				   Talk(10);
			   }

			   void moveGroup(uint32 group, float x, float y, float z)
			   {
				   for (int i = 0; i < 70; i++)
				   if (Creature *milit = Unit::GetCreature(*me, militiamans[i]))
				   if (milit->AI()->GetData(DATA_GROUP) == group)
				   {
					   milit->SetHomePosition(x, y, z, milit->GetOrientation());
					   milit->GetMotionMaster()->MovePoint(0, x, y, z);
				   }
			   }

			   void JustSummoned(Creature* summon)
			   {
				   summons.Summon(summon);
			   }

			   void SummonedCreatureDies(Creature* summon, Unit* /*killer*/)
			   {
				   summons.Despawn(summon);
			   }

			   void WaypointReached(uint32 point)
			   {
				   RefreshMovementMilitary(GROUP_1);
				   switch (point)
				   {
				   case 1:
					   moveGroup(GROUP_2, -1487.67f, 1345.69f, 35.56f);
					   break;
				   case 3:
					   moveGroup(GROUP_2, -1551.90f, 1327.45f, 35.56f);
					   break;
				   case 5:
					   Talk(7);
					   if (Creature *c = me->SummonCreature(38426, -1549.20f, 1285.96f, 11.78f, 3.47f, TEMPSUMMON_MANUAL_DESPAWN))
						   c->AI()->DoAction(ACTION_START_EVENT);
					   SetEscortPaused(true);
					   break;
				   case 6:
					   moveGroup(GROUP_2, me->GetPositionX(), me->GetPositionY(), me->GetPositionZ());
					   Talk(8);
					   break;
				   case 7:
					   moveGroup(GROUP_2, -1657.01f, 1337.70f, 15.135f);
					   break;
				   case 8: //crowley
					   moveGroup(GROUP_2, me->GetPositionX(), me->GetPositionY(), me->GetPositionZ());
					   Talk(9);
					   break;
				   case 10:
				   case 11:
				   case 12:
					   moveGroup(GROUP_2, me->GetPositionX(), me->GetPositionY(), me->GetPositionZ());
					   break;
				   case 13:
					   if (Creature *abo = me->SummonCreature(38331, -1710.466f, 1407.905f, 21.75196f, -1.89314f, TEMPSUMMON_DEAD_DESPAWN, 60000))
					   {
						   abo->GetMotionMaster()->MoveJump(-1710.466f, 1407.905f, 21.75196f, 20.0f, 20.0f);
						   if (Creature *darius = me->SummonCreature(38415, -1771.81f, 1341.33f, 19.75, 0.81314f, TEMPSUMMON_MANUAL_DESPAWN))
							   darius->AI()->SetGUID(abo->GetGUID());
					   }
					   break;
				   case 14:
					   moveGroup(GROUP_2, -1761.24f, 1512.72f, 26.22f);
					   break;
				   case 15:
					   for (int i = 0; i < 70; i++)
					   if (Creature *milit = Unit::GetCreature(*me, militiamans[i]))
						   milit->AI()->SetData(DATA_GROUP, GROUP_1);
					   break;
				   case 19:
					   if (Creature *c = me->FindNearestCreature(38470, 100, true))
						   c->AI()->SetData(0, 1);
					   summons.DespawnAll();
					   me->DespawnOrUnsummon();
					   break;
				   }
			   }

			   void RefreshMovementMilitary(uint32 group)
			   {
				   float x, y, z;
				   for (int i = 0; i < 70; i++)
				   {
					   float distance = urand(0, 7);
					   float angle = frand(-3 * M_PI / 4, 3 * M_PI / 2);
					   me->GetNearPoint(me, x, y, z, me->GetObjectSize(), distance, angle);
					   if (Creature *milit = Unit::GetCreature(*me, militiamans[i]))
					   if (milit->AI()->GetData(DATA_GROUP) == group)
						   milit->SetHomePosition(x, y, z, me->GetOrientation());
				   }
			   }

			   void DoAction(const int32 info)
			   {
				   switch (info)
				   {
				   case ACTION_START_EVENT:
					   startEvent = true;
					   break;
				   default:
					   break;
				   }
			   }

			   void UpdateAI(const uint32 diff)
			   {
				   if (!startEvent)
					   return;
				   npc_escortAI::UpdateAI(diff);

				   if (canFollow)
				   {
					   if (mui_moveTimer <= diff)
					   {
						   RefreshMovementMilitary(GROUP_1);
						   mui_moveTimer = 10000;
					   }
					   else
						   mui_moveTimer -= diff;
				   }

				   if (intro)
				   {
					   if (intoTimer <= diff)
					   {
						   switch (phase)
						   {
						   case 1:
							   Talk(0);
							   intoTimer = 10000;
							   break;
						   case 2:
							   Talk(1);
							   intoTimer = 10000;
							   break;
						   case 3:
							   Talk(2);
							   intoTimer = 12000;
							   break;
						   case 4:
							   Talk(3);
							   intoTimer = 10000;
							   break;
						   case 5:
							   Talk(4);
							   intoTimer = 12000;
							   break;
						   case 6:
							   Talk(5);
							   intoTimer = 5500;
							   break;
						   case 7:
							   Talk(6);
							   intoTimer = 1500;
							   Start(true, true);
							   SetDespawnAtEnd(true);
							   break;
						   case 8:
							   me->PlayDistanceSound(22584);
							   for (int i = 0; i < 70; i++)
							   if (Creature *milit = Unit::GetCreature(*me, militiamans[i]))
							   {
								   milit->HandleEmoteCommand(25);
								   float distance = urand(7, 15);
								   float angle = frand(-3 * M_PI / 4, 3 * M_PI / 4);
								   milit->GetMotionMaster()->MoveFollow(me, distance, angle, MOTION_SLOT_ACTIVE);
							   }
							   intro = false;
							   canFollow = true;
							   break;
						   }
						   phase++;
					   }
					   else intoTimer -= diff;
				   }

				   if (checkTimer <= diff)
				   {
					   if (!me->HasAura(72069))
						   me->CastSpell(me, 72069, true);
					   checkTimer = 3000;
				   }
				   else checkTimer -= diff;

				   if (!UpdateVictim())
					   return;

				   DoMeleeAttackIfReady();
			   }

		   private:
			   bool intro;
			   bool canFollow;
			   uint32 phase;
			   uint32 intoTimer;
			   uint32 checkTimer;
			   uint64 militiamans[70];
			   uint32 mui_moveTimer;
			   bool startEvent;
			   SummonList summons;
		   };

		   CreatureAI* GetAI(Creature* creature) const
		   {
			   return new npc_prince_liam_greymane_gilneasAI(creature);
		   }
};

/*######
## npc_lorna_crowley_event
######*/
class npc_lorna_crowley_event : public CreatureScript
{
public:
	npc_lorna_crowley_event() : CreatureScript("npc_lorna_crowley_event") { }

	CreatureAI* GetAI(Creature* pCreature) const
	{
		return new npc_lorna_crowley_eventAI(pCreature);
	}

	struct npc_lorna_crowley_eventAI : public ScriptedAI
	{
		npc_lorna_crowley_eventAI(Creature *c) : ScriptedAI(c) {}

		void Reset()
		{
			eventInProgress = false;
			mui_moveTimer = 5000;
			mui_attack = 8000;
		}

		void MovementInform(uint32 type, uint32 point)
		{
			if (!eventInProgress)
				Talk(0);
			eventInProgress = true;
			me->DespawnOrUnsummon(60000);
		}

		void DoAction(const int32 info)
		{
			switch (info)
			{
			case ACTION_START_EVENT:
				float summon_x = me->GetPositionX();
				float summon_y = me->GetPositionY();
				float summon_z = me->GetPositionZ();
				for (int i = 0; i < 12; i++)
				if (Creature * milit = me->SummonCreature(38425, summon_x, summon_y, summon_z, 0.0f, TEMPSUMMON_MANUAL_DESPAWN))
				{
					if (i == 0 || i == 1 || i == 2)
						citizenGUID[i] = milit->GetGUID();
					if (i != 0)
					if (Creature *firstCiti = Unit::GetCreature(*me, citizenGUID[0]))
						milit->GetMotionMaster()->MoveFollow(firstCiti, urand(0, 5), frand(-3 * M_PI / 8, 3 * M_PI / 8), MOTION_SLOT_ACTIVE);
				}
				if (Creature *firstCiti = Unit::GetCreature(*me, citizenGUID[0]))
					firstCiti->GetMotionMaster()->MovePoint(0, -1571.79f, 1318.833f, 35.56271f);
				me->GetMotionMaster()->MovePoint(0, -1571.79f, 1318.833f, 35.56271f);
				if (Creature *c = me->SummonCreature(38424, -1542.548f, 1290.957f, 6.239825f, 3.490021f, TEMPSUMMON_MANUAL_DESPAWN))
				{
					c->GetMotionMaster()->MovePoint(0, -1571.79f, 1318.833f, 35.56271f);
					canonGUID = c->GetGUID();
				}
				if (Creature *c = me->SummonCreature(38424, -1541.036f, 1283.645f, 6.35803f, 3.343305f, TEMPSUMMON_MANUAL_DESPAWN))
					c->GetMotionMaster()->MovePoint(0, -1571.79f, 1318.833f, 35.56271f);
				if (Creature *c = me->SummonCreature(38424, -1543.06f, 1287.436f, 7.076612f, 3.446335f, TEMPSUMMON_MANUAL_DESPAWN))
					c->GetMotionMaster()->MovePoint(0, -1571.79f, 1318.833f, 35.56271f);
				break;
			}
		}

		void UpdateAI(const uint32 diff)
		{
			if (!eventInProgress)
				return;

			if (mui_moveTimer <= diff)
			{
				if (Creature *canon = Unit::GetCreature(*me, canonGUID))
				{
					if (Creature *firstCiti = Unit::GetCreature(*me, citizenGUID[1]))
						firstCiti->GetMotionMaster()->MovePoint(1, -1606.97f, 1313.59f, 18.99f);
					if (Creature *firstCiti = Unit::GetCreature(*me, citizenGUID[2]))
						firstCiti->GetMotionMaster()->MovePoint(1, -1610.97f, 1310.59f, 18.11f);
					me->CastCustomSpell(VEHICLE_SPELL_RIDE_HARDCODED, SPELLVALUE_BASE_POINT0, 1, canon, false);
					canon->GetMotionMaster()->MovePoint(1, -1609.97f, 1312.59f, 18.11f);
					if (me->ToTempSummon())
					if (Unit *general = me->ToTempSummon()->GetSummoner())
					{
						me->GetMotionMaster()->MoveFollow(general, urand(0, 5), frand(-3 * M_PI / 8, 3 * M_PI / 8), MOTION_SLOT_ACTIVE);
						if (general->ToCreature())
							CAST_AI(npc_escortAI, (general->ToCreature()->AI()))->SetEscortPaused(false);
					}
				}
				mui_moveTimer = 3600000;
			}
			else  mui_moveTimer -= diff;

			if (mui_attack <= diff)
			{
				if (Creature *c = me->FindNearestCreature(38420, 200))
				if (Creature *canon = Unit::GetCreature(*me, canonGUID))
					canon->CastSpell(c, 72013, true);
				mui_attack = 5000;
			}
			else mui_attack -= diff;
		}

	private:
		bool eventInProgress;
		uint32 mui_moveTimer;
		uint32 mui_attack;
		uint64 canonGUID;
		uint64 citizenGUID[2];
	};
};

/*######
## npc_gilnean_militia
######*/
class npc_gilnean_militia : public CreatureScript
{
public:
	npc_gilnean_militia() : CreatureScript("npc_gilnean_militia") { }

	CreatureAI* GetAI(Creature* pCreature) const
	{
		return new npc_gilnean_militiaAI(pCreature);
	}

	struct npc_gilnean_militiaAI : public ScriptedAI
	{
		npc_gilnean_militiaAI(Creature* pCreature) : ScriptedAI(pCreature) { }

		void Reset()
		{
			mui_spell1 = urand(1200, 5100);
			mui_spell2 = urand(2100, 5400);
			me->AddUnitState(UNIT_STATE_IGNORE_PATHFINDING);
		}

		void AttackStart(Unit* who)
		{
			if (me->Attack(who, true))
				DoStartNoMovement(who);
		}

		void SetData(uint32 uiType, uint32 uiData)
		{
			if (uiType == DATA_GROUP)
				group = uiData;
		}

		uint32 GetData(uint32 uiType) const
		{
			if (uiType == DATA_GROUP)
				return group;
			return 0;
		}

		void JustReachedHome()
		{
			if (group == GROUP_1)
			if (!UpdateVictim())
			if (me->ToTempSummon())
			if (Unit *p = me->ToTempSummon()->GetSummoner())
			{
				float distance = urand(7, 15);
				float angle = frand(-3 * M_PI / 4, 3 * M_PI / 2);
				float x, y, z;
				me->GetNearPoint(p, x, y, z, p->GetObjectSize(), distance, angle);
				me->GetMotionMaster()->MoveFollow(p, distance, angle, MOTION_SLOT_ACTIVE);
			}
		}

		void UpdateAI(const uint32 diff)
		{
			if (!UpdateVictim())
				return;
			if (mui_spell1 <= diff)
			{
				DoCastVictim(15572);
				mui_spell1 = 10000 + urand(1200, 5100);
			}
			else
				mui_spell1 -= diff;

			if (mui_spell2 <= diff)
			{
				DoCastVictim(47168);
				mui_spell2 = 10000 + urand(2100, 5400);
			}
			else
				mui_spell2 -= diff;

			victim = me->GetVictim();
			if (victim && me->GetDistance2d(victim->GetPositionX(), victim->GetPositionY()) > 7)
				DoSpellAttackIfReady(6660);
			else
				DoMeleeAttackIfReady();
		}

	private:
		uint32 mui_spell1;
		uint32 mui_spell2;
		Unit *victim;
		uint32 group;
	};
};

/*######
## npc_lord_darius_crowley_gilneas
######*/
class npc_lord_darius_crowley_gilneas : public CreatureScript
{
public:
	npc_lord_darius_crowley_gilneas() : CreatureScript("npc_lord_darius_crowley_gilneas") { }

	CreatureAI* GetAI(Creature* pCreature) const
	{
		return new npc_lord_darius_crowley_gilneasAI(pCreature);
	}

	struct npc_lord_darius_crowley_gilneasAI : public ScriptedAI
	{
		npc_lord_darius_crowley_gilneasAI(Creature* pCreature) : ScriptedAI(pCreature) { }

		void Reset()
		{
			float angle = 0;
			for (int i = 0; i < 30; i++)
			{
				if (Creature *summon = me->SummonCreature(38348, me->GetPositionX(), me->GetPositionY(), me->GetPositionZ(), 0, TEMPSUMMON_MANUAL_DESPAWN))
				{
					summon->GetMotionMaster()->MovePoint(0, -1710.466f + cos(angle) * 15, 1407.905f + sin(angle) * 15, 21.75196f);
					if (i >= 0 && i <= 3)
						worgenGUID[i] = summon->GetGUID();
				}
				angle += M_PI / 15;
			}
			me->GetMotionMaster()->MovePoint(0, -1710.466f, 1407.905f, 21.75196f);
			aboGUID = 0;
			mui_jumpBoss = 3000;
			moveJumpBoss = true;
		}

		void SetGUID(uint64 guid, int32 type = 0)
		{
			aboGUID = guid;
		}

		void EnterCombat(Unit* pWho)
		{
			if (pWho->GetEntry() == 38348)
				me->CastSpell(pWho, 71921, true);
			Talk(0);
		}

		void UpdateAI(const uint32 diff)
		{
			if (moveJumpBoss)
			{
				if (mui_jumpBoss <= diff)
				{
					if (Creature *abo = Unit::GetCreature(*me, aboGUID))
					for (int i = 0; i <= 3; i++)
					if (Creature *worgen = Unit::GetCreature(*me, worgenGUID[i]))
						worgen->CastCustomSpell(VEHICLE_SPELL_RIDE_HARDCODED, SPELLVALUE_BASE_POINT0, i + 1, abo, false);
					moveJumpBoss = false;
					mui_jumpBoss = 3600000;
				}
				else mui_jumpBoss -= diff;
			}
			if (!UpdateVictim())
				return;
			DoMeleeAttackIfReady();
		}

	private:
		uint64 aboGUID;
		uint32 mui_jumpBoss;
		bool moveJumpBoss;
		uint64 worgenGUID[4];
	};
};

/*######
## npc_worgen_warrior
######*/
class npc_worgen_warrior : public CreatureScript
{
public:
	npc_worgen_warrior() : CreatureScript("npc_worgen_warrior") { }

	CreatureAI* GetAI(Creature* pCreature) const
	{
		return new npc_worgen_warriorAI(pCreature);
	}

	struct npc_worgen_warriorAI : public ScriptedAI
	{
		npc_worgen_warriorAI(Creature* pCreature) : ScriptedAI(pCreature) { }

		void Reset()
		{
		}

		void EnterCombat(Unit* pWho)
		{
			if (pWho->GetEntry() == 38348)
			{
				me->CastSpell(pWho, 71921, true);
			}
		}

		void UpdateAI(const uint32 diff)
		{
			if (!UpdateVictim())
				return;

			DoMeleeAttackIfReady();
		}
	};
};

/*######
## npc_lady_sylvanas_gilneas
######*/
class npc_lady_sylvanas_gilneas : public CreatureScript
{
public:
	npc_lady_sylvanas_gilneas() : CreatureScript("npc_lady_sylvanas_gilneas") { }

	CreatureAI* GetAI(Creature* pCreature) const
	{
		return new npc_lady_sylvanas_gilneasAI(pCreature);
	}

	struct npc_lady_sylvanas_gilneasAI : public ScriptedAI
	{
		npc_lady_sylvanas_gilneasAI(Creature* pCreature) : ScriptedAI(pCreature) { }

		bool outEvent;
		uint32 phase;
		uint32 phaseTimer;

		Creature* liam;
		Creature* genn;

		void Reset()
		{
			outEvent = false;
			phase = 1;
			phaseTimer = 300;
		}

		void EnterCombat(Unit* pWho)
		{

		}

		void DoAction(const int32 type)
		{
			if (type == ACTION_START_EVENT)
				outEvent = true;
		}

		void DamageTaken(Unit* who, uint32 &damage)
		{
			if (me->GetHealthPct() <= 20)
			{
				DoAction(ACTION_START_EVENT);
				damage = 0;
			}
		}

		void UpdateAI(const uint32 diff)
		{
			if (outEvent)
			{
				if (phaseTimer <= diff)
				{
					switch (phase)
					{
					case 1:
						phaseTimer = 1000;
						DoCastAOE(72113);
						break;
					case 2:
						me->SetReactState(REACT_PASSIVE);
						me->SetFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_NON_ATTACKABLE);
						phaseTimer = 2000;
						break;
					case 3:
						if (genn = me->FindNearestCreature(38470, 100))
							genn->CastSpell(genn, 86141, true);
						phaseTimer = 2000;
						break;
					case 4:
						if (liam = me->SummonCreature(38474, -1634.634f, 1631.859f, 21.21159f, 4.694936f, TEMPSUMMON_TIMED_DESPAWN, 60000))
						{
							liam->AI()->Talk(0);
							liam->SetReactState(REACT_PASSIVE);
						}
						phaseTimer = 2000;
						break;
					case 5:
						liam->GetMotionMaster()->MovePoint(0, me->GetPositionX(), me->GetPositionY(), me->GetPositionZ());
						liam->AI()->Talk(1);
						phaseTimer = 6000;
						break;
					case 7:
						liam->AI()->Talk(2);
						me->CastSpell(liam, 72116, true);
						me->Kill(liam);
						genn->RemoveAurasDueToSpell(86141);
						genn->RemoveAurasDueToSpell(72113);
						genn->CastSpell(genn, 68442, true);
						phaseTimer = 2000;
						break;
					case 8:
						Talk(0);
						phaseTimer = 2000;
						break;
					case 9:
						Talk(1);
						phaseTimer = 3500;
						break;
					case 10:
						Talk(2);
						phaseTimer = 2500;
						break;
					case 11:
						me->GetMotionMaster()->MovePoint(0, -1634.634f, 1631.859f, 21.21159f);
						phaseTimer = 2500;
						break;
					case 12:
						genn->DespawnOrUnsummon();
						std::list<Player *> players = me->GetPlayersInRange(100, true);
						for (std::list<Player *>::const_iterator it = players.begin(); it != players.end(); it++)
							(*it)->KilledMonsterCredit(38854, 0);
						outEvent = false;
						me->DespawnOrUnsummon();
						break;
					}
					phase++;
				}
				else phaseTimer -= diff;
			}

			if (!UpdateVictim())
				return;

			DoMeleeAttackIfReady();
		}
	};
};

/*######
## Quest Slowing the Inevitable 24920
######*/

enum qSTI
{
	QUEST_SLOWING_THE_INEVITABLE = 24920,

	NPC_CAPTURED_RIDING_BAT = 38540,
};

const float BatWP[25][3] =
{
	{ -1623.96f, 1678.45f, 24.895f }, { -1387.62f, 1635.81f, 50.594f }, { -1351.91f, 1643.61f, 54.694f },
	{ -1276.31f, 1669.73f, 63.570f }, { -1252.37f, 1685.45f, 66.732f }, { -1226.15f, 1692.88f, 69.788f },
	{ -1210.95f, 1693.08f, 71.477f }, { -1181.78f, 1681.70f, 74.933f }, { -1155.08f, 1665.27f, 78.418f },
	{ -1092.38f, 1623.88f, 86.757f }, { -1061.24f, 1617.66f, 90.315f }, { -1005.78f, 1628.92f, 96.749f },
	{ -986.882f, 1638.44f, 99.158f }, { -974.702f, 1667.29f, 102.61f }, { -968.120f, 1683.84f, 104.58f },
	{ -972.953f, 1711.52f, 107.82f }, { -1003.23f, 1723.95f, 111.92f }, { -1026.01f, 1713.48f, 114.76f },
	{ -1098.00f, 1684.50f, 123.43f }, { -1262.51f, 1632.93f, 90.103f }, { -1353.43f, 1605.96f, 72.326f },
	{ -1563.83f, 1657.32f, 59.702f }, { -1569.20f, 1658.74f, 59.382f }, { -1638.03f, 1671.92f, 30.168f },
	{ -1667.43f, 1666.94f, 22.131f },
};

class spell_captured_riding_bat : public SpellScriptLoader
{
public:
	spell_captured_riding_bat() : SpellScriptLoader("spell_captured_riding_bat") { }

	class spell_captured_riding_bat_SpellScript : public SpellScript
	{
		PrepareSpellScript(spell_captured_riding_bat_SpellScript)

		void OnSummon(SpellEffIndex effIndex)
		{
			PreventHitDefaultEffect(effIndex);

			Unit* caster = GetCaster();

			if (!caster)
				return;

			float x, y, z = caster->GetPositionZ();

			if (Creature* bat = caster->FindNearestCreature(38615, 15.0f))
				bat->GetNearPoint2D(x, y, 3.0f, bat->GetOrientation() + M_PI / 2);
			else
				caster->GetPosition(x, y);

			if (Creature* bat = caster->SummonCreature(NPC_CAPTURED_RIDING_BAT, x, y, z, 0.0f))
			if (npc_escortAI* escort = CAST_AI(npc_escortAI, bat->AI()))
			{
				caster->EnterVehicle(bat, 0);
				//bat->EnableAI();
				bat->SetSpeed(MOVE_FLIGHT, 4.0f, true);

				for (int i = 0; i < 25; ++i)
					escort->AddWaypoint(i, BatWP[i][0], BatWP[i][1], BatWP[i][2]);

				escort->Start(true);
			}
		}

		void Register()
		{
			OnEffectHit += SpellEffectFn(spell_captured_riding_bat_SpellScript::OnSummon, EFFECT_0, SPELL_EFFECT_SUMMON);
		}
	};

	SpellScript *GetSpellScript() const
	{
		return new spell_captured_riding_bat_SpellScript();
	}
};

class npc_glaive_thrower : public CreatureScript
{
public:
	npc_glaive_thrower() : CreatureScript("npc_glaive_thrower") { }

	CreatureAI* GetAI(Creature* creature) const
	{
		return new npc_captured_riding_batAI(creature);
	}

	struct npc_captured_riding_batAI : public npc_escortAI
	{
		npc_captured_riding_batAI(Creature* creature) : npc_escortAI(creature)
		{
			//me->SetCanFly(true);
		}

		void FinishEscort()
		{
			me->DespawnOrUnsummon();
		}

		void WaypointReached(uint32 /*point*/)
		{
		}

		void UpdateAI(uint32 const diff)
		{
			npc_escortAI::UpdateAI(diff);
		}
	};
};

class spell_glaive_thrower : public SpellScriptLoader
{
public:
	spell_glaive_thrower() : SpellScriptLoader("spell_glaive_thrower") { }

	class spell_glaive_thrower_SpellScript : public SpellScript
	{
		PrepareSpellScript(spell_glaive_thrower_SpellScript)

		void OnSummon(SpellEffIndex effIndex)
		{
			PreventHitDefaultEffect(effIndex);

			Unit* caster = GetCaster();

			if (!caster)
				return;

			Creature* thrower = NULL;

			float x, y, z = caster->GetPositionZ();

			if (Creature* thrower = caster->FindNearestCreature(38150, 15.0f)) //spawnd
				caster->GetPosition(x, y);

			if (Creature* thrower = caster->SummonCreature(37927, x, y, z, 0.0f))//summond
				caster->EnterVehicle(thrower, 0);

		}

		void Register()
		{
			OnEffectHit += SpellEffectFn(spell_glaive_thrower_SpellScript::OnSummon, EFFECT_0, SPELL_EFFECT_SUMMON);
		}
	};

	SpellScript *GetSpellScript() const
	{
		return new spell_glaive_thrower_SpellScript();
	}
};

/*######
## npc_lorna_crowley_c2
######*/
class npc_lorna_crowley_c2 : public CreatureScript
{
public:
	npc_lorna_crowley_c2() : CreatureScript("npc_lorna_crowley_c2") { }

	bool OnQuestAccept(Player* player, Creature* creature, Quest const* quest)
	{
		if (quest->GetQuestId() == 24902)
			CAST_AI(npc_lorna_crowley_c2::npc_lorna_crowley_c2AI, creature->AI())->StartEncounter(player);
		return true;
	}

	CreatureAI* GetAI(Creature* pCreature) const
	{
		return new npc_lorna_crowley_c2AI(pCreature);
	}

	struct npc_lorna_crowley_c2AI : public ScriptedAI
	{
		npc_lorna_crowley_c2AI(Creature *c) : ScriptedAI(c) {}

		uint64 tobiasGUID;

		void Reset()
		{
			tobiasGUID = 0;
		}

		void StartEncounter(Player* pl)
		{
			if (Creature* tobias = Unit::GetCreature(*me, tobiasGUID))
				pl->TeleportTo(MAP_GILNEAS, tobias->GetPositionX(), tobias->GetPositionY(), tobias->GetPositionZ(), 5.42f);
			else
			{
				if (Creature* newTobias = me->SummonCreature(NPC_TOBIAS_MISTMANTLE, -1662.616f, 1591.453f, 23.20896f, 3.740811f, TEMPSUMMON_DEAD_DESPAWN, 180000))
				{
					newTobias->SetPhaseMask(pl->GetPhaseMask(), true);
					tobiasGUID = newTobias->GetGUID();
				}
			}
		}
	};
};

/*######
## npc_chauve_souris_capture
######*/
class npc_chauve_souris_capture : public CreatureScript
{
public:
	npc_chauve_souris_capture() : CreatureScript("npc_chauve_souris_capture") { }

	struct npc_chauve_souris_captureAI : public npc_escortAI
	{
		npc_chauve_souris_captureAI(Creature* creature) : npc_escortAI(creature) {}

		void AttackStart(Unit* /*who*/) {}
		void EnterCombat(Unit* /*who*/) {}
		void EnterEvadeMode() {}

		void Reset()
		{
			_checkQuest = 1000;
			_checkDespawn = 1000;
			isBoarded = false;
			isBoarded2 = false;
		}

		void PassengerBoarded(Unit* who, int8 /*seatId*/, bool apply)
		{
			if (apply)
			{
				if (who->GetTypeId() == TYPEID_PLAYER)
				if (who->ToPlayer()->GetQuestStatus(24920) != QUEST_STATUS_INCOMPLETE)
				{
					me->GetVehicleKit()->RemoveAllPassengers();
					return;
				}
				me->SetCanFly(true);
				me->AddUnitMovementFlag(MOVEMENTFLAG_DISABLE_GRAVITY);
				me->SetSpeed(MOVE_FLIGHT, 4.0f);
				isBoarded = true;
				Start(false, true, who->GetGUID(), NULL, false, true);
			}
		}

		void WaypointReached(uint32 i)
		{
			me->SetCanFly(true);
		}

		void JustDied(Unit* /*killer*/)
		{
		}

		void OnCharmed(bool /*apply*/)
		{
		}


		void UpdateAI(const uint32 diff)
		{
			npc_escortAI::UpdateAI(diff);
		}

		void UpdateEscortAI(const uint32 diff)
		{
			if (isBoarded)
			{
				if (isBoarded2)
				{
					if (_checkDespawn <= diff)
					{
						me->GetVehicleKit()->RemoveAllPassengers();
						me->DespawnOrUnsummon();
						_checkDespawn = 1000;
					}
					else
						_checkDespawn -= diff;
				}
				else
				{
					if (_checkQuest <= diff)
					{
						if (me->GetVehicleKit())
						if (Unit *u = me->GetVehicleKit()->GetPassenger(0))
						if (u->GetTypeId() == TYPEID_PLAYER)
						{
							Player *player = u->ToPlayer();
							if (player->GetQuestStatus(24920) == QUEST_STATUS_COMPLETE)
							{
								isBoarded2 = true;
								_checkDespawn = 70000;
								SetEscortPaused(true);
								me->GetMotionMaster()->MovePoint(4242, me->GetHomePosition());
							}
						}
						_checkQuest = 1000;
					}
					else
						_checkQuest -= diff;
				}
			}
		}

	private:
		uint32 _checkQuest;
		uint32 _checkDespawn;
		bool isBoarded;
		bool isBoarded2;
	};

	CreatureAI* GetAI(Creature* creature) const
	{
		return new npc_chauve_souris_captureAI(creature);
	}
};

/*######
## npc_admiral_nightwind
######*/
class npc_admiral_nightwind : public CreatureScript
{
public:
	npc_admiral_nightwind() : CreatureScript("npc_admiral_nightwind") { }
	bool OnQuestReward(Player* player, Creature* /*creature*/, Quest const* quest, uint32 /*opt*/)
	{
		if (quest->GetQuestId() == 14434)
		{
			player->RemoveAllAuras();
			player->TeleportTo(1, 8343, 801, 4, 4);
		}
		return true;
	}
};

/*######
## npc_lord_godfrey_map
######*/
class npc_lord_godfrey_map : public CreatureScript
{
public:
	npc_lord_godfrey_map() : CreatureScript("npc_lord_godfrey_map") { }
	bool OnQuestAccept(Player* player, Creature* creature, Quest const* quest)
	{
		if (quest->GetQuestId() == 14396)
		{
			std::set<uint32> terrainswap;
			std::set<uint32> phaseId;
			terrainswap.insert(655);
			phaseId.insert(1);
			player->GetSession()->SendSetPhaseShift(phaseId, terrainswap);
			player->SaveToDB();
		}

		return true;
	}
};

class npc_gilneas_tremors_credit : public CreatureScript
{
public:
	npc_gilneas_tremors_credit() : CreatureScript("npc_gilneas_tremors_credit") { }

	CreatureAI* GetAI(Creature* creature) const
	{
		return new npc_gilneas_tremors_creditAI(creature);
	}

	struct npc_gilneas_tremors_creditAI : public ScriptedAI
	{
		npc_gilneas_tremors_creditAI(Creature* creature) : ScriptedAI(creature)
		{
			me->setActive(true);
			uiTremorTimerFirst = urand(30000, 150000);
			uiTremorTimerSecond = urand(20000, 100000);
			uiTremorTimerThird = urand(10000, 50000);
		}

		uint32 uiTremorTimerFirst;
		uint32 uiTremorTimerSecond;
		uint32 uiTremorTimerThird;

		void Tremor(uint32 uiPhase, uint32 uiSpellId)
		{
			Map::PlayerList const &PlList = me->GetMap()->GetPlayers();

			if (PlList.isEmpty())
				return;

			for (Map::PlayerList::const_iterator i = PlList.begin(); i != PlList.end(); ++i)
			if (Player* player = i->getSource())
			if (uiPhase & player->GetPhaseMask())
				player->CastSpell(player, uiSpellId, true);
		}

		void UpdateAI(uint32 const diff)
		{
			if (uiTremorTimerFirst <= diff)
			{
				uiTremorTimerFirst = urand(30000, 150000);
				Tremor(4096, SPELL_CATACLYSM_TYPE_1);
			}
			else
				uiTremorTimerFirst -= diff;

			if (uiTremorTimerSecond <= diff)
			{
				uiTremorTimerSecond = urand(20000, 100000);
				Tremor(8192, SPELL_CATACLYSM_TYPE_2);
			}
			else
				uiTremorTimerSecond -= diff;

			if (uiTremorTimerThird <= diff)
			{
				uiTremorTimerThird = urand(10000, 50000);
				Tremor(16384, SPELL_CATACLYSM_TYPE_3);
			}
			else
				uiTremorTimerThird -= diff;
		}
	};
};

void AddSC_gilneas()
{
	new npc_gilnean_crow();
	new spell_attack_lurker();
	new npc_admiral_nightwind();
	new npc_gilneas_city_guard_phase2();
	new npc_prince_liam_greymane_phase2();
	new npc_rampaging_worgen();
	new npc_rampaging_worgen2();
	new npc_sergeant_cleese();
	new npc_josiah_avery();
	new npc_josiah_avery_trigger();
	new npc_lorna_crowley_p4();
	new npc_bloodfang_lurker();
	new npc_gilnean_mastiff();
	//new npc_bloodfang_worgen();
	new npc_frightened_citizen();
	new npc_gilnean_royal_guard();
	new npc_mariam_spellwalker();

	new npc_lord_darius_crowley_c3();
	new npc_king_genn_greymane();
	new npc_crowley_horse();
	new npc_slain_watchman();
	new npc_horrid_abomination();
	new npc_gwen_armstead_qi();
	new npc_prince_liam_greymane_dh();
	//new spell_keg_placed();
	new npc_king_greymanes_horse();
	new npc_krennan_aranas_c2();
	new npc_bloodfang_stalker_c1();
	new npc_lord_darius_crowley_c2();
	new npc_trigger_event_c3();
	new npc_trigger_event_first_c3();
	new npc_king_genn_greymane_c2();

	new npc_lord_darius_crowley_c1();
	new npc_worgen_runt_c1();
	new npc_worgen_alpha_c1();
	new npc_worgen_runt_c2();
	new npc_worgen_alpha_c2();

	new go_mandragore();
	new go_merchant_square_door();
	new npc_drowning_watchman();
	new npc_prince_liam_greymane();
	//new spell_rescue_noyade();
	new spell_round_up_horse();
	new npc_gwen_armstead();
	new npc_swift_mountain_horse();
	new npc_king_genn_greymane_c3();
	new npc_chance_the_cat();
	new npc_lucius_the_cruel();
	new npc_wahl();
	new npc_trigger_quest_24616();

	new npc_krennan_aranas();
	new npc_prince_liam_greymane_gilneas();
	new npc_gilnean_militia();
	new npc_lord_darius_crowley_gilneas();
	new npc_worgen_warrior();
	new npc_lady_sylvanas_gilneas();
	new npc_lorna_crowley_c2();

	new npc_lorna_crowley_event();
	new npc_chauve_souris_capture();
	new npc_commandeered_cannon();
	new npc_lord_godfrey_p4_8();
	new npc_lord_godfrey_map();

	new spell_captured_riding_bat();
	new spell_glaive_thrower();
	new npc_glaive_thrower();
	new npc_swamp_crocolisk();
	new npc_crash_survivor();

	new npc_koroth_the_hillbreaker();
	new go_koroth_banner();
	new npc_koroth_the_hillbreaker_qiao();
	new npc_prince_liam_greymane_qiao();
	new npc_captain_asther_qiao();
	new npc_koroth_the_hillbreaker_qiao_friend();
	new npc_forsaken_catapult_qtbs();
	new npc_gilneas_tremors_credit();
	new npc_james();
	new npc_ashley();
	new npc_cynthia();
	new npc_round_up_horse();
	new npc_mountain_horse_vehicle();
}
