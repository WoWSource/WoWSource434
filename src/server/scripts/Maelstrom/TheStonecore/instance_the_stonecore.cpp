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
#include "InstanceScript.h"
#include "ScriptMgr.h"

#define ENCOUNTERS     4

class instance_the_stonecore : public InstanceMapScript
{
public:
	instance_the_stonecore() : InstanceMapScript("instance_the_stonecore", 725) { }

	struct instance_the_stonecore_InstanceMapScript : public InstanceScript
	{
		instance_the_stonecore_InstanceMapScript(InstanceMap* map) : InstanceScript(map)
		{
			SetBossNumber(ENCOUNTERS);
			CorborusGUID = 0;
		}

		bool SetBossState(uint32 type, EncounterState state)
		{
			if (!InstanceScript::SetBossState(type, state))
				return false;

			return true;
		}

		uint64 GetData64(uint32 type) const
		{
			switch (type)
			{
			case DATA_CORBORUS:
				return CorborusGUID;
			case DATA_ROCKDOOR:
				return RockdoorGUID;
			default:
				break;
			}

			return 0;
		}

		void OnCreatureCreate(Creature* creature)
		{

			switch (creature->GetEntry())
			{
			case BOSS_CORBORUS:
				CorborusGUID = creature->GetGUID();
			default:
				break;
			}
		}

		void OnGameObjectCreate(GameObject* gameobject)
		{
			switch (gameobject->GetEntry())
			{
			case GO_ROCKDOOR_BREAK:
				RockdoorGUID = gameobject->GetGUID();
				break;
			default:
				break;
			}
		}
	protected:
		EventMap Events;
		uint64 CorborusGUID;
		uint64 RockdoorGUID;

		std::string GetSaveData()
		{
			OUT_SAVE_INST_DATA;

			std::ostringstream saveStream;
			saveStream << "S C " << GetBossSaveData();

			OUT_SAVE_INST_DATA_COMPLETE;
			return saveStream.str();
		}

		void Load(char const* str)
		{
			if (!str)
			{
				OUT_LOAD_INST_DATA_FAIL;
				return;
			}

			OUT_LOAD_INST_DATA(str);

			char dataHead1, dataHead2;

			std::istringstream loadStream(str);
			loadStream >> dataHead1 >> dataHead2;

			if (dataHead1 == 'S' && dataHead2 == 'C')
			{
				for (uint8 i = 0; i < ENCOUNTERS; ++i)
				{
					uint32 tmpState;
					loadStream >> tmpState;
					if (tmpState == IN_PROGRESS || tmpState > SPECIAL)
						tmpState = NOT_STARTED;
					SetBossState(i, EncounterState(tmpState));
				}
			}
			else OUT_LOAD_INST_DATA_FAIL;

			OUT_LOAD_INST_DATA_COMPLETE;
		}
	};

	InstanceScript* GetInstanceScript(InstanceMap* map) const
	{
		return new instance_the_stonecore_InstanceMapScript(map);
	}
};

void AddSC_instance_the_stonecore()
{
	new instance_the_stonecore();
}
