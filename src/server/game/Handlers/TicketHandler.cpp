/*
 * Copyright (C) 2008-2013 TrinityCore <http://www.trinitycore.org/>
 * Copyright (C) 2005-2009 MaNGOS <http://getmangos.com/>
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

#include "zlib.h"
#include "Common.h"
#include "Language.h"
#include "ObjectMgr.h"
#include "Opcodes.h"
#include "Player.h"
#include "TicketMgr.h"
#include "Util.h"
#include "World.h"
#include "WorldPacket.h"
#include "WorldSession.h"

//Todo Database Support
void WorldSession::HandleSubmitComplainOpcode(WorldPacket & recvData)
{
    float posX, posY, posZ, posO;
    uint32 mapID;

    ObjectGuid guid;

    guid[5] = recvData.ReadBit();
    guid[0] = recvData.ReadBit();
    guid[1] = recvData.ReadBit();

    uint32 length = recvData.ReadBits(12);

    guid[3] = recvData.ReadBit();
    guid[2] = recvData.ReadBit();
    guid[4] = recvData.ReadBit();
    guid[7] = recvData.ReadBit();
     
    //guid:4 options:0 length :0;posY:648.581055 ,posX:-8851.486328 ,posZ:96.454063 ,MAP:0 ,poso:2.151366 , unk:0 text:           //SPAM
    //guid:4 options:10 length :0;posY:648.581055 ,posX:-8851.486328 ,posZ:96.454063 ,MAP:0 ,poso:2.151366 , unk:0 text:          //Ausdrucksweise

    uint32 options = recvData.ReadBits(4); // ##

    guid[6] = recvData.ReadBit();

    recvData.ReadByteSeq(guid[3]);
    recvData.ReadByteSeq(guid[5]);
    recvData.ReadByteSeq(guid[1]);
    recvData.ReadByteSeq(guid[2]);
    recvData.ReadByteSeq(guid[6]);
    recvData.ReadByteSeq(guid[0]);

    std::string text = recvData.ReadString(length);

    recvData.ReadByteSeq(guid[7]);
    recvData.ReadByteSeq(guid[4]);

    recvData >> posY;
    recvData >> posZ;	    
    recvData >> posX;	
    recvData >> mapID;
    recvData >> posO;

    recvData.ReadBit();

    uint32 count = recvData.ReadBits(22);
    uint32* strLength = new uint32[count];

    // sLog->outInfo(LOG_FILTER_SERVER_LOADING,"CMSG_SUBMIT_COMPLAIN:: count:%u guid:%u option:%u length :%u;posY:%f ,posX:%f ,posZ:%f ,MAP:%u ,poso:%f  text:%s",count,guid,options,length,posY,posX,posZ,mapID,posO,text.c_str());

    switch(options)
    {    
        case COMPLAIN_CHEATER:
        case COMPLAIN_PLAYER_NAME:
        case COMPLAIN_GUILD_NAME:
        case COMPLAIN_ARENA_NAME:
            
            break;
        case COMPLAIN_SPAM:
        case COMPLAIN_BAD_LANG:
            for (uint32 i = 0; i < count; ++i)
                strLength[i] = recvData.ReadBits(13);

            for (uint32 i = 0; i < count; ++i)
            {
                sLog->outInfo(LOG_FILTER_SERVER_LOADING,"time : %u",recvData.ReadPackedTime());
                sLog->outInfo(LOG_FILTER_SERVER_LOADING,"Text : %s",recvData.ReadString(strLength[i]).c_str());
            }

            break;
        default:
             sLog->outInfo(LOG_FILTER_SERVER_LOADING,"CMSG_SUBMIT_COMPLAIN::UNKNOW option:%u",options);

    }
}

//Todo Questlog AuraLog PhaseLog!
void WorldSession::HandleSubmitBugOpcode(WorldPacket & recvData)
{
    float posX, posY, posZ, posO;
    uint32 mapID;

    uint32 length = recvData.ReadBits(12);
    std::string bug = recvData.ReadString(length);
	
    recvData >> posY;
    recvData >> posZ;
    recvData >> posX;
    recvData >> mapID;
    recvData >> posO;

    SQLTransaction trans = SQLTransaction(NULL);

    //    0     1      2      3          4      5      6     7     8          
    // realm, guid, message, createTime, pool, mapId, posX, posY, posZ
    uint8 index = 0;
    PreparedStatement* stmt = LoginDatabase.GetPreparedStatement(LOGIN_INS_BUG_TICKET);
    stmt->setUInt32(index, realmID);
    stmt->setUInt32(++index, GUID_LOPART(GetPlayer()->GetGUID()));
    stmt->setString(++index, bug);
    stmt->setUInt32(++index, uint32(time(NULL)));
    stmt->setString(++index, "change Line" /*GetNode()->GetName()*/);
    stmt->setUInt16(++index, uint16(mapID));
    stmt->setFloat (++index, (float)posX);
    stmt->setFloat (++index, (float)posY);
    stmt->setFloat (++index, (float)posZ);

    LoginDatabase.ExecuteOrAppend(trans, stmt);
}
void WorldSession::HandleSubmitSuggestionOpcode(WorldPacket & recvData)
{
    HandleSubmitBugOpcode(recvData);  //If you want a separate table for the suggestions. Do it !! Same opcode Structure HandleSubmitBugOpcode
}

