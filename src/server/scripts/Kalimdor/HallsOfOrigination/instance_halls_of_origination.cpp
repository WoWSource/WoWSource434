 /*
* Copyright (C) 2010-2011 Trinity <http://www.projecttrinity.org/>
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

//#include"ScriptPCH.h"
#include "halls_of_origination.h"
#include "InstanceScript.h"
#include "ScriptMgr.h"
#include "ScriptedCreature.h"

#define ENCOUNTERS 7

/* Boss Encounters
   Temple Guardian Anhuur
   Earthrager Ptah
   Anraphet
   Isiset
   Ammunae
   Setesh
   Rajh
*/

static const DoorData doorData[] =
{
    {GO_ULDUM_DOOR_14,                      DATA_TEMPLE_GUARDIAN_ANHUUR_EVENT,      DOOR_TYPE_ROOM,     BOUNDARY_E      },
    {GO_ANHUURS_DOOR,                       DATA_TEMPLE_GUARDIAN_ANHUUR_EVENT,      DOOR_TYPE_PASSAGE,  BOUNDARY_W      },
    {GO_ANHUURS_BRIDGE,                     DATA_TEMPLE_GUARDIAN_ANHUUR_EVENT,      DOOR_TYPE_PASSAGE,  BOUNDARY_NONE   },
    {0,                                     0,                                      DOOR_TYPE_ROOM,     BOUNDARY_NONE} // END
};

class instance_halls_of_origination : public InstanceMapScript
{
    public:
        instance_halls_of_origination() : InstanceMapScript("instance_halls_of_origination", 644) { }

        struct instance_halls_of_origination_InstanceMapScript : public InstanceScript
        {
            instance_halls_of_origination_InstanceMapScript(InstanceMap* map) : InstanceScript(map)
            {
                SetBossNumber(ENCOUNTERS);
                LoadDoorData(doorData);
                slainElementals = 0;
                anraphetDoorGUID = 0;
                anraphetGUID = 0;
                brannGUID = 0;
            }

            void OnCreatureCreate(Creature* creature)
            {
                switch (creature->GetEntry())
                {
                case BOSS_ANRAPHET:
                    creature->setActive(true);
                    anraphetGUID = creature->GetGUID();
                    if(slainElementals < 4)
                        creature->SetFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_NON_ATTACKABLE);
                    break;
                case NPC_BRANN_ANRAPHET:
                    creature->setActive(true);
                    brannGUID = creature->GetGUID();
                    break;
                    default:
                        break;
                }
            }

            void OnGameObjectCreate(GameObject* go)
            {
                switch (go->GetEntry())
                {
                case GO_HOO_TELEPORTER:
                    teleporterSet.insert(go->GetGUID());
                    if (GetBossState(DATA_TEMPLE_GUARDIAN_ANHUUR) == DONE)
                        go->RemoveFlag(GAMEOBJECT_FLAGS, GO_FLAG_NOT_SELECTABLE);
                    break;
                case GO_ULDUM_DOOR_14:
                case GO_ANHUURS_DOOR:
                case GO_ANHUURS_BRIDGE:
                    AddDoor(go, true);
                    break;
                case GO_VAULT_OF_LIGHTS_BOSS_DOOR:
                    anraphetDoorGUID = go->GetGUID();
                    if(slainElementals >= 4)
                        HandleGameObject(0, true, go);
                case GO_LIGHTMACHINE_1:
                case GO_LIGHTMACHINE_2:
                case GO_LIGHTMACHINE_3:
                case GO_LIGHTMACHINE_4:
                    go->setActive(true);
                    break;
                    default:
                        break;
                }
            }

            void SetData(uint32 type, uint32 /*data*/)
            {
                switch(type)
                {
                case DATA_WATER_WARDEN:
                case DATA_EARTH_WARDEN:
                case DATA_FLAME_WARDEN:
                case DATA_AIR_WARDEN:
                    ++slainElementals;
                    if(Creature * brann = instance->GetCreature(brannGUID))
                        brann->AI()->SetData(DATA_BRANN_ELEMENTALS, slainElementals);
                    if(slainElementals >= 4)
                    {
                        DoUseDoorOrButton(anraphetDoorGUID);
                        if(Creature * anraphet = instance->GetCreature(anraphetGUID))
                        {
                            anraphet->AI()->SetData(DATA_ANRAPHET_INTRO, 1);
                            anraphet->RemoveFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_NON_ATTACKABLE);
                        }
                    }
                    SaveToDB();
                    break;
                default:
                    break;
                }
            }

            bool SetBossState(uint32 type, EncounterState state)
            {
                if (!InstanceScript::SetBossState(type, state))
                    return false;

                if (type == DATA_TEMPLE_GUARDIAN_ANHUUR && state == DONE)
                {
                    for (std::set<uint64>::iterator itr = teleporterSet.begin(); itr != teleporterSet.end(); ++itr)
                        if (GameObject* teleporter = instance->GetGameObject((*itr)))
                            teleporter->RemoveFlag(GAMEOBJECT_FLAGS, GO_FLAG_NOT_SELECTABLE);
                }
                 return true;
            }

            std::string GetSaveData()
            {
                OUT_SAVE_INST_DATA;

                std::ostringstream saveStream;
                saveStream << "H O " << GetBossSaveData() << slainElementals;

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

                if (dataHead1 == 'H' && dataHead2 == 'O')
                {
                    for (uint8 i = 0; i < ENCOUNTERS; ++i)
                    {
                        uint32 tmpState;
                        loadStream >> tmpState;
                        if (tmpState == IN_PROGRESS || tmpState > SPECIAL)
                            tmpState = NOT_STARTED;
                        SetBossState(i, EncounterState(tmpState));
                    }

                    loadStream >> slainElementals;
                } else OUT_LOAD_INST_DATA_FAIL;

                OUT_LOAD_INST_DATA_COMPLETE;
            }

        private:
            uint32 slainElementals;
            uint64 anraphetDoorGUID;
            uint64 brannGUID;
            uint64 anraphetGUID;
            std::set<uint64> teleporterSet; 
        };

        InstanceScript* GetInstanceScript(InstanceMap* map) const
        {
            return new instance_halls_of_origination_InstanceMapScript(map);
        }
};

void AddSC_instance_halls_of_origination()
{
    new instance_halls_of_origination();
}
