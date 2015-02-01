#include "ScriptPCH.h"
#include "end_time.h"
#include "Vehicle.h"
#include "Unit.h"
#include "ScriptMgr.h"
#include "ScriptedCreature.h"
#include "ScriptedGossip.h"
#include "ScriptedEscortAI.h"
#include "Cell.h"
#include "CellImpl.h"
#include "GridNotifiers.h"
#include "GridNotifiersImpl.h"
#include "CombatAI.h"
#include "PassiveAI.h"
#include "ObjectMgr.h"
#include "SpellInfo.h"
#include "SpellScript.h"
#include "SpellAuraEffects.h"
#include "SpellAuras.h"
#include "CreatureTextMgr.h"
#include "Vehicle.h"
#include "VehicleDefines.h"
#include "Spell.h"
#include "Player.h"
#include "Map.h"
#include "InstanceScript.h"

enum Creatures
{
	NPC_DARK_MOONLIGHT     = 70021,
};

enum Spells
{
    SPELL_IN_SHADOW = 101841,
    SPELL_MOONLIT_TRIGGER = 101946,
    SPELL_MOONBOLT = 102193,
    SPELL_DARK_MOONLIGHT = 102414,
    SPELL_STARDUST = 102173,
    SPELL_MOONLANCE = 102151,
    SPELL_MOONLANCE_TRIGGER = 102150,
    SPELL_LUNAR_GUIDANCE = 102472,
    SPELL_TEARS_OF_ELUNE = 102241,
    SPELL_TEARS_OF_ELUNE_FALL = 102243,
    SPELL_EYES_SUMMON = 102181,
    SPELL_EYES_DAMAGE = 102182,
};

enum Events
{
    EVENT_MOONBOLT = 1, 
    EVENT_STARDUST,
    EVENT_MOONLANCE,
    EVENT_GUIDANCE,
    EVENT_TEARS,
    EVENT_TEARS_FALL,
	EVENT_FALL,
    EVENT_EYE_ELUNE,
};

enum Actions
{
	ACTION_TEAR       = 1,
};

Position const PoolPositions[5] =
{
    {2906.522f, 66.774f, 3.324f, 5.326f},
    {2864.864f, 139.631f, 2.898f, 5.326f},
    {2775.498f, 125.622f, 1.148f, 5.326f},
    {2697.498f, 2.112f, 3.708f, 5.326f},
    {2785.555f, -1.730f, 2.550f, 5.326f},
};

Position const EluneEyes[6] =
{
    {2849.759f, 56.774f, 0.324f},
    {2845.864f, 28.631f, 0.855f},
    {2821.864f, 19.929f, 0.560f},
    {2784.864f, 46.805f, 0.975f},
    {2797.864f, 78.631f, 1.064f},
    {2820.173f, 86.631f, 0.280f},
};


class boss_echo_of_tyrande : public CreatureScript
{
    public:
        boss_echo_of_tyrande() : CreatureScript("boss_echo_of_tyrande") { }

        struct boss_echo_of_tyrandeAI : public BossAI
        {
            boss_echo_of_tyrandeAI(Creature* creature) : BossAI(creature, BOSS_ECHO_OF_TYRANDE)
            {
                instance = me->GetInstanceScript();
            }

            InstanceScript* instance;
            bool LunarGuidance80,LunarGuidance55,TearFall;

            void Reset()
            {
                me->SetReactState(REACT_PASSIVE);
                LunarGuidance80 = false;
                LunarGuidance55 = false;
                TearFall = false;

                events.Reset();
                if (instance)
                    instance->SetBossState(BOSS_ECHO_OF_TYRANDE, NOT_STARTED);
            }

            void JustDied(Unit* killer)
            {
                if (Creature * pool = me->FindNearestCreature(45979, 20.0f))
                    pool->DespawnOrUnsummon();
				if (Creature * pool = me->FindNearestCreature(NPC_DARK_MOONLIGHT, 20.0f))
                    pool->DespawnOrUnsummon();

                if (instance)
                    instance->SetData(DATA_TYRANDE, DONE);
            }

