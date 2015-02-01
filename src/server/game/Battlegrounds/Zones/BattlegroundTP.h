/*
* Copyright (C) 2005 - 2013 MaNGOS <http://www.getmangos.com/>
*
* Copyright (C) 2008 - 2013 Trinity <http://www.trinitycore.org/>
*
* Copyright (C) 2010 - 2013 ProjectSkyfire <http://www.projectskyfire.org/>
*
* Copyright (C) 2011 - 2013 ArkCORE <http://www.arkania.net/>
*
* Copyright (C) 2013 - 2014 WoWSource <http://www.wowsource.info/>
*
* This program is free software; you can redistribute it and/or modify
* it under the terms of the GNU General Public License as published by
* the Free Software Foundation; either version 2 of the License, or
* (at your option) any later version.
*
* This program is distributed in the hope that it will be useful,
* but WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
* GNU General Public License for more details.
*
* You should have received a copy of the GNU General Public License
* along with this program; if not, write to the Free Software
* Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA
*/

#ifndef __BATTLEGROUNDTP_H
#define __BATTLEGROUNDTP_H

#include "Battleground.h"


enum BG_TP_TimerOrScore
{
	BG_TP_MAX_TEAM_SCORE = 3,
	BG_TP_FLAG_RESPAWN_TIME = 23000,
	BG_TP_FLAG_DROP_TIME = 10000,
	BG_TP_SPELL_FORCE_TIME = 600000,
	BG_TP_SPELL_BRUTAL_TIME = 900000
};

enum BG_TP_Sound
{
	BG_TP_SOUND_FLAG_CAPTURED_ALLIANCE = 8173,
	BG_TP_SOUND_FLAG_CAPTURED_HORDE = 8213,
	BG_TP_SOUND_FLAG_PLACED = 8232,
	BG_TP_SOUND_FLAG_RETURNED = 8192,
	BG_TP_SOUND_HORDE_FLAG_PICKED_UP = 8212,
	BG_TP_SOUND_ALLIANCE_FLAG_PICKED_UP = 8174,
	BG_TP_SOUND_FLAGS_RESPAWNED = 8232
};

enum BG_TP_SpellId
{
	BG_TP_SPELL_HORDE_FLAG = 23333,
	BG_TP_SPELL_HORDE_FLAG_DROPPED = 23334,
	BG_TP_SPELL_HORDE_FLAG_PICKED = 61266,    ///< Fake Spell - Used as a start timer event
	BG_TP_SPELL_ALLIANCE_FLAG = 23335,
	BG_TP_SPELL_ALLIANCE_FLAG_DROPPED = 23336,
	BG_TP_SPELL_ALLIANCE_FLAG_PICKED = 61265,    ///< Fake Spell - Used as a start timer event
	BG_TP_SPELL_FOCUSED_ASSAULT = 46392,
	BG_TP_SPELL_BRUTAL_ASSAULT = 46393,
	BG_TP_SPELL_REMOVE_CARRIED_FLAG = 45919     ///< Need implementation. This spell if casted on each player from 1 faction when the flag is captured / dropped. + Remove unnecesary RemoveauraDueToSpell() from code.
};

/// To Do: Find what unk world states means and rename
enum BG_TP_WorldStates
{
	BG_TP_FLAG_UNK_ALLIANCE = 1545, ///< Value: -1 when alliance flag is dropped | 1 when alliance flag is on player | 0 On base | -2 ???
	BG_TP_FLAG_UNK_HORDE = 1546, ///< Value: -1 when horde flag is dropped    | 1 when horde flag is on player    | 0 On base | -2 ???
	BG_TP_FLAG_UNKNOWN = 1547, ///< -1 before capturing flag, 0 after both flags respawned
	BG_TP_FLAG_CAPTURES_ALLIANCE = 1581,
	BG_TP_FLAG_CAPTURES_HORDE = 1582,
	BG_TP_FLAG_CAPTURES_MAX = 1601,
	BG_TP_FLAG_STATE_HORDE = 2338,
	BG_TP_FLAG_STATE_ALLIANCE = 2339,
	BG_TP_STATE_TIMER = 4248,
	BG_TP_STATE_TIMER_ACTIVE = 4247,
	BG_TP_STATE_UNKNOWN = 4249, ///< Used after flag is captured (value: 1)
};

