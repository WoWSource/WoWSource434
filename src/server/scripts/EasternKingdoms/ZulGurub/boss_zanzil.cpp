/*
 * Copyright (C) 2008-2013 TrinityCore <http://www.trinitycore.org/>
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License as published by the
 * Free Software Foundation; either version 2 of the License, or (at your
 * option) any later version.
 *
 * This program is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
 * FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for
 * more details.
 *
 * You should have received a copy of the GNU General Public License along
 * with this program. If not, see <http://www.gnu.org/licenses/>.
 */

#include "ScriptMgr.h"
#include "ScriptedCreature.h"
#include "SpellAuraEffects.h"
#include "SpellScript.h"
#include "ObjectMgr.h"
#include "AccountMgr.h"
#include "zulgurub.h"

enum eSpells
{
    SPELL_DRAIN_BLUE_CAULDRON               = 96488,
    SPELL_DRAIN_RED_CAULDRON                = 96486,
    SPELL_DRAIN_GREEN_CAULDRON              = 96487,
    SPELL_GREEN_GAS                         = 97180,
    SPELL_ZANZIL_FIRE                       = 96914,
    SPELL_ZANZILS_GRAVEYARD_GAS             = 96338,
    SPELL_ZANZILS_RESURRECTION_ELIXIR       = 96319,
    SPELL_TERRIBLE_TONIC                    = 96348,
    SPELL_VOODOO_BOLT                       = 96346,
    SPELL_PURSUIT                           = 96342,
    SPELL_THUNDERCLAP                       = 96340,
    SPELL_KNOCK_AWAY                        = 96341,
    SPELL_TOXIC_GAS                         = 96500,
    SPELL_FROSTBURN_FORMULA                 = 96331,
    SPELL_HYPPOTHERMIA                      = 96332
};

enum eEvents
{
    // intro events
    EVENT_DRAIN_BLUE_CAULDRON               = 1,
    EVENT_DRAIN_RED_CAULDRON                = 2,
    EVENT_DRAIN_GREEN_CAULDRON              = 3,
    EVENT_RITUAL_DANCE                      = 4,
    // Zanzil
    EVENT_ZANZILI_FIRE                      = 1,
    EVENT_GRAVEYARD_GAS                     = 2,
    EVENT_RESURRECTION_ELIXIR_BERSERKER     = 3,
    EVENT_RESURRECTION_ELIXIR_ZOMBIE        = 4,
    EVENT_REMOVE_GAS                        = 5,
    EVENT_RESURRECT_BERSERKER               = 6,
    EVENT_RESURRECT_ZOMBIE                  = 7,
    EVENT_RESPAWN_ZOMBIE                    = 8,
    EVENT_RESPAWN_BERSERKER                 = 9,
    EVENT_TERRIBLE_TONIC                    = 10,
    EVENT_VOODOO_BOLT                       = 11,
    // Zanzili Berserker
    EVENT_PURSUIT                           = 1,
    EVENT_THUNDERCLAP                       = 2,
    EVENT_KNOCK_AWAY                        = 3,
};

enum eTexts
{
    SAY_AGGRO                               = 0,
    EMOTE_ZANZIL_ZOMBIES                    = 1,
    SAY_ZANZIL_ZOMBIES                      = 2,
    EMOTE_ZANZIL_GRAVEYARD_GAS              = 3,
    SAY_ZANZIL_GRAVEYARD_GAS                = 4,
    EMOTE_ZANZIL_BERSEKER                   = 5,
    SAY_ZANZIL_BERSEKER                     = 6,
    SAY_PLAYER_KILL                         = 7,
    SAY_DEATH                               = 8,
};

enum eDefine
{
    ACTION_FILL_GAS                         = 1,
    ID_GREEN_CAULDRON                       = 18,
    ID_RED_CAULDRON                         = 19,
    ID_BLUE_CAULDRON                        = 20,
    ID_GREEN_GAS                            = 21,
};

enum eMisc
{
    POINT_BLUE_CAULDRON                     = 1,
    POINT_RED_CAULDRON                      = 2,
    POINT_GREEN_CAULDRON                    = 3,
};

const float CauldronWP[3][3]=
{
    {-11599.7f, -1292.10f, 77.5160f},
    {-11581.7f, -1305.00f, 78.2143f},
    {-11570.5f, -1271.27f, 77.5758f},
};