            void EnterCombat(Unit* /*who*/)
            {
                if (instance)
                    instance->SetBossState(BOSS_ECHO_OF_TYRANDE, IN_PROGRESS);
				me->SummonCreature(NPC_DARK_MOONLIGHT,me->GetPositionX(),me->GetPositionY(),me->GetPositionZ());
                events.ScheduleEvent(EVENT_MOONBOLT, 2000);
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
                    switch (eventId)
                    {
                        case EVENT_MOONBOLT:
                            if (Unit* target = SelectTarget(SELECT_TARGET_TOPAGGRO, 0, 100.0f))
                                DoCast(target, SPELL_MOONBOLT);

                            events.ScheduleEvent(NextEvent(EVENT_MOONBOLT), 2500);
                            break;
                        case EVENT_STARDUST:
                            DoCastAOE(SPELL_STARDUST);

                            events.ScheduleEvent(NextEvent(EVENT_STARDUST), 3000);
                            break;
                        case EVENT_MOONLANCE:
                            me->AttackStop();
                            if (Unit* target = SelectTarget(SELECT_TARGET_RANDOM, 0, 100.0f, true))
                                DoCast(target,SPELL_MOONLANCE);

                            events.ScheduleEvent(NextEvent(EVENT_MOONLANCE), 2500);
                            break;
                        case EVENT_GUIDANCE:
                            DoCast(SPELL_LUNAR_GUIDANCE);
                            events.ScheduleEvent(NextEvent(EVENT_GUIDANCE), 5000);
                            break;
                        case EVENT_TEARS:
                            DoCastAOE(SPELL_TEARS_OF_ELUNE);
                            events.ScheduleEvent(EVENT_TEARS_FALL, 6000);
                            break;
                        case EVENT_TEARS_FALL:
                            if(Creature * dark = me->FindNearestCreature(NPC_DARK_MOONLIGHT, 100.0f))
							{
								dark->AI()->DoAction(ACTION_TEAR);
							}
							events.ScheduleEvent(NextEvent(EVENT_MOONBOLT), 2500);
                            break;
                        case EVENT_EYE_ELUNE:
                            DoCast(me,SPELL_EYES_SUMMON);
                            events.ScheduleEvent(NextEvent(EVENT_EYE_ELUNE), 3000);
                            break;

                    }
                }
            }

            float NextEvent(float LastEvent)
            {
                if (LastEvent == EVENT_GUIDANCE)
                    return EVENT_EYE_ELUNE;
                else if (me->GetHealthPct() <= 30.0f && !TearFall)
                {
                    TearFall = true;
                    return EVENT_TEARS;
                }
                else if (me->GetHealthPct() <= 80.0f && !LunarGuidance80)
                {
                    LunarGuidance80 = true;
                    return EVENT_GUIDANCE;
                }
                else if (me->GetHealthPct() <= 55.0f && !LunarGuidance55)
                {
                    LunarGuidance55 = true;
                    return EVENT_GUIDANCE;
                }
                else
                {
                    return RAND(EVENT_MOONBOLT,EVENT_STARDUST == LastEvent ? RAND(EVENT_MOONBOLT,EVENT_MOONLANCE) : EVENT_STARDUST,EVENT_MOONLANCE == LastEvent ? RAND(EVENT_MOONBOLT,EVENT_STARDUST) : EVENT_MOONLANCE);
                }
                return 0;
            }
        };

        CreatureAI* GetAI(Creature* creature) const
        {
            return new boss_echo_of_tyrandeAI(creature);
        }
};

class thrash_tyrande : public CreatureScript
{
    public:
        thrash_tyrande() : CreatureScript("thrash_tyrande") { }

        struct thrash_tyrandeAI : public ScriptedAI
        {
            thrash_tyrandeAI(Creature* creature) : ScriptedAI(creature) 
            {
                instance = me->GetInstanceScript();
                me->AddAura(SPELL_IN_SHADOW,me);
            }

            InstanceScript* instance;


