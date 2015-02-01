#include "ScriptPCH.h"
#include "Chat.h"
#include "World.h"
#include "Player.h"
#include "Unit.h"
#include "Util.h"
#include "WorldSession.h"
#include "Language.h"


class KickEvent : public BasicEvent {
    public:
        KickEvent(WorldSession& session) :  _session(session) { }

        bool Execute(uint64 /*time*/, uint32 /*diff*/) {
            _session.KickPlayer("KickEvent Commands");
            return true;
        }

    private:
        WorldSession& _session;
};

class PlayerCommand : public PlayerScript
{
public:
    PlayerCommand() : PlayerScript("Player") { }

    void OnLogin(Player* player) {
        QueryResult result = LoginDatabase.PQuery("SELECT reason FROM account_tempban WHERE accountId = %u", player->GetSession()->GetAccountId());
        if (result) {
            player->GetSession()->SendAreaTriggerMessage("Your account has been taken out of the game! You'll be kicked in 10 seconds.");
            player->GetSession()->SendAreaTriggerMessage("|cffff0000Grund: %s", result->Fetch()[0].GetCString());
            
            if (SpellInfo const* spellInfo = sSpellMgr->GetSpellInfo(9454))
                if (!player->AddAura(9454, player))
                     player->CastSpell(player, 9454, true);

            if (WorldSession* session = player->GetSession())
                player->m_Events.AddEvent(new KickEvent(*session), player->m_Events.CalculateTime(10000), true);     
        } else {
            player->RemoveAurasDueToSpell(9454);
        }

        result = LoginDatabase.PQuery("SELECT FROM_UNIXTIME(`time`), `comment` FROM account_warning WHERE accountid = %u", player->GetSession()->GetAccountId());
        if (result) {
            ChatHandler(player->GetSession()).PSendSysMessage("|cffff0000Your account has been cautioned!");
            do {
                Field* field = result->Fetch();
                const char* date = field[0].GetCString();
                const char* comment = field[1].GetCString();

                ChatHandler(player->GetSession()).PSendSysMessage("%s - %s", date, comment);
            } while (result->NextRow());
        }
    }

    void OnLogout(Player* player) {
        QueryResult result = LoginDatabase.PQuery("SELECT bannedby FROM account_banned WHERE id = %u AND active = 1", player->GetSession()->GetAccountId());
        if (result)
            player->GetSession()->KickPlayer("KickEvent Commands");
    }
};

class commandscript : public CommandScript {
    public:
        commandscript() : CommandScript("commandscript") { }

        static bool HandleStrafenCommand(ChatHandler* handler, const char* args) {
            Player* target;
            uint64 targetGuid;
            std::string targetName;
            uint32 accountId;

            if (handler->GetSession()->GetSecurity() > SEC_PLAYER) {

                if (!handler->extractPlayerTarget((char*)args, &target, &targetGuid, &targetName))
                    return false;

                if (!target) {
                    QueryResult result = CharacterDatabase.PQuery("SELECT account FROM characters WHERE guid = %u", targetGuid);
                    if (!result)
                        return false;
                    accountId = result->Fetch()[0].GetUInt32();
                } else {
                    accountId = target->GetSession()->GetAccountId();
                }
            } else {
                accountId = handler->GetSession()->GetAccountId();
            }

            uint32 penalty_points = 0;

            QueryResult result = LoginDatabase.PQuery("SELECT FROM_UNIXTIME(`time`), `comment`, `by`, penalty_points, `active`, id FROM account_punishment WHERE account = %u", accountId);
            if (result) {
                handler->PSendSysMessage("|cffff0000The account has already been punished!");
                do {
                    Field* field = result->Fetch();
                    const char* date = field[0].GetCString();
                    const char* comment = field[1].GetCString();
                    penalty_points += field[3].GetUInt32();
                    const char* active = field[4].GetUInt8() == 1 ? "" : "[|cffff0000verjaehrt]";

                    QueryResult res = LoginDatabase.PQuery("SELECT username FROM account WHERE id = %u", field[2].GetUInt32());

                    const char* by = res ? res->Fetch()[0].GetCString() : "Unknown";

                    handler->PSendSysMessage("%s - ID[%u] - %u SP - %s - %s - %s", active, field[5].GetUInt32(), penalty_points, by, date, comment);
                } while (result->NextRow());
                handler->PSendSysMessage("|cff00ff00Der Account hat %u penalty_points!", penalty_points);
            } else {
                handler->PSendSysMessage("The account has no penalties!");
            }

            return true;
        }

