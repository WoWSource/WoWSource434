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

#include "ScriptPCH.h"
#include "the_stonecore.h"

/***************************************TRASH SPELLS*************************************/
// Crystalspawn Giant (42810) Health: 536, 810 - 1, 202, 925
// update creature_template set
/*enum Spells
{
// Stonecore Berserker (43430) Health: 312, 753 - 387, 450
SPELL_SCHARGE            = 81574,
SPELL_SPINNING_SLASH     = 81568,

// Stonecore Bruiser (42692) Health: 590, 491 - 1, 202, 925
SPELL_BODY_SLAM          = 80180,
SPELL_SHOCKWAVE          = 80195,
H_SPELL_SHOCKWAVE        = 92640,

// Stonecore Earthshaper (43537) Health: 250, 201 - 309, 960, Mana: 19, 394
SPELL_DUST_STORM         = 81463,
SPELL_FORCE_OF_EARTH     = 81459,
SPELL_GROUND_SHOCK       = 81530,
H_SPELL_GROUND_SHOCK     = 92628,
SPELL_LAVA_BURST         = 81576,
H_SPELL_LAVA_BURST       = 92626,

// Stonecore Flayer (42808) Health: 312, 753 - 387, 450
SPELL_FLAY               = 79922,

// Stonecore Magmalord (42789) Health: 312, 753 - 387, 450, Mana: 25, 014 - 26, 724
SPELL_IGNITE             = 80151,
H_SPELL_IGNITE           = 92636,
SPELL_MAGMA_ERUPTION     = 80038,

// Stonecore Rift Conjurer (42691) Health: 312, 753 - 387, 450, Mana: 16, 676 - 17, 816
SPELL_DEMON_PORTAL       = 80308,
SPELL_SHADOWBOLT         = 80279,
H_SPELL_SHADOWBOLT       = 92637,

//Stonecore Sentry (42695) Health: 6, 702 - 11, 624

// Stonecore Warbringer (42696) Health: 312, 753 - 387, 450
SPELL_CLEAVE             = 15496,
SPELL_RAGE               = 80158,
};

enum eEvents
{
EVENT_NONE,
EVENT_BLUR,
EVENT_TIGULE,
EVENT_SCHARGE,
EVENT_SPINNING_SLASH,
EVENT_BODY_SLAM,
EVENT_SHOCKWAVE,
EVENT_DUST_STORM,
EVENT_FORCE_OF_EARTH,
EVENT_GROUND_SHOCK,
EVENT_LAVA_BURST,
EVENT_FLAY,
EVENT_IGNITE,
EVENT_MAGMA_ERUPTION,
EVENT_DEMON_PORTAL,
EVENT_CLEAVE,
EVENT_RAGE,
};*/

// Rock Borer AI
class mob_rock_borer : public CreatureScript
{
	enum
	{
		SPELL_ROCK_BORE = 80028,
		SPELL_ROCK_BORE_HC = 92630,
	};
public:
	mob_rock_borer() : CreatureScript("mob_rock_borer") { }

	struct mob_rock_borerAI : public ScriptedAI
	{
		mob_rock_borerAI(Creature* creature) : ScriptedAI(creature) {}

		void Reset()
		{
			rockboreTimer = urand(5000, 7000);
		}

		void UpdateAI(uint32 const diff)
		{
			if (!UpdateVictim())
				return;

			if (rockboreTimer <= diff)
			{
				if (Unit* victim = me->GetVictim())
				{
					if (Aura* aur = victim->GetAura(IsHeroic() ? SPELL_ROCK_BORE_HC : SPELL_ROCK_BORE))
					{
						aur->ModStackAmount(1);
						aur->RefreshDuration();
					}
					else
						DoCastVictim(SPELL_ROCK_BORE);
				}
				rockboreTimer = urand(10000, 12000);
			}
			else rockboreTimer -= diff;

			DoMeleeAttackIfReady();
		}
	private:
		uint32 rockboreTimer;
	};

	CreatureAI* GetAI(Creature* creature) const
	{
		return new mob_rock_borerAI(creature);
	}
};

// Millhouse Manastorm AI
class mob_millhouse_manastorm : public CreatureScript
{
	enum
	{
		SPELL_BLUR = 81216,
		SPELL_FEAR = 81442,
		SPELL_FROSTBOLT_VOLLEY = 81440,
		SPELL_IMPENDING_DOOM = 86830,
		SPELL_SHADOW_BOLT = 81439,
		SPELL_SHADOWFURY = 81441,
		SPELL_TIGULE = 81220,
		SPELL_IMPEMDING_DOOM_E = 86838,

		EVENT_MILL_FEAR = 1,
		EVENT_FROSTBOLT_VOLLEY,
		EVENT_IMPENDING_DOOM,
		EVENT_SHADOW_BOLT,
		EVENT_SHADOWFURY
	};
public:
	mob_millhouse_manastorm() : CreatureScript("mob_millhouse_manastorm") { }

	struct mob_millhouse_manastormAI : public ScriptedAI
	{
		mob_millhouse_manastormAI(Creature* creature) : ScriptedAI(creature) { }

