#include "ScriptMgr.h"
#include "ScriptedCreature.h"

// ToDo: Find correct VehicleId (used the Erunak Stonespeaker one) | Timers

enum Spells
{
   SPELL_BLACK_BREATH       = 93611,
   SPELL_DARK_WHISPERS      = 93621,
   SPELL_MASSIVE_SHOCKWAVE  = 93610,

   //Misc Spells
   SPELL_GENERIC_GROW       = 77466,
   SPELL_GAZE_SUMMON        = 94682,

   SPELL_MES_GAZE           = 94664,
};

enum Events
{
   EVENT_BLACK_BREATH       = 1,
   EVENT_DARK_WHISPERS,
   EVENT_MASSIVE_SHOCK,
   EVENT_MES_GAZE,
};

class boss_julak_doom : public CreatureScript
{
public:
    boss_julak_doom() : CreatureScript("boss_julak_doom") { }

    CreatureAI* GetAI(Creature* creature) const
    {
        return new boss_julak_doomAI (creature);
    }

    struct boss_julak_doomAI : public ScriptedAI
    {
        boss_julak_doomAI(Creature* creature) : ScriptedAI(creature) { }

        void Reset()
        {
            me->SetReactState(REACT_AGGRESSIVE);
        }

        void EnterCombat(Unit* /*who*/)
        {
            events.ScheduleEvent(EVENT_DARK_WHISPERS, 50000);
            events.ScheduleEvent(EVENT_BLACK_BREATH, 30000);
            events.ScheduleEvent(EVENT_MASSIVE_SHOCK, 20000);
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
                case EVENT_DARK_WHISPERS:
                    DoCastRandom(SPELL_DARK_WHISPERS, 60.0f);
                    events.ScheduleEvent(EVENT_DARK_WHISPERS, urand(50*IN_MILLISECONDS, 70*IN_MILLISECONDS));
                    break;
                case EVENT_BLACK_BREATH:
                    DoCastRandom(SPELL_BLACK_BREATH, 60.0f);
                    events.ScheduleEvent(EVENT_BLACK_BREATH, urand(30*IN_MILLISECONDS, 60*IN_MILLISECONDS));
                    break;
                case EVENT_MASSIVE_SHOCK:
                    if (Unit* target = SelectTarget(SELECT_TARGET_TOPAGGRO, 0))
                    {
                        DoCast(target, SPELL_MASSIVE_SHOCKWAVE);
                    }
                    events.ScheduleEvent(EVENT_MASSIVE_SHOCK, urand(20*IN_MILLISECONDS, 40*IN_MILLISECONDS));
                    break;
                }
            }
            DoMeleeAttackIfReady();
        }
    private:
        EventMap events;
    };
};

class ExactDistanceCheck
{
    public:
        ExactDistanceCheck(Unit* source, float dist) : _source(source), _dist(dist) {}

        bool operator()(WorldObject* unit)
        {
            return _source->GetExactDist2d(unit) > _dist;
        }

    private:
        Unit* _source;
        float _dist;
};

class npc_julak_mindbender : public CreatureScript
{
public:
    npc_julak_mindbender() : CreatureScript("npc_julak_mindbender") { }

    struct npc_julak_mindbenderAI : public ScriptedAI
    {
        npc_julak_mindbenderAI(Creature* creature) : ScriptedAI(creature)
        {
           me->SetFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_NON_ATTACKABLE | UNIT_FLAG_NOT_SELECTABLE);
        }

        void Reset()
        {
            events.Reset();
        }

        void EnterCombat(Unit* /*who*/)
        {
            events.ScheduleEvent(EVENT_MES_GAZE, 30000);
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
                case EVENT_MES_GAZE:
                    DoCastRandom(SPELL_MES_GAZE, 60.0f);
                    events.ScheduleEvent(EVENT_MES_GAZE, urand(20*IN_MILLISECONDS, 50*IN_MILLISECONDS));
                    break;
                }
            }
        }
    private:
        EventMap events;
    };

    CreatureAI* GetAI(Creature* creature) const
    {
        return new npc_julak_mindbenderAI (creature);
    }
};

class spell_julakdoom_dark_breath : public SpellScriptLoader
{
    public:
        spell_julakdoom_dark_breath() : SpellScriptLoader("spell_julakdoom_dark_breath") { }

        class spell_julakdoom_dark_breath_SpellScript : public SpellScript
        {
            PrepareSpellScript(spell_julakdoom_dark_breath_SpellScript);

            void CorrectRange(std::list<WorldObject*>& targets)
            {
                targets.remove_if(ExactDistanceCheck(GetCaster(), 10.0f * GetCaster()->GetFloatValue(OBJECT_FIELD_SCALE_X)));
            }

            void JulakGrow()
            {
                GetCaster()->CastSpell(GetCaster(), SPELL_GENERIC_GROW, true);
            }

            void Register()
            {
                OnObjectAreaTargetSelect += SpellObjectAreaTargetSelectFn(spell_julakdoom_dark_breath_SpellScript::CorrectRange, EFFECT_0, TARGET_UNIT_SRC_AREA_ENEMY);
                OnHit += SpellHitFn(spell_julakdoom_dark_breath_SpellScript::JulakGrow);
            }
        };

        SpellScript* GetSpellScript() const
        {
            return new spell_julakdoom_dark_breath_SpellScript();
        }
};

void AddSC_boss_julak_doom()
{
    new boss_julak_doom();
    new npc_julak_mindbender();
    new spell_julakdoom_dark_breath();
};
