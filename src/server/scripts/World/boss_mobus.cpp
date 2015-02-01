#include "ScriptMgr.h"
#include "ScriptedCreature.h"

enum Spells
{
    SPELL_ALGAE      = 93491,
    SPELL_RAM        = 93492,
    SPELL_WAKE       = 93494,
    SPELL_ENRAGE     =  8599,  // Retail @ 4%
};

enum Events
{
    EVENT_ALGAE,
    EVENT_RAM,
    EVENT_WAKE,
    EVENT_ENRAGE
};

class boss_mobus : public CreatureScript
{
public:
    boss_mobus() : CreatureScript("boss_mobus") { }

    struct boss_mobusAI : public ScriptedAI
    {
        boss_mobusAI(Creature* creature) : ScriptedAI(creature) { }

        void Reset()
        {
            events.Reset();
            enraged = false;
        }

        void EnterCombat(Unit* /*who*/)
        {
            events.ScheduleEvent(EVENT_ALGAE, 10000);
            events.ScheduleEvent(EVENT_RAM, 25000);
            events.ScheduleEvent(EVENT_WAKE, 15000);
        }

        void EnterEvadeMode()
        {
            Unit* target = me->GetVictim();

            if (target && target->isAlive() && target->GetTypeId() == TYPEID_PLAYER)
                return;
            else
                ScriptedAI::EnterEvadeMode();
        }

        void DamageTaken(Unit* /*attacker*/, uint32& /*damage*/)
        {
            if(!enraged && me->HealthBelowPct(4))
            {
                enraged = true;
                DoCast(me, SPELL_ENRAGE);
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
                case EVENT_ALGAE:
                    DoCastRandom(SPELL_ALGAE, 80.0f);
                    events.ScheduleEvent(EVENT_ALGAE, urand(20*IN_MILLISECONDS, 30*IN_MILLISECONDS));
                    break;
                case EVENT_RAM:
                    DoCastRandom(SPELL_RAM, 50.0f);
                    events.ScheduleEvent(EVENT_RAM, urand(15*IN_MILLISECONDS, 25*IN_MILLISECONDS));
                    break;
                case EVENT_WAKE:
                    DoCast(SPELL_WAKE);
                    events.ScheduleEvent(EVENT_WAKE, urand(25*IN_MILLISECONDS, 35*IN_MILLISECONDS));
                    break;
                }
            }
            DoMeleeAttackIfReady();
        }
    private:
        EventMap events;
        bool enraged;
    };

    CreatureAI* GetAI(Creature* creature) const
    {
        return new boss_mobusAI (creature);
    }
};

void AddSC_boss_mobus()
{
    new boss_mobus();
};