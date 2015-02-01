/*
 * Copyright (C) 2008-2013 TrinityCore <http://www.trinitycore.org/>
 * Copyright (C) 2006-2009 ScriptDev2 <https://scriptdev2.svn.sourceforge.net/>
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

#include "ScriptMgr.h"
#include "InstanceScript.h"
#include "UnitAI.h"
#include "zulgurub.h"

DoorData const doorData[] =
{
    { GO_VENOXIS_COIL,                  DATA_VENOXIS,   DOOR_TYPE_ROOM, BOUNDARY_NONE },
    { GO_ARENA_DOOR_1,                  DATA_MANDOKIR,  DOOR_TYPE_ROOM, BOUNDARY_NONE },
    { GO_FORCEFIELD,                    DATA_KILNARA,   DOOR_TYPE_ROOM, BOUNDARY_NONE },
    { GO_ZANZIL_DOOR,                   DATA_ZANZIL,    DOOR_TYPE_ROOM, BOUNDARY_NONE },
    //{ GO_THE_CACHE_OF_MADNESS_DOOR,     DATA_xxxxxxx,   DOOR_TYPE_ROOM, BOUNDARY_NONE },
    { 0,                                0,              DOOR_TYPE_ROOM, BOUNDARY_NONE }
};

const Position TikiTorchSP[6]=
{
    {-11933.2f, -1824.54f, 51.7838f, 1.53589f},
    {-11919.8f, -1824.58f, 51.4590f, 1.53589f},
    {-11903.5f, -1824.38f, 51.5542f, 1.51844f},
    {-11879.6f, -1824.81f, 50.8839f, 1.46608f},
    {-11864.6f, -1824.44f, 51.1218f, 1.50098f},
    {-11849.5f, -1824.58f, 51.4813f, 1.55334f},
};

class instance_zulgurub : public InstanceMapScript
{
    public:
        instance_zulgurub() : InstanceMapScript(ZGScriptName, 859) { }

        struct instance_zulgurub_InstanceMapScript : public InstanceScript
        {
            instance_zulgurub_InstanceMapScript(Map* map) : InstanceScript(map)
            {
                SetBossNumber(EncounterCount);
                LoadDoorData(doorData);
            }

             uint64 venoxisGUID;
             uint64 mandokirGUID;
             uint64 kilnaraGUID;
             uint64 zanzilGUID;
             uint64 jindoGUID;
             uint64 hazzarahGUID;
             uint64 renatakiGUID;
             uint64 wushoolayGUID;
             uint64 grilekGUID;
             uint64 jindoTiggerGUID;
             uint8 tikiMaskId;			

            void Initialize()
            {
                venoxisGUID         = 0;
                mandokirGUID        = 0;
                kilnaraGUID         = 0;
                zanzilGUID          = 0;
                hazzarahGUID        = 0;
                renatakiGUID        = 0;
                wushoolayGUID       = 0;
                grilekGUID          = 0;
                jindoTiggerGUID     = 0;			   
                jindoGUID           = 0;
                tikiMaskId          = 0;

                for (int i = 0; i < 6; ++i)
                    if (Creature* torch = instance->SummonCreature(52419, TikiTorchSP[i]))
                        torch->GetAI()->SetData(DATA_POSITION_ID, i);
            }
			
            void OnCreatureCreate(Creature* creature)
            {
                switch (creature->GetEntry())
                {
                    case NPC_VENOXIS:
                        venoxisGUID = creature->GetGUID();
                        break;
                    case NPC_MANDOKIR:
                        mandokirGUID = creature->GetGUID();
                        break;
                    case NPC_KILNARA:
                        kilnaraGUID = creature->GetGUID();
                        break;
                    case NPC_ZANZIL:
                        zanzilGUID = creature->GetGUID();
                        break;
                    case NPC_JINDO:
                        jindoGUID = creature->GetGUID();
                        break;
                    case NPC_HAZZARAH:
                        hazzarahGUID = creature->GetGUID();
                        break;
                    case NPC_RENATAKI:
                        renatakiGUID = creature->GetGUID();
                        break;
                    case NPC_WUSHOOLAY:
                        wushoolayGUID = creature->GetGUID();
                        break;
                    case NPC_GRILEK:
                        grilekGUID = creature->GetGUID();
                        break;
                    case NPC_JINDO_TRIGGER:
                        jindoTiggerGUID = creature->GetGUID();
                        break;
                    default:
                        break;
                }
            }

            void OnGameObjectCreate(GameObject* go)
            {
                switch (go->GetEntry())
                {
                    case GO_VENOXIS_COIL:
                    case GO_ARENA_DOOR_1:
                    case GO_FORCEFIELD:
                    case GO_ZANZIL_DOOR:
                    case GO_THE_CACHE_OF_MADNESS_DOOR:
                        AddDoor(go, true);
                        break;
                    default:
                        break;
                }
            }

            void OnGameObjectRemove(GameObject* go)
            {
                switch (go->GetEntry())
                {
                    case GO_VENOXIS_COIL:
                    case GO_ARENA_DOOR_1:
                    case GO_FORCEFIELD:
                    case GO_ZANZIL_DOOR:
                    case GO_THE_CACHE_OF_MADNESS_DOOR:
                        AddDoor(go, false);
                        break;
                    default:
                        break;
                }
            }

            bool SetBossState(uint32 type, EncounterState state)
            {
                if (!InstanceScript::SetBossState(type, state))
                    return false;

                switch (type)
                {
                    case DATA_VENOXIS:
                    case DATA_MANDOKIR:
                    case DATA_KILNARA:
                    case DATA_ZANZIL:
                    case DATA_JINDO:
                    case DATA_HAZZARAH:
                    case DATA_RENATAKI:
                    case DATA_WUSHOOLAY:
                    case DATA_GRILEK:
                        break;
                    default:
                        break;
                }

                return true;
            }

            void SetData(uint32 type, uint32 data)
            {
                switch (type)
                {
                    case DATA_TIKI_MASK_ID:
                        {
                            switch (data)
                            {
                                case IN_PROGRESS:
                                    ++tikiMaskId;
                                    break;
                                case NOT_STARTED:
                                    tikiMaskId = 0;
                                    break;
                            }
                        }
                        break;
                }
            }

            uint32 GetData(uint32 type) const
            {
                switch (type)
                {
                    case DATA_TIKI_MASK_ID:              return tikiMaskId;
                }

                return 0;
            }

            uint64 GetData64(uint32 type) const
            {
                switch (type)
                {
                    case DATA_VENOXIS:
                        return venoxisGUID;
                    case DATA_MANDOKIR:
                        return mandokirGUID;
                    case DATA_KILNARA:
                        return kilnaraGUID;
                    case DATA_ZANZIL:
                        return zanzilGUID;
                    case DATA_JINDO:
                        return jindoGUID;
                    case DATA_HAZZARAH:
                        return hazzarahGUID;
                    case DATA_RENATAKI:
                        return renatakiGUID;
                    case DATA_WUSHOOLAY:
                        return wushoolayGUID;
                    case DATA_GRILEK:
                        return grilekGUID;
                    case DATA_JINDOR_TRIGGER:
                        return jindoTiggerGUID;
                    default:
                        break;
                }

                return 0;
            }

            std::string GetSaveData()
            {
                OUT_SAVE_INST_DATA;

                std::ostringstream saveStream;
                saveStream << "Z G " << GetBossSaveData();

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

                if (dataHead1 == 'Z' && dataHead2 == 'G')
                {
                    for (uint8 i = 0; i < EncounterCount; ++i)
                    {
                        uint32 tmpState;
                        loadStream >> tmpState;
                        if (tmpState == IN_PROGRESS || tmpState > SPECIAL)
                            tmpState = NOT_STARTED;

                        SetBossState(i, EncounterState(tmpState));
                    }
                }
                else
                    OUT_LOAD_INST_DATA_FAIL;

                OUT_LOAD_INST_DATA_COMPLETE;
            }

        };

        InstanceScript* GetInstanceScript(InstanceMap* map) const
        {
            return new instance_zulgurub_InstanceMapScript(map);
        }
};

void AddSC_instance_zulgurub()
{
    new instance_zulgurub();
}
