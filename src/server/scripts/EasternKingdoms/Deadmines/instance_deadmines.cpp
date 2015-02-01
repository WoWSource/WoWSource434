/*
*
* Copyright (C) 2012-2014 Cerber Project <https://bitbucket.org/mojitoice/>
*
*/

#include "ScriptPCH.h"
#include "deadmines.h"

#define NOTE_TEXT "A note falls to the floor!"

Position const NoteSpawn = {-74.36111f, -820.0139f, 40.67145f, 4.014257f};

class instance_deadmines : public InstanceMapScript
{
public:
    instance_deadmines() : InstanceMapScript("instance_deadmines", 36) { }

    struct instance_deadmines_InstanceMapScript : public InstanceScript
    {
        instance_deadmines_InstanceMapScript(Map* map) : InstanceScript(map)
        {
            SetBossNumber(MAX_BOSSES);
        };

        void Initialize()
        {
            FactoryDoorGUID     = 0;
            FoundaryDoorGUID    = 0;
            HeavyDoorGUID       = 0;
            GlubtokGUID         = 0;
            IroncladDoorGUID    = 0;
            TeamInInstance      = 0;
            uiVanessa           = 0;
            uiVanessaNote       = 0;
            uiVanessaBoss       = 0;
        }

        void OnCreatureCreate(Creature* creature)
        {
            Map::PlayerList const &players = instance->GetPlayers();
            if (!players.isEmpty())
            {
                if (Player* player = players.begin()->getSource())
                    TeamInInstance = player->GetTeam();
            }
            switch (creature->GetEntry())
            {
                case 46889: // Kagtha
                    if (TeamInInstance == ALLIANCE)
                        creature->UpdateEntry(42308, ALLIANCE); // Lieutenant Horatio Laine
                    break;
                case 46902: // Miss Mayhem
                    if (TeamInInstance == ALLIANCE)
                        creature->UpdateEntry(491, ALLIANCE); // Quartermaster Lewis <Quartermaster>
                    break;
                case 46903: // Mayhem Reaper Prototype
                    if (TeamInInstance == ALLIANCE)
                        creature->UpdateEntry(1, ALLIANCE); // GM WAYPOINT
                    break;
                case 46906: // Slinky Sharpshiv
                    if (TeamInInstance == ALLIANCE)
                        creature->UpdateEntry(46612, ALLIANCE); // Lieutenant Horatio Laine
                    break;
                case 46613: // Crime Scene Alarm-O-Bot
                    if (TeamInInstance == HORDE)
                        creature->UpdateEntry(1, HORDE); // GM WAYPOINT
                    break;
                case 50595: // Stormwind Defender
                    if (TeamInInstance == HORDE)
                        creature->UpdateEntry(46890, HORDE); // Shattered Hand Assassin
                    break;
                case 46614: // Stormwind Investigator
                    if (TeamInInstance == HORDE)
                        creature->UpdateEntry(1, HORDE); // GM WAYPOINT
                    break;
                case NPC_VANESSA_VANCLEEF:
                    uiVanessa = creature->GetGUID();
                    break;
                case NPC_VANESSA_BOSS:
                    uiVanessaBoss = creature->GetGUID();
                    break;
                case NPC_VANESSA_NOTE:
                    uiVanessaNote = creature->GetGUID();
                    break;
                case NPC_GLUBTOK:
                    GlubtokGUID = creature->GetGUID();
                    break;
            }
        }

        virtual void Update(uint32 diff) { }

        void OnGameObjectCreate(GameObject* go)
        {
            switch (go->GetEntry())
            {
                case GO_FACTORY_DOOR: // Door after first boss
                    FactoryDoorGUID = go->GetGUID();
                    break;
                case GO_FOUNDRY_DOOR: // Door before ship
                    FoundaryDoorGUID = go->GetGUID();
                    break;
                case GO_HEAVY_DOOR_HELIX:
                    HeavyDoorGUID = go->GetGUID();
                    break;
                case GO_IRONCLAD_DOOR:
                    IroncladDoorGUID = go->GetGUID();
                    break;
            }
        }

