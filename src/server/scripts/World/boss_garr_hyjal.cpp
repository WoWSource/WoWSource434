#include "ScriptMgr.h"
#include "ScriptedCreature.h"

enum Spells
{
   SPELL_ANTIMAGIC_PULSE     = 93506,
   SPELL_MAGMA_SHACKLES      = 93507,
   SPELL_MASSIVE_ERUPTION    = 93508,

   // Adds
   SPELL_ERUPTION            = 93512,
   SPELL_IMMOLATE            = 15732
};

enum Events
{
   EVENT_ANTIMAGIC           = 1,
   EVENT_SHACKLES,
   EVENT_MASSIVE_ERUPTION,
   EVENT_ERUPTION,
   EVENT_IMMOLATE
};

enum Misc
{
    NPC_GARR_HYJAL           = 50056,
    NPC_FIRESWORN            = 50055
};

class boss_garr_hyjal : public CreatureScript
{
public:
    boss_garr_hyjal() : CreatureScript("boss_garr_hyjal") { }

    struct boss_garr_hyjalAI : public ScriptedAI
    {
        boss_garr_hyjalAI(Creature* creature) : ScriptedAI(creature)
        {
            me->ApplySpellImmune(0, IMMUNITY_MECHANIC, MECHANIC_INTERRUPT, true);
        }

        void Reset()
        {
            events.Reset();
            me->SetReactState(REACT_AGGRESSIVE);
        }

        void EnterEvadeMode()
        {
            Unit* target = me->GetVictim();

            if (target && target->isAlive() && target->GetTypeId() == TYPEID_PLAYER)
                return;
            else
                ScriptedAI::EnterEvadeMode();
        }

        void EnterCombat(Unit* /*who*/)
        {
            events.ScheduleEvent(EVENT_ANTIMAGIC, 9000);
            events.ScheduleEvent(EVENT_SHACKLES, 30000);
            events.ScheduleEvent(EVENT_MASSIVE_ERUPTION, 31000);
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
                case EVENT_ANTIMAGIC:
                    DoCastAOE(SPELL_ANTIMAGIC_PULSE);
                    events.ScheduleEvent(EVENT_ANTIMAGIC, 27000);
                    break;
                case EVENT_SHACKLES:
                    DoCastAOE(SPELL_MAGMA_SHACKLES);
                    events.ScheduleEvent(EVENT_SHACKLES, 30000);
                    break;
                case EVENT_MASSIVE_ERUPTION:
                    DoCast(SPELL_MASSIVE_ERUPTION);
                    events.ScheduleEvent(EVENT_MASSIVE_ERUPTION, 31000);
                    break;
                }
            }
            DoMeleeAttackIfReady();
        }
    private:
        EventMap events;
    };

    CreatureAI* GetAI(Creature* creature) const
    {
        return new boss_garr_hyjalAI (creature);
    }
};

class mob_garr_firesworn : public CreatureScript
{
public:
    mob_garr_firesworn() : CreatureScript("mob_garr_firesworn") { }

    struct mob_garr_fireswornAI : public ScriptedAI
    {
        mob_garr_fireswornAI(Creature* creature) : ScriptedAI(creature) { }

        void Reset()
        {
            events.Reset();
            me->SetReactState(REACT_DEFENSIVE);
        }

        void EnterCombat(Unit* /*who*/)
        {
            events.ScheduleEvent(EVENT_ERUPTION, 5000);
            events.ScheduleEvent(EVENT_IMMOLATE, 10000);

            if (Creature* garr = me->FindNearestCreature(NPC_GARR_HYJAL, 150.0f, true))
            {
                if (Unit* player = me->FindNearestPlayer(200.0f, true))
                    garr->AI()->AttackStart(player);
            }
        }

        void EnterEvadeMode()
        {
            Unit* target = me->GetVictim();

            if (target && target->isAlive() && target->GetTypeId() == TYPEID_PLAYER)
                return;
            else
                ScriptedAI::EnterEvadeMode();
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
                case EVENT_ERUPTION:
                    DoCastAOE(SPELL_ERUPTION);
                    events.ScheduleEvent(EVENT_ERUPTION, urand(15*IN_MILLISECONDS, 20*IN_MILLISECONDS));
                    break;
                case EVENT_IMMOLATE:
                    DoCastRandom(SPELL_IMMOLATE, 30.0f);
                    events.ScheduleEvent(EVENT_IMMOLATE, urand(20*IN_MILLISECONDS, 25*IN_MILLISECONDS));
                    break;
                }
            }
            DoMeleeAttackIfReady();
        }

    private:
        EventMap events;
    };

    CreatureAI* GetAI(Creature* creature) const
    {
        return new mob_garr_fireswornAI (creature);
    }
};

void AddSC_boss_garr_hyjal()
{
    new boss_garr_hyjal();
    new mob_garr_firesworn();
};