enum BG_TP_ObjectTypes
{
	BG_TP_OBJECT_DOOR_A_1 = 0,
	BG_TP_OBJECT_DOOR_A_2,
	BG_TP_OBJECT_DOOR_A_3,
	BG_TP_OBJECT_DOOR_A_4,
	BG_TP_OBJECT_DOOR_H_1,
	BG_TP_OBJECT_DOOR_H_2,
	BG_TP_OBJECT_DOOR_H_3,
	BG_TP_OBJECT_DOOR_H_4,
	BG_TP_OBJECT_A_FLAG,
	BG_TP_OBJECT_H_FLAG,
	BG_TP_OBJECT_SPEEDBUFF_1,
	BG_TP_OBJECT_SPEEDBUFF_2,
	BG_TP_OBJECT_REGENBUFF_1,
	BG_TP_OBJECT_REGENBUFF_2,
	BG_TP_OBJECT_BERSERKBUFF_1,
	BG_TP_OBJECT_BERSERKBUFF_2,
	BG_TP_OBJECT_MAX
};

enum BG_TP_ObjectEntry
{
	BG_OBJECT_DOOR_A_1_TP_ENTRY = 206655,
	BG_OBJECT_DOOR_A_2_TP_ENTRY = 206654,
	BG_OBJECT_DOOR_A_3_TP_ENTRY = 206653,
	BG_OBJECT_DOOR_A_4_TP_ENTRY = 206653,
	BG_OBJECT_DOOR_H_1_TP_ENTRY = 208205,
	BG_OBJECT_DOOR_H_2_TP_ENTRY = 208206,
	BG_OBJECT_DOOR_H_3_TP_ENTRY = 208206,
	BG_OBJECT_DOOR_H_4_TP_ENTRY = 208207,
	BG_OBJECT_A_FLAG_TP_ENTRY = 179830,
	BG_OBJECT_H_FLAG_TP_ENTRY = 179831,
	BG_OBJECT_A_FLAG_GROUND_TP_ENTRY = 179785,
	BG_OBJECT_H_FLAG_GROUND_TP_ENTRY = 179786
};

enum BG_TP_FlagState
{
	BG_TP_FLAG_STATE_ON_BASE = 0,
	BG_TP_FLAG_STATE_WAIT_RESPAWN,
	BG_TP_FLAG_STATE_ON_PLAYER,
	BG_TP_FLAG_STATE_ON_GROUND,
};

enum BG_TP_Graveyards
{
	TP_GRAVEYARD_FLAGROOM_ALLIANCE = 0,
	TP_GRAVEYARD_FLAGROOM_HORDE = 1,

	TP_GRAVEYARD_START_ALLIANCE = 2,
	TP_GRAVEYARD_START_HORDE = 3,

	TP_GRAVEYARD_MIDDLE_ALLIANCE = 4,
	TP_GRAVEYARD_MIDDLE_HORDE = 5,

	TP_MAX_GRAVEYARDS = 6
};

const uint32 BG_TP_GraveyardIds[TP_MAX_GRAVEYARDS] = { 1726, 1727, 1729, 1728, 1749, 1750 };

enum BG_TP_CreatureTypes
{
	TP_SPIRIT_ALLIANCE = 0,
	TP_SPIRIT_HORDE,

	BG_CREATURES_MAX_TP
};

enum BG_TP_CarrierDebuffs
{
	TP_SPELL_FOCUSED_ASSAULT = 46392,
	TP_SPELL_BRUTAL_ASSAULT = 46393
};

enum BG_TP_Objectives
{
	TP_OBJECTIVE_CAPTURE_FLAG = 290,
	TP_OBJECTIVE_RETURN_FLAG = 291
};

#define TP_EVENT_START_BATTLE   8563

// Class for scorekeeping
class BattlegroundTPScore : public BattlegroundScore
{
public:
	BattlegroundTPScore() : FlagCaptures(0), FlagReturns(0) {};
	virtual ~BattlegroundTPScore() {};

	uint32 FlagCaptures;
	uint32 FlagReturns;
};

// Main class for Twin Peaks Battleground
class BattlegroundTP : public Battleground
{
	friend class BattlegroundMgr;


public:
	BattlegroundTP();
	~BattlegroundTP();
	/**
	* \brief Called every time for update battle data
	*/
	void PostUpdateImpl(uint32 diff);

	/* Inherited from BattlegroundClass */

	/// Called when a player join battle
	void AddPlayer(Player* player);
	/// Called when a player leave battleground
	void RemovePlayer(Player* player, uint64 guid, uint32 team);

