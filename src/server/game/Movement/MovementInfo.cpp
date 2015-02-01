#include "MovementInfo.h"
#include "MovementStructures.h"
#include "Player.h"
#include "WorldSession.h"


void MovementInfo::Write_SMSG_PLAYER_MOVE(ByteBuffer &buff)
{
    ObjectGuid id = guid;
    ObjectGuid tid = t_guid;
    bool hasOrientation = !G3D::fuzzyEq(pos.m_orientation, 0.0f);
    bool hasSplineElevation = !G3D::fuzzyEq(splineElevation, 0.0f);
    bool hasFallData = fallTime || j_zspeed;
    bool hasFallDirection = hasFallData && (j_cosAngle || j_sinAngle || j_xyspeed);
    bool hasPitch = !G3D::fuzzyEq(pitch, 0.0f);

    buff.WriteBit(hasFallData);
    buff.WriteBit(id[3]);
    buff.WriteBit(id[6]);
    buff.WriteBit(!flags2);
    buff.WriteBit(hasSpline);
    buff.WriteBit(!time);
    buff.WriteBit(id[0]);
    buff.WriteBit(id[1]);
    if (flags2)
        buff.WriteBits(flags2, 12);
    buff.WriteBit(id[7]);
    buff.WriteBit(!flags);
    buff.WriteBit(!hasOrientation);
    buff.WriteBit(id[2]);
    buff.WriteBit(!hasSplineElevation);
    buff.WriteBit(0); // unknown
    buff.WriteBit(id[4]);
    if (hasFallData)
        buff.WriteBit(hasFallDirection);
    buff.WriteBit(id[5]);
    buff.WriteBit(t_guid);
    if (flags)
        buff.WriteBits(flags, 30);
    if (t_guid)
    {
        buff.WriteBit(tid[3]);
        buff.WriteBit(t_time3);
        buff.WriteBit(tid[6]);
        buff.WriteBit(tid[1]);
        buff.WriteBit(tid[7]);
        buff.WriteBit(tid[0]);
        buff.WriteBit(tid[4]);
        buff.WriteBit(t_time2);
        buff.WriteBit(tid[5]);
        buff.WriteBit(tid[2]);
    }
    buff.WriteBit(!hasPitch);
    buff.WriteByteSeq(id[5]);
    if (hasFallData)
    {
        if (hasFallDirection)
        {
            buff << j_xyspeed;
            buff << j_cosAngle;
            buff << j_sinAngle;
        }
        buff << j_zspeed;
        buff << fallTime;
    }
    if (hasSplineElevation)
        buff << splineElevation;
    buff.WriteByteSeq(id[7]);
    buff << pos.m_positionY;
    buff.WriteByteSeq(id[3]);
    if (t_guid)
    {
        if (t_time3)
            buff << t_time3;
        buff.WriteByteSeq(tid[6]);
        buff << t_seat;
        buff.WriteByteSeq(tid[5]);
        buff << t_pos.m_positionX;
        buff.WriteByteSeq(tid[1]);
        buff << t_pos.m_orientation;
        buff.WriteByteSeq(tid[2]);
        if (t_time2)
            buff << t_time2;
        buff.WriteByteSeq(tid[0]);
        buff << t_pos.m_positionZ;
        buff.WriteByteSeq(tid[7]);
        buff.WriteByteSeq(tid[4]);
        buff.WriteByteSeq(tid[3]);
        buff << t_pos.m_positionY;
        buff << t_time;
    }
    buff.WriteByteSeq(id[4]);
    buff << pos.m_positionX;
    buff.WriteByteSeq(id[6]);
    buff << pos.m_positionZ;
    if (time)
        buff << time;
    buff.WriteByteSeq(id[2]);
    if (hasPitch)
        buff << pitch;
    buff.WriteByteSeq(id[0]);
    if (hasOrientation)
        buff << pos.m_orientation;
    buff.WriteByteSeq(id[1]);
}

