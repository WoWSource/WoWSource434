#include "ScriptPCH.h"
#include "well_of_eternity.h"
#include "GameObjectAI.h"

enum Spells
{
    SPELL_TELEPORT_TO_START                 = 107934,
    SPELL_TELEPORT_TO_AZSHARA_PALACE        = 107979,
    SPELL_TELEPORT_TO_WELL_OF_ETERNITY      = 107691,
};

enum InstanceTeleporter
{
    START_TELEPORT          = 1,
    AZSHARA_TELEPORT        = 2,
    WOE_TELEPORT            = 3,
};

class go_well_of_eternity_teleport : public GameObjectScript
{
public:
    go_well_of_eternity_teleport() : GameObjectScript("go_well_of_eternity_teleport") { }

    bool OnGossipHello(Player* pPlayer, GameObject* pGo)
    {
        if (pPlayer->isInCombat())
            return true;

        if (InstanceScript* pInstance = pGo->GetInstanceScript())
        {
            pPlayer->ADD_GOSSIP_ITEM(GOSSIP_ICON_CHAT, "Teleport to Start.", GOSSIP_SENDER_MAIN, START_TELEPORT);

            if (pPlayer->isGameMaster())
            {
                pPlayer->ADD_GOSSIP_ITEM(GOSSIP_ICON_CHAT, "Teleport to Azshara's Palace.", GOSSIP_SENDER_MAIN, AZSHARA_TELEPORT);
                pPlayer->ADD_GOSSIP_ITEM(GOSSIP_ICON_CHAT, "Teleport to Well of Eternity.", GOSSIP_SENDER_MAIN, WOE_TELEPORT);
            } else
            {
                if (pInstance->GetBossState(DATA_PEROTHARN) == DONE)
                    pPlayer->ADD_GOSSIP_ITEM(GOSSIP_ICON_CHAT, "Teleport to Azshara's Palace.", GOSSIP_SENDER_MAIN, AZSHARA_TELEPORT);
                if (pInstance->GetBossState(DATA_AZSHARA) == DONE)
                    pPlayer->ADD_GOSSIP_ITEM(GOSSIP_ICON_CHAT, "Teleport to Well of Eternity.", GOSSIP_SENDER_MAIN, WOE_TELEPORT);
            }
        }

        pPlayer->SEND_GOSSIP_MENU(pPlayer->GetGossipTextId(pGo), pGo->GetGUID());
        return true;
    }

    bool OnGossipSelect(Player* player, GameObject* go, uint32 sender, uint32 action)
    {
        //player->PlayerTalkClass->ClearMenus();
        if (player->isInCombat())
            return true;

        InstanceScript* pInstance = player->GetInstanceScript();
        if (!pInstance)
            return true;

        switch (action)
        {
        case START_TELEPORT:
            player->CastSpell(player, SPELL_TELEPORT_TO_START, true);
            player->CLOSE_GOSSIP_MENU();
            break;
        case AZSHARA_TELEPORT:
            player->CastSpell(player, SPELL_TELEPORT_TO_AZSHARA_PALACE, true);
            player->CLOSE_GOSSIP_MENU();
            break;
        case WOE_TELEPORT:
            player->CastSpell(player, SPELL_TELEPORT_TO_WELL_OF_ETERNITY, true);
            player->CLOSE_GOSSIP_MENU();
            break;
        }

        return true;
    }
};

void AddSC_well_of_eternity_teleport()
{
    new go_well_of_eternity_teleport();
}