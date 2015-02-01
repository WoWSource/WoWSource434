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

enum Yells
{
};

enum Spells
{
    SPELL_BLINK = 101398,
    SPELL_HIGHBORNE = 101412, // random target every 20 sec
    SPELL_BLACK_ARROW = 101404, // random target every 30 sec
    SPELL_UNHOLY_SHOT = 101411, // random target every 20 sec
    SPELL_BLIGHTED_ARROWS = 100763, // blow aoe when sylvana jump
    SPELL_HIGHBORNE_IMMUNE = 100686,
    SPELL_HIGHBORNE_GHOUL_VISUAL = 100867,
    SPELL_SUMMON_GHOUL = 101200,
    SPELL_GHOUL_SMOKE = 105766,
    SPELL_SYLVANA_VISUAL = 102581,
    SPELL_GHOUL_SACRIFICE = 101348,
    SPELL_GHOUL_WRACKING_PAIN = 100865,
    SPELL_DEATH_GRIP = 101397,
    SPELL_DEATH_GRIP_JUMP = 101987,
};

enum Events
{
    EVENT_HIGHBORNE = 1, // 20 sec cd
    EVENT_BLACKARROW, // 30 sec cd
    EVENT_UNHOLYSHOT, // 20 sec cd
    EVENT_BLIGHTEDARROW, // 20 sec cd
    EVENT_BLIGHTEDARROW_BLOW,
    EVENT_SYLVANA_BLIGHTEDARROW,
    EVENT_SYLVANA_GROUND,
    EVENT_CALLING_START_BLINK,
    EVENT_CALLING_START,
    EVENT_CHECK_NUMB_GHOUL,
    EVENT_END_CALLING,
};

Position const SummonGhoulPosition[7] =
{
    {3830.576f, 933.641f, 55.834f, 5.326f},
    {3853.877f, 934.927f, 55.833f, 4.395f},
    {3869.030f, 921.208f, 55.879f, 3.554f},
    {3867.259f, 899.734f, 55.953f, 2.644f},
    {3850.979f, 886.746f, 55.963f, 1.858f},
    {3828.180f, 893.654f, 55.931f, 0.624f},
    {3820.514f, 916.384f, 55.899f, 6.033f},
};

class boss_echo_of_sylvanas : public CreatureScript
{
    public:
        boss_echo_of_sylvanas() : CreatureScript("boss_echo_of_sylvanas") { }

        struct boss_echo_of_sylvanasAI : public BossAI
        {
            boss_echo_of_sylvanasAI(Creature* creature) : BossAI(creature, BOSS_ECHO_OF_SYLVANAS)
            {
                instance = me->GetInstanceScript();
            }

            InstanceScript* instance;
            std::list<Creature*> ghouls,BlightedAoe;

            void Reset()
            {
                events.Reset();
                if (instance)
                    instance->SetBossState(BOSS_ECHO_OF_SYLVANAS, NOT_STARTED);

                me->RemoveUnitMovementFlag(MOVEMENTFLAG_FLYING);
                me->RemoveFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_DISABLE_MOVE);

                DespawnCreatures(54403);
                DespawnCreatures(54385);
                DespawnCreatures(54191);
            }