void MovementInfo::Read_CMSG_MOVE_NOT_ACTIVE_MOVER(ByteBuffer &buff)
{
    ObjectGuid id;
    ObjectGuid tid;
    bool hasExtraMovementFlags;
    bool hasTransportData;
    bool hasSplineElevation;
    bool hasTimestamp;
    bool hasOrientation;
    bool hasPitch;
    bool hasFallData;
    bool hasMovementFlags;
    bool hasTransportTime3;
    bool hasTransportTime2;
    bool hasFallDirection;

    buff >> pos.m_positionZ;
    buff >> pos.m_positionX;
    buff >> pos.m_positionY;
    hasExtraMovementFlags = !buff.ReadBit();
    hasTransportData = buff.ReadBit();
    id[6] = buff.ReadBit();
    hasSplineElevation = !buff.ReadBit();
    id[3] = buff.ReadBit();
    buff.ReadBit(); // unknown
    hasTimestamp = !buff.ReadBit();
    id[0] = buff.ReadBit();
    hasOrientation = !buff.ReadBit();
    id[5] = buff.ReadBit();
    hasPitch = !buff.ReadBit();
    id[1] = buff.ReadBit();
    id[4] = buff.ReadBit();
    id[7] = buff.ReadBit();
    hasSpline = buff.ReadBit();
    id[2] = buff.ReadBit();
    hasFallData = buff.ReadBit();
    hasMovementFlags = !buff.ReadBit();
    if (hasTransportData)
    {
        tid[4] = buff.ReadBit();
        tid[0] = buff.ReadBit();
        tid[1] = buff.ReadBit();
        tid[6] = buff.ReadBit();
        tid[2] = buff.ReadBit();
        hasTransportTime3 = buff.ReadBit();
        tid[5] = buff.ReadBit();
        tid[7] = buff.ReadBit();
        hasTransportTime2 = buff.ReadBit();
        tid[3] = buff.ReadBit();
    }
    if (hasFallData)
        hasFallDirection = buff.ReadBit();
    if (hasMovementFlags)
        flags = buff.ReadBits(30);
    if (hasExtraMovementFlags)
        flags2 = buff.ReadBits(12);
    buff.ReadByteSeq(id[1]);
    buff.ReadByteSeq(id[0]);
    buff.ReadByteSeq(id[4]);
    buff.ReadByteSeq(id[2]);
    buff.ReadByteSeq(id[7]);
    buff.ReadByteSeq(id[5]);
    buff.ReadByteSeq(id[6]);
    buff.ReadByteSeq(id[3]);
    if (hasFallData)
    {
        buff >> j_zspeed;
        if (hasFallDirection)
        {
            buff >> j_cosAngle;
            buff >> j_sinAngle;
            buff >> j_xyspeed;
        }
        buff >> fallTime;
    }
    if (hasTransportData)
    {
        if (hasTransportTime3)
            buff >> t_time3;
        buff.ReadByteSeq(tid[1]);
        if (hasTransportTime2)
            buff >> t_time2;
        buff >> t_pos.m_orientation;
        buff.ReadByteSeq(tid[0]);
        buff >> t_seat;
        buff.ReadByteSeq(tid[4]);
        buff.ReadByteSeq(tid[6]);
        buff.ReadByteSeq(tid[3]);
        buff.ReadByteSeq(tid[5]);
        buff >> t_pos.m_positionY;
        buff >> t_pos.m_positionX;
        buff.ReadByteSeq(tid[2]);
        buff >> t_pos.m_positionZ;
        buff.ReadByteSeq(tid[7]);
        buff >> t_time;
    }
    if (hasTimestamp)
        buff >> time;
    if (hasSplineElevation)
        buff >> splineElevation;
    if (hasPitch)
        buff >> pitch;
    if (hasOrientation)
        buff >> pos.m_orientation;

    guid = id;
    t_guid = tid;
}

