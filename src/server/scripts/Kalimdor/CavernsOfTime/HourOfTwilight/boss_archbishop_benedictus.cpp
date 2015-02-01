
/* ScriptData
SDScript by Loukie
SDName: boss_archbishop_benedictus
SD%Complete: 40%
SDToDo: Scripts SPELL_PURIFYING_LIGHT || SPELL_WAVE_VIRTUE || SPELL_CORRUPTING_TWILIGHT || SPELL_WAVE_TWILIGHT || SPEL_TWILIGHT_EPIPHANY
SDCategory: Hour of Twilight
EndScriptData */
 
#include"ScriptPCH.h"
#include"hour_of_twilight.h"

enum
{
    // ToDo: add spells and yells here
    SAY_EMOTE_WAVE_TWILIGHT      = 9,
    SAY_EMOTE_WAVE_VIRTUE        = 8,
    SAY_WAVE_TWILIGHT            = 7,
    SAY_WAVE_VIRTUE              = 6,
    SAY_POWER                    = 5,
    SAY_TRANSFORMATION           = 4,
    SAY_DEATH                    = 2,
    SAY_KILL                     = 1,
    SAY_AGGRO                    = 0,
};

enum Spells 
{
    SPELL_RIGHTEOUS_SHEAR       = 103151, //First Pase
    SPELL_PURIFYING_LIGHT       = 103565,
    SPELL_WAVE_VIRTUE           = 103678,
	
    SPELL_TWILIGHT_SHEAR		= 103363, //Second Phase
    SPELL_CORRUPTING_TWILIGHT	= 103767,
    SPELL_WAVE_TWILIGHT			= 103780,
	
    SPELL_TWILIGHT_EPITAPHY_2   = 103755,
    SPELL_TWILIGHT_EPITAPHY     = 103754
};

enum Events 
{
    EVENT_RIGHTEOUS_SHEAR       = 1, //First Pase
    EVENT_PURIFYING_LIGHT       = 2,
    EVENT_WAVE_VIRTUE           = 3,
	
    EVENT_TWILIGHT_SHEAR		= 4, //Second Phase
    EVENT_CORRUPTING_TWILIGHT	= 5,
    EVENT_WAVE_TWILIGHT			= 6,
	
    EVENT_SAY_TRANSFORMATION    = 7,
    EVENT_TWILIGHT_EPITAPHY_2   = 8,
};

class boss_archbishop_benedictus : public CreatureScript
{
public:
    boss_archbishop_benedictus() : CreatureScript("boss_archbishop_benedictus") { }

    CreatureAI* GetAI(Creature* creature) const
    {
        return new boss_archbishop_benedictusAI (creature);
    }

    struct boss_archbishop_benedictusAI : public BossAI
    {
        boss_archbishop_benedictusAI(Creature* creature) : BossAI(creature, DATA_ARCHBISHOP_EVENT)
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
            events.Reset();
            m_uiHealthAmountModifier = 1;
        }

        void JustDied(Unit* /*Kill*/)
        {
            Talk(SAY_DEATH);

            if (instance)
            instance->DoRemoveAurasDueToSpellOnPlayers(SPELL_TWILIGHT_EPITAPHY_2);
            instance->SetData(DATA_ARCHBISHOP_EVENT, DONE);

			
        }

        void KilledUnit(Unit* /*victim*/)
        {
            Talk(SAY_KILL);
		
        }				
		
        void EnterCombat(Unit* /*Ent*/)
        {
            Talk(SAY_AGGRO);
            events.ScheduleEvent(EVENT_RIGHTEOUS_SHEAR, 4000);
            events.ScheduleEvent(EVENT_PURIFYING_LIGHT, 6000);
            events.ScheduleEvent(EVENT_WAVE_VIRTUE, 30000);			
            DoZoneInCombat();
            if (instance)
            instance->SetData(DATA_ARCHBISHOP_EVENT, IN_PROGRESS);
        }
				