            void UpdateAI(uint32 const diff)
            {
                if (Creature * pool = me->FindNearestCreature(54508, 5.0f))
                {
                    if (me->HasAura(SPELL_IN_SHADOW))
                        me->RemoveAura(SPELL_IN_SHADOW);
                }
                else 
                {
                    if (!me->HasAura(SPELL_IN_SHADOW))
                        me->AddAura(SPELL_IN_SHADOW,me);
                    if (!me->HasAura(101842))
                        me->RemoveAura(101842);
                }
                 DoMeleeAttackIfReady();
            }

            void JustDied(Unit* killer)
            {
                if (Creature * pool = me->FindNearestCreature(54508, 10.0f))
                {
                    pool->SetObjectScale(pool->GetFloatValue(OBJECT_FIELD_SCALE_X) - 0.05);
                    Creature * NewCreature = me->FindNearestCreature(RAND(54688,54699,54700,54701,54512), 40.0f);
                    for (uint8 i = 0;i<4;++i)
                        NewCreature = me->FindNearestCreature(RAND(54688,54699,54700,54701,54512), 40.0f);

                    if (!NewCreature)
                        me->SummonCreature(RAND(54688,54699,54700,54701,54512),me->GetPositionX()+urand(0,20),me->GetPositionY()+urand(0,20),me->GetPositionZ());
                    else
                        NewCreature->AI()->AttackStart(killer);
                }
                
            }
        };

        CreatureAI* GetAI(Creature* creature) const
        {
            return new thrash_tyrandeAI(creature);
        }
};

class pool_moonlight : public CreatureScript
{
    public:
        pool_moonlight() : CreatureScript("pool_moonlight") { }

        struct pool_moonlightAI : public ScriptedAI
        {
            pool_moonlightAI(Creature* creature) : ScriptedAI(creature) 
            {
                instance = me->GetInstanceScript();
                me->AddAura(SPELL_MOONLIT_TRIGGER,me);
                despawn = false;
                me->SetReactState(REACT_PASSIVE);
            }

            InstanceScript* instance;
            bool despawn;

            void UpdateAI(uint32 const diff)
            {
                if (me->GetFloatValue(OBJECT_FIELD_SCALE_X) <= 0.3f && !despawn)
                {
                    despawn = true;
                    uint8 NumSpawnPool = 0;
                    for (uint8 i = 0;i<6;++i)
                        if (me->GetDistance(PoolPositions[i]) < me->GetDistance(PoolPositions[NumSpawnPool]))
                            NumSpawnPool = i;

                    if (NumSpawnPool != 4)
                        me->SummonCreature(54508,PoolPositions[NumSpawnPool+1]);
                    else
                    {
                        if (Creature * Tyrande = me->FindNearestCreature(NPC_ECHO_OF_TYRANDE, 500.0f))
                            Tyrande->RemoveFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_NON_ATTACKABLE | UNIT_FLAG_NOT_SELECTABLE);

                        DespawnCreatures(54688);
                        DespawnCreatures(54699);
                        DespawnCreatures(54700);
                        DespawnCreatures(54701);
                        DespawnCreatures(54512);
                    }
                    me->DespawnOrUnsummon();
                }
            }

            void DespawnCreatures(uint32 entry)
            {
                std::list<Creature*> creatures;
                   GetCreatureListWithEntryInGrid(creatures, me, entry, 1000.0f);

                if (creatures.empty())
                   return;

                for (std::list<Creature*>::iterator iter = creatures.begin(); iter != creatures.end(); ++iter)
                     (*iter)->DespawnOrUnsummon();
            }

        };

        CreatureAI* GetAI(Creature* creature) const
        {
            return new pool_moonlightAI(creature);
        }
};

class dark_moonlight : public CreatureScript
{
    public:
        dark_moonlight() : CreatureScript("dark_moonlight") { }

        struct dark_moonlightAI : public ScriptedAI
        {
            dark_moonlightAI(Creature* creature) : ScriptedAI(creature) 
            {
                instance = me->GetInstanceScript();
                me->SetReactState(REACT_PASSIVE);
                me->SetFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_DISABLE_MOVE | UNIT_FLAG_NON_ATTACKABLE | UNIT_FLAG_NOT_SELECTABLE);
                DoCast(me,SPELL_DARK_MOONLIGHT);
            }
	