const Position GasSP[22]=
{
    {-11609.9f, -1222.70f, 87.9190f, 5.20108f},
    {-11623.7f, -1355.40f, 82.2363f, 0.94247f},
    {-11627.2f, -1354.40f, 86.6990f, 0.87266f},
    {-11526.5f, -1256.37f, 87.4362f, 3.87463f},
    {-11623.1f, -1276.96f, 83.8367f, 6.05629f},
    {-11624.9f, -1275.04f, 87.3894f, 6.03884f},
    {-11531.4f, -1292.22f, 83.7441f, 2.32129f},
    {-11625.2f, -1278.44f, 87.4016f, 6.09120f},
    {-11607.4f, -1223.03f, 84.6856f, 5.20108f},
    {-11607.4f, -1220.11f, 87.6618f, 5.18363f},
    {-11534.7f, -1225.67f, 85.6265f, 3.78736f},
    {-11529.4f, -1292.84f, 87.3956f, 2.39110f},
    {-11529.3f, -1254.09f, 87.5675f, 3.94444f},
    {-11529.5f, -1256.81f, 83.3696f, 3.76991f},
    {-11532.1f, -1294.95f, 87.5052f, 2.32129f},
    {-11626.5f, -1357.79f, 86.3576f, 0.90757f},
    {-11536.6f, -1225.10f, 81.7799f, 4.18879f},
    {-11537.2f, -1223.78f, 85.8028f, 4.22370f},
    {-11564.2f, -1258.80f, 78.7486f, 0.00000f},
    {-11570.5f, -1313.09f, 79.3255f, 0.00000f},
    {-11612.7f, -1283.43f, 78.4788f, 0.00000f},
    {-11580.9f, -1287.01f, 77.3751f, 0.00000f},
};

const Position BerserkerSP[3]=
{
    {-11541.4f, -1298.15f, 85.2326f, 2.33874f},
    {-11603.6f, -1233.60f, 81.3652f, 5.20108f},
    {-11545.0f, -1240.56f, 81.5043f, 3.92699f},
};

const Position ZombieSP[56]=
{
    // first
    {-11617.4f, -1320.57f, 79.0296f, 0.0f},
    {-11621.9f, -1320.01f, 78.0138f, 0.0f},
    {-11620.8f, -1318.51f, 78.0817f, 0.0f},
    {-11615.5f, -1315.01f, 78.5370f, 0.0f},
    {-11622.0f, -1316.31f, 77.9510f, 0.0f},
    {-11611.5f, -1314.75f, 78.4497f, 0.0f},
    {-11614.7f, -1312.70f, 79.0384f, 0.0f},
    {-11618.1f, -1311.61f, 78.6514f, 0.0f},
    {-11616.9f, -1313.22f, 78.6516f, 0.0f},
    {-11617.5f, -1317.20f, 78.6535f, 0.0f},
    {-11614.4f, -1317.85f, 78.3475f, 0.0f},
    {-11618.6f, -1315.94f, 78.3023f, 0.0f},
    {-11620.2f, -1314.36f, 78.2763f, 0.0f},
    {-11614.1f, -1314.81f, 78.5450f, 0.0f},
    // second
    {-11582.8f, -1335.41f, 80.3524f, 0.0f},
    {-11585.7f, -1338.02f, 80.3073f, 0.0f},
    {-11584.3f, -1335.33f, 80.1910f, 0.0f},
    {-11588.9f, -1337.81f, 79.7257f, 0.0f},
    {-11592.2f, -1336.09f, 78.7726f, 0.0f},
    {-11594.4f, -1334.59f, 78.2969f, 0.0f},
    {-11588.6f, -1333.96f, 80.1995f, 0.0f},
    {-11592.2f, -1331.85f, 78.3594f, 0.0f},
    {-11583.4f, -1330.97f, 79.5243f, 0.0f},
    {-11590.7f, -1329.90f, 78.3559f, 0.0f},
    {-11588.7f, -1327.12f, 78.3854f, 0.0f},
    {-11582.1f, -1332.14f, 79.9445f, 0.0f},
    {-11586.6f, -1332.13f, 79.1948f, 0.0f},
    {-11586.5f, -1327.73f, 78.6510f, 0.0f},
    // third
    {-11593.8f, -1254.56f, 77.9005f, 0.0f},
    {-11587.3f, -1260.73f, 77.5984f, 0.0f},
    {-11596.0f, -1257.31f, 78.4782f, 0.0f},
    {-11593.8f, -1258.81f, 77.8374f, 0.0f},
    {-11590.3f, -1249.84f, 77.8057f, 0.0f},
    {-11583.7f, -1254.85f, 78.0329f, 0.0f},
    {-11585.0f, -1253.69f, 77.9192f, 0.0f},
    {-11590.5f, -1260.52f, 77.5950f, 0.0f},
    {-11584.4f, -1258.12f, 77.8612f, 0.0f},
    {-11592.3f, -1252.61f, 77.6953f, 0.0f},
    {-11590.2f, -1256.68f, 77.6088f, 0.0f},
    {-11585.9f, -1258.05f, 77.7285f, 0.0f},
    {-11588.1f, -1250.44f, 77.8582f, 0.0f},
    {-11588.2f, -1254.84f, 77.6245f, 0.0f},
    // fourth
    {-11544.3f, -1262.48f, 78.4120f, 0.0f},
    {-11544.3f, -1258.24f, 78.4740f, 0.0f},
    {-11540.7f, -1260.35f, 79.0832f, 0.0f},
    {-11546.4f, -1260.98f, 78.2899f, 0.0f},
    {-11537.8f, -1264.41f, 79.3455f, 0.0f},
    {-11541.0f, -1264.20f, 78.9616f, 0.0f},
    {-11534.9f, -1261.80f, 79.6320f, 0.0f},
    {-11536.4f, -1261.72f, 79.4727f, 0.0f},
    {-11542.8f, -1256.29f, 79.0399f, 0.0f},
    {-11538.7f, -1258.52f, 79.0910f, 0.0f},
    {-11534.2f, -1258.53f, 79.5070f, 0.0f},
    {-11540.8f, -1253.51f, 78.9323f, 0.0f},
    {-11538.6f, -1254.12f, 79.2066f, 0.0f},
    {-11535.5f, -1257.36f, 79.5052f, 0.0f},
};