void MovementInfo::Read_CMSG_MOVE_KNOCK_BACK_ACK(ByteBuffer &buff)
{
    ObjectGuid id;
    ObjectGuid tid;
    bool hasOrientation;
    bool hasPitch;
    bool hasSplineElevation;
    bool hasExtraMovementFlags;
    bool hasTimestamp;
    bool hasMovementFlags;
    bool hasTransportData;
    bool hasFallData;
    bool hasFallDirection;
    bool hasTransportTime2;
    bool hasTransportTime3;

    buff >> pos.m_positionY;
    buff >> pos.m_positionZ;
    buff.read_skip<uint32>(); // unknown
    buff >> pos.m_positionX;
    id[6] = buff.ReadBit();
    hasOrientation = !buff.ReadBit();
    hasPitch = !buff.ReadBit();
    hasSplineElevation = !buff.ReadBit();
    id[3] = buff.ReadBit();
    id[4] = buff.ReadBit();
    id[1] = buff.ReadBit();
    id[2] = buff.ReadBit();
    hasSpline = buff.ReadBit();
    id[7] = buff.ReadBit();
    buff.ReadBit(); // unknown
    hasExtraMovementFlags = !buff.ReadBit();
    hasTimestamp = !buff.ReadBit();
    id[0] = buff.ReadBit();
    hasMovementFlags = !buff.ReadBit();
    hasTransportData = buff.ReadBit();
    id[5] = buff.ReadBit();
    hasFallData = buff.ReadBit();
    if (hasMovementFlags)
        flags = buff.ReadBits(30);
    if (hasTransportData)
    {
        tid[4] = buff.ReadBit();
        tid[1] = buff.ReadBit();
        hasTransportTime2 = buff.ReadBit();
        tid[0] = buff.ReadBit();
        hasTransportTime3 = buff.ReadBit();
        tid[6] = buff.ReadBit();
        tid[2] = buff.ReadBit();
        tid[3] = buff.ReadBit();
        tid[5] = buff.ReadBit();
        tid[7] = buff.ReadBit();
    }
    if (hasExtraMovementFlags)
        flags2 = buff.ReadBits(12);
    if (hasFallData)
        hasFallDirection = buff.ReadBit();
    buff.ReadByteSeq(id[4]);
    buff.ReadByteSeq(id[5]);
    buff.ReadByteSeq(id[1]);
    buff.ReadByteSeq(id[6]);
    buff.ReadByteSeq(id[0]);
    buff.ReadByteSeq(id[3]);
    buff.ReadByteSeq(id[2]);
    buff.ReadByteSeq(id[7]);
    if (hasFallData)
    {
        if (hasFallDirection)
        {
            buff >> j_cosAngle;
            buff >> j_xyspeed;
            buff >> j_sinAngle;
        }
        buff >> fallTime;
        buff >> j_zspeed;
    }
    if (hasTimestamp)
        buff >> time;
    if (hasTransportData)
    {
        buff >> t_pos.m_positionY;
        buff.ReadByteSeq(tid[4]);
        if (hasTransportTime2)
            buff >> t_time2;
        buff.ReadByteSeq(tid[7]);
        buff >> t_pos.m_orientation;
        buff.ReadByteSeq(tid[6]);
        buff >> t_time;
        buff.ReadByteSeq(tid[3]);
        buff.ReadByteSeq(tid[1]);
        if (hasTransportTime3)
            buff >> t_time3;
        buff.ReadByteSeq(tid[2]);
        buff >> t_pos.m_positionZ;
        buff.ReadByteSeq(tid[0]);
        buff.ReadByteSeq(tid[5]);
        buff >> t_pos.m_positionX;
        buff >> t_seat;
    }
    if (hasPitch)
        buff >> pitch;
    if (hasSplineElevation)
        buff >> splineElevation;
    if (hasOrientation)
        buff >> pos.m_orientation;

    guid = id;
    t_guid = tid;
}

void MovementInfo::Write_SMSG_MOVE_TELEPORT(ByteBuffer &buff)
{
    ObjectGuid id = guid;
    ObjectGuid tid = t_guid;

    buff.WriteBit(id[6]);
    buff.WriteBit(id[0]);
    buff.WriteBit(id[3]);
    buff.WriteBit(id[2]);
    buff.WriteBit(0); // Unk
    buff.WriteBit(uint64(tid));
    buff.WriteBit(id[1]);

    if (tid)
    {
        buff.WriteBit(tid[1]);
        buff.WriteBit(tid[3]);
        buff.WriteBit(tid[2]);
        buff.WriteBit(tid[5]);
        buff.WriteBit(tid[0]);
        buff.WriteBit(tid[7]);
        buff.WriteBit(tid[6]);
        buff.WriteBit(tid[4]);
    }

    buff.WriteBit(id[4]);
    buff.WriteBit(id[7]);
    buff.WriteBit(id[5]);
    buff.FlushBits();

    if (tid)
    {
        buff.WriteByteSeq(tid[6]);
        buff.WriteByteSeq(tid[5]);
        buff.WriteByteSeq(tid[1]);
        buff.WriteByteSeq(tid[7]);
        buff.WriteByteSeq(tid[0]);
        buff.WriteByteSeq(tid[2]);
        buff.WriteByteSeq(tid[4]);
        buff.WriteByteSeq(tid[3]);
    }

    buff << ackCount;
    buff.WriteByteSeq(id[1]);
    buff.WriteByteSeq(id[2]);
    buff.WriteByteSeq(id[3]);
    buff.WriteByteSeq(id[5]);
    buff << pos.m_positionX;
    buff.WriteByteSeq(id[4]);
    buff << pos.m_orientation;
    buff.WriteByteSeq(id[7]);
    buff << pos.m_positionZ;
    buff.WriteByteSeq(id[0]);
    buff.WriteByteSeq(id[6]);
    buff << pos.m_positionY;
}

