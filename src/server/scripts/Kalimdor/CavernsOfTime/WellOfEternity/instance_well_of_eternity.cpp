// 108469
#include "ScriptPCH.h"
#include "well_of_eternity.h"

#define MAX_ENCOUNTER 3

static const DoorData doordata[] =
{
    {GO_INVISIBLE_FIREWALL_DOOR,    DATA_PEROTHARN, DOOR_TYPE_PASSAGE,  BOUNDARY_NONE},
    {0,                             0,              DOOR_TYPE_ROOM,     BOUNDARY_NONE},
};

class instance_well_of_eternity : public InstanceMapScript
{
public:
    instance_well_of_eternity() : InstanceMapScript("instance_well_of_eternity", 939) { }

    InstanceScript* GetInstanceScript(InstanceMap* map) const
    {
        return new instance_well_of_eternity_InstanceMapScript(map);
    }

    struct instance_well_of_eternity_InstanceMapScript : public InstanceScript
    {
        instance_well_of_eternity_InstanceMapScript(Map* map) : InstanceScript(map)
        {
            SetBossNumber(MAX_ENCOUNTER);
            LoadDoorData(doordata);

            uiEventNozdormu = 0;
            uiEventDemon = 0;
            uiEventIllidan1 = 0;

            uiPerotharnGUID = 0;
            uiIllidan2GUID = 0;
            uiVarothenGUID = 0;
            uiMannorothGUID = 0;

            uiRoyalCacheGUID = 0;
            uiMinorCacheGUID = 0;
            uiCourtyardDoor1GUID = 0;
            uiLargeFirewallDoorGUID = 0;
            uiPerotharnDoors.clear();
            uiAfterPerotharnDoors.clear();
        }

        void OnCreatureCreate(Creature* pCreature)
        {
            switch (pCreature->GetEntry())
            {
                case NPC_PEROTHARN:
                    uiPerotharnGUID = pCreature->GetGUID();
                    break;
                case NPC_ILLIDAN_2:
                    uiIllidan2GUID = pCreature->GetGUID();
                    break;
                case NPC_VAROTHEN:
                    uiVarothenGUID = pCreature->GetGUID();
                    break;
                case NPC_MANNOROTH:
                    uiMannorothGUID = pCreature->GetGUID();
                    break;
                default:
                    break;
            }
        }

        void OnGameObjectCreate(GameObject* pGo)
        {
            switch (pGo->GetEntry())
            {
                case GO_ROYAL_CACHE:
                    uiRoyalCacheGUID = pGo->GetGUID();
                    break;
                case GO_MINOR_CACHE:
                    uiMinorCacheGUID = pGo->GetGUID();
                    break;
                case GO_INVISIBLE_FIREWALL_DOOR:
                    AddDoor(pGo, true);
                    break;
                case GO_COURTYARD_DOOR_1:
                    uiCourtyardDoor1GUID = pGo->GetGUID();
                    if (uiEventDemon == DONE)
                        HandleGameObject(uiCourtyardDoor1GUID, true, pGo);
                    break;
                case GO_LARGE_FIREWALL_DOOR:
                    if (pGo->GetPositionX() <= 3200.0f)
                    {
                        uiLargeFirewallDoorGUID = pGo->GetGUID();
                        if (uiEventDemon == DONE)
                            HandleGameObject(uiLargeFirewallDoorGUID, true, pGo);
                    } else
                    {
                        uiPerotharnDoors.push_back(pGo->GetGUID());
                        if (GetBossState(DATA_PEROTHARN) == IN_PROGRESS)
                            HandleGameObject(pGo->GetGUID(), false, pGo);
                        else
                            HandleGameObject(pGo->GetGUID(), true, pGo);
                    }
                    break;
                case GO_SMALL_FIREWALL_DOOR:
                    if (pGo->GetPositionX() <= 3340.0f)
                    {
                        uiPerotharnDoors.push_back(pGo->GetGUID());
                        if (GetBossState(DATA_PEROTHARN) == IN_PROGRESS)
                            HandleGameObject(pGo->GetGUID(), false, pGo);
                        else
                            HandleGameObject(pGo->GetGUID(), true, pGo);
                    } else
                    {
                        uiAfterPerotharnDoors.push_back(pGo->GetGUID());
                        if (GetBossState(DATA_PEROTHARN) == DONE)
                            HandleGameObject(pGo->GetGUID(), true, pGo);
                        else
                            HandleGameObject(pGo->GetGUID(), false, pGo);
                    }
                    break;
            }
        }

        void SetData(uint32 type, uint32 data)
        {
            switch (type)
            {
                case DATA_EVENT_NOZDORMU:
                    uiEventNozdormu = data;
                    if (data == DONE)
                        SaveToDB();
                    break;
                case DATA_EVENT_DEMON:
                    uiEventDemon = data;
                    if (data == DONE)
                    {
                        HandleGameObject(uiCourtyardDoor1GUID, true);
                        HandleGameObject(uiLargeFirewallDoorGUID, true);
                        SaveToDB();
                    }
                    break;
                case DATA_EVENT_ILLIDAN_1:
                    uiEventIllidan1 = data;
                    if (data == DONE)
                        SaveToDB();
                    break;
                default:
                    break;
            }
        }

