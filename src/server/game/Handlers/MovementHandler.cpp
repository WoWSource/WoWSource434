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

#include "AnticheatMgr.h"
#include "Common.h"
#include "WorldPacket.h"
#include "WorldSession.h"
#include "Opcodes.h"
#include "Log.h"
#include "Corpse.h"
#include "Player.h"
#include "SpellAuras.h"
#include "MapManager.h"
#include "Transport.h"
#include "Battleground.h"
#include "WaypointMovementGenerator.h"
#include "InstanceSaveMgr.h"
#include "ObjectMgr.h"
#include "Chat.h"
#include "MovementStructures.h"
#include "Vehicle.h" 

#define MOVEMENT_PACKET_TIME_DELAY 0 

void WorldSession::HandleMoveWorldportAckOpcode(WorldPacket&)
{
	sLog->outDebug(LOG_FILTER_NETWORKIO, "WORLD: got MSG_MOVE_WORLDPORT_ACK.");
    HandleMoveWorldportAckOpcode();
}

void WorldSession::HandleMoveWorldportAckOpcode()
{
    // ignore unexpected far teleports
    if (!GetPlayer()->IsBeingTeleportedFar())
        return;

    GetPlayer()->SetSemaphoreTeleportFar(false);

	// get the teleport destination
    WorldLocation const& loc = GetPlayer()->GetTeleportDest();

    // possible errors in the coordinate validity check
    if (!MapManager::IsValidMapCoord(loc))
    {
        LogoutPlayer(false);
        return;
    }

    // get the destination map entry, not the current one, this will fix homebind and reset greeting
    MapEntry const* mEntry = sMapStore.LookupEntry(loc.GetMapId());
    InstanceTemplate const* mInstance = sObjectMgr->GetInstanceTemplate(loc.GetMapId());

    // reset instance validity, except if going to an instance inside an instance
    if (GetPlayer()->m_InstanceValid == false && !mInstance)
        GetPlayer()->m_InstanceValid = true;

    Map* oldMap = GetPlayer()->GetMap();
	Map* newMap = sMapMgr->CreateMap(loc.GetMapId(), GetPlayer());
    if (GetPlayer()->IsInWorld())
    {
		if (sLog != NULL && oldMap != NULL && oldMap->GetId() != NULL && newMap != NULL && newMap->GetMapName() != NULL
			&& loc.GetMapId() != NULL){
			sLog->outError(LOG_FILTER_NETWORKIO, "%u is still in world when teleported from map %u to new map %u",
				oldMap->GetId(), newMap ? newMap->GetMapName() : "Unknown", loc.GetMapId());
			oldMap->RemovePlayerFromMap(GetPlayer(), false);
		}
    }

    // relocate the player to the teleport destination
    // the CanEnter checks are done in TeleporTo but conditions may change
    // while the player is in transit, for example the map may get full
    if (!newMap || !newMap->CanEnter(GetPlayer()))
    {
        sLog->outError(LOG_FILTER_NETWORKIO, "Map %d could not be created for player %d, porting player to homebind", loc.GetMapId(), GetPlayer()->GetGUIDLow());
        GetPlayer()->TeleportTo(GetPlayer()->m_homebindMapId, GetPlayer()->m_homebindX, GetPlayer()->m_homebindY, GetPlayer()->m_homebindZ, GetPlayer()->GetOrientation());
        return;
    }
    
	float z = loc.GetPositionZ();
	if (GetPlayer()->HasUnitMovementFlag(MOVEMENTFLAG_HOVER))
		z += GetPlayer()->GetFloatValue(UNIT_FIELD_HOVERHEIGHT);
	GetPlayer()->Relocate(loc.GetPositionX(), loc.GetPositionY(), z, loc.GetOrientation());

    GetPlayer()->ResetMap();
    GetPlayer()->SetMap(newMap);

    GetPlayer()->SendInitialPacketsBeforeAddToMap();
    if (!GetPlayer()->GetMap()->AddPlayerToMap(GetPlayer()))
    {
        sLog->outError(LOG_FILTER_NETWORKIO, "WORLD: failed to teleport player %s (%d) to map %d because of unknown reason!",
            GetPlayer()->GetName().c_str(), GetPlayer()->GetGUIDLow(), loc.GetMapId());
        GetPlayer()->ResetMap();
        GetPlayer()->SetMap(oldMap);
        GetPlayer()->TeleportTo(GetPlayer()->m_homebindMapId, GetPlayer()->m_homebindX, GetPlayer()->m_homebindY, GetPlayer()->m_homebindZ, GetPlayer()->GetOrientation());
        return;
    }

    if (_player->InBattleground())
    {
        if (!mEntry->IsBattlegroundOrArena())
        {
            // remove bg team and id when leaving bg
            _player->SetBattlegroundId(0, BATTLEGROUND_TYPE_NONE);
            _player->SetBGTeam(0);
        }
        else if (Battleground* bg = _player->GetBattleground())
        {
            if (_player->IsInvitedForBattlegroundInstance(_player->GetBattlegroundId()))
                bg->AddPlayer(_player);
        }
    }

    GetPlayer()->SendInitialPacketsAfterAddToMap();

    // flight fast teleport case
    if (GetPlayer()->GetMotionMaster()->GetCurrentMovementGeneratorType() == FLIGHT_MOTION_TYPE)
    {
        if (!_player->InBattleground())
        {
            // short preparations to continue flight
            FlightPathMovementGenerator* flight = (FlightPathMovementGenerator*)(GetPlayer()->GetMotionMaster()->top());
            flight->Initialize(GetPlayer());
            return;
        }

        // battleground state prepare, stop flight
        GetPlayer()->GetMotionMaster()->MovementExpired();
        GetPlayer()->CleanupAfterTaxiFlight();
    }

    // resurrect character at enter into instance where his corpse exist after add to map
    Corpse* corpse = GetPlayer()->GetCorpse();
    if (corpse && corpse->GetType() != CORPSE_BONES && corpse->GetMapId() == GetPlayer()->GetMapId() && mEntry->IsDungeon())
    {
        GetPlayer()->ResurrectPlayer(0.5f, false);
        GetPlayer()->SpawnCorpseBones();
    }

    bool allowMount = !mEntry->IsDungeon() || mEntry->IsBattlegroundOrArena();
    if (mInstance)
    {
        Difficulty diff = GetPlayer()->GetDifficulty(mEntry->IsRaid());
        if (MapDifficulty const* mapDiff = GetMapDifficultyData(mEntry->MapID, diff))
        {
            if (mapDiff->resetTime)
            {
                if (time_t timeReset = sInstanceSaveMgr->GetResetTimeFor(mEntry->MapID, diff))
                {
                    uint32 timeleft = uint32(timeReset - time(NULL));
                    GetPlayer()->SendInstanceResetWarning(mEntry->MapID, diff, timeleft);
                }
            }
        }
        allowMount = mInstance->AllowMount;
    }

    if (!allowMount)
        _player->RemoveAurasByType(SPELL_AURA_MOUNTED);

    // update zone immediately, otherwise leave channel will cause crash in mtmap
    uint32 newzone, newarea;
    GetPlayer()->GetZoneAndAreaId(newzone, newarea);
    GetPlayer()->UpdateZone(newzone, newarea);

    if (GetPlayer()->pvpInfo.inHostileArea)
        GetPlayer()->CastSpell(GetPlayer(), 2479, true); // honorless target
    else if (GetPlayer()->IsPvP() && !GetPlayer()->HasFlag(PLAYER_FLAGS, PLAYER_FLAGS_IN_PVP))
        GetPlayer()->UpdatePvP(false, false);

    GetPlayer()->ResummonPetTemporaryUnSummonedIfAny();
    GetPlayer()->ProcessDelayedOperations();
}