void MovementInfo::Write_SMSG_MOVE_UPDATE_KNOCK_BACK(ByteBuffer &buff)
{
    ObjectGuid id = guid;
    ObjectGuid tid = t_guid;
    bool hasOrientation = !G3D::fuzzyEq(pos.m_orientation, 0.0f);
    bool hasSplineElevation = !G3D::fuzzyEq(splineElevation, 0.0f);
    bool hasFallData = fallTime || j_zspeed;
    bool hasFallDirection = hasFallData && (j_cosAngle || j_sinAngle || j_xyspeed);
    bool hasPitch = !G3D::fuzzyEq(pitch, 0.0f);

    buff.WriteBit(0); // unknown
    buff.WriteBit(id[4]);
    buff.WriteBit(!flags);
    buff.WriteBit(!hasPitch);
    buff.WriteBit(!time);
    buff.WriteBit(id[1]);
    buff.WriteBit(id[0]);
    buff.WriteBit(id[3]);
    buff.WriteBit(id[2]);
    buff.WriteBit(id[7]);
    buff.WriteBit(hasSpline);
    buff.WriteBit(t_guid);
    if (t_guid)
    {
        buff.WriteBit(tid[7]);
        buff.WriteBit(tid[5]);
        buff.WriteBit(tid[1]);
        buff.WriteBit(tid[6]);
        buff.WriteBit(t_time2);
        buff.WriteBit(tid[2]);
        buff.WriteBit(tid[4]);
        buff.WriteBit(tid[0]);
        buff.WriteBit(t_time3);
        buff.WriteBit(tid[3]);
    }
    buff.WriteBit(id[5]);
    buff.WriteBit(!hasSplineElevation);
    buff.WriteBit(!flags2);
    buff.WriteBit(id[6]);
    if (flags)
        buff.WriteBits(flags, 30);
    buff.WriteBit(hasFallData);
    if (hasFallData)
        buff.WriteBit(hasFallDirection);
    buff.WriteBit(!hasOrientation);
    if (flags2)
        buff.WriteBits(flags2, 12);
    if (hasOrientation)
        buff << pos.m_orientation;
    if (hasFallData)
    {
        if (hasFallDirection)
        {
            buff << j_cosAngle;
            buff << j_xyspeed;
            buff << j_sinAngle;
        }
        buff << fallTime;
        buff << j_zspeed;
    }
    if (hasSplineElevation)
        buff << splineElevation;
    buff.WriteByteSeq(id[3]);
    if (t_guid)
    {
        buff.WriteByteSeq(tid[5]);
        if (t_time3)
            buff << t_time3;
        buff.WriteByteSeq(tid[7]);
        buff << t_seat;
        buff.WriteByteSeq(tid[3]);
        buff.WriteByteSeq(tid[6]);
        buff << t_pos.m_positionZ;
        buff.WriteByteSeq(tid[1]);
        buff << t_pos.m_positionY;
        buff << t_pos.m_positionX;
        buff.WriteByteSeq(tid[2]);
        buff.WriteByteSeq(tid[0]);
        buff << t_pos.m_orientation;
        buff.WriteByteSeq(tid[4]);
        if (t_time2)
            buff << t_time2;
    }
    if (hasPitch)
        buff << pitch;
    buff << pos.m_positionZ;
    if (time)
        buff << time;
    buff << pos.m_positionX;
    buff.WriteByteSeq(id[4]);
    buff.WriteByteSeq(id[6]);
    buff.WriteByteSeq(id[7]);
    buff.WriteByteSeq(id[2]);
    buff.WriteByteSeq(id[1]);
    buff << pos.m_positionY;
    buff.WriteByteSeq(id[0]);
    buff.WriteByteSeq(id[5]);
}

void MovementInfo::ReadFromPacket(WorldPacket &packet)
{
    switch (packet.GetOpcode())
    {
    case CMSG_MOVE_KNOCK_BACK_ACK:
        Read_CMSG_MOVE_KNOCK_BACK_ACK(packet);
        break;
    case CMSG_MOVE_NOT_ACTIVE_MOVER:
        Read_CMSG_MOVE_NOT_ACTIVE_MOVER(packet);
        break;
    default:
        Read_OLD(packet);
    }
}