const float ZombieCenter[4][3]=
{
    {-11618.7f, -1315.94f, 78.2161f},
    {-11586.8f, -1333.17f, 79.2700f},
    {-11591.0f, -1254.50f, 77.5513f},
    {-11539.4f, -1259.90f, 78.9608f},
};

class boss_zanzil : public CreatureScript
{
    class GasEvent : public BasicEvent
    {
        public:
            GasEvent(Creature* creature) : me(creature) {}

        private:
            bool Execute(uint64 /*time*/, uint32 /*diff*/)
            {
                me->AI()->DoAction(ACTION_FILL_GAS);
                return true;
            }

        private:
            Creature* me;
    };

    public:
        boss_zanzil() : CreatureScript("boss_zanzil") { }

    private:
        struct boss_zanzilAI : public BossAI
        {
            boss_zanzilAI(Creature* creature) : BossAI(creature, DATA_ZANZIL) { }

            uint64 GasGUID[22];
            uint64 ZombieGUID[56];
            uint64 BerserkerGUID[3];
            uint8 ResurrectionId;

            void InitializeAI()
            {
                for (int i = 0; i < ID_GREEN_CAULDRON; ++i)
                    if (Creature* gas = me->SummonCreature(52062, GasSP[i]))
                        GasGUID[i] = gas->GetGUID();

                static uint32 GasSpell[4]=
                {
                    96868,
                    96869,
                    96867,
                    97180,
                };

                for (int i = ID_GREEN_CAULDRON; i <= ID_GREEN_GAS; ++i)
                    if (Creature* gas = me->SummonCreature(52062, GasSP[i]))
                    {
                        GasGUID[i] = gas->GetGUID();
                        gas->CastSpell(gas, GasSpell[i - ID_GREEN_CAULDRON], false);
                    }

                memset(&ZombieGUID, 0, sizeof(ZombieGUID));
                memset(&BerserkerGUID, 0, sizeof(BerserkerGUID));
                Reset();
            }

            void Reset()
            {
                _Reset();
                RemoveGas();
                ResurrectionId = 0;
                me->SetWalk(true);
                me->SetReactState(REACT_AGGRESSIVE);
                events.ScheduleEvent(EVENT_DRAIN_BLUE_CAULDRON, 1000);
                instance->SendEncounterUnit(ENCOUNTER_FRAME_DISENGAGE, me);

                if (GameObject* forcefield = me->FindNearestGameObject(GO_ZANZIL_DOOR, 550.0f))
                    me->RemoveGameObject(forcefield, true);

                for (int i = 0; i < 56; ++i)
                    if (!ZombieGUID[i])
                        if (Creature* zombie = me->SummonCreature(52055, ZombieSP[i], TEMPSUMMON_CORPSE_TIMED_DESPAWN, 5000))
                            ZombieGUID[i] = zombie->GetGUID();

                for (int i = 0; i < 3; ++i)
                    if (!BerserkerGUID[i])
                        if (Creature* berserker = me->SummonCreature(52054, BerserkerSP[i], TEMPSUMMON_CORPSE_TIMED_DESPAWN, 5000))
                            BerserkerGUID[i] = berserker->GetGUID();
            }

            void EnterEvadeMode()
            {
                me->m_Events.KillAllEvents(false);
                BossAI::EnterEvadeMode();
            }

