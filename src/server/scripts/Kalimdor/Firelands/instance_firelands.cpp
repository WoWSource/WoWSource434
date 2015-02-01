/*
 * Copyright (C) 2013 WoW Source  <http://wowsource.info/>
 *
 * Copyright (C) 2013 WoWSource [WS] <http://wowsource.info/>
 *
 * Dont Share The SourceCode
 * and read our WoWSource Terms
 *
 */

#include "ObjectMgr.h"
#include "ScriptMgr.h"
#include "InstanceScript.h"
#include "ScriptedCreature.h"
#include "Map.h"
#include "PoolMgr.h"
#include "AccountMgr.h"
#include "firelands.h"
#include"ScriptPCH.h"

#define ENCOUNTERS 7

class instance_firelands: public InstanceMapScript
{
    public:
        instance_firelands() :
                InstanceMapScript("instance_firelands", 720)
        {
        }

        InstanceScript* GetInstanceScript(InstanceMap* map) const
        {
            return new instance_firelands_InstanceMapScript(map);
        }

        struct instance_firelands_InstanceMapScript: public InstanceScript
        {

                instance_firelands_InstanceMapScript(InstanceMap* map) :
                        InstanceScript(map)
                {
                    memset(&uiEncounter, 0, sizeof(uiEncounter));
                }

                uint32 uiEncounter[ENCOUNTERS];

                // Creatures
                uint64 uiShannox;
                uint64 uiRhyolith;
                uint64 uiBethtilac;
                uint64 uiAlysrazor;
                uint64 uiBaloroc;
                uint64 uiMajordomus;
                uint64 uiRagnarosCata;
                uint64 uiTeamInInstance;
                uint64 uiRageface;
                uint64 uiRiplimb;
                uint64 uiShannoxSpear;

                // Gobs
                uint64 BethtilacDoorGUID;
                uint64 BalorocDoorGUID;
                uint64 AlysrazorVolcanoGUID;
                uint64 SulfuronWallGUID;
                uint64 RagnarosPlatformGUID;