void WorldSession::HandleGMTicketCreateOpcode(WorldPacket& recvData)
{
    // Don't accept tickets if the ticket queue is disabled. (Ticket UI is greyed out but not fully dependable)
    if (sTicketMgr->GetStatus() == GMTICKET_QUEUE_STATUS_DISABLED)
        return;

    if (GetPlayer()->getLevel() < sWorld->getIntConfig(CONFIG_TICKET_LEVEL_REQ))
    {
        SendNotification(GetTrinityString(LANG_TICKET_REQ), sWorld->getIntConfig(CONFIG_TICKET_LEVEL_REQ));
        return;
    }

    GMTicketResponse response = GMTICKET_RESPONSE_CREATE_ERROR;
    // Player must not have ticket
    if (!sTicketMgr->GetTicketByPlayer(GetPlayer()->GetGUID()))
    {
        GmTicket* ticket = new GmTicket(GetPlayer(), recvData);

        uint32 count;
        std::list<uint32> times;
        uint32 decompressedSize;
        std::string chatLog;

        recvData >> count;

        for (uint32 i = 0; i < count; i++)
        {
            uint32 time;
            recvData >> time;
            times.push_back(time);
        }

        recvData >> decompressedSize;

        if (count && decompressedSize && decompressedSize < 0xFFFF)
        {
            uint32 pos = recvData.rpos();
            ByteBuffer dest;
            dest.resize(decompressedSize);

            uLongf realSize = decompressedSize;
            if (uncompress(dest.contents(), &realSize, recvData.contents() + pos, recvData.size() - pos) == Z_OK)
            {
                dest >> chatLog;
                ticket->SetChatLog(times, chatLog);
            }
            else
            {
                sLog->outError(LOG_FILTER_NETWORKIO, "CMSG_GMTICKET_CREATE possibly corrupt. Uncompression failed.");
                recvData.rfinish();
                return;
            }

            recvData.rfinish(); // Will still have compressed data in buffer.
        }

        sTicketMgr->AddTicket(ticket);
        sTicketMgr->UpdateLastChange();

        sWorld->SendGMText(LANG_COMMAND_TICKETNEW, GetPlayer()->GetName().c_str(), ticket->GetId());

        response = GMTICKET_RESPONSE_CREATE_SUCCESS;
    }

    WorldPacket data(SMSG_GMTICKET_CREATE, 4);
    data << uint32(response);
    SendPacket(&data);
}

void WorldSession::HandleGMTicketUpdateOpcode(WorldPacket& recvData)
{
    std::string message;
    recvData >> message;

    GMTicketResponse response = GMTICKET_RESPONSE_UPDATE_ERROR;
    if (GmTicket* ticket = sTicketMgr->GetTicketByPlayer(GetPlayer()->GetGUID()))
    {
        SQLTransaction trans = SQLTransaction(NULL);
        ticket->SetMessage(message);
        ticket->SaveToDB(trans);

        sWorld->SendGMText(LANG_COMMAND_TICKETUPDATED, GetPlayer()->GetName().c_str(), ticket->GetId());

        response = GMTICKET_RESPONSE_UPDATE_SUCCESS;
    }

    WorldPacket data(SMSG_GMTICKET_UPDATETEXT, 4);
    data << uint32(response);
    SendPacket(&data);
}

void WorldSession::HandleGMTicketDeleteOpcode(WorldPacket & /*recvData*/)
{
    if (GmTicket* ticket = sTicketMgr->GetTicketByPlayer(GetPlayer()->GetGUID()))
    {
        WorldPacket data(SMSG_GMTICKET_DELETETICKET, 4);
        data << uint32(GMTICKET_RESPONSE_TICKET_DELETED);
        SendPacket(&data);

        sWorld->SendGMText(LANG_COMMAND_TICKETPLAYERABANDON, GetPlayer()->GetName().c_str(), ticket->GetId());

        sTicketMgr->CloseTicket(ticket->GetId(), GetPlayer()->GetGUID());
        sTicketMgr->SendTicket(this, NULL);
    }
}