            void MovementInform(uint32 type, uint32 id)
            {
                if (type != POINT_MOTION_TYPE)
                    return;

                switch (id)
                {
                    case POINT_BLUE_CAULDRON:
                        {
                            if (GameObject* cauldron = me->FindNearestGameObject(208314, 20.0f))
                                me->SetFacingTo(me->GetAngle(cauldron));

                            me->CastSpell((Unit*)NULL, SPELL_DRAIN_BLUE_CAULDRON, false);
                            events.ScheduleEvent(EVENT_RITUAL_DANCE, 4000);
                            events.ScheduleEvent(RAND(EVENT_DRAIN_RED_CAULDRON, EVENT_DRAIN_GREEN_CAULDRON), urand(20000, 30000));
                        }
                        break;
                    case POINT_RED_CAULDRON:
                        {
                            if (GameObject* cauldron = me->FindNearestGameObject(208313, 20.0f))
                                me->SetFacingTo(me->GetAngle(cauldron));

                            me->CastSpell((Unit*)NULL, SPELL_DRAIN_RED_CAULDRON, false);
                            events.ScheduleEvent(EVENT_RITUAL_DANCE, 4000);
                            events.ScheduleEvent(RAND(EVENT_DRAIN_BLUE_CAULDRON, EVENT_DRAIN_GREEN_CAULDRON), urand(20000, 30000));
                        }
                        break;
                    case POINT_GREEN_CAULDRON:
                        {
                            if (GameObject* cauldron = me->FindNearestGameObject(208315, 20.0f))
                                me->SetFacingTo(me->GetAngle(cauldron));

                            me->CastSpell((Unit*)NULL, SPELL_DRAIN_GREEN_CAULDRON, false);
                            events.ScheduleEvent(EVENT_RITUAL_DANCE, 4000);
                            events.ScheduleEvent(RAND(EVENT_DRAIN_BLUE_CAULDRON, EVENT_DRAIN_RED_CAULDRON), urand(20000, 30000));
                        }
                        break;
                }
            }

            void RemoveGas()
            {
                if (Creature* gas = ObjectAccessor::GetCreature(*me, GasGUID[ID_GREEN_GAS]))
                    gas->RemoveAllAuras();

                for (int i = 0; i < ID_GREEN_CAULDRON; ++i)
                    if (Creature* gas = ObjectAccessor::GetCreature(*me, GasGUID[i]))
                        gas->RemoveAllAuras();

                instance->DoRemoveAurasDueToSpellOnPlayers(96434);
            }

            void JustSummoned(Creature* summoned)
            {
                switch (summoned->GetEntry())
                {
                    case 52062:
                    case 52054:
                    case 52055:
                        return;
                }

                summons.Summon(summoned);
            }

            void DoAction(int32 const action)
            {
                if (action == ACTION_FILL_GAS)
                {
                    Talk(SAY_ZANZIL_GRAVEYARD_GAS);
                    events.ScheduleEvent(EVENT_REMOVE_GAS, 15000);
                    events.ScheduleEvent(RAND(EVENT_RESURRECTION_ELIXIR_BERSERKER, EVENT_RESURRECTION_ELIXIR_ZOMBIE), 30000);

                    if (Creature* gas = ObjectAccessor::GetCreature(*me, GasGUID[ID_GREEN_GAS]))
                        gas->CastSpell(gas, SPELL_GREEN_GAS, false);

                    Map::PlayerList const &players = me->GetMap()->GetPlayers();

                    for (Map::PlayerList::const_iterator itr = players.begin(); itr != players.end(); ++itr)
                        if (Player* player = itr->getSource())
                            player->CastSpell(player, 96434, true, NULL, NULL, me->GetGUID());
                }
            }

            void EnterCombat(Unit* /*who*/)
            {
                _EnterCombat();
                Talk(SAY_AGGRO);
                events.Reset();
                events.ScheduleEvent(EVENT_ZANZILI_FIRE, urand(5000, 15000));
                events.ScheduleEvent(EVENT_VOODOO_BOLT, urand(5000, 15000));
                events.ScheduleEvent(EVENT_TERRIBLE_TONIC, urand(5000, 15000));
                events.ScheduleEvent(RAND(EVENT_RESURRECTION_ELIXIR_BERSERKER, EVENT_GRAVEYARD_GAS, EVENT_RESURRECTION_ELIXIR_ZOMBIE), urand(15000, 30000));
                instance->SendEncounterUnit(ENCOUNTER_FRAME_ENGAGE, me);
                me->SummonGameObject(GO_ZANZIL_DOOR, -11613.1f, -1360.27f, 92.2573f, 1.50443f, 0, 0, 0, 0, 0);
            }

            void KilledUnit(Unit* victim)
            {
                if (victim->GetTypeId() == TYPEID_PLAYER)
                    Talk(SAY_PLAYER_KILL);
            }

            void JustDied(Unit* /*killer*/)
            {
                _JustDied();
                Talk(SAY_DEATH);
                RemoveGas();
                instance->SendEncounterUnit(ENCOUNTER_FRAME_DISENGAGE, me);

                if (GameObject* forcefield = me->FindNearestGameObject(GO_ZANZIL_DOOR, 550.0f))
                    me->RemoveGameObject(forcefield, true);
            }

