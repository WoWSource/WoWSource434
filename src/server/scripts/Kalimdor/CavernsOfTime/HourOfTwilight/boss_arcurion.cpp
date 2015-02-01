/* ScriptData
SDScript by Loukie
SDName: boss_arcurion
SD%Complete: 60% or less
SDCategory: Hour of Twilight
EndScriptData */

#include"ScriptPCH.h"
#include "ObjectMgr.h"
#include "ScriptMgr.h"
#include "ScriptedCreature.h"
#include "SpellScript.h"
#include "SpellAuraEffects.h"
#include "SpellAuras.h"
#include "MapManager.h"
#include "Spell.h"
#include "Vehicle.h"
#include "Cell.h"
#include "CellImpl.h"
#include "GridNotifiers.h"
#include "GridNotifiersImpl.h"
#include "CreatureTextMgr.h"
#include "MoveSplineInit.h"
#include"hour_of_twilight.h"

enum eArcurionYells 
{
    SAY_AGGRO  = 0,
    SAY_KILL   = 1,
    SAY_DEATH  = 2,
    SAY_HP_LOW = 3,
    SAY_FORCES = 4	
};

enum Spells 
{
    SPELL_HANDS_OF_FROST               = 102593,
    SPELL_CHAINS_OF_FROST              = 102582,
    SPELL_ICY_TOMB                     = 103252,
    SPELL_VISUAL_ICY_BOULDER_CRASH     = 102199,
    SPELL_VISUAL_ICY_BOULDER_TARGET    = 102198,
    SPELL_ICY_BOULDER                  = 102480, // Needs Scripting include SPELL_VISUAL_ICY_BOULDER_TARGET || SPELL_VISUAL_ICY_BOULDER_CRASH
    SPELL_TORRENT_OF_FROST             = 103962,
    SPELL_ARCURION_SPAWN_VISUAL        = 104767,
    SPELL_TELE_ARCURION_DEAD_VISUAL    = 108928
};

enum icedoor
{
    GO_ICEWALL_DOOR                    = 210048
};

enum Events 
{
    EVENT_SUMMON                       = 1,
    EVENT_HANDS_OF_FROST               = 2,
    EVENT_CHAINS_OF_FROST              = 3,
    EVENT_ICY_BOULDER                  = 4,
    EVENT_ICY_TOMB                     = 5
};

enum thrall_spels
{
    SPELL_LAVA_BURST                   = 107980, 
};

enum thrall_eevents
{
    EVENT_LAVA_BURST                   = 107980,
};

Position const SpawnPositions[14] =
{
	{4739.72f, 84.1997f, 107.23f, 5.30915f},
	{4756.07f, 103.248f, 114.95f, 5.74322f},
	{4750.13f, 97.3125f, 112.217f, 5.8437f},
	{4818.98f, 44.75f, 106.324f, 3.22131f},
	{4771.36f, 110.743f, 121.498f, 5.00128f},
	{4838.94f, 90.1892f, 108.409f, 3.56877f},
	{4737.55f, 75.5538f, 105.757f, 5.68455f},
	{4842.17f, 110.13f, 107.272f, 4.03014f},
	{4777.38f, 30.809f, 92.5167f, 1.58226f},
	{4788.47f, 125.67f, 129.112f, 4.73712f},
	{4796.06f, 131.432f, 132.468f, 4.60976f},
	{4827.03f, 50.566f, 108.63f,	3.05247f},
	{4831.26f, 64.6198f, 108.553f, 3.28342f},
	{4810.14f, 31.5191f, 104.593f, 2.3692f},
};

class boss_arcurion : public CreatureScript
{
public:
    boss_arcurion() : CreatureScript("boss_arcurion") { }

    CreatureAI* GetAI(Creature* creature) const
    {
        return new boss_arcurionAI (creature);
    }

    struct boss_arcurionAI : public BossAI
    {
        boss_arcurionAI(Creature* creature) : BossAI(creature, DATA_ARCURION_EVENT)
        {
            instance = creature->GetInstanceScript();			
        }
		
        uint32 m_uiHealthAmountModifier;
        InstanceScript* instance;
        EventMap events;
        Creature* frozen;

        void Reset() 
        {
            events.Reset();
            m_uiHealthAmountModifier = 1;
            //frozen->DespawnOrUnsummon();
            me->SummonGameObject(GO_ICEWALL_DOOR, 4711.87f, 33.5087f, 64.559f, 0.471238f, 0, 0, 0, 0, 0);
        }
		
        void EnterCombat(Unit* /*Ent*/)
        {
            if (instance)
            instance->SetData(DATA_ARCURION_EVENT, IN_PROGRESS);
            Talk(SAY_FORCES);	
            Talk(SAY_AGGRO);
            events.ScheduleEvent(EVENT_SUMMON, 5000);
            DoCast(SPELL_HANDS_OF_FROST);
            events.ScheduleEvent(EVENT_HANDS_OF_FROST, urand(5000,10000));
            events.ScheduleEvent(EVENT_CHAINS_OF_FROST, urand(12000,19000));
            events.ScheduleEvent(EVENT_ICY_TOMB, 1000);
		}