            void JustDied(Unit* killer)
            {
               if (instance)
                   instance->SetData(DATA_SYLVANA, DONE);
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

            void EnterCombat(Unit* /*who*/)
            {
                if (instance)
                    instance->SetBossState(BOSS_ECHO_OF_SYLVANAS, IN_PROGRESS);

                events.ScheduleEvent(EVENT_HIGHBORNE, 5000);
                events.ScheduleEvent(EVENT_UNHOLYSHOT, 13000);
                events.ScheduleEvent(EVENT_BLACKARROW, 22000);
                events.ScheduleEvent(EVENT_BLIGHTEDARROW, 30000);
                events.ScheduleEvent(EVENT_CALLING_START_BLINK, 40000);
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
                        case EVENT_HIGHBORNE:
                            if (Unit* target = SelectTarget(SELECT_TARGET_RANDOM, 0, 100.0f, true))
                                DoCast(target, SPELL_HIGHBORNE);
                            events.ScheduleEvent(EVENT_HIGHBORNE, 20000);
                            break;

                        case EVENT_BLACKARROW:
                            if (Unit* target = SelectTarget(SELECT_TARGET_RANDOM, 0, 100.0f, true))
                                DoCast(target, SPELL_BLACK_ARROW);
                            events.ScheduleEvent(EVENT_BLACKARROW, 30000);
                            break;

                        case EVENT_UNHOLYSHOT:
                            if (Unit* target = SelectTarget(SELECT_TARGET_RANDOM, 0, 100.0f, true))
                                DoCast(target, SPELL_UNHOLY_SHOT);
                            events.ScheduleEvent(EVENT_UNHOLYSHOT, 20000);
                            break;

                        case EVENT_BLIGHTEDARROW:
                            if (Unit* target = SelectTarget(SELECT_TARGET_RANDOM, 0, 100.0f, true))
                            {
                                me->SummonCreature(54403,target->GetPositionX(),target->GetPositionY(),target->GetPositionZ());
                                me->SummonCreature(54403,target->GetPositionX(),target->GetPositionY()-3.0f,target->GetPositionZ());
                                me->SummonCreature(54403,target->GetPositionX(),target->GetPositionY()-6.0f,target->GetPositionZ());
                                me->SummonCreature(54403,target->GetPositionX(),target->GetPositionY()+3.0f,target->GetPositionZ());
                                me->SummonCreature(54403,target->GetPositionX(),target->GetPositionY()+6.0f,target->GetPositionZ());
                            }
                            events.ScheduleEvent(EVENT_SYLVANA_BLIGHTEDARROW, 1000);
                            events.ScheduleEvent(EVENT_BLIGHTEDARROW_BLOW, 6000);
                            break;

                        case EVENT_SYLVANA_BLIGHTEDARROW:
                            me->AttackStop();
                            me->SetReactState(REACT_PASSIVE);
                            me->AddUnitMovementFlag(MOVEMENTFLAG_FLYING);
                            Position pos;
                            me->GetPosition(&pos);
                            pos.m_positionZ += 8.f;
                            me->GetMotionMaster()->MoveJump(pos.GetPositionX(),pos.GetPositionY(),pos.GetPositionZ(), 8, 8);
                            me->SetFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_DISABLE_MOVE);
                            me->SetFacingToObject(me->FindNearestCreature(54403,100.0f));
                            events.ScheduleEvent(EVENT_SYLVANA_GROUND, 5000);
                            break;
                        
                        case EVENT_SYLVANA_GROUND:
                            me->RemoveUnitMovementFlag(MOVEMENTFLAG_FLYING);
                            me->RemoveFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_DISABLE_MOVE);
                            me->SetReactState(REACT_AGGRESSIVE);
                            me->SetCanFly(false);
                            me->RemoveByteFlag(UNIT_FIELD_BYTES_1, 3, 0x02);
                            me->RemoveUnitMovementFlag(MOVEMENTFLAG_CAN_FLY | MOVEMENTFLAG_FLYING);
                            me->GetMotionMaster()->MoveFall();
                            break;

                        case EVENT_CALLING_START_BLINK:
                            events.CancelEvent(EVENT_SYLVANA_BLIGHTEDARROW);
                            events.CancelEvent(EVENT_BLIGHTEDARROW);
                            events.CancelEvent(EVENT_UNHOLYSHOT);
                            events.CancelEvent(EVENT_BLACKARROW);
                            events.CancelEvent(EVENT_HIGHBORNE);
                            events.CancelEvent(EVENT_SYLVANA_GROUND);

                            me->AttackStop();
                            me->SetReactState(REACT_PASSIVE);
                            DoCast(me,SPELL_BLINK);
                            me->NearTeleportTo(me->GetHomePosition().GetPositionX(),me->GetHomePosition().GetPositionY(),me->GetHomePosition().GetPositionZ(),me->GetHomePosition().GetOrientation());
                            me->UpdatePosition(me->GetHomePosition().GetPositionX(),me->GetHomePosition().GetPositionY(),me->GetHomePosition().GetPositionZ(),me->GetHomePosition().GetOrientation());
                            events.ScheduleEvent(EVENT_CALLING_START, 1000);
                            break;