        static bool HandleWarnInfoCommand(ChatHandler* handler, const char* args) {
            Player* target;
            uint64 targetGuid;
            std::string targetName;
            uint32 accountId;

            if (handler->GetSession()->GetSecurity() > SEC_PLAYER) {

                if (!handler->extractPlayerTarget((char*)args, &target, &targetGuid, &targetName))
                    return false;

                if (!target) {
                    QueryResult result = CharacterDatabase.PQuery("SELECT account FROM characters WHERE guid = %u", targetGuid);
                    if (!result)
                        return false;
                    accountId = result->Fetch()[0].GetUInt32();
                } else {
                    accountId = target->GetSession()->GetAccountId();
                }
            } else {
                accountId = handler->GetSession()->GetAccountId();
            }

            QueryResult result = LoginDatabase.PQuery("SELECT FROM_UNIXTIME(`time`), `comment`, by_account FROM account_warning WHERE accountid = %u", accountId);
            if (result) {
                handler->PSendSysMessage("|cffff0000The account has been cautioned!");
                do {
                    Field* field = result->Fetch();
                    const char* date = field[0].GetCString();
                    const char* comment = field[1].GetCString();

                    QueryResult res = LoginDatabase.PQuery("SELECT username FROM account WHERE id = %u", field[2].GetUInt32());

                    const char* by = res ? res->Fetch()[0].GetCString() : "Unknown";
                    if (handler->GetSession()->GetSecurity() > SEC_PLAYER)
                        handler->PSendSysMessage("%s - %s - %s", by, date, comment);
                    else 
                        handler->PSendSysMessage("%s - %s", date, comment);
                } while (result->NextRow());
            } else {
                handler->PSendSysMessage("The account has no warnings!");
            }

            return true;
        }

        static bool HandleWarnCommand(ChatHandler* handler, const char* args) {
            if (!*args)
                return false;

            char* playerName = strtok((char*)args, " ");
            char* comment = strtok(NULL, "");

            if (!playerName || !comment)
                return false;

            QueryResult result = CharacterDatabase.PQuery("SELECT account, guid FROM characters WHERE name = '%s'", playerName);

            if (!result) {
                handler->PSendSysMessage("Player not found!");
                return true;
            }

            uint32 accountId = result->Fetch()[0].GetUInt32();
            uint32 guid = result->Fetch()[1].GetUInt32();

            if (Player* player = ObjectAccessor::FindPlayer(guid)) {
                ChatHandler(player->GetSession()).PSendSysMessage("You were just booked! |cffff0000Grund: %s", comment);
                player->GetSession()->SendAreaTriggerMessage("You were just booked! |cffff0000Grund: %s", comment);
            }

            LoginDatabase.PExecute("INSERT INTO account_warning (accountId, by_account, comment, `time`) VALUES (%u, %u, '%s', UNIX_TIMESTAMP())", accountId, handler->GetSession()->GetAccountId(), comment);
            handler->PSendSysMessage("Players warned!");

            return true;
        }