        void UpdateAI(uint32 const diff)
        {
            if (!UpdateVictim())
                return;

            if (me->HasUnitState(UNIT_STATE_CASTING))
                return;
								
                // Health check
               if (me->HealthBelowPct(60 * m_uiHealthAmountModifier) && !casted)
                {
				
                    events.ScheduleEvent(EVENT_RIGHTEOUS_SHEAR, 2000);
                    events.ScheduleEvent(EVENT_PURIFYING_LIGHT, 3000);
                    events.ScheduleEvent(EVENT_WAVE_VIRTUE, 30000);	
                    events.Reset();	// need better way to rest events				
                    events.ScheduleEvent(EVENT_SAY_TRANSFORMATION, 1);
                    events.ScheduleEvent(EVENT_TWILIGHT_EPITAPHY_2, 30000);
                    casted = true;
                    me->SetDisplayId(38992);
                    events.ScheduleEvent(EVENT_TWILIGHT_SHEAR, 4000);
                    events.ScheduleEvent(EVENT_CORRUPTING_TWILIGHT, 6000);
                    events.ScheduleEvent(EVENT_WAVE_TWILIGHT, 30000);						
                }
								
            events.Update(diff);

            while (uint32 eventId = events.ExecuteEvent()) 
            {
                switch (eventId)
                {
                    case EVENT_RIGHTEOUS_SHEAR:
                        if (Unit* target = SelectTarget(SELECT_TARGET_RANDOM, 0))
                        DoCast(SPELL_RIGHTEOUS_SHEAR);
                        events.ScheduleEvent(EVENT_RIGHTEOUS_SHEAR, 1500);
                        break;
    
                    case EVENT_PURIFYING_LIGHT:
                       if (Unit* target = SelectTarget(SELECT_TARGET_RANDOM, 0))
                       DoCast(SPELL_PURIFYING_LIGHT);
                        events.ScheduleEvent(EVENT_PURIFYING_LIGHT, 17000);
                        break;

                    case EVENT_TWILIGHT_SHEAR:
                        if (Unit* target = SelectTarget(SELECT_TARGET_RANDOM, 0))
                        DoCast(SPELL_TWILIGHT_SHEAR);
                        events.ScheduleEvent(EVENT_TWILIGHT_SHEAR, 1500);
                        break;
    
                    case EVENT_CORRUPTING_TWILIGHT:
                        if (Unit* target = SelectTarget(SELECT_TARGET_RANDOM, 0))
                        DoCast(SPELL_CORRUPTING_TWILIGHT);
                        events.ScheduleEvent(EVENT_CORRUPTING_TWILIGHT, 17000);
                        break;

                    case EVENT_SAY_TRANSFORMATION: //todo: script spell
                         Talk(SAY_TRANSFORMATION);
                         Talk(SAY_POWER);
                        break;						

                    case EVENT_WAVE_VIRTUE: //todo: script spell
                       if (Unit* target = SelectTarget(SELECT_TARGET_RANDOM, 0))
                       Talk(SAY_WAVE_VIRTUE);
                       Talk(SAY_EMOTE_WAVE_VIRTUE);
                       events.ScheduleEvent(EVENT_WAVE_VIRTUE, 30000);
                        break;

                    case EVENT_WAVE_TWILIGHT: //todo: script spell
                       if (Unit* target = SelectTarget(SELECT_TARGET_RANDOM, 0))
                       Talk(SAY_WAVE_TWILIGHT);
                       Talk(SAY_EMOTE_WAVE_TWILIGHT);
                       events.ScheduleEvent(EVENT_WAVE_TWILIGHT, 30000);
                        break;

                    case EVENT_TWILIGHT_EPITAPHY_2: //todo: script spell
                        if (Creature* thrall_hot = me->FindNearestCreature(NPC_THRALL, 50.0f))
                         DoCast(thrall_hot, SPELL_TWILIGHT_EPITAPHY_2);
                        break;
						
					default:
                       break;
                }
            }		
									
            DoMeleeAttackIfReady();
        }
    };
};

void AddSC_boss_archbishop_benedictus() 
{
    new boss_archbishop_benedictus();
}