void WorldSession::HandleMoveTeleportAck(WorldPacket& recvPacket)
{
	sLog->outDebug(LOG_FILTER_NETWORKIO, "MSG_MOVE_TELEPORT_ACK");
    ObjectGuid guid;
    uint32 ackCount = recvPacket.read<uint32>();
    recvPacket.read_skip<uint32>(); // should be new position in kind of struction

    guid[5] = recvPacket.ReadBit();
    guid[0] = recvPacket.ReadBit();
    guid[1] = recvPacket.ReadBit();
    guid[6] = recvPacket.ReadBit();
    guid[3] = recvPacket.ReadBit();
    guid[7] = recvPacket.ReadBit();
    guid[2] = recvPacket.ReadBit();
    guid[4] = recvPacket.ReadBit();

    recvPacket.ReadByteSeq(guid[4]);
    recvPacket.ReadByteSeq(guid[2]);
    recvPacket.ReadByteSeq(guid[7]);
    recvPacket.ReadByteSeq(guid[6]);
    recvPacket.ReadByteSeq(guid[5]);
    recvPacket.ReadByteSeq(guid[1]);
    recvPacket.ReadByteSeq(guid[3]);
    recvPacket.ReadByteSeq(guid[0]);

    // Skip old result
    if (_player->m_movement_ack[ACK_TELEPORT] != ackCount)
        return;

    Player* plMover = _player->m_mover->ToPlayer();

    if (!plMover || !plMover->IsBeingTeleportedNear() || guid != plMover->GetGUID())
        return;

    plMover->SetSemaphoreTeleportNear(false);

    uint32 old_zone = plMover->GetZoneId();

    WorldLocation const& dest = plMover->GetTeleportDest();
    plMover->UpdatePosition(dest, true);

    WorldPacket data(SMSG_MOVE_UPDATE_TELEPORT);
    MovementInfo info = MovementInfo(plMover->m_movementInfo);
    info.pos.Relocate(dest.m_positionX, dest.m_positionY, dest.m_positionZ, dest.m_orientation);
    info.guid = plMover->GetGUID();
    info.time = getMSTime();
    info.WriteToPacket(data);
    _player->SendMessageToSet(&data, _player);

    uint32 newzone, newarea;
    plMover->GetZoneAndAreaId(newzone, newarea);
    plMover->UpdateZone(newzone, newarea);

    if (old_zone != newzone)
    {
        if (plMover->pvpInfo.inHostileArea)
            plMover->CastSpell(plMover, 2479, true); // honorless target
        else if (plMover->IsPvP() && !plMover->HasFlag(PLAYER_FLAGS, PLAYER_FLAGS_IN_PVP))
            plMover->UpdatePvP(false, false);
    }

    GetPlayer()->ResummonPetTemporaryUnSummonedIfAny();
    GetPlayer()->ProcessDelayedOperations();
}

