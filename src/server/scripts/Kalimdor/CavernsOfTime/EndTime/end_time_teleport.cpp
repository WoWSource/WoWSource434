#include "ScriptedGossip.h"
#include "ScriptMgr.h"
#include "InstanceScript.h"
#include "end_time.h"
#include "Spell.h"
#include "GameObjectAI.h"

enum InstanceTeleporeter
{
    START_TELEPORT          = 1,
    JAINA_TELEPORT          = 2,
    SYLVANAS_TELEPORT       = 3,
    TYRANDE_TELEPORT        = 4,
    BAINE_TELEPORT          = 5,
    MUROZOND_TELEPORT       = 6,
};

class end_time_teleport : public GameObjectScript
{
    public:
        end_time_teleport() : GameObjectScript("end_time_teleport") { }

        struct end_time_teleportAI : public GameObjectAI
        {
            end_time_teleportAI(GameObject* go) : GameObjectAI(go)
            {
                result1 = urand(1, 2);
                result2 = urand(3, 4);
                result3 = urand(5, 6);				
            }

            int result1, result2, result3;
            
            bool GossipHello(Player* player)
            {
                player->ADD_GOSSIP_ITEM(GOSSIP_ICON_CHAT, "Back to Nozdormu", GOSSIP_SENDER_MAIN, START_TELEPORT);
            
                if (InstanceScript* instance = go->GetInstanceScript())
                {
                    if (instance->GetData(DATA_JAINA) != DONE || instance->GetData(DATA_BAINE) != DONE)
                    {
                        if (instance->GetData(DATA_BAINE) != IN_PROGRESS && instance->GetData(DATA_JAINA) != IN_PROGRESS)
                        {
                            if (instance->GetData(DATA_BAINE) == DONE)
                                result1 = 1;

                            if (instance->GetData(DATA_JAINA) == DONE)
                                result1 = 2;
                        }

                        switch (result1)
                        {
                            case 1:
                                if (instance->GetData(DATA_BAINE) != IN_PROGRESS)
                                {
                                    instance->SetData(DATA_JAINA, IN_PROGRESS);
                                    instance->SetData(DATA_JAINA_PICKED_STATE, IN_PROGRESS); // Show stuff in UI for fragments.  
                                    player->ADD_GOSSIP_ITEM(GOSSIP_ICON_CHAT, "Azure Dragonshrine", GOSSIP_SENDER_MAIN, JAINA_TELEPORT);
                                }
                                break;
                        
                            case 2:
                                if (instance->GetData(DATA_JAINA) != IN_PROGRESS)
                                {
                                    instance->SetData(DATA_BAINE, IN_PROGRESS);
                                    player->ADD_GOSSIP_ITEM(GOSSIP_ICON_CHAT, "Obsidian Dragonshrine", GOSSIP_SENDER_MAIN, BAINE_TELEPORT);
                                }
                                break;                        
                        }
                    }

                    else if (instance->GetData(DATA_TYRANDE) != DONE || instance->GetData(DATA_SYLVANA) != DONE)
                    {
                        if (instance->GetData(DATA_SYLVANA) != IN_PROGRESS && instance->GetData(DATA_TYRANDE) != IN_PROGRESS)
                        {
                            if (instance->GetData(DATA_SYLVANA) == DONE)
                                result2 = 3;

                            if (instance->GetData(DATA_TYRANDE) == DONE)
                                result2 = 4;
								
						    if (instance->GetData(DATA_SYLVANA) == DONE)
                                result3 = 5;

                            if (instance->GetData(DATA_TYRANDE) == DONE)
                                result3 = 6;
                        }
                        switch (result2)
                        {
                            case 3:
                                if (instance->GetData(DATA_SYLVANA) != IN_PROGRESS)
                                {
                                    instance->SetData(DATA_TYRANDE, IN_PROGRESS);
                                    player->ADD_GOSSIP_ITEM(GOSSIP_ICON_CHAT, "Emerald Dragonshrine", GOSSIP_SENDER_MAIN, TYRANDE_TELEPORT);
                                }
                                break; 
                        
                            case 4:        
                                if (instance->GetData(DATA_TYRANDE) != IN_PROGRESS)
                                {
                                    instance->SetData(DATA_SYLVANA, IN_PROGRESS);
                                    player->ADD_GOSSIP_ITEM(GOSSIP_ICON_CHAT, "Ruby Dragonshrine", GOSSIP_SENDER_MAIN, SYLVANAS_TELEPORT);
                                }
                                break;   
                        }
						switch (result3)
                        {
                            case 5:
                                if (instance->GetData(DATA_SYLVANA) != IN_PROGRESS)
                                {
                                    instance->SetData(DATA_TYRANDE, IN_PROGRESS);
                                    player->ADD_GOSSIP_ITEM(GOSSIP_ICON_CHAT, "Bronze Dragonshrine", GOSSIP_SENDER_MAIN, MUROZOND_TELEPORT);
                                }
                                break; 
                        
                            case 6:        
                                if (instance->GetData(DATA_TYRANDE) != IN_PROGRESS)
                                {
                                    instance->SetData(DATA_SYLVANA, IN_PROGRESS);
                                    player->ADD_GOSSIP_ITEM(GOSSIP_ICON_CHAT, "Emerald Dragonshrine", GOSSIP_SENDER_MAIN, TYRANDE_TELEPORT);
                                }
                                break;   
                        }
                    }
            
                    else if (instance->GetData(DATA_MUROZOND) != DONE)
                    {
                        instance->SetData(DATA_MUROZOND, IN_PROGRESS);
                        player->ADD_GOSSIP_ITEM(GOSSIP_ICON_CHAT, "Bronze Dragonshrine", GOSSIP_SENDER_MAIN, MUROZOND_TELEPORT);
                    }
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
                    player->TeleportTo(938, 3711.591064f, -375.388763f, 113.064049f, 2.235886f);
                    player->CLOSE_GOSSIP_MENU();
                    break;
            
                case JAINA_TELEPORT:
                    player->TeleportTo(938, 2997.370850f, 570.221863f, 25.307350f, 5.543086f);
                    player->CLOSE_GOSSIP_MENU();
                    break;
            
                case SYLVANAS_TELEPORT:
                    player->TeleportTo(938, 3833.332520f, 1101.351929f, 83.412987f, 3.786126f);
                    player->CLOSE_GOSSIP_MENU();
                    break;
            
                case TYRANDE_TELEPORT:
                    player->TeleportTo(938, 2948.708740f, 68.523956f, 9.076958f, 3.289821f);
                    player->CLOSE_GOSSIP_MENU();
                    break;
            
                case BAINE_TELEPORT:
                    player->TeleportTo(938, 4343.328613f, 1294.588135f, 147.503693f, 0.765022f);
                    player->CLOSE_GOSSIP_MENU();
                    break;
            
                case MUROZOND_TELEPORT:
                    player->TeleportTo(938, 4042.709717f, -351.774353f, 122.215546f, 4.328253f);
                    player->CLOSE_GOSSIP_MENU();
                    break;
            }
            
            return true;
        }

        GameObjectAI* GetAI(GameObject* go) const
        {
            return new end_time_teleportAI(go);
        }
};

void AddSC_end_time_teleport()
{
    new end_time_teleport;
}
