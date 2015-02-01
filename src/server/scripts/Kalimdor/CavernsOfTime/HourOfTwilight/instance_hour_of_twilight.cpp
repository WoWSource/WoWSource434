#include "ScriptPCH.h"
#include "hour_of_twilight.h"
#include "ScriptedGossip.h"
#include "ScriptMgr.h"
#include "InstanceScript.h"
#include "Spell.h"
#include "GameObjectAI.h"

#define ENCOUNTERS 3

/* Boss Encounters
Arcurion
Asira Dawnslayer
Archbishop Benedictus
*/


enum InstanceTeleporeter
{
	START_TELEPORT = 1,
	Arcurion_TELEPORT = 2,
	Asira_TELEPORT = 3,
	Archbishop_TELEPORT = 4,
};


class instance_hour_of_twilight : public InstanceMapScript
{
public:
	instance_hour_of_twilight() : InstanceMapScript("instance_hour_of_twilight", 940)
	{ }

	InstanceScript* GetInstanceScript(InstanceMap* map) const
	{
		return new instance_hour_of_twilight_InstanceMapScript(map);
	}

	struct instance_hour_of_twilight_InstanceMapScript : public InstanceScript
	{
		instance_hour_of_twilight_InstanceMapScript(InstanceMap* map) : InstanceScript(map) { }

		uint32 uiEncounter[ENCOUNTERS];

		uint64 uiAsira;
		uint64 uiArcurion;
		uint64 uiArchbishop;
		uint64 uiTeamInInstance;

		void Initialize()
		{
			uiAsira = 0;
			uiArcurion = 0;
			uiArchbishop = 0;
			uiTeamInInstance = 0;

			for (uint8 i = 0; i < ENCOUNTERS; ++i)
				uiEncounter[i] = NOT_STARTED;
		}

		bool IsEncounterInProgress() const
		{
			for (uint8 i = 0; i < ENCOUNTERS; ++i)
			{
				if (uiEncounter[i] == IN_PROGRESS)
					return true;
			}
			return false;
		}

		void OnCreatureCreate(Creature* pCreature)
		{
			switch (pCreature->GetEntry())
			{
			case BOSS_ARCURION:
				uiArcurion = pCreature->GetGUID();
				break;
			case BOSS_ASIRA:
				uiAsira = pCreature->GetGUID();
				break;
			case BOSS_ARCHBISHOP:
				uiArchbishop = pCreature->GetGUID();
				break;
			}
		}

		uint64 getData64(uint32 identifier)
		{
			switch (identifier)
			{

			case DATA_ARCURION:
				return uiArcurion;
			case DATA_ASIRA:
				return uiAsira;
			case DATA_ARCHBISHOP:
				return uiArchbishop;
			}
			return 0;
		}

		void SetData(uint32 type, uint32 data)
		{
			switch (type)
			{
			case DATA_ARCURION:
				uiEncounter[0] = data;
				break;
			case DATA_ASIRA:
				uiEncounter[1] = data;
				break;
			case DATA_ARCHBISHOP:
				uiEncounter[2] = data;
				break;
			}

			if (data == DONE)
				SaveToDB();
		}

		uint32 GetData(uint32 type) const
		{
			switch (type)
			{
			case DATA_ARCURION_EVENT:
				return uiEncounter[0];
			case DATA_ASIRA_EVENT:
				return uiEncounter[1];
			case DATA_ARCHBISHOP_EVENT:
				return uiEncounter[2];
			}
			return 0;
		}

		std::string GetSaveData()
		{
			OUT_SAVE_INST_DATA;

			std::string str_data;
			std::ostringstream saveStream;
			saveStream << "H T" << uiEncounter[0] << " " << uiEncounter[1] << " " << uiEncounter[2];
			str_data = saveStream.str();

			OUT_SAVE_INST_DATA_COMPLETE;
			return str_data;
		}

		void Load(const char* in)
		{
			if (!in)
			{
				OUT_LOAD_INST_DATA_FAIL;
				return;
			}

			OUT_LOAD_INST_DATA(in);

			char dataHead1, dataHead2;
			uint16 data0, data1, data2;

			std::istringstream loadStream(in);
			loadStream >> dataHead1 >> dataHead2 >> data0 >> data1 >> data2;

			if (dataHead1 == 'H' && dataHead2 == 'T')
			{
				uiEncounter[0] = data0;
				uiEncounter[1] = data1;
				uiEncounter[2] = data2;

				for (uint8 i = 0; i < ENCOUNTERS; ++i)
					if (uiEncounter[i] == IN_PROGRESS)
						uiEncounter[i] = NOT_STARTED;
			}
			else
				OUT_LOAD_INST_DATA_FAIL;

			OUT_LOAD_INST_DATA_COMPLETE;
		}
	};
};



class Hourtwilight_teleport : public GameObjectScript
{
public:
	Hourtwilight_teleport() : GameObjectScript("Hourtwilight_teleport") { }

	struct Hourtwilight_teleportAI : public GameObjectAI
	{
		Hourtwilight_teleportAI(GameObject* go) : GameObjectAI(go)
		{
		}

		bool GossipHello(Player* player)
		{
			player->ADD_GOSSIP_ITEM(GOSSIP_ICON_CHAT, "Teleport to Start", GOSSIP_SENDER_MAIN, START_TELEPORT);

			if (InstanceScript* instance = go->GetInstanceScript())
			{
				if (instance->GetData(DATA_ARCURION_EVENT) == DONE)
					player->ADD_GOSSIP_ITEM(GOSSIP_ICON_CHAT, "Teleport to Asira Dawnslayer", GOSSIP_SENDER_MAIN, Asira_TELEPORT);
				if (instance->GetData(DATA_ASIRA_EVENT) == DONE)
					player->ADD_GOSSIP_ITEM(GOSSIP_ICON_CHAT, "Teleport to Archbishop Benedictus", GOSSIP_SENDER_MAIN, Archbishop_TELEPORT);
			}
			player->SEND_GOSSIP_MENU(player->GetGossipTextId(go), go->GetGUID());
			return true;
		}

		void UpdateAI(uint32 diff) { }
	};

	bool OnGossipSelect(Player* player, GameObject* go, uint32 sender, uint32 action)
	{
		//player->PlayerTalkClass->ClearMenus();
		if (!player->getAttackers().empty())
			return false;

		switch (action)
		{
		case START_TELEPORT:
			player->TeleportTo(940, 4927.211f, 320.534f, 102.37f, 4.79f);
			player->CLOSE_GOSSIP_MENU();
			break;

		case Asira_TELEPORT:
			player->TeleportTo(940, 4416.76f, 453.371f, 38.554f, 3.90f);
			player->CLOSE_GOSSIP_MENU();
			break;

		case Archbishop_TELEPORT:
			player->TeleportTo(940, 3932.045f, 303.857f, 12.63f, 3.16f);
			player->CLOSE_GOSSIP_MENU();
			break;

		}

		return true;
	}

	GameObjectAI* GetAI(GameObject* go) const
	{
		return new Hourtwilight_teleportAI(go);
	}
};


void AddSC_instance_hour_of_twilight()
{
	new instance_hour_of_twilight();
	new Hourtwilight_teleport;
}
