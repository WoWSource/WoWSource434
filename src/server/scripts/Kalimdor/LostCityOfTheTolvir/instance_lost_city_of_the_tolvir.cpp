/*
 * Copyright (C) 2011- 2013 ArkCORE <http://www.arkania.net/>
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

#include "ScriptPCH.h"
#include "the_lost_city_of_tol_vir.h"

#define ENCOUNTERS 5

/* Boss Encounters
   General Husam
   High Prophet Barim
   Lockmaw
   Augh
   Siamat
 */
const Position aughPos = { -11071.3f, -1663.87f, 0.74509f, 0.662058f };

class instance_lost_city_of_the_tolvir : public InstanceMapScript
{
public:
    instance_lost_city_of_the_tolvir() : InstanceMapScript("instance_lost_city_of_the_tolvir", 755) { }

    struct instance_lost_city_of_the_tolvir_InstanceMapScript: public InstanceScript
    {
        instance_lost_city_of_the_tolvir_InstanceMapScript(InstanceMap* map) : InstanceScript(map) {}

        uint32 uiEncounter[ENCOUNTERS];

        uint64 GeneralHusamGUID;
        uint64 HighProphetBarimGUID;
        uint64 LockmawGUID;
        uint64 AughGUID;
        uint64 SiamatGUID;
        uint64 SiamatPlatform;

        void Initialize()
        {
            GeneralHusamGUID     = 0;
            HighProphetBarimGUID = 0;
            LockmawGUID          = 0;
            AughGUID             = 0;
            SiamatGUID           = 0;
            SiamatPlatform       = 0;
            introDone            = false;

            for(uint8 i=0 ; i<ENCOUNTERS; ++i)
                uiEncounter[i] = NOT_STARTED;
        }

        bool IsEncounterInProgress() const
        {
            for(uint8 i=0; i<ENCOUNTERS; ++i)
            {
                if (uiEncounter[i] == IN_PROGRESS)
                    return true;
            }
            return false;
        }

        void OnCreatureCreate(Creature* creature)
        {
            switch(creature->GetEntry())
            {
                 case BOSS_GENERAL_HUSAM:
                     GeneralHusamGUID = creature->GetGUID();
                     break;
                 case BOSS_HIGH_PROPHET_BARIM:
                     HighProphetBarimGUID = creature->GetGUID();
                     break;
                 case BOSS_LOCKMAW:
                     LockmawGUID = creature->GetGUID();
                     break;
                 case BOSS_AUGH:
                     AughGUID = creature->GetGUID();
                     break;
                 case BOSS_SIAMAT:
                     SiamatGUID = creature->GetGUID();
                     break;
            }
        }

        void OnGameObjectCreate(GameObject* gameobject)
        {
            switch (gameobject->GetEntry())
            {
                case GO_SIAMAT_PLATFORM:
                    if (GetData(DATA_GENERAL_HUSAM_EVENT) == DONE && GetData(DATA_HIGH_PROPHET_BARIM_EVENT) == DONE && GetData(DATA_AUGH_EVENT) == DONE && GetData(DATA_LOCKMAW_EVENT) == DONE)
                        gameobject->SetDestructibleState(GO_DESTRUCTIBLE_DESTROYED);
                    SiamatPlatform = gameobject->GetGUID();
                    break;
                default:
                    break;
            }
        }

        uint64 GetData64(uint32 identifier) const
        {
            switch(identifier)
            {
                case DATA_GENERAL_HUSAM:
                    return GeneralHusamGUID;
                case DATA_HIGH_PROPHET_BARIM:
                    return HighProphetBarimGUID;
                case DATA_LOCKMAW:
                    return LockmawGUID;
                case DATA_AUGH:
                    return AughGUID;
                case DATA_SIAMAT:
                    return SiamatGUID;
            }
            return 0;
        }

        void SetData(uint32 type, uint32 data)
        {
            switch(type)
            {
                case DATA_GENERAL_HUSAM_EVENT:
                    uiEncounter[0] = data;
                    break;
                case DATA_HIGH_PROPHET_BARIM_EVENT:
                    uiEncounter[1] = data;
                    break;
                case DATA_LOCKMAW_EVENT:
                    uiEncounter[2] = data;
                    break;
                case DATA_AUGH_EVENT:
                    uiEncounter[3] = data;
                    break;
                case DATA_SIAMAT_EVENT:
                    uiEncounter[4] = data;
                    break;
            }

            if (!introDone && GetData(DATA_GENERAL_HUSAM_EVENT) == DONE && GetData(DATA_HIGH_PROPHET_BARIM_EVENT) == DONE && GetData(DATA_AUGH_EVENT) == DONE && GetData(DATA_LOCKMAW_EVENT) == DONE)
            {
                introDone = true;
                if (GameObject* platform = instance->GetGameObject(SiamatPlatform))
                    platform->SetDestructibleState(GO_DESTRUCTIBLE_DESTROYED);

                if (Creature* siamat = instance->GetCreature(SiamatGUID))
                    siamat->AI()->DoAction(ACTION_INTRO);
            }

            if (GetData(DATA_LOCKMAW_EVENT) == DONE && GetData(DATA_AUGH) != DONE)
                if (instance->GetCreature(AughGUID) == NULL)
                    if (Creature *augh = instance->SummonCreature(BOSS_AUGH, aughPos))
                        augh->AI()->DoAction(ACTION_START_EVENT);

           if (data == DONE)
               SaveToDB();
        }

        uint32 GetData(uint32 type) const
        {
            switch(type)
            {
                case DATA_GENERAL_HUSAM_EVENT:
                    return uiEncounter[0];
                case DATA_HIGH_PROPHET_BARIM_EVENT:
                    return uiEncounter[1];
                case DATA_LOCKMAW_EVENT:
                    return uiEncounter[2];
                case DATA_AUGH_EVENT:
                    return uiEncounter[3];
                case DATA_SIAMAT_EVENT:
                    return uiEncounter[4];
            }
            return 0;
        }

        std::string GetSaveData()
        {
            OUT_SAVE_INST_DATA;

            std::string str_data;
            std::ostringstream saveStream;
            saveStream << "L V" << uiEncounter[0] << " " << uiEncounter[1]  << " " << uiEncounter[2]  << " " << uiEncounter[3] << " " << uiEncounter[4];
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
            uint16 data0, data1, data2, data3, data4;

            std::istringstream loadStream(in);
            loadStream >> dataHead1 >> dataHead2 >> data0 >> data1 >> data2 >> data3 >> data4;

            if (dataHead1 == 'L' && dataHead2 == 'V')
            {
                uiEncounter[0] = data0;
                uiEncounter[1] = data1;
                uiEncounter[2] = data2;
                uiEncounter[3] = data3;
                uiEncounter[4] = data4;

                for(uint8 i=0; i<ENCOUNTERS; ++i)
                    if (uiEncounter[i] == IN_PROGRESS)
                        uiEncounter[i] = NOT_STARTED;
            }
            else OUT_LOAD_INST_DATA_FAIL;

            OUT_LOAD_INST_DATA_COMPLETE;
        }
        private:
            bool introDone;
    };

    InstanceScript* GetInstanceScript(InstanceMap* map) const
    {
        return new instance_lost_city_of_the_tolvir_InstanceMapScript(map);
    }
};

void AddSC_instance_lost_city_of_the_tolvir()
{
    new instance_lost_city_of_the_tolvir();
}
