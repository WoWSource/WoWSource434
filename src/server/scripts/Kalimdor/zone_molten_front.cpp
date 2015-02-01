/*
 * Copyright (C) 2008-2014 TrinityCore <http://www.trinitycore.org/>
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
Scripts by Loukie
SDName: Molten Front
SD%Complete: 20%
SDComment: Placeholder
SDCategory: Molten Front
EndScriptData */

/* ContentData
EndContentData */

#include "ScriptPCH.h"
#include "Spell.h"
#include "SpellAuraEffects.h"
#include "SpellScript.h"

enum Spells
{

};

enum eEvents
{
    // npc_pyrelord
    EVENT_INCINERATE           = 1,
    EVENT_BACKDRAFT            = 2,
    
    //npc_ancient_charhound
    EVENT_FLAME_LASH          = 3,
    EVENT_LEAPING_BITE        = 4,
    EVENT_SLEEPING_SLEEP      = 5,

    //npc_ancient_charhound	
	EVENT_INCATFORM           = 6,
	EVENT_PYROBLAST           = 7,
	EVENT_SCORCH              = 8,
	
	//npc_subterranean_magma_worm
    EVENT_LAVA_SHOWER         = 9,
	EVENT_BURNING_HUNGER      = 10,

    EVENT_ONE                 = 11,
    EVENT_TWO                 = 12,
    EVENT_THREE	              = 13
};

/*######
# npc_pyrelord - 52683
######*/

class npc_pyrelord : public CreatureScript
{
public:
    npc_pyrelord() : CreatureScript("npc_pyrelord") {}
 
       struct npc_pyrelordAI : public ScriptedAI
       {
           npc_pyrelordAI(Creature *c): ScriptedAI(c) {}

       uint32 spell_2_Timer;
       uint32 spell_1_Timer;

           void EnterCombat(Unit* pWho)
           {
                DoCast(79938);
           }

       void Reset()
           {
             spell_1_Timer = 2000;
             spell_2_Timer = 6000;
           }

       void UpdateAI(const uint32 diff)
           {
               if (!UpdateVictim())
               return;

               if(spell_1_Timer <= diff)
               {
                   if (!me->IsWithinMeleeRange(me->GetVictim()))
                   {
                       DoCast(me->GetVictim(), 79938);
                       spell_1_Timer = 2000;
           }
               }else spell_1_Timer -= diff;

               if(spell_2_Timer <= diff)
               {
                   if (!me->IsWithinMeleeRange(me->GetVictim()))
                   {
                       DoCast(me->GetVictim(), 98839);
                       spell_2_Timer = 6000;
           }
               }else spell_2_Timer -= diff;

           }
       };

CreatureAI* GetAI(Creature *pCreature) const
{
        return new npc_pyrelordAI(pCreature);
}
};

/*######
# npc_ancient_charhound - 54339
######*/

class npc_ancient_charhound : public CreatureScript
{
public:
    npc_ancient_charhound() : CreatureScript("npc_ancient_charhound") {}
 
       struct npc_ancient_charhoundAI : public ScriptedAI
       {
           npc_ancient_charhoundAI(Creature *c): ScriptedAI(c) {}

       uint32 spell_1_Timer;
       uint32 spell_2_Timer;
       uint32 spell_3_Timer;

           void EnterCombat(Unit* pWho)
           {
                DoCast(98701);
           }

       void Reset()
           {
             spell_1_Timer = 3000;
             spell_2_Timer = 6000;
             spell_3_Timer = 14000;
           }

       void UpdateAI(const uint32 diff)
           {
               if (!UpdateVictim())
               return;

               if(spell_1_Timer <= diff)
               {
                   if (!me->IsWithinMeleeRange(me->GetVictim()))
                   {
                       DoCast(me->GetVictim(), 3356);
                       spell_1_Timer = 3000;
           }
               }else spell_1_Timer -= diff;

               if(spell_2_Timer <= diff)
               {
                   if (!me->IsWithinMeleeRange(me->GetVictim()))
                   {
                       DoCast(me->GetVictim(), 98701);
                       spell_2_Timer = 6000;
           }
               }else spell_2_Timer -= diff;

               if(spell_3_Timer <= diff)
               {
                   if (!me->IsWithinMeleeRange(me->GetVictim()))
                   {
                       DoCast(me->GetVictim(), 42648);
                       spell_3_Timer = 14000;
           }
               }else spell_3_Timer -= diff;			   
           
		   DoMeleeAttackIfReady();
           }
       };

CreatureAI* GetAI(Creature *pCreature) const
{
        return new npc_ancient_charhoundAI(pCreature);
}
};

/*######
# npc_druid_of_the_flame - 54343
######*/

class npc_druid_of_the_flame : public CreatureScript
{
public:
    npc_druid_of_the_flame() : CreatureScript("npc_druid_of_the_flame") { }

    CreatureAI* GetAI(Creature* pCreature) const
    {
        return new npc_druid_of_the_flameAI(pCreature);
    }

    struct npc_druid_of_the_flameAI : public ScriptedAI
    {
        npc_druid_of_the_flameAI(Creature* c) : ScriptedAI(c)
        {
        }

        EventMap events;

        void Reset()
        {
            events.Reset();
        }