void MovementInfo::Sanitize(Unit *target)
{
    #define REMOVE_VIOLATING_FLAGS(check, maskToRemove) if (check) RemoveMovementFlag((maskToRemove));

    /*! This must be a packet spoofing attempt. MOVEMENTFLAG_ROOT sent from the client is not valid
        in conjunction with any of the moving movement flags such as MOVEMENTFLAG_FORWARD.
        It will freeze clients that receive this player's movement info.
    */
    REMOVE_VIOLATING_FLAGS(HasMovementFlag(MOVEMENTFLAG_ROOT), MOVEMENTFLAG_ROOT);

    //! Cannot hover without SPELL_AURA_HOVER
    REMOVE_VIOLATING_FLAGS(HasMovementFlag(MOVEMENTFLAG_HOVER) && !target->HasAuraType(SPELL_AURA_HOVER), MOVEMENTFLAG_HOVER);

    //! Cannot ascend and descend at the same time
    REMOVE_VIOLATING_FLAGS(HasMovementFlag(MOVEMENTFLAG_ASCENDING) && HasMovementFlag(MOVEMENTFLAG_DESCENDING), MOVEMENTFLAG_ASCENDING | MOVEMENTFLAG_DESCENDING);

    //! Cannot move left and right at the same time
    REMOVE_VIOLATING_FLAGS(HasMovementFlag(MOVEMENTFLAG_LEFT) && HasMovementFlag(MOVEMENTFLAG_RIGHT), MOVEMENTFLAG_LEFT | MOVEMENTFLAG_RIGHT);

    //! Cannot strafe left and right at the same time
    REMOVE_VIOLATING_FLAGS(HasMovementFlag(MOVEMENTFLAG_STRAFE_LEFT) && HasMovementFlag(MOVEMENTFLAG_STRAFE_RIGHT), MOVEMENTFLAG_STRAFE_LEFT | MOVEMENTFLAG_STRAFE_RIGHT);

    //! Cannot pitch up and down at the same time
    REMOVE_VIOLATING_FLAGS(HasMovementFlag(MOVEMENTFLAG_PITCH_UP) && HasMovementFlag(MOVEMENTFLAG_PITCH_DOWN), MOVEMENTFLAG_PITCH_UP | MOVEMENTFLAG_PITCH_DOWN);

    //! Cannot move forwards and backwards at the same time
    REMOVE_VIOLATING_FLAGS(HasMovementFlag(MOVEMENTFLAG_FORWARD) && HasMovementFlag(MOVEMENTFLAG_BACKWARD), MOVEMENTFLAG_FORWARD | MOVEMENTFLAG_BACKWARD);

    //! Cannot walk on water without SPELL_AURA_WATER_WALK
    REMOVE_VIOLATING_FLAGS(HasMovementFlag(MOVEMENTFLAG_WATERWALKING) && !target->HasAuraType(SPELL_AURA_WATER_WALK), MOVEMENTFLAG_WATERWALKING);

    //! Cannot feather fall without SPELL_AURA_FEATHER_FALL
    REMOVE_VIOLATING_FLAGS(HasMovementFlag(MOVEMENTFLAG_FALLING_SLOW) && !target->HasAuraType(SPELL_AURA_FEATHER_FALL), MOVEMENTFLAG_FALLING_SLOW);

    //! Cannot fly if no fly auras present. Exception is being a GM.
    if (target->GetTypeId() == TYPEID_PLAYER)
    {
        REMOVE_VIOLATING_FLAGS(
            HasMovementFlag(MOVEMENTFLAG_FLYING | MOVEMENTFLAG_CAN_FLY) && ((Player*)target)->GetSession()->GetSecurity() == SEC_PLAYER &&
            !target->HasAuraType(SPELL_AURA_FLY) && !target->HasAuraType(SPELL_AURA_MOD_INCREASE_MOUNTED_FLIGHT_SPEED),
            MOVEMENTFLAG_FLYING | MOVEMENTFLAG_CAN_FLY);
    }
    else
    {
        REMOVE_VIOLATING_FLAGS(
            HasMovementFlag(MOVEMENTFLAG_FLYING | MOVEMENTFLAG_CAN_FLY) && !target->HasAuraType(SPELL_AURA_FLY) && !target->HasAuraType(SPELL_AURA_MOD_INCREASE_FLIGHT_SPEED),
            MOVEMENTFLAG_FLYING | MOVEMENTFLAG_CAN_FLY
        );
    }

    #undef REMOVE_VIOLATING_FLAGS
}