void WorldSession::HandleSetActiveMover(WorldPacket& recvPacket)
{
    ObjectGuid guid;

    guid[7] = recvPacket.ReadBit();
    guid[2] = recvPacket.ReadBit();
    guid[1] = recvPacket.ReadBit();
    guid[0] = recvPacket.ReadBit();
    guid[4] = recvPacket.ReadBit();
    guid[5] = recvPacket.ReadBit();
    guid[6] = recvPacket.ReadBit();
    guid[3] = recvPacket.ReadBit();

    recvPacket.ReadByteSeq(guid[3]);
    recvPacket.ReadByteSeq(guid[2]);
    recvPacket.ReadByteSeq(guid[4]);
    recvPacket.ReadByteSeq(guid[0]);
    recvPacket.ReadByteSeq(guid[5]);
    recvPacket.ReadByteSeq(guid[1]);
    recvPacket.ReadByteSeq(guid[6]);
    recvPacket.ReadByteSeq(guid[7]);

    if (_player->m_mover->GetGUID() != guid)
        return;
}

void WorldSession::HandleMovementOpcodes(WorldPacket& recvPacket)
{
    MovementInfo info;
    info.ReadFromPacket(recvPacket);

    //Stop Emote
    if (GetPlayer()->GetUInt32Value(UNIT_NPC_EMOTESTATE))
        GetPlayer()->HandleEmoteCommand(0);

    HandleMovementInfo(info, recvPacket.GetOpcode(), recvPacket.size(), _player->m_mover);
}

bool WorldSession::CanMovementBeProcessed(uint16 opcode)
{
    switch (opcode)
    {
        case CMSG_MOVE_NOT_ACTIVE_MOVER:
        case CMSG_FORCE_MOVE_ROOT_ACK:
        case MSG_MOVE_STOP:
            return true;
    }

    return false;
}