void WorldSession::HandleGMTicketGetTicketOpcode(WorldPacket & /*recvData*/)
{
    SendQueryTimeResponse();

    if (GmTicket* ticket = sTicketMgr->GetTicketByPlayer(GetPlayer()->GetGUID()))
    {
        if (ticket->IsCompleted())
            ticket->SendResponse(this);
        else
            sTicketMgr->SendTicket(this, ticket);
    }
    else
        sTicketMgr->SendTicket(this, NULL);
}

void WorldSession::HandleGMTicketSystemStatusOpcode(WorldPacket & /*recvData*/)
{
    // Note: This only disables the ticket UI at client side and is not fully reliable
    // are we sure this is a uint32? Should ask Zor
    WorldPacket data(SMSG_GMTICKET_SYSTEMSTATUS, 4);
    data << uint32(sTicketMgr->GetStatus() ? GMTICKET_QUEUE_STATUS_ENABLED : GMTICKET_QUEUE_STATUS_DISABLED);
    SendPacket(&data);
}

void WorldSession::HandleGMSurveySubmit(WorldPacket& recvData)
{
    uint32 nextSurveyID = sTicketMgr->GetNextSurveyID();
    // just put the survey into the database
    uint32 mainSurvey; // GMSurveyCurrentSurvey.dbc, column 1 (all 9) ref to GMSurveySurveys.dbc
    recvData >> mainSurvey;

    // sub_survey1, r1, comment1, sub_survey2, r2, comment2, sub_survey3, r3, comment3, sub_survey4, r4, comment4, sub_survey5, r5, comment5, sub_survey6, r6, comment6, sub_survey7, r7, comment7, sub_survey8, r8, comment8, sub_survey9, r9, comment9, sub_survey10, r10, comment10,
    for (uint8 i = 0; i < 15; i++)
    {
        uint32 subSurveyId; // ref to i'th GMSurveySurveys.dbc field (all fields in that dbc point to fields in GMSurveyQuestions.dbc)
        recvData >> subSurveyId;
        if (!subSurveyId)
            break;

        uint8 rank; // probably some sort of ref to GMSurveyAnswers.dbc
        recvData >> rank;
        std::string comment; // comment ("Usage: GMSurveyAnswerSubmit(question, rank, comment)")
        recvData >> comment;

        PreparedStatement* stmt = CharacterDatabase.GetPreparedStatement(CHAR_INS_GM_SUBSURVEY);
        stmt->setUInt32(0, nextSurveyID);
        stmt->setUInt32(1, subSurveyId);
        stmt->setUInt32(2, rank);
        stmt->setString(3, comment);
        CharacterDatabase.Execute(stmt);
    }

    std::string comment; // just a guess
    recvData >> comment;

    PreparedStatement* stmt = CharacterDatabase.GetPreparedStatement(CHAR_INS_GM_SURVEY);
    stmt->setUInt32(0, GUID_LOPART(GetPlayer()->GetGUID()));
    stmt->setUInt32(1, nextSurveyID);
    stmt->setUInt32(2, mainSurvey);
    stmt->setString(3, comment);

    CharacterDatabase.Execute(stmt);
}

void WorldSession::HandleReportLag(WorldPacket& recvData)
{
    // just put the lag report into the database...
    // can't think of anything else to do with it
    uint32 lagType, mapId;
    recvData >> lagType;
    recvData >> mapId;
    float x, y, z;
    recvData >> x;
    recvData >> y;
    recvData >> z;

    PreparedStatement* stmt = CharacterDatabase.GetPreparedStatement(CHAR_INS_LAG_REPORT);
    stmt->setUInt32(0, GUID_LOPART(GetPlayer()->GetGUID()));
    stmt->setUInt8 (1, lagType);
    stmt->setUInt16(2, mapId);
    stmt->setFloat (3, x);
    stmt->setFloat (4, y);
    stmt->setFloat (5, z);
    stmt->setUInt32(6, GetLatency());
    stmt->setUInt32(7, time(NULL));
    CharacterDatabase.Execute(stmt);
}

void WorldSession::HandleGMResponseResolve(WorldPacket& /*recvPacket*/)
{
    // empty packet
    if (GmTicket* ticket = sTicketMgr->GetTicketByPlayer(GetPlayer()->GetGUID()))
    {
        uint8 getSurvey = 0;
        if (float(rand_chance()) < sWorld->getFloatConfig(CONFIG_CHANCE_OF_GM_SURVEY))
            getSurvey = 1;

        WorldPacket data(SMSG_GMRESPONSE_STATUS_UPDATE, 4);
        data << uint8(getSurvey);
        SendPacket(&data);

        WorldPacket data2(SMSG_GMTICKET_DELETETICKET, 4);
        data2 << uint32(GMTICKET_RESPONSE_TICKET_DELETED);
        SendPacket(&data2);

        sTicketMgr->CloseTicket(ticket->GetId(), GetPlayer()->GetGUID());
        sTicketMgr->SendTicket(this, NULL);
    }
}
