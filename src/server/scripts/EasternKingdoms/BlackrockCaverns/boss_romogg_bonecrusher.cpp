/*
 * Copyright (C) 2011-2012 Project SkyFire <http://www.projectskyfire.org/>
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License as published by the
 * Free Software Foundation; either version 3 of the License, or (at your
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

#include "ScriptPCH.h"
#include "blackrock_caverns.h"

enum eRomoggYells
{
    SAY_AGGRO                   = 0,
    SAY_CALL_FOR_HELP           = 1,
    SAY_SKULLCRAKER             = 2,
    SAY_UNLEASH_SKULLCRAKER     = 3,
    SAY_KILL                    = 4,
    SAY_DEATH                   = 5,
};

enum eRomoggSpells
{
    SPELL_CALL_FOR_HELP             = 82137,
    SPELL_CHAINS_OF_WOE             = 75539,
    SPELL_CHAINS_OF_WOE_CHAIN       = 75441,
    SPELL_CHAINS_OF_WOE_TELEPORT    = 75464,
    SPELL_CHAINS_OF_WOE_TELEPORT_H  = 95311,

    SPELL_QUAKE                     = 75272,
    SPELL_QUAKE_TRIGGER             = 75379,
    // Normal
    SPELL_WOUNDING_STRIKE           = 69651,
    SPELL_THE_SKULLCRAKER           = 75543,
    // Heroic
    SPELL_WOUNDING_STRIKE_H         = 93452,
    SPELL_THE_SKULLCRAKER_H         = 93453,
};

class boss_romogg_bonecrusher : public CreatureScript
{
public:
    boss_romogg_bonecrusher() : CreatureScript("boss_romogg_bonecrusher") { }

    CreatureAI* GetAI(Creature* creature) const
    {
        return new boss_romogg_bonecrusherAI (creature);
    }

    struct boss_romogg_bonecrusherAI : public ScriptedAI
    {
        boss_romogg_bonecrusherAI(Creature* creature) : ScriptedAI(creature)
        {
            instance = creature->GetInstanceScript();
        }

        InstanceScript* instance;

        uint32 WoundingStrikeTimer;
        bool ChainsOfWoeCounter;
        uint32 QuakeTimer;

        void Reset()
        {
            ChainsOfWoeCounter = false;

            QuakeTimer = urand(15000, 25000);
            WoundingStrikeTimer = urand(5000, 15000);

            if (instance)
                instance->SetData(BOSS_ROMOGG_BONECRUSHER, NOT_STARTED);
        }

        void KilledUnit(Unit* /*victim*/)
        {
            Talk(SAY_KILL);
        }

        void JustDied(Unit* /*Kill*/)
        {
            Talk(SAY_DEATH);

            if (instance)
                instance->SetData(BOSS_ROMOGG_BONECRUSHER, DONE);

            if (Creature * pRaz = GetClosestCreatureWithEntry(me, NPC_RAZ_THE_CRAZED, 100.0f))
            {
                pRaz->RemoveAurasDueToSpell(SPELL_SHADOW_PRISON);
                // Jump out of the prison
                pRaz->GetMotionMaster()->MoveJump(227.6f, 949.8f, 192.6f, 12.0f, 15.0f);
            }
        }

        void EnterCombat(Unit* /*Ent*/)
        {
            Talk(SAY_AGGRO);

            me->CastSpell(me, SPELL_CALL_FOR_HELP, false);

            if (instance)
                instance->SetData(BOSS_ROMOGG_BONECRUSHER, IN_PROGRESS);
        }

        void JustSummoned(Creature * pcreat)
        {
            if (pcreat->GetEntry() == NPC_CHAINS_OF_WOE)
            {
                Map::PlayerList const& players = instance->instance->GetPlayers();

                // Teleport players near to the boss
                if (!players.isEmpty())
                {
                    for (Map::PlayerList::const_iterator itr = players.begin(); itr != players.end(); ++itr)
                        if (Player * player = itr->getSource())
                        {
                            player->NearTeleportTo(pcreat->GetPositionX()+irand(-1, 1), pcreat->GetPositionY()+irand(-1, 1), pcreat->GetPositionZ(), player->GetOrientation());
                            pcreat->CastSpell(player, SPELL_CHAINS_OF_WOE_CHAIN, true);
                        }
                }

                // Cast The Skullcraker
                me->CastSpell(me, SPELL_THE_SKULLCRAKER, false);
                Talk(SAY_SKULLCRAKER);
                Talk(SAY_UNLEASH_SKULLCRAKER);
            }
            else if (pcreat->GetEntry() == NPC_QUAKE)
            {
                // At quake summon, cast visual and spawn trigger.
                pcreat->CastSpell(pcreat, SPELL_QUAKE_TRIGGER, true, NULL, NULL, me->GetGUID());
            }
            else if (pcreat->GetEntry() == NPC_ANGERED_EARTH)
            {
                // Make summoned units in combat.
                DoZoneInCombat(pcreat);
            }
        }

        void SpellHitTarget(Unit* /*target*/, const SpellInfo* spell)
        {
            // Call nearby creatures to help
            if (spell->Id == SPELL_CALL_FOR_HELP)
                me->CallForHelp(30.0f);
                Talk(SAY_CALL_FOR_HELP);
        }

        void UpdateAI(uint32 const Diff)
        {
            if (!UpdateVictim())
                return;

            // Chains of Woe on 66% and 33% of health
            if ((me->GetHealthPct() < 66.0f && !ChainsOfWoeCounter && me->GetHealthPct() > 33.0f) || (me->GetHealthPct() < 33.0f && ChainsOfWoeCounter))
            {
                ChainsOfWoeCounter = !ChainsOfWoeCounter;

                me->CastSpell(me, SPELL_CHAINS_OF_WOE, false);
            }

            if (WoundingStrikeTimer <= Diff)
            {
                if (me->IsNonMeleeSpellCasted(false))
                    return;

                DoCastVictim(IsHeroic() ? SPELL_WOUNDING_STRIKE_H : SPELL_WOUNDING_STRIKE);

                WoundingStrikeTimer = urand(15000, 30000);
            }
            else
                WoundingStrikeTimer -= Diff;

            if (QuakeTimer <= Diff)
            {
                if (me->IsNonMeleeSpellCasted(false))
                    return;

                me->CastSpell(me, SPELL_QUAKE, false);

                QuakeTimer = urand(20000, 25000);
            }
            else
                QuakeTimer -= Diff;

            DoMeleeAttackIfReady();
        }
    };
};

void AddSC_boss_romogg_bonecrusher()
{
    new boss_romogg_bonecrusher();
}