bool WorldSession::HandleMovementInfo(MovementInfo &movementInfo, const uint16 opcode, const size_t packSize, Unit *mover)
{
    ASSERT(mover != NULL);

    if (movementInfo.guid != mover->GetGUID() || !movementInfo.pos.IsPositionValid())
        return false;

    Player* plrMover = mover->ToPlayer();

    if (plrMover && plrMover->IsBeingTeleported())
        return false;

    if (plrMover && plrMover->HasUnitState(UNIT_STATE_CONTROLLED_BY_SERVER) && !CanMovementBeProcessed(opcode))
        return false;

    movementInfo.Sanitize(mover);

    if (movementInfo.t_guid)
    {
        if (movementInfo.t_pos.GetPositionX() > 50.0f || movementInfo.t_pos.GetPositionY() > 50.0f || movementInfo.t_pos.GetPositionZ() > 50.0f)
            return false; // moved out of transport size

        if (!Trinity::IsValidMapCoord(
                movementInfo.pos.GetPositionX() + movementInfo.t_pos.GetPositionX(),
                movementInfo.pos.GetPositionY() + movementInfo.t_pos.GetPositionY(),
                movementInfo.pos.GetPositionZ() + movementInfo.t_pos.GetPositionZ(),
                movementInfo.pos.GetOrientation() + movementInfo.t_pos.GetOrientation())
            )
            return false; // just bad coords

        if (plrMover)
        {
            if (!plrMover->GetTransport()) // didn't have transport. now have
            {
                for (MapManager::TransportSet::const_iterator iter = sMapMgr->m_Transports.begin(); iter != sMapMgr->m_Transports.end(); ++iter)
                {
                    if ((*iter)->GetGUID() == movementInfo.t_guid)
                    {
                        plrMover->m_transport = *iter;
                        (*iter)->AddPassenger(plrMover);
                        break;
                    }
                }
            }
            else if (plrMover->GetTransport()->GetGUID() != movementInfo.t_guid) // changes transport
            {
                bool foundNewTransport = false;
                plrMover->m_transport->RemovePassenger(plrMover);
                for (MapManager::TransportSet::const_iterator iter = sMapMgr->m_Transports.begin(); iter != sMapMgr->m_Transports.end(); ++iter)
                {
                    if ((*iter)->GetGUID() == movementInfo.t_guid)
                    {
                        foundNewTransport = true;
                        plrMover->m_transport = *iter;
                        (*iter)->AddPassenger(plrMover);
                        break;
                    }
                }

                if (!foundNewTransport)
                {
                    plrMover->m_transport = NULL;
                    movementInfo.t_pos.Relocate(0.0f, 0.0f, 0.0f, 0.0f);
                    movementInfo.t_time = 0;
                    movementInfo.t_seat = -1;
                }
            }
            else if (movementInfo.pos.m_positionX != movementInfo.t_pos.m_positionX)
                plrMover->GetTransport()->UpdatePosition(&movementInfo);
        }

        if (!mover->GetTransport() && !mover->GetVehicle())
        {
            GameObject* go = mover->GetMap()->GetGameObject(movementInfo.t_guid);
            if (!go || go->GetGoType() != GAMEOBJECT_TYPE_TRANSPORT)
                movementInfo.t_guid = 0;
        }
    }
    else if (plrMover && plrMover->GetTransport()) // had transport, no loger have
    {
        plrMover->m_transport->RemovePassenger(plrMover);
        plrMover->m_transport = NULL;
        movementInfo.t_pos.Relocate(0.0f, 0.0f, 0.0f, 0.0f);
        movementInfo.t_time = 0;
        movementInfo.t_seat = -1;
    }

    if (opcode == MSG_MOVE_FALL_LAND && plrMover && !plrMover->isInFlight())
        plrMover->HandleFall(movementInfo);

    if (plrMover && ((movementInfo.flags & MOVEMENTFLAG_SWIMMING) != 0) != plrMover->IsInWater())
    {
        // now client not include swimming flag in case jumping under water
        plrMover->SetInWater(!plrMover->IsInWater() || plrMover->GetBaseMap()->IsUnderWater(movementInfo.pos.GetPositionX(), movementInfo.pos.GetPositionY(), movementInfo.pos.GetPositionZ()));
    }
	
	if (plrMover)
        sAnticheatMgr->StartHackDetection(plrMover, movementInfo, opcode);

    WorldPacket data(movementInfo.GetSMSGOpcodeForCMSG(Opcodes(opcode)), packSize);
    movementInfo.time = getMSTime();
    movementInfo.guid = mover->GetGUID();
    movementInfo.WriteToPacket(data);
    _player->SendMessageToSet(&data, _player);

    mover->m_movementInfo = movementInfo;

    if (mover->GetVehicle()) // this is almost never true (not sure why it is sometimes, but it is), normally use mover->IsVehicle()
    {
        mover->SetOrientation(movementInfo.pos.GetOrientation());
        return true;
    }

    // Remove pet when starting to fly
    if (plrMover && plrMover->IsFlying())
        plrMover->UnsummonPetTemporaryIfAny();

    mover->UpdatePosition(movementInfo.pos);

    if (plrMover)
    {
        plrMover->UpdateFallInformationIfNeed(movementInfo, opcode);
        float underMapValueZ, upperLimitValueZ;
        bool check = false;
        switch (plrMover->GetMapId())
        {
            case 617: // Dalaran Arena
                underMapValueZ = 3.0f;
                upperLimitValueZ = 30.0f;
                break;
            case 562: // Blades Edge Arena
                underMapValueZ = -1.0f;
                upperLimitValueZ = 22.0f;
                break;
            case 559: // Nagrand Arena
                underMapValueZ = -1.0f;
                upperLimitValueZ = 21.0f;
                break;
            case 572: // Ruins of Lordaeron
                underMapValueZ = -1.0f;
                upperLimitValueZ = 45.0f;
                break;
            case 618: // Ring of Valor
                underMapValueZ = 28.0f;
                upperLimitValueZ = 60.0f;
                break;
            case 566: // Eye of the storm
                underMapValueZ = 1000.0f;
                upperLimitValueZ = MAX_HEIGHT;
                break;
            default:
                AreaTableEntry const* zone = GetAreaEntryByAreaID(plrMover->GetAreaId());
                underMapValueZ = zone ? zone->MaxDepth : -500.0f;
                upperLimitValueZ = MAX_HEIGHT;
                break;
        }

        check = movementInfo.pos.GetPositionZ() < underMapValueZ || movementInfo.pos.GetPositionZ() > upperLimitValueZ;
        if (check && plrMover->isAlive() && !(plrMover->GetBattleground() && plrMover->GetBattleground()->HandlePlayerUnderMap(_player)))
        {
            plrMover->EnvironmentalDamage(DAMAGE_FALL_TO_VOID, GetPlayer()->GetMaxHealth());
            if (!plrMover->isAlive())
                plrMover->KillPlayer(); // prevent death timer
            plrMover->RepopAtGraveyard();
        }
    }

    return true;
}