            void UpdateAI(uint32 const diff)
            {
                events.Update(diff);

                if (!UpdateVictim())
                {
                    if (uint32 eventId = events.ExecuteEvent())
                    {
                        switch (eventId)
                        {
                            case EVENT_DRAIN_BLUE_CAULDRON:
                                me->SetUInt32Value(UNIT_NPC_EMOTESTATE, EMOTE_ONESHOT_NONE);
                                me->GetMotionMaster()->MovePoint(POINT_BLUE_CAULDRON, CauldronWP[0][0], CauldronWP[0][1], CauldronWP[0][2]);
                                break;
                            case EVENT_DRAIN_RED_CAULDRON:
                                me->SetUInt32Value(UNIT_NPC_EMOTESTATE, EMOTE_ONESHOT_NONE);
                                me->GetMotionMaster()->MovePoint(POINT_RED_CAULDRON, CauldronWP[1][0], CauldronWP[1][1], CauldronWP[1][2]);
                                break;
                            case EVENT_DRAIN_GREEN_CAULDRON:
                                me->SetUInt32Value(UNIT_NPC_EMOTESTATE, EMOTE_ONESHOT_NONE);
                                me->GetMotionMaster()->MovePoint(POINT_GREEN_CAULDRON, CauldronWP[2][0], CauldronWP[2][1], CauldronWP[2][2]);
                                break;
                            case EVENT_RITUAL_DANCE:
                                me->SetUInt32Value(UNIT_NPC_EMOTESTATE, EMOTE_STATE_DANCE);
                                break;
                        }
                    }

                    return;
                }

                if (me->HasUnitState(UNIT_STATE_CASTING))
                    return;

                if (uint32 eventId = events.ExecuteEvent())
                {
                    switch (eventId)
                    {
                        case EVENT_ZANZILI_FIRE:
                            {
                                if (Unit* target = SelectTarget(SELECT_TARGET_RANDOM, 0, 50.0f, true))
                                {
                                    me->StopMoving();
                                    me->SetFacingTo(me->GetAngle(target));
                                    me->CastSpell(target, SPELL_ZANZIL_FIRE, false);
                                }

                                events.ScheduleEvent(EVENT_ZANZILI_FIRE, urand(5000, 15000));
                            }
                            break;
                        case EVENT_GRAVEYARD_GAS:
                            Talk(EMOTE_ZANZIL_GRAVEYARD_GAS);
                            me->CastSpell(me, SPELL_ZANZILS_GRAVEYARD_GAS, false);
                            me->m_Events.AddEvent(new GasEvent(me), me->m_Events.CalculateTime(2*IN_MILLISECONDS));
                            break;
                        case EVENT_REMOVE_GAS:
                            RemoveGas();
                            break;
                        case EVENT_RESURRECTION_ELIXIR_BERSERKER:
                            Talk(EMOTE_ZANZIL_BERSEKER);
                            ResurrectionId = urand(0, 2);
                            float x, y, z;
                            BerserkerSP[ResurrectionId].GetPosition(x, y, z);
                            me->SetReactState(REACT_PASSIVE);
                            me->AttackStop();
                            me->StopMoving();
                            me->SetFacingTo(me->GetAngle(x, y));
                            me->CastSpell(x, y, z, SPELL_ZANZILS_RESURRECTION_ELIXIR, false);
                            events.ScheduleEvent(EVENT_RESURRECT_BERSERKER, 7000);
                            events.ScheduleEvent(EVENT_RESPAWN_BERSERKER, 15000);
                            break;
                        case EVENT_RESURRECT_BERSERKER:
                            {
                                Talk(SAY_ZANZIL_BERSEKER);
                                me->SetReactState(REACT_AGGRESSIVE);
                                events.ScheduleEvent(RAND(EVENT_GRAVEYARD_GAS, EVENT_RESURRECTION_ELIXIR_ZOMBIE), 30000);

                                if (Creature* berserker = ObjectAccessor::GetCreature(*me, BerserkerGUID[ResurrectionId]))
                                {
                                    BerserkerGUID[ResurrectionId] = 0;
                                    berserker->GetMotionMaster()->MoveFall();
                                    berserker->SetReactState(REACT_AGGRESSIVE);
                                    berserker->RemoveFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_NON_ATTACKABLE | UNIT_FLAG_NOT_SELECTABLE);
                                    berserker->SetInCombatWithZone();
                                }
                            }
                            break;
                        case EVENT_RESPAWN_BERSERKER:
                            {
                                if (Creature* berserker = me->SummonCreature(52054, BerserkerSP[ResurrectionId], TEMPSUMMON_CORPSE_TIMED_DESPAWN, 5000))
                                    BerserkerGUID[ResurrectionId] = berserker->GetGUID();
                            }
                            break;
                        case EVENT_RESURRECTION_ELIXIR_ZOMBIE:
                            Talk(EMOTE_ZANZIL_ZOMBIES);
                            ResurrectionId = urand(0, 3);
                            me->SetReactState(REACT_PASSIVE);
                            me->AttackStop();
                            me->StopMoving();
                            me->SetFacingTo(me->GetAngle(ZombieCenter[ResurrectionId][0], ZombieCenter[ResurrectionId][1]));
                            me->CastSpell(ZombieCenter[ResurrectionId][0], ZombieCenter[ResurrectionId][1], ZombieCenter[ResurrectionId][2], 96316, false);
                            events.ScheduleEvent(EVENT_RESURRECT_ZOMBIE, 7000);
                            events.ScheduleEvent(EVENT_RESPAWN_ZOMBIE, 15000);
                            break;
                        case EVENT_RESURRECT_ZOMBIE:
                            {
                                Talk(SAY_ZANZIL_ZOMBIES);
                                me->SetReactState(REACT_AGGRESSIVE);
                                events.ScheduleEvent(RAND(EVENT_GRAVEYARD_GAS, EVENT_RESURRECTION_ELIXIR_BERSERKER), 30000);

                                for (int i = ResurrectionId * 14; i < ResurrectionId * 14 + 14; ++i)
                                    if (Creature* zombie = ObjectAccessor::GetCreature(*me, ZombieGUID[i]))
                                    {
                                        ZombieGUID[i] = 0;
                                        zombie->SetReactState(REACT_AGGRESSIVE);
                                        zombie->RemoveFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_NON_ATTACKABLE | UNIT_FLAG_NOT_SELECTABLE);
                                        zombie->SetInCombatWithZone();
                                    }
                            }
                            break;
                        case EVENT_RESPAWN_ZOMBIE:
                            {
                                for (int i = ResurrectionId * 14; i < ResurrectionId * 14 + 14; ++i)
                                    if (Creature* zombie = me->SummonCreature(52055, ZombieSP[i], TEMPSUMMON_CORPSE_TIMED_DESPAWN, 5000))
                                        ZombieGUID[i] = zombie->GetGUID();
                            }
                            break;
                        case EVENT_TERRIBLE_TONIC:
                            {
                                if (Unit* target = SelectTarget(SELECT_TARGET_RANDOM, 0, 50.0f, true))
                                    me->CastSpell(target, SPELL_TERRIBLE_TONIC, false);

                                events.ScheduleEvent(EVENT_TERRIBLE_TONIC, urand(5000, 15000));
                            }
                            break;
                        case EVENT_VOODOO_BOLT:
                            {
                                if (Unit* target = SelectTarget(SELECT_TARGET_RANDOM, 0, 50.0f, true))
                                    me->CastSpell(target, SPELL_VOODOO_BOLT, false);

                                events.ScheduleEvent(EVENT_VOODOO_BOLT, urand(5000, 15000));
                            }
                            break;
                    }
                }