        void EnterCombat(Unit* /*who*/)
        {
            events.ScheduleEvent(EVENT_INCATFORM, 2000);
            events.ScheduleEvent(EVENT_PYROBLAST, 3000);
			events.ScheduleEvent(EVENT_SCORCH, 6000);
        }

        void UpdateAI(const uint32 diff)
        {
            if (!UpdateVictim())
                return;

            events.Update(diff);

            if (me->HasUnitState(UNIT_STATE_CASTING))
                return;

            while (uint32 eventId = events.ExecuteEvent())
            {
                switch(eventId)
                {
                    case EVENT_INCATFORM:
                        DoCast(me, 97678);
					    events.ScheduleEvent(EVENT_PYROBLAST, 3000);
                        return;
                    case EVENT_PYROBLAST:
                        if (Unit *pTarget = SelectTarget(SELECT_TARGET_RANDOM, 0))
                            DoCast(pTarget, 17273);
                        return;
                    case EVENT_SCORCH:
                        if (Unit *pTarget = SelectTarget(SELECT_TARGET_RANDOM, 0))
                            DoCast(pTarget, 13878);
                        return;							
                }
            }
          DoMeleeAttackIfReady();			
        }
    };
};

/*######
# npc_subterranean_magma_worm - 53112
######*/

class npc_subterranean_magma_worm : public CreatureScript
{
public:
    npc_subterranean_magma_worm() : CreatureScript("npc_subterranean_magma_worm") { }

    CreatureAI* GetAI(Creature* pCreature) const
    {
        return new npc_subterranean_magma_wormAI(pCreature);
    }

        struct npc_subterranean_magma_wormAI: public Scripted_NoMovementAI
        {
                npc_subterranean_magma_wormAI(Creature* pCreature) :
                        Scripted_NoMovementAI(pCreature)
                {
                }

        EventMap events;

        void Reset()
        {
            events.Reset();
        }

        void EnterCombat(Unit* /*who*/)
        {
            events.ScheduleEvent(EVENT_LAVA_SHOWER, 15000);
			events.ScheduleEvent(EVENT_BURNING_HUNGER, urand(9000, 17000));
        }

        void UpdateAI(const uint32 diff)
        {
            if (!UpdateVictim())
                return;

            events.Update(diff);

            if (me->HasUnitState(UNIT_STATE_CASTING))
                return;

            while (uint32 eventId = events.ExecuteEvent())
            {
                switch(eventId)
                {
                    case EVENT_LAVA_SHOWER:
                        if (Unit *pTarget = SelectTarget(SELECT_TARGET_RANDOM, 0))
                            DoCast(pTarget, 97549);
                        events.ScheduleEvent(EVENT_LAVA_SHOWER, 55000);
                        return;
                    case EVENT_BURNING_HUNGER:
                        if (Unit *pTarget = SelectTarget(SELECT_TARGET_RANDOM, 0))
                            DoCast(pTarget, 98268);
                        events.RescheduleEvent(EVENT_BURNING_HUNGER, urand(12000, 17000));
                        return;						
                }
            }
           DoSpellAttackIfReady(97306);			
        }
    };
};

/*######
# npc_ancient_firelord - 53864
######*/

class npc_ancient_firelord : public CreatureScript
{
public:
    npc_ancient_firelord() : CreatureScript("npc_ancient_firelord") { }

    CreatureAI* GetAI(Creature* pCreature) const
    {
        return new npc_ancient_firelordAI(pCreature);
    }

    struct npc_ancient_firelordAI : public ScriptedAI
    {
        npc_ancient_firelordAI(Creature* c) : ScriptedAI(c)
        {
        }

        EventMap events;

        void Reset()
        {
            events.Reset();
        }

        void EnterCombat(Unit* /*who*/)
        {
            events.ScheduleEvent(EVENT_ONE, 4000);
            events.ScheduleEvent(EVENT_TWO, 6000);
			events.ScheduleEvent(EVENT_THREE, 9000);
        }

        void UpdateAI(const uint32 diff)
        {
            if (!UpdateVictim())
                return;

            events.Update(diff);

            if (me->HasUnitState(UNIT_STATE_CASTING))
                return;

            while (uint32 eventId = events.ExecuteEvent())
            {
                switch(eventId)
                {
                    case EVENT_ONE:
                        if (Unit *pTarget = SelectTarget(SELECT_TARGET_RANDOM, 0))
                        DoCast(pTarget, 100270);
					    events.ScheduleEvent(EVENT_ONE, 4000);
                        return;
                    case EVENT_TWO:
                        if (Unit *pTarget = SelectTarget(SELECT_TARGET_RANDOM, 0))
                            DoCast(pTarget, 100267);
                            events.ScheduleEvent(EVENT_TWO, 6000);
                        return;
                    case EVENT_THREE:
                        if (Unit *pTarget = SelectTarget(SELECT_TARGET_RANDOM, 0))
                            DoCast(pTarget, 100378);
                            events.ScheduleEvent(EVENT_THREE, 9000);
                        return;							
                }
            }
          DoMeleeAttackIfReady();			
        }
    };
};

void AddSC_molten_front()
{
  new npc_pyrelord();
  new npc_ancient_charhound();
  new npc_druid_of_the_flame();
  new npc_subterranean_magma_worm();
  new npc_ancient_firelord();
}