void MovementInfo::Read_OLD(WorldPacket &data)
{
    bool hasMovementFlags = false;
    bool hasMovementFlags2 = false;
    bool hasTimestamp = false;
    bool hasOrientation = false;
    bool hasTransportData = false;
    bool hasTransportTime2 = false;
    bool hasTransportTime3 = false;
    bool hasPitch = false;
    bool hasFallData = false;
    bool hasFallDirection = false;
    bool hasSplineElevation = false;

    MovementStatusElements* sequence = GetMovementStatusElementsSequence(data.GetOpcode());
    if (sequence == NULL)
    {
        sLog->outError(LOG_FILTER_NETWORKIO, "WorldSession::ReadMovementInfo: No movement sequence found for opcode 0x%04X", uint32(data.GetOpcode()));
        return;
    }

    ObjectGuid id;
    ObjectGuid tid;

    for (uint32 i = 0; i < MSE_COUNT; ++i)
    {
        MovementStatusElements element = sequence[i];
        if (element == MSEEnd)
            break;

        if (element >= MSEHasGuidByte0 && element <= MSEHasGuidByte7)
        {
            id[element - MSEHasGuidByte0] = data.ReadBit();
            continue;
        }

        if (element >= MSEHasTransportGuidByte0 &&
            element <= MSEHasTransportGuidByte7)
        {
            if (hasTransportData)
                tid[element - MSEHasTransportGuidByte0] = data.ReadBit();
            continue;
        }

        if (element >= MSEGuidByte0 && element <= MSEGuidByte7)
        {
            data.ReadByteSeq(id[element - MSEGuidByte0]);
            continue;
        }

        if (element >= MSETransportGuidByte0 &&
            element <= MSETransportGuidByte7)
        {
            if (hasTransportData)
                data.ReadByteSeq(tid[element - MSETransportGuidByte0]);
            continue;
        }

        if (element >= MSESpeedWalk && 
            element <= MSESpeedPitchRate)
        {
            data >> ackSpeed;
            continue;
        }

        switch (element)
        {
            case MSEHasMovementFlags:
                hasMovementFlags = !data.ReadBit();
                break;
            case MSEHasMovementFlags2:
                hasMovementFlags2 = !data.ReadBit();
                break;
            case MSEHasTimestamp:
                hasTimestamp = !data.ReadBit();
                break;
            case MSEHasOrientation:
                hasOrientation = !data.ReadBit();
                break;
            case MSEHasTransportData:
                hasTransportData = data.ReadBit();
                break;
            case MSEHasTransportTime2:
                if (hasTransportData)
                    hasTransportTime2 = data.ReadBit();
                break;
            case MSEHasTransportTime3:
                if (hasTransportData)
                    hasTransportTime3 = data.ReadBit();
                break;
            case MSEHasPitch:
                hasPitch = !data.ReadBit();
                break;
            case MSEHasFallData:
                hasFallData = data.ReadBit();
                break;
            case MSEHasFallDirection:
                if (hasFallData)
                    hasFallDirection = data.ReadBit();
                break;
            case MSEHasSplineElevation:
                hasSplineElevation = !data.ReadBit();
                break;
            case MSEHasSpline:
                hasSpline = data.ReadBit();
                break;
            case MSEMovementFlags:
                if (hasMovementFlags)
                    flags = data.ReadBits(30);
                break;
            case MSEMovementFlags2:
                if (hasMovementFlags2)
                    flags2 = data.ReadBits(12);
                break;
            case MSETimestamp:
                if (hasTimestamp)
                    data >> time;
                break;
            case MSEPositionX:
                data >> pos.m_positionX;
                break;
            case MSEPositionY:
                data >> pos.m_positionY;
                break;
            case MSEPositionZ:
                data >> pos.m_positionZ;
                break;
            case MSEOrientation:
                if (hasOrientation)
                    pos.SetOrientation(data.read<float>());
                break;
            case MSETransportPositionX:
                if (hasTransportData)
                    data >> t_pos.m_positionX;
                break;
            case MSETransportPositionY:
                if (hasTransportData)
                    data >> t_pos.m_positionY;
                break;
            case MSETransportPositionZ:
                if (hasTransportData)
                    data >> t_pos.m_positionZ;
                break;
            case MSETransportOrientation:
                if (hasTransportData)
                    t_pos.SetOrientation(data.read<float>());
                break;
            case MSETransportSeat:
                if (hasTransportData)
                    data >> t_seat;
                break;
            case MSETransportTime:
                if (hasTransportData)
                    data >> t_time;
                break;
            case MSETransportTime2:
                if (hasTransportData && hasTransportTime2)
                    data >> t_time2;
                break;
            case MSETransportTime3:
                if (hasTransportData && hasTransportTime3)
                    data >> t_time3;
                break;
            case MSEPitch:
                if (hasPitch)
                    data >> pitch;
                break;
            case MSEFallTime:
                if (hasFallData)
                    data >> fallTime;
                break;
            case MSEFallVerticalSpeed:
                if (hasFallData)
                    data >> j_zspeed;
                break;
            case MSEFallCosAngle:
                if (hasFallData && hasFallDirection)
                    data >> j_cosAngle;
                break;
            case MSEFallSinAngle:
                if (hasFallData && hasFallDirection)
                    data >> j_sinAngle;
                break;
            case MSEFallHorizontalSpeed:
                if (hasFallData && hasFallDirection)
                    data >> j_xyspeed;
                break;
            case MSESplineElevation:
                if (hasSplineElevation)
                    data >> splineElevation;
                break;
            case MSECounter:
                data >> ackCount;
                break;
            case MSEHeight:
                data >> height;
                break;
            case MSEZeroBit:
            case MSEOneBit:
                data.ReadBit();
                break;
            default:
                ASSERT(false && "Incorrect sequence element detected at ReadMovementInfo");
                break;
        }
    }

    guid = id;
    t_guid = tid;
}