			void DoAction(int32 const action)  
			{	
				switch (action)
				{
					case ACTION_TEAR:
						events.ScheduleEvent(EVENT_FALL, 1000);
						break;
					default:
						break;
				}
			}
			
			void UpdateAI(uint32 const diff)  
            {
                if (!UpdateVictim())
                    return;

                events.Update(diff);

                while (uint32 eventId = events.ExecuteEvent())
                {
					switch (eventId)
                    {
                        case EVENT_FALL:
							DoCast(me,SPELL_TEARS_OF_ELUNE_FALL);
							events.ScheduleEvent(EVENT_FALL, 1000);
                            break;
							
                        default:
                            break;
                    }
				}
			}

			
			InstanceScript* instance;
			
			private:
				EventMap events;
            
        };
		
		

        CreatureAI* GetAI(Creature* creature) const
        {
            return new dark_moonlightAI(creature);
        }
};

class tyrande_moonlance : public CreatureScript
{
    public:
        tyrande_moonlance() : CreatureScript("tyrande_moonlance") { }

        struct tyrande_moonlanceAI : public ScriptedAI
        {
            tyrande_moonlanceAI(Creature* creature) : ScriptedAI(creature) 
            {
                instance = me->GetInstanceScript();
                me->SetReactState(REACT_PASSIVE);
                me->SetFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_NON_ATTACKABLE | UNIT_FLAG_NOT_SELECTABLE);
                DoCast(me,SPELL_MOONLANCE_TRIGGER);
                timer = 3500;
                Despawn = false;
            }

            InstanceScript* instance;
            uint32 timer;
            bool Despawn;

            void UpdateAI(uint32 const diff)
            {
                if (timer <= diff && !Despawn)
                {
                    Despawn = true;
                    if (me->GetEntry() == 54574)
                    {
                        me->SummonCreature(54580, me->GetPositionX(), me->GetPositionY(), me->GetPositionZ(), me->GetOrientation() - 0.7f);
                        me->SummonCreature(54582, me->GetPositionX(), me->GetPositionY(), me->GetPositionZ(), me->GetOrientation());
                        me->SummonCreature(54581, me->GetPositionX(), me->GetPositionY(), me->GetPositionZ(), me->GetOrientation() + 0.7f);
                    }
                    me->DespawnOrUnsummon(600);
                }
                else timer -= diff;

                float x, y, z;
                me->GetClosePoint(x, y, z, me->GetObjectSize() / 3);
                me->GetMotionMaster()->MovePoint(0, x, y, z);
            }
        };

        CreatureAI* GetAI(Creature* creature) const
        {
            return new tyrande_moonlanceAI(creature);
        }
};

class eyes_of_elune : public CreatureScript
{
    public:
        eyes_of_elune() : CreatureScript("eyes_of_elune") { }

        struct eyes_of_eluneAI : public ScriptedAI
        {
            eyes_of_eluneAI(Creature* creature) : ScriptedAI(creature) 
            {
                instance = me->GetInstanceScript();
                me->SetReactState(REACT_PASSIVE);
                me->SetFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_NON_ATTACKABLE | UNIT_FLAG_NOT_SELECTABLE);
                DoCast(me,SPELL_EYES_DAMAGE);
                timer = 100;
                if (me->GetEntry()== 54594)
                    num = 3;
                else
                    num = 0;
            }

            InstanceScript* instance;
            uint32 timer,num;

            void UpdateAI(uint32 const diff)
            {
                if (timer <= diff)
                {
                    me->GetMotionMaster()->MovePoint(0,EluneEyes[num]);
                    if (num == 5)
                        num = 0;
                    else
                        ++num;
                    timer = 3000;
                }
                else timer -= diff;
            }
        };

        CreatureAI* GetAI(Creature* creature) const
        {
            return new eyes_of_eluneAI(creature);
        }
};

void AddSC_boss_echo_of_tyrande()
{
    new boss_echo_of_tyrande();
    new thrash_tyrande();
    new pool_moonlight();
    new dark_moonlight();
    new tyrande_moonlance();
    new eyes_of_elune();
}