                DoMeleeAttackIfReady();
                EnterEvadeIfOutOfCombatArea(diff);
            }
        };

        CreatureAI* GetAI(Creature* creature) const
        {
            return GetZulGurubAI<boss_zanzilAI>(creature);
        }
};

struct npc_gurubashi_berserkerAI : public ScriptedAI
{
    npc_gurubashi_berserkerAI(Creature* creature) : ScriptedAI(creature) { }

    EventMap events;

    void EnterCombat(Unit* /*who*/)
    {
        events.ScheduleEvent(EVENT_PURSUIT, 1000);
        events.ScheduleEvent(EVENT_THUNDERCLAP, urand(5000, 15000));
        events.ScheduleEvent(EVENT_KNOCK_AWAY, urand(5000, 15000));
    }

    void UpdateAI(uint32 const diff)
    {
        if (!UpdateVictim())
            return;

        events.Update(diff);

        if (me->HasUnitState(UNIT_STATE_CASTING))
            return;

        if (uint32 eventId = events.ExecuteEvent())
        {
            switch (eventId)
            {
            case EVENT_PURSUIT:
                {
                    if (Unit* target = SelectTarget(SELECT_TARGET_RANDOM, 0, 100.0f, true))
                    {
                        Talk(0, target->GetGUID());
                        Talk(1, target->GetGUID());
                        me->CastSpell(target, SPELL_PURSUIT, false);
                    }

                    events.ScheduleEvent(EVENT_PURSUIT, urand(10000, 20000));
                }
                break;
            case EVENT_THUNDERCLAP:
                me->CastSpell((Unit*)NULL, SPELL_THUNDERCLAP, false);
                events.ScheduleEvent(EVENT_THUNDERCLAP, urand(10000, 20000));
                break;
            case EVENT_KNOCK_AWAY:
                me->CastSpell(me->GetVictim(), SPELL_KNOCK_AWAY, false);
                events.ScheduleEvent(EVENT_KNOCK_AWAY, urand(10000, 20000));
                break;
            }
        }

        DoMeleeAttackIfReady();
    }
};

class npc_gurubashi_berserker : public CreatureScript
{
    public:
        npc_gurubashi_berserker() : CreatureScript("npc_gurubashi_berserker") { }

    private:
        CreatureAI* GetAI(Creature* creature) const
        {
            return GetZulGurubAI<npc_gurubashi_berserkerAI>(creature);
        }
};

class npc_zanzili_berserker : public CreatureScript
{
    public:
        npc_zanzili_berserker() : CreatureScript("npc_zanzili_berserker") { }

    private:
        struct npc_zanzili_berserkerAI : public npc_gurubashi_berserkerAI
        {
            npc_zanzili_berserkerAI(Creature* creature) : npc_gurubashi_berserkerAI(creature) { }

            void InitializeAI()
            {
            }

            void EnterEvadeMode()
            {
                me->DespawnOrUnsummon();
            }
        };