void WorldSession::HandleMovementSpeedChangeAck(WorldPacket& recvPacket)
{
    UnitMoveType moveType;
    bool updatePacketNotKnown = false;

    switch (recvPacket.GetOpcode())
    {
        case CMSG_MOVE_FORCE_WALK_SPEED_CHANGE_ACK: moveType = MOVE_WALK; break;
        case CMSG_MOVE_FORCE_RUN_SPEED_CHANGE_ACK: moveType = MOVE_RUN; break;
        case CMSG_MOVE_FORCE_RUN_BACK_SPEED_CHANGE_ACK: moveType = MOVE_RUN_BACK; break;
        case CMSG_MOVE_FORCE_SWIM_SPEED_CHANGE_ACK: moveType = MOVE_SWIM; break;
        case CMSG_MOVE_FORCE_SWIM_BACK_SPEED_CHANGE_ACK: moveType = MOVE_SWIM_BACK; break;
        case CMSG_MOVE_FORCE_TURN_RATE_CHANGE_ACK: moveType = MOVE_TURN_RATE; updatePacketNotKnown = true; break;
        case CMSG_MOVE_FORCE_FLIGHT_SPEED_CHANGE_ACK: moveType = MOVE_FLIGHT; break;
        case CMSG_MOVE_FORCE_FLIGHT_BACK_SPEED_CHANGE_ACK: moveType = MOVE_FLIGHT_BACK; updatePacketNotKnown = true; break;
        case CMSG_MOVE_FORCE_PITCH_RATE_CHANGE_ACK: moveType = MOVE_PITCH_RATE; updatePacketNotKnown = true; break;
        default:
            sLog->outInfo(LOG_FILTER_BAD_OPCODE_HANDLER, "WorldSession::HandleMovementSpeedChangeAck for unknown opcode: %s", GetOpcodeNameForLogging(recvPacket.GetOpcode()).c_str());
            return;
    }

    MovementInfo info;
    info.ReadFromPacket(recvPacket);

    // skip old result
    if (_player->m_forced_speed_changes[moveType] != info.ackCount)
    {
        recvPacket.rfinish();
        return;
    }
        
    if (!_player->GetTransport() && fabs(_player->GetSpeed(moveType) - info.ackSpeed) > 0.01f)
    {
        if (_player->GetSpeed(moveType) > info.ackSpeed)
            _player->SetSpeed(moveType, _player->GetSpeedRate(moveType), true);

        recvPacket.rfinish();
        return;
    }

    // TODO: Find all structure (remove me)
    if (updatePacketNotKnown)
    {
        WorldPacket data;
        switch (moveType)
        {
            case MOVE_TURN_RATE:
                data.Initialize(SMSG_MOVE_SET_TURN_RATE, 1 + 8 + 4 + 4);
                info.WriteToPacket(data);
                _player->SendMessageToSet(&data, _player);
                break;
            case MOVE_FLIGHT_BACK:
                data.Initialize(SMSG_MOVE_SET_FLIGHT_BACK_SPEED, 1 + 8 + 4 + 4);
                info.WriteToPacket(data);
                _player->SendMessageToSet(&data, _player);
                break;
            case MOVE_PITCH_RATE:
                data.Initialize(SMSG_MOVE_SET_PITCH_RATE, 1 + 8 + 4 + 4);
                info.WriteToPacket(data);
                _player->SendMessageToSet(&data, _player);
                break;
            default:
                break;
        }
    }

    HandleMovementInfo(info, recvPacket.GetOpcode(), recvPacket.size()-4, _player->m_mover);
}