        void SetData(uint32 type, uint32 value)
        {
            switch (type)
            {
                case DATA_NIGHTMARE_HELIX:
                    if (value == DONE)
                        if (GameObject* go = instance->GetGameObject(FoundaryDoorGUID))
                            go->SetGoState(GO_STATE_ACTIVE);
                    break;
                case DATA_NIGHTMARE_MECHANICAL:
                    if (value == DONE)
                        if (GameObject* go = instance->GetGameObject(IroncladDoorGUID))
                            go->SetGoState(GO_STATE_ACTIVE);
                    break;
                case DATA_GLUBTOK:
                    if (value == DONE)
                        if (GameObject* go = instance->GetGameObject(FactoryDoorGUID))
                            go->SetGoState(GO_STATE_ACTIVE);
                    break;
            }
        }

        bool SetBossState(uint32 id, EncounterState state)
        {
            if (!InstanceScript::SetBossState(id, state))
                return false;

            switch (id)
            {
                case DATA_GLUBTOK:
                    if (state == DONE)
                        if (GameObject* go = instance->GetGameObject(FactoryDoorGUID))
                            go->SetGoState(GO_STATE_ACTIVE);
                    break;
                case DATA_HELIX:
                    if (state == DONE)
                        if (GameObject* go = instance->GetGameObject(HeavyDoorGUID))
                            go->SetGoState(GO_STATE_ACTIVE);
                    break;
                case DATA_FOEREAPER:
                    if (state == DONE)
                        if (GameObject* go = instance->GetGameObject(FoundaryDoorGUID))
                            go->SetGoState(GO_STATE_ACTIVE);
                    break;
                case DATA_RIPSNARL:
                    //if (state == DONE)
                    //    break;
                    break;
                case DATA_COOKIE:
                    if (state == DONE)
                    {
                        if (instance->IsHeroic())
                        {
                            if (Creature* Note = instance->SummonCreature(NPC_VANESSA_NOTE, NoteSpawn))
                            {
                                Note->MonsterTextEmote(NOTE_TEXT, 0, true);

                                if (GameObject* go = instance->GetGameObject(IroncladDoorGUID))
                                    go->SetGoState(GO_STATE_READY);

                                if (GameObject* go = instance->GetGameObject(HeavyDoorGUID))
                                    go->SetGoState(GO_STATE_READY);

                                if (GameObject* go = instance->GetGameObject(FoundaryDoorGUID))
                                    go->SetGoState(GO_STATE_READY);

                            }
                        }
                    }
                    break;
                case DATA_VANNESSA_NIGHTMARE:
                    if (state == FAIL)
                    {
                        if (Creature* Note = instance->SummonCreature(NPC_VANESSA_NOTE, NoteSpawn))
                        {
                            if (GameObject* go = instance->GetGameObject(IroncladDoorGUID))
                                go->SetGoState(GO_STATE_ACTIVE);

                            if (GameObject* go = instance->GetGameObject(HeavyDoorGUID))
                                go->SetGoState(GO_STATE_ACTIVE);

                            if (GameObject* go = instance->GetGameObject(FoundaryDoorGUID))
                                go->SetGoState(GO_STATE_ACTIVE);
                        }
                    }
                case DATA_VANESSA:
                    break;
            }

            return true;
        }

        uint64 GetData64(uint32 data) const
        {
            switch (data)
            {
                case NPC_VANESSA_VANCLEEF:
                    return uiVanessa;
                    break;
                case NPC_VANESSA_BOSS:
                    return uiVanessaBoss;
                    break;
                case NPC_VANESSA_NOTE:
                    return uiVanessaNote;
                    break;
                case DATA_GLUBTOK:
                    return GlubtokGUID;
                    break;
            }

            return 0;
        }

    private:
        uint64 FactoryDoorGUID;
        uint64 FoundaryDoorGUID;
        uint64 HeavyDoorGUID;
        uint64 IroncladDoorGUID;
        uint64 uiVanessa;
        uint64 uiVanessaNote;
        uint64 uiVanessaBoss;
        uint64 GlubtokGUID;

        uint32 TeamInInstance;
    };

    InstanceScript* GetInstanceScript(InstanceMap* map) const
    {
        return new instance_deadmines_InstanceMapScript(map);
    }
};

void AddSC_instance_deadmines()
{
    new instance_deadmines();
}
