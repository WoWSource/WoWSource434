/*
 * Copyright (C) 2008-2013 TrinityCore <http://www.trinitycore.org/>
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

#ifndef DEF_LOST_CITY_OF_THE_TOLVIR_H
#define DEF_LOST_CITY_OF_THE_TOLVIR_H

#define MAX_ENCOUNTER 5
#define ACTION_INTRO  0

enum Data
{
    DATA_GENERAL_HUSAM_EVENT      = 0,
    DATA_HIGH_PROPHET_BARIM_EVENT = 1,
    DATA_LOCKMAW_EVENT            = 2,
    DATA_AUGH_EVENT               = 3,
    DATA_SIAMAT_EVENT             = 4
};

enum Data64
{
    DATA_GENERAL_HUSAM           = 0,
    DATA_HIGH_PROPHET_BARIM      = 1,
    DATA_LOCKMAW                 = 2,
    DATA_AUGH                    = 3,
    DATA_SIAMAT                  = 4
};

enum Creatures
{
    // general husam
    BOSS_GENERAL_HUSAM           = 44577,
    NPC_SHOCKWAVE                = 44712,
    NPC_SHOCKWAVE_VISUAL         = 44711,
    NPC_LAND_MINE_VEHICLE        = 44798,
    NPC_LAND_MINE_TARGET         = 44840,
    NPC_BAD_INTENTIONS_TARGET    = 44586,
    // high prophet barim
    BOSS_HIGH_PROPHET_BARIM      = 43612,
    NPC_BARIM                    = 43612,
    NPC_BLAZE_OF_HEAVENS         = 48906,
    NPC_HARBINGER_OF_DARKNESS    = 43927,
    NPC_SOUL_FRAGMENT            = 43934,
    NPC_VEIL_OF_TWILIGHT         = 43926,
    NPC_HEAVENS_FURY             = 43801,
    // Lockmaw
    BOSS_LOCKMAW                 = 43614,
    BOSS_AUGH                    = 49045,
    BOSS_AUGH_FAKE               = 45378,
    NPC_FRENZIED_CROCOLISK       = 43658,
    NPC_ADD_STALKER              = 45124,
    // Siamat
    BOSS_SIAMAT                  = 44819,
    NPC_MINION_OF_SIAMAT_STORM   = 44713,
    NPC_MINION_OF_SIAMAT         = 44704,
    NPC_SERVANT_OF_SIAMAT        = 45259,
    // Trash mobs
    NPC_NEFERSET_DARKCASTER      = 44982,
    NPC_NEFERSET_PLAGUEBRINGER   = 44976,
    NPC_NEFERSET_THEURGIST       = 44980,
    NPC_NEFERSET_TORTURER        = 44977,
    NPC_OATHSWORN_AXEMASTER      = 44922,
    NPC_OATHSWORN_CAPTAIN        = 45122,
    NPC_OATHSWORN_MYRMIDON       = 44924,
    NPC_OATHSWORN_PATHFINDER     = 44932,
    NPC_OATHSWORN_SCORPID_KEEPER = 45062,
    NPC_OATHSWORN_SKINNER        = 44981,
    NPC_OATHSWORN_TAMER          = 45097,
    NPC_WANDERER                 = 44926,
    NPC_PYGMY_BRUTE              = 44896,
    NPC_PYGMY_FIREBREATHER       = 44898,
    NPC_PYGMY_SCOUT              = 44897,
    // Various NPCs
    NPC_CAPTAIN_HADAN            = 50038,
    NPC_WIND_TUNNEL              = 48092
};

enum GameObjectIds
{
    GO_TOLVIR_GRAVE              = 207409,
    GO_SIAMAT_PLATFORM           = 205365,
    //    GO_SIAMAT_PLATFORM_GUID = 124545,
};

enum aughEvent
{
    ACTION_START_EVENT,
};

#endif