	/// Called when battle start
	void StartingEventCloseDoors();
	void StartingEventOpenDoors();
	/// Called for initialize battleground, after that the first player be entered (Mainly used to generate NPCs)
	bool SetupBattleground();
	void Reset();
	/// Called for generate packet contain worldstate data (Time + Score on the top of the screen)
	void FillInitialWorldStates(WorldPacket& data);

	/// Called on battleground ending
	void EndBattleground(uint32 winner);

	/// Return the nearest graveyard where player can respawn (Spirits in this Battle are: in Base, in Middle and if a player dies before battle start, to prevent cheating in main room(improbably to happen)) 
	WorldSafeLocsEntry const* GetClosestGraveYard(Player* player);
	/// Called when a player is muredered by another player (If the killed player has the flag to drop it) (In this BG a player can murdered only by another player)
	void HandleKillPlayer(Player *player, Player *killer);

	/// Called in HandleBattlegroundPlayerPositionsOpcode for tracking player on map
	uint64 GetFlagPickerGUID(int32 team) const
	{
		if (team == TEAM_ALLIANCE || team == TEAM_HORDE)
			return _flagKeepers[team];
		return 0;
	}
	/// Called when a player hits an area. (Like when is within distance to capture the flag (mainly used for this))
	void HandleAreaTrigger(Player* Source, uint32 Trigger);

	/* Update Score */
	/// Update score board
	void UpdatePlayerScore(Player* Source, uint32 type, uint32 value, bool doAddHonor = true);
	/// Update score on the top of screen by worldstates
	void UpdateTeamScore(uint32 team);

	void SetDroppedFlagGUID(uint64 guid, int32 team = -1)
	{
		if (team == TEAM_ALLIANCE || team == TEAM_HORDE)
			_droppedFlagGUID[team] = guid;
	}
	/// EVENT: On purpose to kill players that go to zone 'Hazardous water' to avoid being killed with flag
	bool HandlePlayerUnderMap(Player* /*plr*/);

private:
	/// Internal Battlegorund methods 

	/// Scorekeeping
	/// Add 1 point after a team captures the flag
	void AddPoint(uint32 teamID)                            { ++m_TeamScores[GetTeamIndexByTeamId(teamID)]; }

	/// Flag Events
	/// Update Flag state of one team, if the flag is in base, is waitng for respawn, is on player or on ground(if a player droped it) 
	void UpdateFlagState(uint32 team, uint32 value, uint32 flagKeeperGUID = 0);
	/// Used to maintain the last team witch captured the flag (see def of _lastFlagCaptureTeam)
	void SetLastFlagCapture(uint32 teamID)                  { _lastFlagCaptureTeam = teamID; }
	/// Respawn flag method
	void RespawnFlag(uint32 team, bool captured = false);
	/// EVENT: Happened when a player drops the flag
	void EventPlayerDroppedFlag(Player* source);
	/// EVENT: Happened when a player clicks on the flag
	void EventPlayerClickedOnFlag(Player* source, GameObject* target_obj);
	/// EVENT: Happened when a player captured(placed it in base) the flag
	void EventPlayerCapturedFlag(Player* source);


	/// Members:
	uint64 _flagKeepers[2];         ///< Maintains the flag picker GUID: 0 for ALLIANCE FLAG and 1 for HORDE FLAG (EX: _flagKeepers[TEAM_ALLIANCE] is guid for a horde player)
	uint64 _droppedFlagGUID[2];     ///< If the flag is on the ground(dropped by a player) we must maintain its guid to dispawn it when a player clicks on it. (else it will automatically dispawn)
	uint8 _flagState[2];            ///< Show where flag is (in base / on ground / on player)
	int32 _flagsTimer;              ///< Timer for flags that are unspawn after a capture
	int32 _flagsDropTimer[2];       ///< Used for counting how much time have passed since the flag dropped
	uint32 _lastFlagCaptureTeam;    ///< If the score is equal and the time expires the winer is based on witch team captured the last flag 
	int32 _flagSpellForceTimer;     ///< Used for counting how much time have passed since the both flags are kept
	bool _bothFlagsKept;            ///< shows if both flags are kept
	uint8 _flagDebuffState;         ///< This maintain the debuff state of the flag carrier. If the flag is on a player for more then X minutes, the player will be cursed with an debuff. (0 - No debuff, 1 - Focus assault, 2 - Brutal assault)
	uint8 _minutesElapsed;          ///< Elapsed time since the beginning of the battleground (It counts as well the beginning time(when the doors are closed))


};

#endif