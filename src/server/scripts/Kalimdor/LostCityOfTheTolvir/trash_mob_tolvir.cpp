/*
 * Copyright (C) 2011- 2013 ArkCORE <http://www.arkania.net/>
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

#include "ScriptPCH.h"
#include "Spell.h"
#include "SpellAuraEffects.h"
#include "SpellScript.h"

/********************
**Bonesnapper Scorpid
*********************/

#define SPELL_PARALYTIC_VENOM        84125
#define SPELL_PARALYTIC_VENOM_H      90022

class mob_bonesnapper_scorpid : public CreatureScript
{
public:
    mob_bonesnapper_scorpid() : CreatureScript("mob_bonesnapper_scorpid") {}
 
       struct mob_bonesnapper_scorpidAI : public ScriptedAI
       {
           mob_bonesnapper_scorpidAI(Creature *c): ScriptedAI(c) {}

       uint32 ParalyticVenom_Timer;

       void Reset()
           {
               ParalyticVenom_Timer = 2000;
       }

       void UpdateAI(const uint32 diff)
           {
               if (!UpdateVictim())
               return;

           if(ParalyticVenom_Timer<= diff)
               {
             DoCast(me->GetVictim(), IsHeroic() ? SPELL_PARALYTIC_VENOM_H : SPELL_PARALYTIC_VENOM);
                     ParalyticVenom_Timer = 6000;          
               }else ParalyticVenom_Timer -= diff;

          DoMeleeAttackIfReady();
           }
       };

CreatureAI* GetAI(Creature *pCreature) const
{
        return new mob_bonesnapper_scorpidAI(pCreature);
}
};

/*******************
**Oathsworn Wanderer
********************/

#define SPELL_CALL_CROCOLISK      82791
#define SPELL_FORKED_FIRESHOT     82794
#define SPELL_FORKED_FIRESHOT_H   89992
#define SPELL_SHOOT               83877

class mob_oathsworn_wanderer : public CreatureScript
{
public:
    mob_oathsworn_wanderer() : CreatureScript("mob_oathsworn_wanderer") {}
 
       struct mob_oathsworn_wandererAI : public ScriptedAI
       {
           mob_oathsworn_wandererAI(Creature *c): ScriptedAI(c) {}

       uint32 ForkedFireShot_Timer;
       uint32 Shoot_Timer;

           void EnterCombat(Unit* pWho)
           {
                DoCast(SPELL_CALL_CROCOLISK);
           }

       void Reset()
           {
               Shoot_Timer = 2000;
           ForkedFireShot_Timer = 13000;
           }

       void UpdateAI(const uint32 diff)
           {
               if (!UpdateVictim())
               return;

               if(Shoot_Timer <= diff)
               {
                   if (!me->IsWithinMeleeRange(me->GetVictim()))
                   {
                       DoCast(me->GetVictim(), SPELL_SHOOT);
                       Shoot_Timer = 2000;
           }
               }else Shoot_Timer -= diff;

               if(ForkedFireShot_Timer <= diff)
               {
                   if (!me->IsWithinMeleeRange(me->GetVictim()))
                   {
                       DoCast(me->GetVictim(),IsHeroic() ? SPELL_FORKED_FIRESHOT_H : SPELL_FORKED_FIRESHOT);
                       ForkedFireShot_Timer = 15000;
           }
               }else ForkedFireShot_Timer -= diff;

          DoMeleeAttackIfReady();
           }
       };

CreatureAI* GetAI(Creature *pCreature) const
{
        return new mob_oathsworn_wandererAI(pCreature);
}
};

/*****************
**Sharptalon Eagle
******************/

#define SPELL_EAGLE_CLAW       89212
#define SPELL_EAGLE_CLAW_H     89988

class mob_sharptalon_eagle : public CreatureScript
{
public:
    mob_sharptalon_eagle() : CreatureScript("mob_sharptalon_eagle") {}
 
       struct mob_sharptalon_eagleAI : public ScriptedAI
       {
           mob_sharptalon_eagleAI(Creature *c): ScriptedAI(c) {}

       uint32 EagleClaw_Timer;

       void Reset()
           {
           EagleClaw_Timer = 2000;
           }

       void UpdateAI(const uint32 diff)
           {
               if (!UpdateVictim())
               return;

           if(EagleClaw_Timer <= diff)
               {
                DoCast(me->GetVictim(), IsHeroic() ? SPELL_EAGLE_CLAW_H : SPELL_EAGLE_CLAW);
                    EagleClaw_Timer = 15000;
               }else EagleClaw_Timer -= diff;                          

          DoMeleeAttackIfReady();
           }
       };

CreatureAI* GetAI(Creature *pCreature) const
{
        return new mob_sharptalon_eagleAI(pCreature);
}
};

void AddSC_trash_lost_city_of_tolvir()
{
     new mob_bonesnapper_scorpid();
     new mob_oathsworn_wanderer();                                               
     new mob_sharptalon_eagle();
}