        static bool HandleStrafeCommand(ChatHandler* handler, const char* args) {
            if (!*args)
                return false;

            sLog->outError(LOG_FILTER_GENERAL, "[Commands] .strafe executed. Player: %u - Arguments: %s", handler->GetSession()->GetPlayer()->GetGUID(), (char*)args);

            char* playerName = strtok((char*)args, " ");
            uint32 penalty_points = atoi(strtok(NULL, " "));
            char* comment = strtok(NULL, "");
            uint32 _penalty_points = penalty_points;

            if (!playerName || !comment | !penalty_points) 
                return false;

            sLog->outError(LOG_FILTER_GENERAL, "[Commands] .strafe Parameters: PN%s - %uSP - Comment[%s]", playerName, penalty_points, comment);

            if (penalty_points > 7 && handler->GetSession()->GetSecurity() < 6) {
                handler->PSendSysMessage("You can assign a maximum 7 penalty_points points!");
                return true;
            } 

            if (penalty_points < 1) {
                handler->PSendSysMessage("You have to assign at least one penalty point!");
                return true;
            }

            if (!playerName || !comment)
                return false;

            QueryResult result = CharacterDatabase.PQuery("SELECT account, guid FROM characters WHERE name = '%s'", playerName);

            if (!result) {
                handler->PSendSysMessage("Player not found!");
                return true;
            }

            uint32 accountId = result->Fetch()[0].GetUInt32();
            uint32 guid = result->Fetch()[1].GetUInt32();
            sLog->outError(LOG_FILTER_GENERAL, "[Commands] Get Data GUID: %u - AccountId: %u", guid, accountId);
            

            QueryResult res = LoginDatabase.PQuery("SELECT penalty_points FROM account_punishment WHERE account = %u", accountId);

            if (res) {
                do {
                    _penalty_points += res->Fetch()[0].GetUInt32();
                } while (res->NextRow());
            }

            uint32 banTime = penalty_points * 60 * 60 * 24;

            if (Player* player = ObjectAccessor::FindPlayer(guid)) {
                if (WorldSession* session = player->GetSession()) {
                    sLog->outError(LOG_FILTER_GENERAL, "[Commands] init kick event");
                    ChatHandler(session).PSendSysMessage("|cffff0000Your account gets a penalty! You will be logged off in 10 seconds. |cff00ff00Grund: %s", comment);
                    session->SendAreaTriggerMessage("|cffff0000Your account gets a penalty! You will be logged off in 10 seconds. |cff00ff00Grund: %s", comment);
                    player->m_Events.AddEvent(new KickEvent(*session), player->m_Events.CalculateTime(10000), true);
                }
            }

            LoginDatabase.PExecute("INSERT INTO account_punishment (account, `by`, `comment`, `penalty_points`, `time`, `active`) VALUES (%u, %u, '%s', %u, UNIX_TIMESTAMP(), 1)", accountId, handler->GetSession()->GetAccountId(), comment, penalty_points);
            LoginDatabase.PExecute("INSERT INTO account_banned (id, realm, bandate, unbandate, bannedby, banreason, active) VALUES (%u, '-1', UNIX_TIMESTAMP(), UNIX_TIMESTAMP()+%u, '%s', '%s', 1)", accountId, banTime, handler->GetSession()->GetPlayer()->GetName().c_str(), comment);

            if (_penalty_points >= 10) {
                LoginDatabase.PExecute("UPDATE account SET locked = 1, last_ip = '1337' WHERE id = %u", accountId);
                sLog->outError(LOG_FILTER_GENERAL, "[Commands] Account %u locked", accountId);
            }

            handler->PSendSysMessage("Player is suspended!");
            return true;
        }

        static bool HandleDelStrafeCommand(ChatHandler* handler, const char* args) {
            if (!*args)
                return false;

            int id = atoi((char*)args);

            QueryResult result = LoginDatabase.PQuery("SELECT active FROM account_punishment WHERE account = %u", id);

            if (!result) {
                handler->PSendSysMessage("Punishment was not found. check ID!");
                return true;
            }

            if (result->Fetch()[0].GetUInt8() == 0) {
                handler->PSendSysMessage("Punishment is barred!");
                return true;
            }

            LoginDatabase.PExecute("UPDATE account_punishment SET active = 0 WHERE id = %u", id);
            LoginDatabase.PExecute("UPDATE account_banned SET active = 0 WHERE id = %u", id);
            LoginDatabase.PExecute("UPDATE account SET locked = 0 WHERE id = %u", id);
            handler->PSendSysMessage("Penalty has been removed.");
            
            return true;
        }

