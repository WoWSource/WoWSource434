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
    SAY_INTRO = 1,
    SAY_AGGRO,
    SAY_DEATH,
    SAY_KILL_1,
    SAY_KILL_2,
    SAY_KILL_3,
    SAY_THROW_TOTEM,
    SAY_PULVERIZE,
};

enum Spells
{
    SPELL_BAINE_VIS   = 101624, // Visuals on boss (the totems on the back etc.)
    SPELL_BAINE_AXE_VIS = 101834,

    SPELL_THROW_TOTEM = 101615, // Triggers missile at location, with summon totem and kb.
    SPELL_PULVERIZE_J = 101626, // Jump b. target, activate platform.
    SPELL_PULVERIZE_D = 101627, // Damage spell.
    SPELL_PULV_DBM    = 101625, // DBM says this, fuck it.
    SPELL_MOLTEN_AXE  = 101836, // Extra damage on melee attack, change targets from caster to target. When he falls into lava after Pulverize.
    SPELL_MOLTEN_FIST = 101866, // Extra dmg on melee for players when they touch the lava, they get this when baine gets Molten Axe.

    SPELL_TB_TOTEM    = 101603, // Throw totem back at Baine on click.
    SPELL_TB_TOTEM_A  = 107837, // Visual aura: player has totem to throw.
};

enum Events
{
    EVENT_PULVERIZE = 1,
    EVENT_PULVERIZE_DAMAGE,
    EVENT_THROW_TOTEM,
};

enum Creatures
{
    NPC_ROCK_ISLAND      = 54496,
    NPC_WALL_OF_FLAME    = 203006,
    NPC_PUL_LOCATION     = 70024,
    NPC_BAINE            = 54431,
};

#define SAY_INTRO_CHAT "What dark horrors have you wrought in this place? By my ancestors' honor. I shall take you to task!"
#define SAY_LAVA_CHAT "My wrath knows no bounds!"
#define SAY_LAVA2_CHAT "There will be no escape!"
#define SAY_TEST "TEST"
#define SAY_DIE_CHAT "Where... is this place? What... have i done? Forgive me, my father..."

class boss_echo_of_baine : public CreatureScript
{
    public:
        boss_echo_of_baine() : CreatureScript("boss_echo_of_baine") { }

        struct boss_echo_of_baineAI : public BossAI
        {
            boss_echo_of_baineAI(Creature* creature) : BossAI(creature, BOSS_ECHO_OF_BAINE)
            {
                introDone = false;
                instance = me->GetInstanceScript();
            }

            InstanceScript* instance;
            bool introDone;
            Unit* pulverizeTarget;
            EventMap events;

            void Reset()
            {
                events.Reset();

                me->SetReactState(REACT_AGGRESSIVE);

                if (GameObject* go = me->FindNearestGameObject(NPC_WALL_OF_FLAME, 500.0f))
                    go->Delete();

                if (instance) // Open the doors.
                {
                    instance->SetBossState(BOSS_ECHO_OF_BAINE, NOT_STARTED);
                    instance->HandleGameObject(4001, true);
                    instance->HandleGameObject(4002, true);
                }

                if(GameObject* platform = me->FindNearestGameObject(209693, 500.0f))
                    platform->SetDestructibleState(GO_DESTRUCTIBLE_REBUILDING);
                else if(GameObject* platform = me->FindNearestGameObject(209694, 500.0f))
                    platform->SetDestructibleState(GO_DESTRUCTIBLE_REBUILDING);
                else if(GameObject* platform = me->FindNearestGameObject(209695, 500.0f))
                    platform->SetDestructibleState(GO_DESTRUCTIBLE_REBUILDING);
                else if(GameObject* platform = me->FindNearestGameObject(209670, 500.0f))
                    platform->SetDestructibleState(GO_DESTRUCTIBLE_REBUILDING);

                std::list<Creature*> creatures;
                GetCreatureListWithEntryInGrid(creatures, me, 54434, 1000.0f);
                if (!creatures.empty())
                    for (std::list<Creature*>::iterator iter = creatures.begin(); iter != creatures.end(); ++iter)
                         (*iter)->DespawnOrUnsummon();

                if (!me->HasAura(SPELL_BAINE_VIS))
                    DoCast(me, SPELL_BAINE_VIS);
            }

            void EnterEvadeMode()
            {
                me->GetMotionMaster()->MoveTargetedHome();
                Reset();

                me->SetHealth(me->GetMaxHealth());

                if (instance)
                {
                    instance->SetBossState(BOSS_ECHO_OF_BAINE, FAIL);
                    instance->SendEncounterUnit(ENCOUNTER_FRAME_DISENGAGE, me); // Remove
                }

                _EnterEvadeMode();
            }

            void MoveInLineOfSight(Unit* who)
            {
                if (introDone)
                    return;

                if (!me->IsWithinDistInMap(who, 40.0f, false))
                    return;

                Talk(SAY_INTRO);
                introDone = true;
            }

            void JustDied(Unit* killer)
            {
                Talk(SAY_DEATH);
                me->MonsterYell(SAY_DIE_CHAT,0,0);

                if (GameObject* go = me->FindNearestGameObject(NPC_WALL_OF_FLAME, 500.0f))
                    go->Delete();

                if (instance)
                {
                    instance->SetBossState(BOSS_ECHO_OF_BAINE, DONE);

                    instance->SetData(DATA_BAINE, DONE);

                    instance->HandleGameObject(4001, true); // Open the doors.
                    instance->HandleGameObject(4002, true);

                    instance->SendEncounterUnit(ENCOUNTER_FRAME_DISENGAGE, me); // Remove
                }
            }