        CreatureAI* GetAI(Creature* creature) const
        {
            return GetZulGurubAI<npc_zanzili_berserkerAI>(creature);
        }
};

class npc_zanzili_zombie : public CreatureScript
{
    public:
        npc_zanzili_zombie() : CreatureScript("npc_zanzili_zombie") { }

    private:
        struct npc_zanzili_zombieAI : public ScriptedAI
        {
            npc_zanzili_zombieAI(Creature* creature) : ScriptedAI(creature) { }

            void InitializeAI()
            {
            }

            void UpdateAI(uint32 const /*diff*/)
            {
                if (!UpdateVictim())
                    return;

                DoMeleeAttackIfReady();
            }
        };

        CreatureAI* GetAI(Creature* creature) const
        {
            return GetZulGurubAI<npc_zanzili_zombieAI>(creature);
        }
};

class spell_zanzili_fire : public SpellScriptLoader
{
public:
    spell_zanzili_fire() : SpellScriptLoader("spell_zanzili_fire") { }
    
    class spell_zanzili_fire_SpellScript : public SpellScript
    {
        PrepareSpellScript(spell_zanzili_fire_SpellScript);
        
        void OnTargetHit(SpellEffIndex /*fffff*/)
        {
            if (!GetExplTargetDest())
                return;
            
            DynamicObject* dynObj = new DynamicObject(false);
            if (!dynObj->CreateDynamicObject(sObjectMgr->GenerateLowGuid(HIGHGUID_DYNAMICOBJECT), GetCaster(), GetSpellInfo(), *GetExplTargetDest(), 0.0f, DYNAMIC_OBJECT_AREA_SPELL))
            {
                delete dynObj;
                return;
            }
            dynObj->SetDuration(1);
        }
        
        void Register()
        {
            OnEffectLaunch += SpellEffectFn(spell_zanzili_fire_SpellScript::OnTargetHit, EFFECT_0, SPELL_EFFECT_DUMMY);
        }
    };
    
    class spell_zanzili_fire_AuraScript : public AuraScript
    {
        PrepareAuraScript(spell_zanzili_fire_AuraScript);
        
        bool Validate(SpellInfo const* /*spell*/)
        {
            return true;
        }
        
        void HandleEffectApply(AuraEffect const* aurEff, AuraEffectHandleModes handles)
        {
            if (DynamicObject* dynObj = GetUnitOwner()->GetDynObject(GetSpellInfo()->Id))
            {
                myAngle = GetUnitOwner()->GetAngle(dynObj);
                lastPos.Relocate(GetCaster());
            }
            
            /*if (Player* castplr = GetCaster()->ToPlayer())
                if (!AccountMgr::IsPlayerAccount(castplr->GetSession()->GetSecurity()))
                    maxTicks = -1;*/
        }
        
        void HandleEffectPeriodic(AuraEffect const* aurEff)
        {
            if (Unit* caster = GetCaster())
            {
                caster->MovePositionToFirstCollision(lastPos, 2.0f, myAngle - GetCaster()->GetOrientation());
                lastPos.m_positionZ -= 0.5f;
                caster->CastSpell(lastPos.m_positionX, lastPos.m_positionY, lastPos.m_positionZ, 96916, true);
            }
            
           //if (aurEff->GetTickNumber() == maxTicks)
             //   Remove(AURA_REMOVE_BY_DEFAULT);
        }
        
        void Register()
        {
            OnEffectApply += AuraEffectApplyFn(spell_zanzili_fire_AuraScript::HandleEffectApply, EFFECT_1, SPELL_AURA_PERIODIC_DUMMY, AURA_EFFECT_HANDLE_REAL_OR_REAPPLY_MASK);
            OnEffectPeriodic += AuraEffectPeriodicFn(spell_zanzili_fire_AuraScript::HandleEffectPeriodic, EFFECT_1, SPELL_AURA_PERIODIC_DUMMY);
        }
    
        //int8 maxTicks = 15;
        float myAngle;
        Position lastPos;
    };
    
    AuraScript* GetAuraScript() const
    {
        return new spell_zanzili_fire_AuraScript();
    }
    
    SpellScript* GetSpellScript() const
    {
        return new spell_zanzili_fire_SpellScript();
    }
};

class AuraCheck : public std::unary_function<Unit*, bool>
{
    public:
        explicit AuraCheck() {}

        bool operator()(WorldObject* object)
        {
            return object->ToUnit()->HasAuraType(SPELL_AURA_DUMMY);
        }
};

class spell_zanzils_graveyard_gas : public SpellScriptLoader
{
    public:
        spell_zanzils_graveyard_gas() : SpellScriptLoader("spell_zanzils_graveyard_gas") { }

    private:
        class spell_zanzils_graveyard_gas_SpellScript : public SpellScript
        {
            PrepareSpellScript(spell_zanzils_graveyard_gas_SpellScript);

            void FilterTargets(std::list<WorldObject*>& unitList)
            {
                unitList.remove_if(AuraCheck());

                if (unitList.size() > 3)
                    unitList.resize(3);
            }