void MovementInfo::WriteToPacket(WorldPacket &packet)
{
    switch (packet.GetOpcode())
    {
    case SMSG_PLAYER_MOVE:
        Write_SMSG_PLAYER_MOVE(packet);
        break;
    case SMSG_MOVE_UPDATE_KNOCK_BACK:
        Write_SMSG_MOVE_UPDATE_KNOCK_BACK(packet);
        break;
    case SMSG_MOVE_TELEPORT:
        Write_SMSG_MOVE_TELEPORT(packet);
        break;
    default:
        Write_OLD(packet);
        break;
    }
}

void MovementInfo::Write_OLD(WorldPacket &data)
{
    ObjectGuid id = guid;
    ObjectGuid tid = t_guid;

    bool hasOrientation = !G3D::fuzzyEq(pos.m_orientation, 0.0f);
    bool hasSplineElevation = !G3D::fuzzyEq(splineElevation, 0.0f);
    bool hasFallData = fallTime || j_zspeed;
    bool hasFallDirection = hasFallData && (j_cosAngle || j_sinAngle || j_xyspeed);
    bool hasPitch = !G3D::fuzzyEq(pitch, 0.0f);

    MovementStatusElements* sequence = GetMovementStatusElementsSequence(data.GetOpcode());
    if (!sequence)
    {
        sLog->outError(LOG_FILTER_NETWORKIO, "WorldSession::WriteMovementInfo: No movement sequence found for opcode 0x%04X", uint32(data.GetOpcode()));
        return;
    }

    for (uint32 i = 0; i < MSE_COUNT; ++i)
    {
        MovementStatusElements element = sequence[i];
        if (element == MSEEnd)
            break;

        if (element >= MSEHasGuidByte0 && element <= MSEHasGuidByte7)
        {
            data.WriteBit(id[element - MSEHasGuidByte0]);
            continue;
        }

        if (element >= MSEHasTransportGuidByte0 &&
            element <= MSEHasTransportGuidByte7)
        {
            if (t_guid)
                data.WriteBit(tid[element - MSEHasTransportGuidByte0]);
            continue;
        }

        if (element >= MSEGuidByte0 && element <= MSEGuidByte7)
        {
            data.WriteByteSeq(id[element - MSEGuidByte0]);
            continue;
        }

        if (element >= MSETransportGuidByte0 &&
            element <= MSETransportGuidByte7)
        {
            if (t_guid)
                data.WriteByteSeq(tid[element - MSETransportGuidByte0]);
            continue;
        }

        if (element >= MSESpeedWalk && 
            element <= MSESpeedPitchRate)
        {
            data << ackSpeed;
            continue;
        }

        switch (element)
        {
        case MSEHasMovementFlags:
            data.WriteBit(!flags);
            break;
        case MSEHasMovementFlags2:
            data.WriteBit(!flags2);
            break;
        case MSEHasTimestamp:
            data.WriteBit(!time);
            break;
        case MSEHasOrientation:
            data.WriteBit(!hasOrientation);
            break;
        case MSEHasTransportData:
            data.WriteBit(t_guid);
            break;
        case MSEHasTransportTime2:
            if (t_guid)
                data.WriteBit(t_time2);
            break;
        case MSEHasTransportTime3:
            if (t_guid)
                data.WriteBit(t_time3);
            break;
        case MSEHasPitch:
            data.WriteBit(!hasPitch);
            break;
        case MSEHasFallData:
            data.WriteBit(hasFallData);
            break;
        case MSEHasFallDirection:
            if (hasFallData)
                data.WriteBit(hasFallDirection);
            break;
        case MSEHasSplineElevation:
            data.WriteBit(!hasSplineElevation);
            break;
        case MSEHasSpline:
            data.WriteBit(hasSpline);
            break;
        case MSEMovementFlags:
            if (flags)
                data.WriteBits(flags, 30);
            break;
        case MSEMovementFlags2:
            if (flags2)
                data.WriteBits(flags2, 12);
            break;
        case MSETimestamp:
            if (time)
                data << time;
            break;
        case MSEPositionX:
            data << pos.m_positionX;
            break;
        case MSEPositionY:
            data << pos.m_positionY;
            break;
        case MSEPositionZ:
            data << pos.m_positionZ;
            break;
        case MSEOrientation:
            if (hasOrientation)
                data << pos.m_orientation;
            break;
        case MSETransportPositionX:
            if (t_guid)
                data << t_pos.m_positionX;
            break;
        case MSETransportPositionY:
            if (t_guid)
                data << t_pos.m_positionY;
            break;
        case MSETransportPositionZ:
            if (t_guid)
                data << t_pos.m_positionZ;
            break;
        case MSETransportOrientation:
            if (t_guid)
                data << t_pos.m_orientation;
            break;
        case MSETransportSeat:
            if (t_guid)
                data << t_seat;
            break;
        case MSETransportTime:
            if (t_guid)
                data << t_time;
            break;
        case MSETransportTime2:
            if (t_guid && t_time2)
                data << t_time2;
            break;
        case MSETransportTime3:
            if (t_guid && t_time3)
                data << t_time3;
            break;
        case MSEPitch:
            if (hasPitch)
                data << pitch;
            break;
        case MSEFallTime:
            if (hasFallData)
                data << fallTime;
            break;
        case MSEFallVerticalSpeed:
            if (hasFallData)
                data << j_zspeed;
            break;
        case MSEFallCosAngle:
            if (hasFallData && hasFallDirection)
                data << j_cosAngle;
            break;
        case MSEFallSinAngle:
            if (hasFallData && hasFallDirection)
                data << j_sinAngle;
            break;
        case MSEFallHorizontalSpeed:
            if (hasFallData && hasFallDirection)
                data << j_xyspeed;
            break;
        case MSESplineElevation:
            if (hasSplineElevation)
                data << splineElevation;
            break;
        case MSECounter:
            data << ackCount;
            break;
        case MSEHeight:
            data << height;
            break;
        case MSEZeroBit:
            data.WriteBit(0);
            break;
        case MSEOneBit:
            data.WriteBit(1);
            break;
        default:
            ASSERT(false && "Incorrect sequence element detected at ReadMovementInfo");
            break;
        }
    }
}