                        case EVENT_END_CALLING:
                            if (me->HasAura(SPELL_HIGHBORNE_IMMUNE))
                                me->RemoveAurasDueToSpell(SPELL_HIGHBORNE_IMMUNE);
                            if (me->HasAura(SPELL_GHOUL_WRACKING_PAIN))
                                me->RemoveAurasDueToSpell(SPELL_GHOUL_WRACKING_PAIN);
                            me->SetReactState(REACT_AGGRESSIVE);

                            GetCreatureListWithEntryInGrid(ghouls, me, 54191, 200.0f);

                            for (std::list<Creature*>::iterator iter = ghouls.begin(); iter != ghouls.end(); ++iter)
                                (*iter)->DespawnOrUnsummon(1000);

                            ghouls.clear();

                            events.CancelEvent(EVENT_SYLVANA_BLIGHTEDARROW);
                            events.CancelEvent(EVENT_BLIGHTEDARROW);
                            events.CancelEvent(EVENT_UNHOLYSHOT);
                            events.CancelEvent(EVENT_BLACKARROW);
                            events.CancelEvent(EVENT_HIGHBORNE);
                            events.CancelEvent(EVENT_SYLVANA_GROUND);

                            events.ScheduleEvent(EVENT_HIGHBORNE, 5000);
                            events.ScheduleEvent(EVENT_UNHOLYSHOT, 13000);
                            events.ScheduleEvent(EVENT_BLACKARROW, 22000);
                            events.ScheduleEvent(EVENT_BLIGHTEDARROW, 30000);
                            events.ScheduleEvent(EVENT_CALLING_START_BLINK, 40000);
                            break;

                        case EVENT_CHECK_NUMB_GHOUL:
                            GetCreatureListWithEntryInGrid(ghouls, me, 54191, 200.0f);

                            for (std::list<Creature*>::iterator iter = ghouls.begin(); iter != ghouls.end(); ++iter)
                                if ((*iter)->isAlive())
                                {
                                    (*iter)->CastSpell((*iter),SPELL_GHOUL_SACRIFICE,true);
                                    (*iter)->DespawnOrUnsummon(1000);
                                }

                            events.ScheduleEvent(EVENT_END_CALLING, 1000);

                            ghouls.clear();
                            break;

                        case EVENT_BLIGHTEDARROW_BLOW:
                            GetCreatureListWithEntryInGrid(BlightedAoe, me, 54403, 1000.0f);

                            if (BlightedAoe.empty())
                               break;

                            for (std::list<Creature*>::iterator iter = BlightedAoe.begin(); iter != BlightedAoe.end(); ++iter)
                            {
                                (*iter)->CastSpell((*iter),SPELL_BLIGHTED_ARROWS,true);
                                (*iter)->DespawnOrUnsummon(1000);
                            }

                            BlightedAoe.clear();

                            events.ScheduleEvent(EVENT_BLIGHTEDARROW, 20000);
                            break;

                        case EVENT_CALLING_START:
                            me->AddAura(SPELL_GHOUL_WRACKING_PAIN,me);
                            DoCast(me,SPELL_HIGHBORNE_IMMUNE);
                            for (uint8 i = 0;i<7;++i)
                                me->SummonCreature(54385, SummonGhoulPosition[i]);

                            Map::PlayerList const& players = me->GetMap()->GetPlayers();
                            if (!players.isEmpty())
                                for (Map::PlayerList::const_iterator itr = players.begin(); itr != players.end(); ++itr)
                                    if (Player* player = itr->getSource())
                                        player->CastSpell(me,SPELL_DEATH_GRIP_JUMP,true);

                            DoCast(me,SPELL_DEATH_GRIP);

