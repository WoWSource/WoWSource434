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

enum Action
{
    ACTION_REWIND_TIME = 1,
};

enum Spells
{
    SPELL_TEMPORAL_BLAST = 102381,
    SPELL_TAIL_SWIP = 108589,
    SPELL_INFINITE_BREATH = 102569,
    SPELL_DISTORTION_BOMB = 101983,
    SPELL_SAND_HOURGLASS = 102668,
    SPELL_BLESSING = 102364,
};

enum Events
{
    EVENT_TEMPORAL_BLAST = 1,
    EVENT_TAIL_SWIP,
    EVENT_INFINITE_BREATH,
    EVENT_DISTORTION_BOMB,
};

class boss_murozond : public CreatureScript
{
    public:
        boss_murozond() : CreatureScript("boss_murozond") { }

        struct boss_murozondAI : public BossAI
        {
            boss_murozondAI(Creature* creature) : BossAI(creature, BOSS_MUROZOND)
            {
                instance = me->GetInstanceScript();
            }

            InstanceScript* instance;
            uint8 NumOfRewind;

            void Reset()
            {
                NumOfRewind = 5;
                me->RemoveDynObject(101984);
                AddProgressBarToPlayers(true);
                
                if (GameObject* go = me->FindNearestGameObject(209249, 500.0f))
                    go->SetFlag(GAMEOBJECT_FLAGS, GO_FLAG_NOT_SELECTABLE);


                events.Reset();
            }

            void DoAction(const int32 action)
            {
                switch(action)
                {
                    case ACTION_REWIND_TIME:
                        if (NumOfRewind == 0)
                        {
                            if (GameObject* go = me->FindNearestGameObject(209249, 300.0f))
                                go->SetFlag(GAMEOBJECT_FLAGS, GO_FLAG_NOT_SELECTABLE);
                        }
                        else
                            --NumOfRewind;

                        me->RemoveDynObject(101984);

                        Map::PlayerList const& players = me->GetMap()->GetPlayers();
                        if (!players.isEmpty())
                            for (Map::PlayerList::const_iterator itr = players.begin(); itr != players.end(); ++itr)
                                if (Player* player = itr->getSource())
                                {
                                    player->SetPower(POWER_ALTERNATE_POWER, NumOfRewind);
                                    player->SetHealth(player->GetMaxHealth());

                                    switch (player->getClass())
                                    {
                                        case CLASS_PRIEST:
                                        case CLASS_MAGE:
                                        case CLASS_PALADIN:
                                        case CLASS_WARLOCK:
                                        case CLASS_DRUID:
                                        case CLASS_SHAMAN:
                                            player->SetPower(POWER_MANA, player->GetMaxPower(POWER_MANA));
                                            break;
                                    }

                                    player->RemoveAllSpellCooldown();
                                    DoCast(player,SPELL_BLESSING);
                                }
                        break;
                }
            }
            
            void AddProgressBarToPlayers(bool remove)
            {
                Map::PlayerList const& players = me->GetMap()->GetPlayers();
                if (!players.isEmpty())
                    for (Map::PlayerList::const_iterator itr = players.begin(); itr != players.end(); ++itr)
                        if (Player* player = itr->getSource())
                        {
                            if (remove)
                            {
                                if (player->HasAura(SPELL_SAND_HOURGLASS))
                                    player->RemoveAurasDueToSpell(SPELL_SAND_HOURGLASS);
                            }
                            else 
                            {
                                if (!player->HasAura(SPELL_SAND_HOURGLASS))
                                    me->AddAura(SPELL_SAND_HOURGLASS,player);
                            }
                        }
            }

            void JustDied(Unit* killer)
            {
                if (instance)
                    instance->SetData(DATA_MUROZOND, DONE);

                if (GameObject* go = me->FindNearestGameObject(209249, 300.0f))
                    go->SetFlag(GAMEOBJECT_FLAGS, GO_FLAG_NOT_SELECTABLE);

                if (GameObject* go = me->FindNearestGameObject(209547, 500.0f))
                {
                    //me->SummonGameObject(209547,go->GetPositionX(),go->GetPositionY(),go->GetPositionZ(),me->GetOrientation(),0.0f, 0.0f, 0.0f, 0.0f, uint32(go->GetRespawnTime()));
                    go->SetPhaseMask(PHASEMASK_ANYWHERE,true);
                    go->UpdateObjectVisibility();
                }
                
				instance->DoUpdateAchievementCriteria(ACHIEVEMENT_CRITERIA_TYPE_BE_SPELL_TARGET, 110158, 0, me);
							
                AddProgressBarToPlayers(true);
				_JustDied();
            }

            void EnterCombat(Unit* /*who*/)
            {
                AddProgressBarToPlayers(false);

                if (GameObject* go = me->FindNearestGameObject(209249, 300.0f))
                    go->RemoveFlag(GAMEOBJECT_FLAGS, GO_FLAG_NOT_SELECTABLE);

                me->SetCanFly(false);
                me->RemoveByteFlag(UNIT_FIELD_BYTES_1, 3, 0x02);
                me->RemoveUnitMovementFlag(MOVEMENTFLAG_CAN_FLY | MOVEMENTFLAG_FLYING);

                events.ScheduleEvent(EVENT_DISTORTION_BOMB, 10000);
                events.ScheduleEvent(EVENT_TEMPORAL_BLAST, 15000);
                events.ScheduleEvent(EVENT_INFINITE_BREATH, 20000);
                events.ScheduleEvent(EVENT_TAIL_SWIP, 23000);					
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
                        case EVENT_TEMPORAL_BLAST:
                            DoCastAOE(SPELL_TEMPORAL_BLAST);
                            events.ScheduleEvent(EVENT_TEMPORAL_BLAST, 15000);
                            break;
                        case EVENT_INFINITE_BREATH:
                            DoCastAOE(SPELL_INFINITE_BREATH);
                            events.ScheduleEvent(EVENT_INFINITE_BREATH, 20000);
                            break;
                        case EVENT_TAIL_SWIP:
                            DoCastAOE(SPELL_TAIL_SWIP);
                            events.ScheduleEvent(EVENT_TAIL_SWIP, 23000);
                            break;
                        case EVENT_DISTORTION_BOMB:
                            if (Unit* target = SelectTarget(SELECT_TARGET_RANDOM, 0, 100.0f))
                                DoCast(target, SPELL_DISTORTION_BOMB);
                            events.ScheduleEvent(EVENT_DISTORTION_BOMB, 7000);
                            break;
                        default:
                            break;
                    }
                }

                DoMeleeAttackIfReady();
            }
        };

        CreatureAI* GetAI(Creature* creature) const
        {
            return new boss_murozondAI(creature);
        }
};

void AddSC_boss_murozond()
{
    new boss_murozond;
}