void WorldSession::HandleMovementGravityAck(WorldPacket& recvPacket)
{
    MovementInfo info;
    info.ReadFromPacket(recvPacket);
    
    // skip old result
    if (_player->m_movement_ack[ACK_GRAVITY] != info.ackCount)
    {
        recvPacket.rfinish();
        return;
    }

    HandleMovementInfo(info, recvPacket.GetOpcode(), recvPacket.size()-4, _player->m_mover);
}

void WorldSession::HandleMovementHoverAck(WorldPacket& recvPacket)
{
    MovementInfo info;
    info.ReadFromPacket(recvPacket);
    
    // skip old result
    if (_player->m_movement_ack[ACK_HOVER] != info.ackCount)
    {
        recvPacket.rfinish();
        return;
    }

    HandleMovementInfo(info, recvPacket.GetOpcode(), recvPacket.size()-4, _player->m_mover);
}

void WorldSession::HandleMovementWaterWalkAck(WorldPacket& recvPacket)
{
    MovementInfo info;
    info.ReadFromPacket(recvPacket);
    
    // skip old result
    if (_player->m_movement_ack[ACK_WATER_WALK] != info.ackCount)
    {
        recvPacket.rfinish();
        return;
    }

    HandleMovementInfo(info, recvPacket.GetOpcode(), recvPacket.size()-4, _player->m_mover);
}

void WorldSession::HandleMovementCanFlyAck(WorldPacket& recvPacket)
{
    MovementInfo info;
    info.ReadFromPacket(recvPacket);
    
    // skip old result
    if (_player->m_movement_ack[ACK_CAN_FLY] != info.ackCount)
    {
        recvPacket.rfinish();
        return;
    }

    HandleMovementInfo(info, recvPacket.GetOpcode(), recvPacket.size()-4, _player->m_mover);
}