        uint32 GetData(uint32 type) const
        {
            switch (type)
            {
                case DATA_EVENT_NOZDORMU:
                    return uiEventNozdormu;
                case DATA_EVENT_DEMON:
                    return uiEventDemon;
                case DATA_EVENT_ILLIDAN_1:
                    return uiEventIllidan1;
                default:
                    return 0;
            }
            return 0;
        }

        uint64 GetData64(uint32 type) const
        {
            switch (type)
            {
                case DATA_PEROTHARN:
                    return uiPerotharnGUID;
                case DATA_ROYAL_CACHE:
                    return uiRoyalCacheGUID;
                case DATA_MINOR_CACHE:
                    return uiMinorCacheGUID;
                case DATA_EVENT_ILLIDAN_2:
                    return uiIllidan2GUID;
                case DATA_VAROTHEN:
                    return uiVarothenGUID;
                case DATA_MANNOROTH:
                    return uiMannorothGUID;
                default:
                    return 0;
            }

            return 0;
        }

        bool SetBossState(uint32 type, EncounterState state)
        {
            if (!InstanceScript::SetBossState(type, state))
                return false;

            if (type == DATA_PEROTHARN)
            {
                if (state == IN_PROGRESS)
                {
                    if (!uiPerotharnDoors.empty())
                    for (std::vector<uint64>::const_iterator itr = uiPerotharnDoors.begin(); itr != uiPerotharnDoors.end(); ++itr)
                        HandleGameObject((*itr), false);
                } else
                {
                    if (!uiPerotharnDoors.empty())
                    for (std::vector<uint64>::const_iterator itr = uiPerotharnDoors.begin(); itr != uiPerotharnDoors.end(); ++itr)
                        HandleGameObject((*itr), true);
                }
                if (state == DONE)
                {
                    if (!uiAfterPerotharnDoors.empty())
                    for (std::vector<uint64>::const_iterator itr = uiAfterPerotharnDoors.begin(); itr != uiAfterPerotharnDoors.end(); ++itr)
                        HandleGameObject((*itr), true);
                } else
                {
                    if (!uiAfterPerotharnDoors.empty())
                    for (std::vector<uint64>::const_iterator itr = uiAfterPerotharnDoors.begin(); itr != uiAfterPerotharnDoors.end(); ++itr)
                        HandleGameObject((*itr), false);
                }
            }

            return true;
        }

        std::string GetSaveData()
        {
            OUT_SAVE_INST_DATA;

            std::string str_data;

            std::ostringstream saveStream;
            saveStream << "W o E " << GetBossSaveData()
                << uiEventNozdormu << " " << uiEventDemon << " " << uiEventIllidan1 << " ";

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

            char dataHead1, dataHead2, dataHead3;

            std::istringstream loadStream(in);
            loadStream >> dataHead1 >> dataHead2 >> dataHead3;

            if (dataHead1 == 'W' && dataHead2 == 'o' && dataHead3 == 'E')
            {
                for (uint8 i = 0; i < MAX_ENCOUNTER; ++i)
                {
                    uint32 tmpState;
                    loadStream >> tmpState;
                    if (tmpState == IN_PROGRESS || tmpState > SPECIAL)
                        tmpState = NOT_STARTED;
                    SetBossState(i, EncounterState(tmpState));
                }

                uint32 tmpEvent1;
                loadStream >> tmpEvent1;
                uiEventNozdormu = ((tmpEvent1 != DONE) ? NOT_STARTED : DONE);

                uint32 tmpEvent2;
                loadStream >> tmpEvent2;
                uiEventDemon = ((tmpEvent2 != DONE) ? NOT_STARTED : DONE);

                uint32 tmpEvent3;
                loadStream >> tmpEvent3;
                uiEventIllidan1 = ((tmpEvent3 != DONE) ? NOT_STARTED : DONE);


            } else OUT_LOAD_INST_DATA_FAIL;

            OUT_LOAD_INST_DATA_COMPLETE;
        }

    private:
        uint32 uiEventNozdormu;
        uint32 uiEventDemon;
        uint32 uiEventIllidan1;

        uint64 uiPerotharnGUID;
        uint64 uiIllidan2GUID;
        uint64 uiVarothenGUID;
        uint64 uiMannorothGUID;

        uint64 uiRoyalCacheGUID;
        uint64 uiMinorCacheGUID;
        uint64 uiCourtyardDoor1GUID;
        uint64 uiLargeFirewallDoorGUID;
        std::vector<uint64> uiAfterPerotharnDoors;
        std::vector<uint64> uiPerotharnDoors;

    };
};

void AddSC_instance_well_of_eternity()
{
    new instance_well_of_eternity();
}