            void KilledUnit(Unit * /*victim*/)
            {
                Talk(RAND(SAY_KILL_1, SAY_KILL_2, SAY_KILL_3));
            }

            void EnterCombat(Unit* /*who*/)
            {
                Talk(SAY_AGGRO);
                me->MonsterYell(SAY_INTRO_CHAT,0,0);

                me->SummonGameObject(NPC_WALL_OF_FLAME,4383.0908f,1394.9914f,129.504f,4.849097f,0,0,0,0,9000000);

                if (instance)
                {
                    instance->SetBossState(BOSS_ECHO_OF_BAINE, IN_PROGRESS);
                    instance->SendEncounterUnit(ENCOUNTER_FRAME_ENGAGE, me); // Add

                    instance->HandleGameObject(4001, false); // Close doors.
                    instance->HandleGameObject(4002, false);
                }

                events.ScheduleEvent(EVENT_PULVERIZE, 30000);
                events.ScheduleEvent(EVENT_THROW_TOTEM, 10000);
            }

            void UpdateAI(const uint32 diff)
            {
                if (!UpdateVictim() || me->HasUnitState(UNIT_STATE_CASTING))
                    return;

                if (me->HasAura(SPELL_MOLTEN_AXE) && !me->HasAura(SPELL_BAINE_AXE_VIS))
                    DoCast(me, SPELL_BAINE_AXE_VIS);
                else if (!me->HasAura(SPELL_MOLTEN_AXE) && me->HasAura(SPELL_BAINE_AXE_VIS))
                    me->RemoveAurasDueToSpell(SPELL_BAINE_AXE_VIS);

                if (me->IsInWater() && !me->HasAura(SPELL_MOLTEN_AXE))
                    DoCast(me, SPELL_MOLTEN_AXE);

                Map::PlayerList const &PlayerList = me->GetMap()->GetPlayers();
                if (!PlayerList.isEmpty())
                    for (Map::PlayerList::const_iterator i = PlayerList.begin(); i != PlayerList.end(); ++i)
                        if (Player* playr = i->getSource())
                            if (playr->IsInWater() && !playr->HasAura(SPELL_MOLTEN_FIST))
                                playr->AddAura(SPELL_MOLTEN_FIST, playr); // Add the damage aura to players in Magma.

                events.Update(diff);

                while (uint32 eventId = events.ExecuteEvent())
                {
                    switch (eventId)
                    {
                        case EVENT_PULVERIZE_DAMAGE:
                            if(GameObject* platform = me->FindNearestGameObject(209693, 10.0f))
                                platform->SetDestructibleState(GO_DESTRUCTIBLE_DESTROYED);
                            else if(GameObject* platform = me->FindNearestGameObject(209694, 10.0f))
                                platform->SetDestructibleState(GO_DESTRUCTIBLE_DESTROYED);
                            else if(GameObject* platform = me->FindNearestGameObject(209695, 10.0f))
                                platform->SetDestructibleState(GO_DESTRUCTIBLE_DESTROYED);
                            else if(GameObject* platform = me->FindNearestGameObject(209670, 10.0f))
                                platform->SetDestructibleState(GO_DESTRUCTIBLE_DESTROYED);
                            if (Creature * target = me->FindNearestCreature(NPC_PUL_LOCATION, 100.0f))
                                DoCast(target, SPELL_PULVERIZE_D);
                            break;

                        case EVENT_THROW_TOTEM:
                            Talk(SAY_THROW_TOTEM);
                            if (Unit* target = SelectTarget(SELECT_TARGET_RANDOM, 0, 100.0f, true)) // SELECT_TARGET_RANDOM, 0, urand(10.0f, 100.0f), true)
                            {
                                DoCast(target, SPELL_THROW_TOTEM);
                            }
                            events.ScheduleEvent(EVENT_THROW_TOTEM, 25000); // every 25 secs.
                            break;

                        case EVENT_PULVERIZE:
                            me->MonsterYell(urand(0,1) == 1 ? SAY_LAVA_CHAT : SAY_LAVA2_CHAT,0,0);
                            Talk(SAY_PULVERIZE);

                            if (Creature * target = me->FindNearestCreature(NPC_PUL_LOCATION, 250.0f))
                            {
                                me->CastSpell(target,SPELL_PULV_DBM,true);
                            }

                            events.ScheduleEvent(EVENT_PULVERIZE, 40000); // every 40 secs.
                            events.ScheduleEvent(EVENT_PULVERIZE_DAMAGE, 3000); // You have 3 secs to run.
                            break;
                    }
                }

                DoMeleeAttackIfReady();
            }
        };

        CreatureAI* GetAI(Creature* creature) const
        {
            return new boss_echo_of_baineAI(creature);
        }
};

class baint_totem : public CreatureScript
{
    public:
        baint_totem() : CreatureScript("baint_totem") { }

        struct baint_totemAI : public ScriptedAI
        {
            baint_totemAI(Creature* creature) : ScriptedAI(creature) { }
            bool totem;

            void Reset()
            {
                totem = true;
            }

            void DamageTaken(Unit* /*attacker*/, uint32& damage)
            {
                me->setFaction(2110);
                if (Creature * boss = me->FindNearestCreature(NPC_BAINE, 300.0f))
                    if (totem)
                    {
                        DoCast(boss, SPELL_TB_TOTEM);
                        me->DespawnOrUnsummon(1000);
                        totem = false;
                    }
            }

            private:
                EventMap events;
        };

        CreatureAI* GetAI(Creature* creature) const
        {
            return new baint_totemAI(creature);
        }
};

void AddSC_boss_echo_of_baine()
{
    new boss_echo_of_baine();
    new baint_totem();
}