        static bool HandleTempBanCommand(ChatHandler* handler, const char* args) {
            Player* target;
            uint64 targetGuid;
            std::string targetName;
            uint32 accountId;

            if (!*args)
                return false;

            char* playerName = strtok((char*)args, " ");
            char* reason = strtok(NULL, "");

            if (!reason) {
                handler->PSendSysMessage("Please enter a reason!");
                return true;
            }

            if (!handler->extractPlayerTarget(playerName, &target, &targetGuid, &targetName)) {
                handler->PSendSysMessage("Player not found!");
                return true;
            }

            if (!target) {
                QueryResult result = CharacterDatabase.PQuery("SELECT account FROM characters WHERE guid = %u", targetGuid);
                    
                if (!result) {
                    handler->PSendSysMessage("that can not usualy happen");
                    return true;
                }

                accountId = result->Fetch()[0].GetUInt32();
            } else {
                accountId = target->GetSession()->GetAccountId();
            }

            QueryResult result = LoginDatabase.PQuery("SELECT reason FROM account_tempban WHERE accountId = %u", accountId);
            if (result) {
                handler->PSendSysMessage("The player already has a TempBan!");
                return true;
            }

            LoginDatabase.PQuery("INSERT INTO account_tempban (accountId, reason) VALUES (%u, '%s')", accountId, reason);

            if (target) {
                target->GetSession()->KickPlayer("KickEvent Commands");
            }

            handler->PSendSysMessage("TempBan set!");
            
            return true;
        }

        static bool HandleDelTempBanCommand(ChatHandler* handler, const char* args) {
            Player* target;
            uint64 targetGuid;
            std::string targetName;
            uint32 accountId;

            if (!*args)
                return false;

            if (!handler->extractPlayerTarget((char*)args, &target, &targetGuid, &targetName)) {
                handler->PSendSysMessage("Player not found!");
                return true;
            }

            if (!target) {
                QueryResult result = CharacterDatabase.PQuery("SELECT account FROM characters WHERE guid = %u", targetGuid);
                    
                if (!result) {
                    handler->PSendSysMessage("that can not usualy happen");
                    return true;
                }

                accountId = result->Fetch()[0].GetUInt32();
            } else {
                accountId = target->GetSession()->GetAccountId();
            }

            LoginDatabase.PExecute("DELETE FROM account_tempban WHERE accountId = %u", accountId);
            handler->PSendSysMessage("TempBan away");
            
            return true;
        }

        ChatCommand* GetCommands() const {
            static ChatCommand PWSCommandTable[] = {
                { "warnings",    SEC_PLAYER,        true, &HandleWarnInfoCommand,   "",   NULL },
                { "warn",       SEC_MODERATOR,     true, &HandleWarnCommand,       "",   NULL },
                { "punishment",          SEC_MODERATOR,     true, &HandleStrafeCommand,     "",   NULL },
                { "punish",         SEC_PLAYER,        true, &HandleStrafenCommand,    "",   NULL },
                { "delpenalty",       SEC_ADMINISTRATOR, true, &HandleDelStrafeCommand,  "",   NULL },
                { "tempban",         SEC_ADMINISTRATOR, true, &HandleTempBanCommand,    "",   NULL },
                { "deltempban",      SEC_ADMINISTRATOR, true, &HandleDelTempBanCommand, "",   NULL },
                { NULL, 0, false, NULL, "", NULL }
            };

            return PWSCommandTable;
        }
};


void AddSC_commandscript() {
    new commandscript();
    new PlayerCommand();
}