                            events.ScheduleEvent(EVENT_CHECK_NUMB_GHOUL, 20000);
                            break;
                    }
                }

                DoMeleeAttackIfReady();
            }

        };

        CreatureAI* GetAI(Creature* creature) const
        {
            return new boss_echo_of_sylvanasAI(creature);
        }
};

class blighted_arrow : public CreatureScript
{
    public:
        blighted_arrow() : CreatureScript("blighted_arrow") { }

        struct blighted_arrowAI : public ScriptedAI
        {
            blighted_arrowAI(Creature* creature) : ScriptedAI(creature) 
            {
                instance = me->GetInstanceScript();
                me->SetFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_NON_ATTACKABLE | UNIT_FLAG_NOT_SELECTABLE);
                me->SetReactState(REACT_PASSIVE);
            }

            InstanceScript* instance;
        };

        CreatureAI* GetAI(Creature* creature) const
        {
            return new blighted_arrowAI(creature);
        }
};

class higborne_vis : public CreatureScript
{
    public:
        higborne_vis() : CreatureScript("higborne_vis") { }

        struct higborne_visAI : public ScriptedAI
        {
            higborne_visAI(Creature* creature) : ScriptedAI(creature) 
            {
                instance = me->GetInstanceScript();
                me->SetFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_NON_ATTACKABLE | UNIT_FLAG_NOT_SELECTABLE);
                me->SetReactState(REACT_PASSIVE);
                me->AddAura(SPELL_HIGHBORNE_GHOUL_VISUAL,me);
                timer = 4000;
                ghoulSummoned = false;
                notEnd = false;
            }

            InstanceScript* instance;
            uint32 timer,afterSummonTimer;
            bool ghoulSummoned,notEnd;

            void UpdateAI(uint32 const diff)
            {
                if (!ghoulSummoned)
                {
                    if (timer <= diff)
                    {
                        ghoulSummoned = true;
                        afterSummonTimer = 1000;
                        DoCast(me,SPELL_SUMMON_GHOUL);
                    } else timer -= diff;
                }

                if (ghoulSummoned && !notEnd)
                {
                    if (afterSummonTimer <= diff )
                    {
                        notEnd = true;
                        if (me->HasAura(SPELL_HIGHBORNE_GHOUL_VISUAL))
                            me->RemoveAurasDueToSpell(SPELL_HIGHBORNE_GHOUL_VISUAL);
                    } else afterSummonTimer -= diff;
                }
            }
        };

        CreatureAI* GetAI(Creature* creature) const
        {
            return new higborne_visAI(creature);
        }
};

class sylvanas_ghoul : public CreatureScript
{
    public:
        sylvanas_ghoul() : CreatureScript("sylvanas_ghoul") { }

        struct sylvanas_ghoulAI : public ScriptedAI
        {
            sylvanas_ghoulAI(Creature* creature) : ScriptedAI(creature) 
            {
                instance = me->GetInstanceScript();
                me->SetSpeed(MOVE_RUN, 0.25f);
                me->SetSpeed(MOVE_WALK, 0.25f);
                me->SetReactState(REACT_PASSIVE);
                me->RemoveFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_NON_ATTACKABLE | UNIT_FLAG_NOT_SELECTABLE);
                me->SetFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_PACIFIED);
            }

            InstanceScript* instance;
            void UpdateAI(uint32 const diff)
            {
                if (!me->HasAura(SPELL_GHOUL_SMOKE))
                    DoCast(me,SPELL_GHOUL_SMOKE);

                if (Creature * sylvana = me->FindNearestCreature(NPC_ECHO_OF_SYLVANAS, 500.0f))
                    me->GetMotionMaster()->MovePoint(0, sylvana->GetPositionX(), sylvana->GetPositionY(), sylvana->GetPositionZ());
            }
        };

        CreatureAI* GetAI(Creature* creature) const
        {
            return new sylvanas_ghoulAI(creature);
        }
};


void AddSC_boss_echo_of_sylvanas()
{
    new boss_echo_of_sylvanas();
    new blighted_arrow();
    new higborne_vis();
    new sylvanas_ghoul();
}
