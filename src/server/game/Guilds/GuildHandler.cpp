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

#include "Common.h"
#include "WorldPacket.h"
#include "WorldSession.h"
#include "World.h"
#include "ObjectMgr.h"
#include "GuildMgr.h"
#include "Log.h"
#include "Opcodes.h"
#include "Guild.h"
#include "GossipDef.h"
#include "SocialMgr.h"
void WorldSession::HandleGuildAchievementMembers(WorldPacket& recvPacket)
{
    uint32 achievementId;
    ObjectGuid guildGuid;
    ObjectGuid playerGuid;

    recvPacket >> achievementId;

    guildGuid[0] = recvPacket.ReadBit();
    playerGuid[5] = recvPacket.ReadBit();
    playerGuid[4] = recvPacket.ReadBit();
    playerGuid[7] = recvPacket.ReadBit();
    playerGuid[0] = recvPacket.ReadBit();
    guildGuid[5] = recvPacket.ReadBit();
    guildGuid[7] = recvPacket.ReadBit();
    playerGuid[3] = recvPacket.ReadBit();
    guildGuid[3] = recvPacket.ReadBit();
    playerGuid[2] = recvPacket.ReadBit();
    guildGuid[4] = recvPacket.ReadBit();
    guildGuid[1] = recvPacket.ReadBit();
    guildGuid[6] = recvPacket.ReadBit();
    playerGuid[6] = recvPacket.ReadBit();
    guildGuid[2] = recvPacket.ReadBit();
    playerGuid[1] = recvPacket.ReadBit();

    recvPacket.ReadByteSeq(guildGuid[0]);
    recvPacket.ReadByteSeq(guildGuid[3]);
    recvPacket.ReadByteSeq(playerGuid[2]);
    recvPacket.ReadByteSeq(guildGuid[7]);
    recvPacket.ReadByteSeq(guildGuid[2]);
    recvPacket.ReadByteSeq(playerGuid[5]);
    recvPacket.ReadByteSeq(playerGuid[0]);
    recvPacket.ReadByteSeq(playerGuid[3]);
    recvPacket.ReadByteSeq(guildGuid[5]);
    recvPacket.ReadByteSeq(guildGuid[1]);
    recvPacket.ReadByteSeq(playerGuid[4]);
    recvPacket.ReadByteSeq(playerGuid[1]);
    recvPacket.ReadByteSeq(playerGuid[6]);
    recvPacket.ReadByteSeq(playerGuid[7]);
    recvPacket.ReadByteSeq(guildGuid[4]);
    recvPacket.ReadByteSeq(guildGuid[6]);

    sLog->outDebug(LOG_FILTER_NETWORKIO, "WORLD: Received CMSG_GUILD_ACHIEVEMENT_MEMBERS");

    if(Guild* pGuild = sGuildMgr->GetGuildByGuid(guildGuid))
    {
        if(pGuild->GetAchievementMgr().HasAchieved(achievementId))
        {
            ObjectGuid gguid = pGuild->GetGUID();
            CompletedAchievementData* achievement = pGuild->GetAchievementMgr().GetCompletedDataForAchievement(achievementId);
            WorldPacket data(SMSG_GUILD_ACHIEVEMENT_MEMBERS);

            data.WriteBit(gguid[3]);
            data.WriteBit(gguid[4]);
            data.WriteBit(gguid[7]);
            data.WriteBit(gguid[0]);

            data.WriteBits(achievement->guids.size(),26);

            for(std::set<uint64>::iterator itr = achievement->guids.begin(); itr != achievement->guids.end(); ++itr)
            {
                ObjectGuid pguid = (*itr);

                data.WriteBit(pguid[3]);
                data.WriteBit(pguid[1]);
                data.WriteBit(pguid[4]);
                data.WriteBit(pguid[5]);
                data.WriteBit(pguid[7]);
                data.WriteBit(pguid[0]);
                data.WriteBit(pguid[6]);
                data.WriteBit(pguid[2]);
            }

            data.WriteBit(gguid[2]);
            data.WriteBit(gguid[6]);
            data.WriteBit(gguid[5]);
            data.WriteBit(gguid[1]);

            data.FlushBits();

            data.WriteByteSeq(gguid[5]);

            for(std::set<uint64>::iterator itr = achievement->guids.begin(); itr != achievement->guids.end(); ++itr)
            {
                ObjectGuid pguid = (*itr);

                data.WriteByteSeq(pguid[1]);
                data.WriteByteSeq(pguid[5]);
                data.WriteByteSeq(pguid[7]);
                data.WriteByteSeq(pguid[0]);
                data.WriteByteSeq(pguid[6]);
                data.WriteByteSeq(pguid[4]);
                data.WriteByteSeq(pguid[3]);
                data.WriteByteSeq(pguid[2]);
            }

            data.WriteByteSeq(gguid[7]);
            data.WriteByteSeq(gguid[2]);
            data.WriteByteSeq(gguid[4]);
            data.WriteByteSeq(gguid[3]);
            data.WriteByteSeq(gguid[6]);
            data.WriteByteSeq(gguid[0]);

            data << achievementId;

            data.WriteByteSeq(gguid[1]);

            SendPacket(&data);
        }
    }
}