		void Reset()
		{
			events.Reset();
		}

		void EnterCombat(Unit* /*who*/)
		{
			events.ScheduleEvent(EVENT_MILL_FEAR, 10000);
			events.ScheduleEvent(EVENT_FROSTBOLT_VOLLEY, urand(7000, 17000));
			events.ScheduleEvent(EVENT_IMPENDING_DOOM, urand(25000, 35000));
			events.ScheduleEvent(EVENT_SHADOW_BOLT, 2000);
			events.ScheduleEvent(EVENT_SHADOWFURY, urand(10000, 15000));
		}

		void UpdateAI(uint32 const diff)
		{
			if (!UpdateVictim())
				return;

			events.Update(diff);

			if (me->HasUnitState(UNIT_STATE_CASTING))
				return;

			while (uint32 eventId = events.ExecuteEvent())
			{
				switch (eventId)
				{
				case EVENT_MILL_FEAR:
					DoCastRandom(SPELL_FEAR, 0.0f);
					events.ScheduleEvent(EVENT_MILL_FEAR, 10000);
					return;
				case EVENT_SHADOW_BOLT:
					DoCastVictim(SPELL_SHADOW_BOLT);
					events.ScheduleEvent(EVENT_SHADOW_BOLT, 2000);
					return;
				case EVENT_FROSTBOLT_VOLLEY:
					DoCast(SPELL_FROSTBOLT_VOLLEY);
					events.ScheduleEvent(EVENT_FROSTBOLT_VOLLEY, urand(8000, 15000));
					return;
				case EVENT_IMPENDING_DOOM:
					DoCast(SPELL_IMPENDING_DOOM);
					DoCast(me, SPELL_IMPEMDING_DOOM_E, true);
					events.ScheduleEvent(EVENT_IMPENDING_DOOM, urand(60000, 65000));
					return;
				case EVENT_SHADOWFURY:
					DoCastRandom(SPELL_SHADOWFURY, 0.0f);
					events.ScheduleEvent(EVENT_SHADOWFURY, urand(8000, 20000));
					return;
				}
			}

			DoMeleeAttackIfReady();
		}
	private:
		EventMap events;
	};

	CreatureAI* GetAI(Creature* creature) const
	{
		return new mob_millhouse_manastormAI(creature);
	}
};

enum Teleporter
{
	SPELL_TELEPORT_VISUAL = 87459,
};

class stonecore_teleport : public CreatureScript
{
public:
	stonecore_teleport() : CreatureScript("stonecore_teleport") { }

	bool OnGossipHello(Player* player, Creature* me)
	{
		player->ADD_GOSSIP_ITEM(GOSSIP_ICON_CHAT, "Return to Entrance.", GOSSIP_SENDER_MAIN, 0);
		if (InstanceScript* instance = me->GetInstanceScript())
		{
			if (instance->GetBossState(DATA_CORBORUS) == DONE)
				player->ADD_GOSSIP_ITEM(GOSSIP_ICON_CHAT, "Teleport To Corborus", GOSSIP_SENDER_MAIN, 1);
			if (instance->GetBossState(DATA_SLABHIDE) == DONE)
				player->ADD_GOSSIP_ITEM(GOSSIP_ICON_CHAT, "Teleport To Slabhide", GOSSIP_SENDER_MAIN, 2);
			if (instance->GetBossState(DATA_OZRUK) == DONE)
				player->ADD_GOSSIP_ITEM(GOSSIP_ICON_CHAT, "Teleport To Ozruk", GOSSIP_SENDER_MAIN, 3);
		}
		player->ADD_GOSSIP_ITEM(GOSSIP_ICON_CHAT, "Nevermind.", GOSSIP_SENDER_MAIN, 5);

		player->PlayerTalkClass->SendGossipMenu(9425, me->GetGUID());
		return true;
	}

	bool OnGossipSelect(Player * player, Creature * Creature, uint32 sender, uint32 uiAction)
	{
		player->PlayerTalkClass->ClearMenus();
		switch (uiAction)
		{
		case 0:
			player->CastSpell(player, SPELL_TELEPORT_VISUAL);
			player->TeleportTo(725, 853.70, 999.90, 317.33, 0.29); // Retorn To Base
			break;
		case 1:
			player->CastSpell(player, SPELL_TELEPORT_VISUAL);
			player->TeleportTo(725, 1152.48, 897.27, 285.03, 1.30); // Corborus
			break;
		case 2:
			player->CastSpell(player, SPELL_TELEPORT_VISUAL);
			player->TeleportTo(725, 1286.24, 1217.99, 246.95, 6.19); // Slabhide
			break;
		case 3:
			player->CastSpell(player, SPELL_TELEPORT_VISUAL);
			player->TeleportTo(725, 1467.99, 1060.72, 216.38, 3.48); // Ozruk
			break;

		case 5:
		{
				  player->PlayerTalkClass->SendCloseGossip();
		}
			break;
		}
		return true;
	}
};

void AddSC_the_stonecore()
{
	new mob_rock_borer();
	new mob_millhouse_manastorm();
	new stonecore_teleport();
}
