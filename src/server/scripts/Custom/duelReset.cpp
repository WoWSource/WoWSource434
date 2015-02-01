/*
 * Copyright (C) 2008-2012 TrinityCore <http://www.trinitycore.org/>
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

/* ScriptData
SDName: DuelResetCooldown
SD%Complete: 100%
SDComment: reset cooldowns in Elewynn Forest and Durotar.
SDCategory:
EndScriptData */

#include "ScriptPCH.h"
#include "Config.h"
#include "World.h"

class DuelResetCooldown : public PlayerScript
{
    public:
        DuelResetCooldown() : PlayerScript("DuelResetCooldown") {}

    void OnDuelEnd(Player* winner, Player* loser, DuelCompleteType type)
    {
        // reset cooldowns in Elewynn Forest and Durotar
        if ((ConfigMgr::GetIntDefault("DuelResetCooldown.Enable", 1)) && 
        (winner->GetAreaId() == (ConfigMgr::GetIntDefault("DuelReset.Area.One", NULL)) || winner->GetAreaId() == (ConfigMgr::GetIntDefault("DuelReset.Area.Two", NULL))))
        {
            winner->ResetAllPowers();
            winner->RemoveArenaSpellCooldowns(true);
            loser->ResetAllPowers();
            loser->RemoveArenaSpellCooldowns(true);
        }
    }
};

void AddSC_DuelReset()
{
    new DuelResetCooldown;
}
