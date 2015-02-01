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

#ifndef TRINITYCORE_PET_DEFINES_H
#define TRINITYCORE_PET_DEFINES_H

#include "Unit.h"

enum PetType
{
    SUMMON_PET              = 0,
    HUNTER_PET              = 1,
    MAX_PET_TYPE            = 4
};

enum PetState
{
    PET_STATE_NONE              = 0,
    PET_STATE_ALIVE             = 1,
    PET_STATE_DEAD              = 2
};

enum PetTameResult
{
    PET_TAME_ERROR_UNKNOWN_ERROR            = 0,
    PET_TAME_ERROR_INVALID_CREATURE         = 1,
    PET_TAME_ERROR_TOO_MANY_PETS            = 2,
    PET_TAME_ERROR_CREATURE_ALREADY_OWNED   = 3,
    PET_TAME_ERROR_NOT_TAMEABLE             = 4,
    PET_TAME_ERROR_ANOTHER_SUMMON_ACTIVE    = 5,
    PET_TAME_ERROR_YOU_CANT_TAME            = 6,
    PET_TAME_ERROR_NO_PET_AVAILABLE         = 7,
    PET_TAME_ERROR_INTERNAL_ERROR           = 8,
    PET_TAME_ERROR_TOO_HIGH_LEVEL           = 9,
    PET_TAME_ERROR_DEAD                     = 10,
    PET_TAME_ERROR_NOT_DEAD                 = 11,
    PET_TAME_ERROR_CANT_CONTROL_EXOTIC      = 12,
    PET_TAME_ERROR_INVALID_SLOT             = 13 
};

#define MAX_PET_STABLES         PET_SLOT_STABLE_LAST

// stored in character_pet.slot
enum PetSaveMode
{
    PET_SAVE_AS_DELETED        = -1,                        // not saved in fact
    PET_SAVE_AS_CURRENT        =  0,                        // in current slot (with player)
    PET_SAVE_FIRST_STABLE_SLOT =  1,
    PET_SAVE_LAST_STABLE_SLOT  =  MAX_PET_STABLES,          // last in DB stable slot index (including), all higher have same meaning as PET_SLOT_ACTUAL_PET_SLOT
    PET_SAVE_NOT_IN_SLOT       =  100                       // for avoid conflict with stable size grow will use 100
};

enum HappinessState
{
    UNHAPPY = 1,
    CONTENT = 2,
    HAPPY   = 3
};

enum PetSpellType
{
    PETSPELL_NONE   = -1,
    PETSPELL_NORMAL = 0,
    PETSPELL_FAMILY = 1,
    PETSPELL_TALENT = 2
};

enum ActionFeedback
{
    FEEDBACK_NONE            = 0,
    FEEDBACK_PET_DEAD        = 1,
    FEEDBACK_NOTHING_TO_ATT  = 2,
    FEEDBACK_CANT_ATT_TARGET = 3
};

enum PetTalk
{
    PET_TALK_SPECIAL_SPELL  = 0,
    PET_TALK_ATTACK         = 1
};

struct PetSpellData
{
    ActiveStates    active;
    PetSpellType    type;
    DataState       _state;
    uint32          Id;
};

typedef UNORDERED_MAP<uint32, PetSpellData> PetSpellMap;


struct PetSpellCooldownData
{
    uint32          time;
    DataState       _state;
};
typedef UNORDERED_MAP<uint32, PetSpellCooldownData> PetSpellCooldown;

struct PetAuraData
{
    int32           damage[3];
    int32           baseDamage[3];
    uint64          caster_guid;
    uint32          spellid;
    uint8           effmask;
    uint8           recalculatemask;
    uint8           stackcount;
    int32           maxduration;
    int32           remaintime;
    uint8           remaincharges;

    DataState       _state;
};

typedef std::vector<PetAuraData> PetAuraMap;

struct PetData
{
   PetData() : state(PET_STATE_NONE), _state(DATA_NEW) {}
   
   uint32               id;
   uint32               entry;
   uint32               owner;
   uint32               modelid;
   uint16               level;
   uint32               exp;
   ReactStates          reactstate;
   uint32               slot;
   std::string          name;
   bool                 renamed;
   uint32               curhealth;
   uint32               curmana;
   std::string          abdata;
   uint32               summon_spell_id;
   uint32               savetime;
   PetType              pet_type;
   PetState             state;
   PetSpellCooldown     m_spellscooldown;
   PetAuraMap           m_auras;
   std::list<PetSpellData> m_learnSpells;

   DataState            _state;
};

typedef UNORDERED_MAP<uint32, PetData*> Stable;

#define PET_FOLLOW_DIST  1.0f
#define PET_FOLLOW_ANGLE (M_PI/2)

#endif
