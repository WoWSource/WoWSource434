/* ScriptData
SDScript by Loukie
SDName: boss_asira_dawnslayer
SD%Complete: 40% or less
SDCategory: Hour of Twilight
EndScriptData */

#include"ScriptPCH.h"
#include"hour_of_twilight.h"

enum eAsira_dawnslayerYells 
{
    SAY_AGGRO       = 0,
    SAY_SMOKE_BOMB  = 1,
    SAY_KILL        = 2,
    SAY_DEATH       = 3
};

enum Spells 
{
    SPELL_MARK_OF_SILENCE        = 102726,
    SPELL_SILENCED               = 103587,
    SPELL_CHOKING_SMOKE_BOMB     = 103558,
    SPELL_BLADE_BARRIER          = 103419,
    LESSER_BLADE_BARRIER         = 103562
};

enum Events 
{
    EVENT_MARK_OF_SILENCE,
    EVENT_CHOKING_SMOKE_BOMB,
    EVENT_LESSER_BLADE_BARRIER
};

class boss_asira_dawnslayer : public CreatureScript
{
public:
    boss_asira_dawnslayer() : CreatureScript("boss_asira_dawnslayer") { }

    CreatureAI* GetAI(Creature* creature) const
    {
        return new boss_asira_dawnslayerAI (creature);
    }

    struct boss_asira_dawnslayerAI : public BossAI
    {
        boss_asira_dawnslayerAI(Creature* creature) : BossAI(creature, DATA_ASIRA_EVENT)
        {
            instance = creature->GetInstanceScript();
        }
		
        bool casted;
		
        uint32 m_uiHealthAmountModifier;
        InstanceScript *instance;
        EventMap events;

        void Reset() 
        {
            casted = false;
            m_uiHealthAmountModifier = 1;
            events.Reset();
        }

        void JustDied(Unit* /*Kill*/)
        {
            Talk(SAY_DEATH);
            if (instance)
            instance->SetData(DATA_ASIRA_EVENT, DONE);
        }
			
        void KilledUnit(Unit* /*victim*/)
        {
            Talk(SAY_KILL);
        }		
		
        void EnterCombat(Unit* /*Ent*/)
        {
            Talk(SAY_AGGRO);
            DoCast(SPELL_MARK_OF_SILENCE);
            events.ScheduleEvent(EVENT_MARK_OF_SILENCE, urand(10000,11000));
            events.ScheduleEvent(EVENT_CHOKING_SMOKE_BOMB, urand(17000,20000));
            DoZoneInCombat();

            if (instance)
                instance->SetData(DATA_ASIRA_EVENT, IN_PROGRESS);
        }

        void UpdateAI(uint32 const diff)
        {
            if (!UpdateVictim())
                return;

            if (me->HasUnitState(UNIT_STATE_CASTING))
                return;			
				
            events.Update(diff);

            while (uint32 eventId = events.ExecuteEvent()) 
            {
                switch (eventId)
                {
                    case EVENT_MARK_OF_SILENCE:
                        if (Unit* target = SelectTarget(SELECT_TARGET_RANDOM, 0))
                           DoCast(SPELL_MARK_OF_SILENCE);
                           events.ScheduleEvent(EVENT_MARK_OF_SILENCE, 11000);
                        break;

                    case EVENT_CHOKING_SMOKE_BOMB:
                           Talk(SAY_SMOKE_BOMB);
                           DoCast(SPELL_CHOKING_SMOKE_BOMB);
                           events.ScheduleEvent(EVENT_CHOKING_SMOKE_BOMB, 17000);
                        break;
						
                    case EVENT_LESSER_BLADE_BARRIER:
                         DoCast(me, LESSER_BLADE_BARRIER, true);
                        break;					

					default:
                       break;
				}
            }

            // Health check
            if (HealthBelowPct(30 * m_uiHealthAmountModifier) && !casted)
            {
                DoCast(me, SPELL_BLADE_BARRIER);
                events.ScheduleEvent(EVENT_LESSER_BLADE_BARRIER, 12000);
                casted = true;
            }			
            DoMeleeAttackIfReady();
        }
    };
};

// 103558 - Choking Smoke Bomb
class spell_choking_smoke_bomb : public SpellScriptLoader
{
    public:
        spell_choking_smoke_bomb() : SpellScriptLoader("spell_choking_smoke_bomb") { }

        class spell_choking_smoke_bomb_AuraScript : public AuraScript
        {
            PrepareAuraScript(spell_choking_smoke_bomb_AuraScript);
       
            void OnTick(AuraEffect const* aurEff)
            {
                Unit* caster = GetCaster();

                if(!caster)
                    return;

                if (DynamicObject* dynObj = caster->GetDynObject(103558))
                {   
                    // Casts AOE interfere targeting aura
                    caster->CastSpell(dynObj->GetPositionX(), dynObj->GetPositionY(), dynObj->GetPositionZ(), 103790, true);
                }
            }

            void Register()
            {
                OnEffectPeriodic += AuraEffectPeriodicFn(spell_choking_smoke_bomb_AuraScript::OnTick, EFFECT_1, SPELL_AURA_PERIODIC_DUMMY);
            }
        };
    
        AuraScript* GetAuraScript() const
        {
            return new spell_choking_smoke_bomb_AuraScript();
        }
};

void AddSC_boss_asira_dawnslayer() 
{
    new boss_asira_dawnslayer();
    new spell_choking_smoke_bomb();
}