Opcodes MovementInfo::GetSMSGOpcodeForCMSG(Opcodes cmsg)
{

    switch (cmsg)
    {
        case CMSG_MOVE_FORCE_WALK_SPEED_CHANGE_ACK:
            return SMSG_MOVE_UPDATE_WALK_SPEED;
        case CMSG_MOVE_FORCE_RUN_SPEED_CHANGE_ACK:
            return SMSG_MOVE_UPDATE_RUN_SPEED;
        case CMSG_MOVE_FORCE_RUN_BACK_SPEED_CHANGE_ACK:
            return SMSG_MOVE_UPDATE_RUN_BACK_SPEED;
        case CMSG_MOVE_FORCE_SWIM_SPEED_CHANGE_ACK:
            return SMSG_MOVE_UPDATE_SWIM_SPEED;
        case CMSG_MOVE_FORCE_SWIM_BACK_SPEED_CHANGE_ACK:
            return SMSG_MOVE_UPDATE_SWIM_BACK_SPEED;
        case CMSG_MOVE_FORCE_FLIGHT_SPEED_CHANGE_ACK:
            return SMSG_MOVE_UPDATE_FLIGHT_SPEED;
        case CMSG_MOVE_SET_COLLISION_HEIGHT_ACK:
            return SMSG_MOVE_UPDATE_COLLISION_HEIGHT;
        /* TODO: Find packet smsg structure to replace by the good opcode */
        case CMSG_MOVE_FORCE_TURN_RATE_CHANGE_ACK:
        case CMSG_MOVE_FORCE_FLIGHT_BACK_SPEED_CHANGE_ACK:
        case CMSG_MOVE_FORCE_PITCH_RATE_CHANGE_ACK:
        case CMSG_MOVE_HOVER_ACK: // Should be MSG_MOVE_HOVER
            return SMSG_PLAYER_MOVE;
        /* END TODO */
        /* Seems to be broadcast with SMSG_PLAYER_MOVE (not sure) */
        case CMSG_MOVE_GRAVITY_ENABLE_ACK:
        case CMSG_MOVE_GRAVITY_DISABLE_ACK:
        case CMSG_MOVE_WATER_WALK_ACK:
        case CMSG_MOVE_SET_CAN_FLY_ACK:
        case CMSG_MOVE_FEATHER_FALL_ACK:
        case CMSG_FORCE_MOVE_ROOT_ACK:
        case CMSG_FORCE_MOVE_UNROOT_ACK:
            return SMSG_PLAYER_MOVE;
        /* */
        default:
            break;
    }

    return SMSG_PLAYER_MOVE;
}