void WorldSession::HandleGuildSwitchRankOpcode(WorldPacket& recvPacket)
{
    uint32 rankId;
    bool up;

    recvPacket >> rankId;
    up = recvPacket.ReadBit();

    sLog->outDebug(LOG_FILTER_NETWORKIO,"WORLD: Received CMSG_GUILD_SWITCH_RANK rank %u up %u", rankId, up);

    Guild* guild = GetPlayer()->GetGuild();
    if (!guild)
    {
        Guild::SendCommandResult(this, GUILD_COMMAND_CREATE, ERR_GUILD_PLAYER_NOT_IN_GUILD);
        return;
    }

    if (GetPlayer()->GetGUID() != guild->GetLeaderGUID())
    {
        Guild::SendCommandResult(this, GUILD_COMMAND_INVITE, ERR_GUILD_PERMISSIONS);
        return;
    }

    guild->SwitchRank(rankId, up);
    guild->HandleRoster();
    guild->HandleQuery(GetPlayer()->GetSession());
    guild->SendGuildRankInfo(GetPlayer()->GetSession());
}

void WorldSession::HandleGuildRenameRequest(WorldPacket& recvPacket)
{
    uint32 lenght;
    std::string newName;

    lenght = recvPacket.ReadBits(8);
    newName = recvPacket.ReadString(lenght);

    Guild* pGuild = GetPlayer()->GetGuild();

    if(pGuild)
    {
        PreparedStatement* stmt = CharacterDatabase.GetPreparedStatement(CHAR_UPD_GUILD_NAME);

        _guildRenameCallback.SetParam(newName);

        stmt->setUInt32(1,pGuild->GetId());
        stmt->setString(0,newName);

        _guildRenameCallback.SetFutureResult(CharacterDatabase.AsyncQuery(stmt));

        WorldPacket data(SMSG_GUILD_FLAGGED_FOR_RENAME,1);

        data.WriteBit(true);            // it may send false also, but we don't know how to handle exeptions by the DB layer
        data.FlushBits();

        pGuild->BroadcastPacket(&data);
    }
}

void WorldSession::HandleGuildChallengeRequest(WorldPacket& recvPacket)
{
    uint8 counter = 4;

    if(Guild* pGuild = GetPlayer()->GetGuild())
    {
        if (Guild::ChallengesMgr* challengesMgr = pGuild->GetChallengesMgr())
        {
            // First check if it's time to reset the challenges.
            time_t thisTime = time(NULL);
            if (pGuild->GetChallengesMgr()->CompletedFirstChallenge(pGuild->GetId()) && pGuild->GetChallengesMgr()->GetFirstCompletedChallengeTime(pGuild->GetId()) + WEEK <= thisTime)
                pGuild->GetChallengesMgr()->ResetWeeklyChallenges();

            WorldPacket data(SMSG_GUILD_CHALLENGE_UPDATED,5*4*4);

            //Guild Experience Reward block

            data << uint32(0);                                                                      //in the block its always 1
            data << challengesMgr->GetXPRewardForType(CHALLENGE_TYPE_DUNGEON);                      //dungeon
            data << challengesMgr->GetXPRewardForType(CHALLENGE_TYPE_RAID);                         //raid
            data << challengesMgr->GetXPRewardForType(CHALLENGE_TYPE_RATEDBG);                      //rated BG

            //Gold Bonus block
            data << uint32(0);                                                                      //in the block its always 1
            data << challengesMgr->GetGoldBonusForType(CHALLENGE_TYPE_DUNGEON);                     //dungeon
            data << challengesMgr->GetGoldBonusForType(CHALLENGE_TYPE_RAID);                        //raid
            data << challengesMgr->GetGoldBonusForType(CHALLENGE_TYPE_RATEDBG);                     //rated BG

            //Total Count block

            data << uint32(0);                                                                      //in the block its always 1
            data << challengesMgr->GetTotalCountFor(CHALLENGE_TYPE_DUNGEON);                        //dungeon
            data << challengesMgr->GetTotalCountFor(CHALLENGE_TYPE_RAID);                           //raid
            data << challengesMgr->GetTotalCountFor(CHALLENGE_TYPE_RATEDBG);                        //rated BG            

            //Completion Gold Reward block

            data << uint32(0);                                                                      //in the block its always 1
            data << challengesMgr->GetGoldRewardForType(CHALLENGE_TYPE_DUNGEON);                    //dungeon
            data << challengesMgr->GetGoldRewardForType(CHALLENGE_TYPE_RAID);                       //raid
            data << challengesMgr->GetGoldRewardForType(CHALLENGE_TYPE_RATEDBG);                    //rated BG

            //Current Count block

            data << uint32(0);                                                                      //in the block its always 1
            data << challengesMgr->GetCurrentCountFor(CHALLENGE_TYPE_DUNGEON);                      //dungeon
            data << challengesMgr->GetCurrentCountFor(CHALLENGE_TYPE_RAID);                         //raid
            data << challengesMgr->GetCurrentCountFor(CHALLENGE_TYPE_RATEDBG);                      //rated BG

            SendPacket(&data);
        }
        return;
    }
}


void WorldSession::HandleGuildSwitchRank(WorldPacket& recvPacket)
{
	uint32 rank;
	bool direction; // if its true, then the rank rises, if no, it goes down
	recvPacket >> rank;
	direction = recvPacket.ReadBit();
	Guild* guild = GetPlayer()->GetGuild();
	if (!guild)
	{
		Guild::SendCommandResult(this, GUILD_COMMAND_CREATE, ERR_GUILD_PLAYER_NOT_IN_GUILD);
		return;
	}
	if (GetPlayer()->GetGUID() != guild->GetLeaderGUID())
	{
		Guild::SendCommandResult(this, GUILD_COMMAND_INVITE, ERR_GUILD_PERMISSIONS);
		return;
	}

  //  if(guild)
  //  {
    //    guild->MoveRank(rank,direction);
  //  }
}
