#include "ScriptMgr.h"
#include "Player.h"
#include "MapManager.h"
#include "ObjectMgr.h"
#include "Chat.h"
#include "Language.h"

#include <map>
#include <set>

class cms_commandscript : public CommandScript
{
    public:
        cms_commandscript() : CommandScript("cms_commandscript") { }
        
        ChatCommand* GetCommands() const
        {
            static ChatCommand WarcryCommandTable[] =
            {
                { "pteleport",       SEC_CONSOLE,       true,   &HandlePTeleportCommand,     "", NULL },
                { "refunditem",      SEC_CONSOLE,       true,   &HandleItemDestroy,          "", NULL },
                { NULL,              0,                 false,  NULL,                        "", NULL }
            };
            return WarcryCommandTable;
        }
        
        //teleport Player with Name at coordinates, including Z and orientation
        static bool HandlePTeleportCommand(ChatHandler* handler, const char *args)
        {
            if (!*args)
                return false;

            char* player_name = strtok((char*)args, " ");
            char* goX = strtok((char*)NULL, " ");
            char* goY = strtok((char*)NULL, " ");
            char* goZ = strtok((char*)NULL, " ");
            char* id = strtok((char*)NULL, " ");
            char* port = strtok((char*)NULL, " ");

            Player* player;
            uint64 target_guid;
            if (!handler->extractPlayerTarget(player_name, &player, &target_guid))
                return false;

            //check if the player is online, otherwise the server will crash
            if (!player)
                return false;

            if (!goX || !goY)
                return false;

            float x = (float)atof(goX);
            float y = (float)atof(goY);
            float z;
            float ort = port ? (float)atof(port) : player->GetOrientation();
            uint32 mapId = id ? (uint32)atoi(id) : player->GetMapId();

            if (goZ)
            {
                z = (float)atof(goZ);
                if (!MapManager::IsValidMapCoord(mapId, x, y, z))
                {
                    handler->PSendSysMessage(LANG_INVALID_TARGET_COORD, x, y, mapId);
                    handler->SetSentErrorMessage(true);
                    return false;
                }
            }
            else
            {
                if (!MapManager::IsValidMapCoord(mapId, x, y))
                {
                    handler->PSendSysMessage(LANG_INVALID_TARGET_COORD, x, y, mapId);
                    handler->SetSentErrorMessage(true);
                    return false;
                }
                Map const* map = sMapMgr->CreateBaseMap(mapId);
                z = std::max(map->GetHeight(x, y, MAX_HEIGHT), map->GetWaterLevel(x, y));
            }

            // stop flight if need
            if (player->isInFlight())
            {
                player->GetMotionMaster()->MovementExpired();
                player->CleanupAfterTaxiFlight();
            }
            // save only in non-flight case
            else
                player->SaveRecallPosition();

            player->TeleportTo(mapId, x, y, z, ort);
            return true;
        }

        static bool HandleItemDestroy(ChatHandler* handler, const char* args)
        {
            if (!*args)
                return false;
            
            uint32 itemId = 0;
            
            char* playerName = strtok((char*)args, " ");
            char* itemEntry  = strtok((char*)NULL, " ");
            
            if (!playerName || !itemEntry)
                return false;
            
            itemId = uint32(atol(itemEntry));
            
            Player* player;
            uint64 target_guid;
            if (!handler->extractPlayerTarget(playerName, &player, &target_guid))
                return false;
            
            ItemTemplate const* itemTemplate = sObjectMgr->GetItemTemplate(itemId);
            if (!itemTemplate)
            {
                handler->PSendSysMessage("Invalid item entry: %u.", itemId);
                handler->SetSentErrorMessage(true);
                return false;
            }
            
            //Check if the player is online
            if (player)
            {
                Item* item = player->GetItemByEntry(itemTemplate->ItemId);
                if (!item)
                {
                    handler->PSendSysMessage("The player does not posses that item!");
                    handler->SetSentErrorMessage(true);
                    return false;
                }
                else
                {
                    player->DestroyItem(item->GetBagSlot(), item->GetSlot(), true);
                    handler->PSendSysMessage("Item Destroyed.");
                }
            }
            else
            {
                handler->PSendSysMessage("The character is offline or invalid.");
                handler->SetSentErrorMessage(true);
                return false; 
            }
            
            return true;
        }
};

void AddSC_cms_commandscript()
{
    new cms_commandscript();
}