                void JustSummoned(Creature *summon)
                {
                    summons.Summon(summon);
                    summon->setActive(true);
                    summon->SetVisible(true);

                    if (me->isInCombat())
                        summon->AI()->DoZoneInCombat();

                    switch (summon->GetEntry())
                    {
                        case NPC_FROZEN_SERVITOR:
                            summon->SetFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_DISABLE_MOVE);
                            //summon->CastSpell(summon, SPELL_VISUAL_ICY_BOULDER_TARGET, false);
                            break;

                        default:
                            break;
                    }
                }		

        void DespawnGameobjects(uint32 entry, float distance)
        {
            std::list<GameObject*> gameobjects;
            GetGameObjectListWithEntryInGrid(gameobjects, me, entry, distance);

            if (gameobjects.empty())
                return;

            for (std::list<GameObject*>::iterator iter = gameobjects.begin(); iter != gameobjects.end(); ++iter)
                (*iter)->RemoveFromWorld();
        }
								
        void JustDied(Unit* /*Kill*/)
        {
             Talk(SAY_DEATH);
             DoCast(me, SPELL_TELE_ARCURION_DEAD_VISUAL, true);
			 
                DespawnGameobjects(GO_ICEWALL_DOOR, 100.0f);
                frozen->DespawnOrUnsummon();

             if (instance)
             instance->SetData(DATA_ARCURION_EVENT, DONE);
        }		
		
        void KilledUnit(Unit* /*victim*/)
        {
             Talk(SAY_KILL);
        }		
		
        void UpdateAI(const uint32 diff)
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

                case EVENT_SUMMON: // way ius this not happening what am i doing wrong???
                     for (int8 i = 0; i < 14; i++)
                      {
                        frozen = me->SummonCreature(NPC_FROZEN_SERVITOR, SpawnPositions[i], TEMPSUMMON_MANUAL_DESPAWN);
                      }  
                        break;				
				
                 case EVENT_HANDS_OF_FROST:
                     if (Unit* target = SelectTarget(SELECT_TARGET_RANDOM, 0))
                        DoCast(SPELL_HANDS_OF_FROST);
                        events.ScheduleEvent(EVENT_HANDS_OF_FROST, 1500);
                        break;
    
                 case EVENT_CHAINS_OF_FROST:
                     if (Unit* target = SelectTarget(SELECT_TARGET_RANDOM, 0))
                        DoCast(SPELL_CHAINS_OF_FROST);
                        events.ScheduleEvent(EVENT_CHAINS_OF_FROST, 17000);
                        break;
												
                    case EVENT_ICY_TOMB:
                        DoCast(SPELL_ICY_TOMB);
                        events.ScheduleEvent(EVENT_ICY_TOMB, urand(111000, 113000));
                        break;						
                        default:
                        break;
                 }
              }
            // Health check
            if (HealthBelowPct(30 * m_uiHealthAmountModifier))
            {
                Talk(SAY_HP_LOW);
                DoCast(SPELL_TORRENT_OF_FROST);
            }			
			
            DoMeleeAttackIfReady();
        }
    };
};

class npc_frozen_servitor : public CreatureScript
{
    public:
        npc_frozen_servitor() : CreatureScript("npc_frozen_servitor") { }

        struct npc_frozen_servitorAI : public ScriptedAI
        {
            npc_frozen_servitorAI(Creature* creature) : ScriptedAI(creature){}
			
        private:
            EventMap events;
			
            void Reset() 
                 {
                   me->SetVisible(true);
                  //events.ScheduleEvent(EVENT_ICY_BOULDER, 4000);
                  }
			
            void IsSummonedBy(Unit* /*summoner*/) 
            {
                events.ScheduleEvent(EVENT_ICY_BOULDER, urand(5000, 10000));
            }

            void UpdateAI(const uint32 diff) 
            {
                if (!UpdateVictim())
                    return;

                events.Update(diff);

                while (uint32 eventId = events.ExecuteEvent())
                {
                    switch (eventId)
                    {
                        case EVENT_ICY_BOULDER:
                          if(Unit* target = SelectTarget(SELECT_TARGET_RANDOM, 0, 100, true))
                             DoCast(target, SPELL_VISUAL_ICY_BOULDER_TARGET); // Temp untill  SPELL ICY_BOULDERS is scripted
                             events.ScheduleEvent(EVENT_ICY_BOULDER, urand(8000, 12000));
                            break;

                        default:
                            break;
                    }
                }
            }
        };

        CreatureAI* GetAI(Creature* creature) const 
        {
            return new npc_frozen_servitorAI(creature);
        }
};

void AddSC_boss_arcurion() 
{
    new boss_arcurion();
    new npc_frozen_servitor();
}