            void HandleScript(SpellEffIndex /*eff*/)
            {
                GetHitUnit()->CastSpell(GetHitUnit(), SPELL_TOXIC_GAS, false);
            }

            void Register()
            {
                OnObjectAreaTargetSelect += SpellObjectAreaTargetSelectFn(spell_zanzils_graveyard_gas_SpellScript::FilterTargets, EFFECT_0, TARGET_UNIT_DEST_AREA_ENTRY);
                OnEffectHitTarget += SpellEffectFn(spell_zanzils_graveyard_gas_SpellScript::HandleScript, EFFECT_0, SPELL_EFFECT_SCRIPT_EFFECT);
            }
        };

        SpellScript* GetSpellScript() const
        {
            return new spell_zanzils_graveyard_gas_SpellScript();
        }
};

class spell_pursuit_fixate : public SpellScriptLoader
{
    public:
        spell_pursuit_fixate() : SpellScriptLoader("spell_pursuit_fixate") { }

    private:
        class spell_pursuit_fixate_SpellScript : public SpellScript
        {
            PrepareSpellScript(spell_pursuit_fixate_SpellScript)

            void CastSpell(SpellEffIndex effIndex)
            {
                if (Creature* caster = GetCaster()->ToCreature())
                {
                    caster->getThreatManager().resetAllAggro();
                    caster->AddThreat(GetHitUnit(), GetSpellInfo()->Effects[effIndex].BasePoints);
                }
            }

            void Register()
            {
                OnEffectHitTarget += SpellEffectFn(spell_pursuit_fixate_SpellScript::CastSpell, EFFECT_1, SPELL_EFFECT_SCRIPT_EFFECT);
            }
        };

        SpellScript *GetSpellScript() const
        {
            return new spell_pursuit_fixate_SpellScript();
        }
};

class spell_zanzils_graveyard_gas_damage : public SpellScriptLoader
{
    public:
        spell_zanzils_graveyard_gas_damage() : SpellScriptLoader("spell_zanzils_graveyard_gas_damage") { }

    private:
        class spell_zanzils_graveyard_gas_damage_AuraScript : public AuraScript
        {
            PrepareAuraScript(spell_zanzils_graveyard_gas_damage_AuraScript)

            void HandleTick(AuraEffect const* aurEff)
            {
                PreventDefaultAction();

                if (!GetCaster())
                    return;

                float damage = GetTarget()->GetMaxHealth() * GetSpellInfo()->Effects[EFFECT_0].BasePoints / 100;
                damage = GetTarget()->SpellDamageBonusTaken(GetSpellInfo(), damage, DOT, 1, GetCasterGUID());

                int32 overkill = damage - GetTarget()->GetHealth();
                if (overkill < 0)
                    overkill = 0;

                SpellPeriodicAuraLogInfo info(aurEff, damage, overkill, 0.0f, 0.0f, 0.0f, 0.0f);
                GetTarget()->SendPeriodicAuraLog(&info);

                GetCaster()->DealDamage(GetTarget(), damage, 0, DOT, GetSpellInfo()->GetSchoolMask(), GetSpellInfo(), true);
            }

            void Register()
            {
                OnEffectPeriodic += AuraEffectPeriodicFn(spell_zanzils_graveyard_gas_damage_AuraScript::HandleTick, EFFECT_0, SPELL_AURA_PERIODIC_DAMAGE_PERCENT);
            }
        };

        AuraScript *GetAuraScript() const
        {
            return new spell_zanzils_graveyard_gas_damage_AuraScript();
        }
};

class spell_frostburn_formula : public SpellScriptLoader
{
    public:
        spell_frostburn_formula() : SpellScriptLoader("spell_frostburn_formula") { }

        class spell_frostburn_formula_SpellScript : public SpellScript
        {
            PrepareSpellScript(spell_frostburn_formula_SpellScript);
            

            void HandleScript(SpellEffIndex effIndex)
            {
                if (!GetCaster() || !GetHitUnit())
                    return;

                SetHitDamage(CalculatePct(GetHitUnit()->GetMaxHealth(), 65));
                GetHitUnit()->CastSpell(GetHitUnit(), SPELL_HYPPOTHERMIA, true);
            }

            void Register()
            {
                OnEffectHitTarget += SpellEffectFn(spell_frostburn_formula_SpellScript::HandleScript, EFFECT_0, SPELL_EFFECT_APPLY_AURA);
            }
        };

        SpellScript* GetSpellScript() const
        {
            return new spell_frostburn_formula_SpellScript();
        }
};

void AddSC_boss_zanzil()
{
    new boss_zanzil();
    new npc_gurubashi_berserker();
    new npc_zanzili_berserker();
    new npc_zanzili_zombie();

    new spell_zanzili_fire();
    new spell_zanzils_graveyard_gas();
    new spell_pursuit_fixate();
    new spell_zanzils_graveyard_gas_damage();
    new spell_frostburn_formula();
}