                void Initialize()
                {
                    for (uint8 i = 0; i < ENCOUNTERS; ++i)
                        uiEncounter[i] = NOT_STARTED;

                    uiShannox = 0;
                    uiRhyolith = 0;
                    uiBethtilac = 0;
                    uiAlysrazor = 0;
                    uiBaloroc = 0;
                    uiMajordomus = 0;
                    uiRagnarosCata = 0;
                    BethtilacDoorGUID = 0;
                    BalorocDoorGUID = 0;
                    AlysrazorVolcanoGUID = 0;
                    SulfuronWallGUID = 0;
                    RagnarosPlatformGUID = 0;
                    uiRageface = 0;
                    uiRiplimb = 0;
                    uiShannoxSpear = 0;
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

                void OnGameObjectCreate(GameObject* go)
                {
                    switch (go->GetEntry())
                    {
                        case GOB_DOOR_BETHILAC:
                            BethtilacDoorGUID = go->GetGUID();
                            break;

                        case GOB_DOOR_BALOROC:
                            BalorocDoorGUID = go->GetGUID();
                            break;

                        case GOB_VOLCANO_ALYS:
                            AlysrazorVolcanoGUID = go->GetGUID();
                            break;

                        case GOB_WALL_SULFURON:
                            SulfuronWallGUID = go->GetGUID();
                            break;

                        case GOB_PLATFORM_RAGN:
                            RagnarosPlatformGUID = go->GetGUID();
                            break;
                    }
                }

                void OnCreatureCreate(Creature* creature)
                {
                    switch (creature->GetEntry())
                    {
                        case NPC_SHANNOX:
                            uiShannox = creature->GetGUID();
                            break;
                        case NPC_RHYOLITH:
                            uiRhyolith = creature->GetGUID();
                            break;
                        case NPC_BETHTILAC:
                            uiBethtilac = creature->GetGUID();
                            break;
                        case NPC_ALYSRAZOR:
                            uiAlysrazor = creature->GetGUID();
                            break;
                        case NPC_BALOROC:
                            uiBaloroc = creature->GetGUID();
                            break;
                        case NPC_MAJORDOMUS:
                            uiMajordomus = creature->GetGUID();
                            break;
                        case NPC_RAGNAROS_CATA:
                            uiRagnarosCata = creature->GetGUID();
                            break;
                        case NPC_RAGEFACE:
                            uiRageface = creature->GetGUID();
                            break;
                        case NPC_RIPLIMB:
                            uiRiplimb = creature->GetGUID();
                            break;
                        case NPC_SHANNOX_SPEAR:
                            uiShannoxSpear = creature->GetGUID();
                            break;
                        case NPC_SMOULDERING_HATCHLING:
                            creature->m_Events.AddEvent(new DelayedAttackStartEvent(creature),
                                    creature->m_Events.CalculateTime(500));
                            break;
                    }
                }

                uint64 GetData64(uint32 identifier) const
                {
                    switch (identifier)
                    {
                        case DATA_SHANNOX:
                            return uiShannox;
                        case DATA_LORD_RHYOLITH:
                            return uiRhyolith;
                        case DATA_BETHTILAC:
                            return uiBethtilac;
                        case DATA_ALYSRAZAR:
                            return uiAlysrazor;
                        case DATA_BALOROC:
                            return uiBaloroc;
                        case DATA_MAJORDOMUS:
                            return uiMajordomus;
                        case DATA_RAGNAROS:
                            return uiRagnarosCata;

                            // Npc's
                        case DATA_RAGEFACE:
                            return uiRageface;

                        case DATA_RIPLIMB:
                            return uiRiplimb;

                        case DATA_SHANNOX_SPEAR:
                            return uiShannoxSpear;

                            // Go's
                        case DATA_BETHTILAC_DOOR:
                            return BethtilacDoorGUID;
                            break;

                        case DATA_BALOROC_DOOR:
                            return BalorocDoorGUID;
                            break;

                        case DATA_ALYSRAZOR_VOLCANO:
                            return AlysrazorVolcanoGUID;
                            break;

                        case DATA_SULFURON_DOOR:
                            return SulfuronWallGUID;
                            break;

                        case DATA_RAGNAROS_PLATFORM:
                            return RagnarosPlatformGUID;
                            break;

                        default:
                            break;
                    }

                    return NULL;
                }

                void SetData(uint32 type, uint32 data)
                {
                    switch (type)
                    {
                        case DATA_SHANNOX:
                            uiEncounter[0] = data;
                            if (data == DONE)
                            {
                            }
                            ;
                            break;

                        case DATA_BETHTILAC:
                            uiEncounter[1] = data;
                            HandleGameObject(BethtilacDoorGUID, data != IN_PROGRESS);
                            if (data == DONE)
                            {
                            }
                            ;
                            break;

                        case DATA_LORD_RHYOLITH:
                            uiEncounter[2] = data;
                            if (data == DONE)
                            {
                            }
                            ;
                            break;

                        case DATA_ALYSRAZAR:
                            uiEncounter[3] = data;
                            if (data == DONE)
                            {
                            }
                            ;
                            break;

                        case DATA_BALOROC:
                            uiEncounter[4] = data;
                            if (data == DONE)
                            {
                            }
                            ;
                            break;

                        case DATA_MAJORDOMUS:
                            uiEncounter[5] = data;
                            if (data == DONE)
                            {
                            }
                            ;
                            break;

                        case DATA_RAGNAROS:
                            uiEncounter[6] = data;
                            if (data == DONE)
                            {
                            }
                            ;
                            break;
                    }

                    if (data == DONE)
                        SaveToDB();
                }

                uint32 GetData(uint32 type) const
                {
                    return uiEncounter[type];
                }

                std::string GetSaveData()
                {
                    OUT_SAVE_INST_DATA;

                    std::ostringstream saveStream;
                    saveStream << "F L" << GetBossSaveData();

                    OUT_SAVE_INST_DATA_COMPLETE;
                    return saveStream.str();
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

                    std::istringstream loadStream(in);
                    loadStream >> dataHead1 >> dataHead2;

                    if (dataHead1 == 'F' && dataHead2 == 'L')
                    {
                        for (uint8 i = 0; i < ENCOUNTERS; ++i)
                        {
                            uint32 tmpState;
                            loadStream >> tmpState;
                            if (tmpState == IN_PROGRESS || tmpState > SPECIAL)
                            tmpState = NOT_STARTED;
                            uiEncounter[i] = tmpState;
                        }
                    }
                    else OUT_LOAD_INST_DATA_FAIL;

                    OUT_LOAD_INST_DATA_COMPLETE;
                }
            };
        };

void AddSC_instance_firelands()
{
    new instance_firelands();
}