void WorldSession::HandleMovementCollisionHeightAck(WorldPacket& recvPacket)
{
    MovementInfo info;
    info.ReadFromPacket(recvPacket);
    
    // skip old result
    if (_player->m_movement_ack[ACK_COLLISION_HEIGHT] != info.ackCount)
    {
        recvPacket.rfinish();
        return;
    }

    HandleMovementInfo(info, recvPacket.GetOpcode(), recvPacket.size()-4, _player->m_mover);
}

void WorldSession::HandleMovementFeatherFallAck(WorldPacket& recvPacket)
{
    MovementInfo info;
    info.ReadFromPacket(recvPacket);
    
    // skip old result
    if (_player->m_movement_ack[ACK_FEATHER_FALL] != info.ackCount)
    {
        recvPacket.rfinish();
        return;
    }

    HandleMovementInfo(info, recvPacket.GetOpcode(), recvPacket.size()-4, _player->m_mover);
}

void WorldSession::HandleMovementRootAck(WorldPacket& recvPacket)
{
    MovementInfo info;
    info.ReadFromPacket(recvPacket);
    
    // skip old result
    if (_player->m_movement_ack[ACK_ROOT] != info.ackCount)
    {
        recvPacket.rfinish();
        return;
    }

    HandleMovementInfo(info, recvPacket.GetOpcode(), recvPacket.size()-4, _player->m_mover);
}

void WorldSession::HandleMovementUnrootAck(WorldPacket& recvPacket)
{
    MovementInfo info;
    info.ReadFromPacket(recvPacket);
    
    // skip old result
    if (_player->m_movement_ack[ACK_UNROOT] != info.ackCount)
    {
        recvPacket.rfinish();
        return;
    }

    HandleMovementInfo(info, recvPacket.GetOpcode(), recvPacket.size()-4, _player->m_mover);
}

void WorldSession::HandleMoveKnockBackAck(WorldPacket& recvData)
{
    sLog->outError(LOG_FILTER_NETWORKIO, "CMSG_MOVE_KNOCK_BACK_ACK");

    MovementInfo movementInfo;
    GetPlayer()->ReadMovementInfo(recvData, &movementInfo);

    if (_player->m_mover->GetGUID() != movementInfo.guid)
        return;

    _player->m_movementInfo = movementInfo;

    WorldPacket data(SMSG_MOVE_UPDATE_KNOCK_BACK, 66);
    _player->WriteMovementInfo(data);
    _player->SendMessageToSet(&data, false);
}

void WorldSession::HandleMoveHoverAck(WorldPacket& recvPacket)
{
	sLog->outDebug(LOG_FILTER_NETWORKIO, "CMSG_MOVE_HOVER_ACK");

	MovementInfo info;
	info.ReadFromPacket(recvPacket);

	recvPacket.read_skip<uint32>();                           // unk

	MovementInfo movementInfo;
	GetPlayer()->ReadMovementInfo(recvPacket, &movementInfo);

	recvPacket.read_skip<uint32>();                           // unk2
}

void WorldSession::HandleMoveWaterWalkAck(WorldPacket& recvPacket)
{
	sLog->outDebug(LOG_FILTER_NETWORKIO, "CMSG_MOVE_WATER_WALK_ACK");

	MovementInfo info;
	info.ReadFromPacket(recvPacket);

	recvPacket.read_skip<uint32>();                           // unk

	MovementInfo movementInfo;
	GetPlayer()->ReadMovementInfo(recvPacket, &movementInfo);

	recvPacket.read_skip<uint32>();                           // unk2
}

void WorldSession::HandleSetCollisionHeightAck(WorldPacket& recvPacket)
{
	sLog->outDebug(LOG_FILTER_NETWORKIO, "CMSG_MOVE_SET_COLLISION_HEIGHT_ACK");

	static MovementStatusElements const heightElement = MSEExtraFloat;
	Movement::ExtraMovementStatusElement extra(&heightElement);
	MovementInfo movementInfo;
	GetPlayer()->ReadMovementInfo(recvPacket, &movementInfo, &extra);
}
