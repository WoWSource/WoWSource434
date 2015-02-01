#include "ScriptMgr.h"

enum Spells
{
   SPELL_FURY_OF_THE_SANDS         = 94946,
   SPELL_FURY_OF_THE_SANDS_SUM     = 93574,
   SPELL_SANDS_OF_TIME             = 93578, // how much targets? 15 or 22?
   SPELL_SHOCKWAVE                 = 94968,
   SPELL_STONE_MANTLE              = 93561
};

enum Events
{
   EVENT_FURY                      = 1,
   EVENT_SHOCK,
   EVENT_STONE,
   EVENT_FURY_SUM,
   EVENT_BELOW50,
};

enum Misc
{
   DATA_BELOW_50                   = 1,
};

class boss_akmahat : public CreatureScript
{
public:
    boss_akmahat() : CreatureScript("boss_akmahat") { }

    CreatureAI* GetAI(Creature* creature) const
    {
        return new boss_akmahatAI (creature);
    }

    struct boss_akmahatAI : public ScriptedAI
    {
        boss_akmahatAI(Creature* creature) : ScriptedAI(creature) { }

        void Reset()
        {
            below50 = false;
            me->RemoveAllAuras();
        }

        void EnterCombat(Unit* /*who*/)
        {
            events.ScheduleEvent(EVENT_FURY_SUM, 2000);
            events.ScheduleEvent(EVENT_FURY, 3000);
            events.ScheduleEvent(EVENT_SHOCK, 12000);
            events.ScheduleEvent(EVENT_STONE, 21000);
        }

        void EnterEvadeMode()
        {
            Unit* target = me->GetVictim();

            if (target && target->isAlive() && target->GetTypeId() == TYPEID_PLAYER)
                return;
            else
                ScriptedAI::EnterEvadeMode();
        }

        void DamageTaken(Unit* /*done_by*/, uint32 &damage)
        {
            if(!below50 && me->HealthBelowPctDamaged(50, damage))
            {
                below50 = true;
                events.ScheduleEvent(EVENT_BELOW50, 5000);
            }
        }

        void UpdateAI(const uint32 diff)
        {
            if(!UpdateVictim())
                return;

            events.Update(diff);

            if(me->HasUnitState(UNIT_STATE_CASTING))
                return;

            if(uint32 eventId = events.ExecuteEvent())
            {
                switch(eventId)
                {
                case EVENT_FURY_SUM:
                    DoCast(SPELL_FURY_OF_THE_SANDS_SUM);
                    events.ScheduleEvent(EVENT_FURY_SUM, 17000);
                    events.ScheduleEvent(EVENT_FURY, 18000);
                    break;
                case EVENT_FURY:
                    DoCast(SPELL_FURY_OF_THE_SANDS);
                    break;
                case EVENT_SHOCK:
                    DoCast(SPELL_SHOCKWAVE);
                    events.ScheduleEvent(EVENT_SHOCK, urand(35*IN_MILLISECONDS, 45*IN_MILLISECONDS));
                    break;
                case EVENT_STONE:
                    DoCast(SPELL_STONE_MANTLE);
                    events.ScheduleEvent(EVENT_STONE, urand(80*IN_MILLISECONDS, 90*IN_MILLISECONDS));
                    break;
                case EVENT_BELOW50:
                    DoCastRandom(SPELL_SANDS_OF_TIME, 60.0f);
                    events.ScheduleEvent(EVENT_BELOW50, urand(25*IN_MILLISECONDS, 35*IN_MILLISECONDS));
                    break;
                }
            }

            DoMeleeAttackIfReady();
        }
    private:
        bool below50;
        EventMap events;
    };
};

class npc_fury_of_sands : public CreatureScript
{
public:
    npc_fury_of_sands() : CreatureScript("npc_fury_of_sands") {}

    struct npc_fury_of_sandsAI : public ScriptedAI
    {
        npc_fury_of_sandsAI(Creature* creature) : ScriptedAI(creature) {}

        void Reset()
        {
            active = false;
            activeTimer = 4000;
        }

        void UpdateAI(uint32 const diff)
        {
            if (!active)
            {
                if (activeTimer <= diff)
                {
                    active = true;
                    me->RemoveAllAuras();
                    me->DespawnOrUnsummon(5000);
                }else activeTimer -= diff;
            }
        }

    private:
        bool active;
        uint32 activeTimer;
    };

    CreatureAI* GetAI(Creature* creature) const
    {
        return new npc_fury_of_sandsAI(creature);
    }
};

void AddSC_boss_akmahat()
{
    new boss_akmahat();
    new npc_fury_of_sands();
};