/*
 * This file is part of the TrinityCore Project. See AUTHORS file for Copyright information
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

/* ScriptData
SDName: Boss Black Knight
SD%Complete: 80%
SDComment: missing yells. not sure about timers.
SDCategory: Trial of the Champion
EndScriptData */

#include "ScriptMgr.h"
#include "InstanceScript.h"
#include "ScriptedEscortAI.h"
#include "SpellInfo.h"
#include "SpellScript.h"
#include "trial_of_the_champion.h"

enum Spells
{
    //phase 1
    SPELL_PLAGUE_STRIKE     = 67884,
    SPELL_PLAGUE_STRIKE_2   = 67724,
    SPELL_ICY_TOUCH_H       = 67881,
    SPELL_ICY_TOUCH         = 67718,
    SPELL_DEATH_RESPITE     = 67745,
    SPELL_DEATH_RESPITE_2   = 68306,
    SPELL_DEATH_RESPITE_3   = 66798,
    SPELL_OBLITERATE_H      = 67883,
    SPELL_OBLITERATE        = 67725,
    //in this phase should rise herald (the spell is missing)

    //phase 2 - During this phase, the Black Knight will use the same abilities as in phase 1, except for Death's Respite
    SPELL_ARMY_DEAD         = 67761,
    SPELL_DESECRATION       = 67778,
    SPELL_DESECRATION_2     = 67778,
    SPELL_GHOUL_EXPLODE     = 67751,

    //phase 3
    SPELL_DEATH_BITE_H      = 67875,
    SPELL_DEATH_BITE        = 67808,
    SPELL_MARKED_DEATH      = 67882,
    SPELL_MARKED_DEATH_2    = 67823,

    SPELL_BLACK_KNIGHT_RES  = 67693,

    SPELL_LEAP              = 67749,
    SPELL_LEAP_H            = 67880,

    SPELL_KILL_CREDIT       = 68663
};

enum Models
{
    MODEL_SKELETON = 29846,
    MODEL_GHOST    = 21300
};

enum Phases
{
    PHASE_UNDEAD    = 1,
    PHASE_SKELETON  = 2,
    PHASE_GHOST     = 3
};

class boss_black_knight : public CreatureScript
{
public:
    boss_black_knight() : CreatureScript("boss_black_knight") { }

    struct boss_black_knightAI : public ScriptedAI
    {
        boss_black_knightAI(Creature* creature) : ScriptedAI(creature), summons(creature)
        {
            Initialize();
            instance = creature->GetInstanceScript();
        }

        void Initialize()
        {
            bEventInProgress = false;
            bEvent = false;
            bSummonArmy = false;
            bDeathArmyDone = false;

            uiPhase = PHASE_UNDEAD;

            uiIcyTouchTimer = urand(5000, 9000);
            uiPlagueStrikeTimer = urand(10000, 13000);
            uiDeathRespiteTimer = urand(15000, 16000);
            uiObliterateTimer = urand(17000, 19000);
            uiDesecration = urand(15000, 16000);
            uiDeathArmyCheckTimer = 7000;
            uiResurrectTimer = 4000;
            uiGhoulExplodeTimer = 8000;
            uiDeathBiteTimer = urand(2000, 4000);
            uiMarkedDeathTimer = urand(5000, 7000);
        }

        InstanceScript* instance;

        SummonList summons;

        bool bEventInProgress;
        bool bEvent;
        bool bSummonArmy;
        bool bDeathArmyDone;

        uint8 uiPhase;

        uint32 uiPlagueStrikeTimer;
        uint32 uiIcyTouchTimer;
        uint32 uiDeathRespiteTimer;
        uint32 uiObliterateTimer;
        uint32 uiDesecration;
        uint32 uiResurrectTimer;
        uint32 uiDeathArmyCheckTimer;
        uint32 uiGhoulExplodeTimer;
        uint32 uiDeathBiteTimer;
        uint32 uiMarkedDeathTimer;

        void Reset() override
        {
            summons.DespawnAll();
            me->SetDisplayId(me->GetNativeDisplayId());
            me->ClearUnitState(UNIT_STATE_ROOT | UNIT_STATE_STUNNED);

            Initialize();
        }

        void JustSummoned(Creature* summon) override
        {
            summons.Summon(summon);
            summon->AI()->AttackStart(me->GetVictim());
        }

        void SummonedCreatureDespawn(Creature* summon) override
        {
            summons.Despawn(summon);
        }

        void UpdateAI(uint32 uiDiff) override
        {
            //Return since we have no target
            if (!UpdateVictim())
                return;

            if (bEventInProgress)
            {
                if (uiResurrectTimer <= uiDiff)
                {
                    me->SetFullHealth();
                    DoCast(me, SPELL_BLACK_KNIGHT_RES, true);
                    uiPhase++;
                    uiResurrectTimer = 4000;
                    bEventInProgress = false;
                    me->ClearUnitState(UNIT_STATE_ROOT | UNIT_STATE_STUNNED);
                } else uiResurrectTimer -= uiDiff;
            }

            switch (uiPhase)
            {
                case PHASE_UNDEAD:
                case PHASE_SKELETON:
                {
                    if (uiIcyTouchTimer <= uiDiff)
                    {
                        DoCastVictim(SPELL_ICY_TOUCH);
                        uiIcyTouchTimer = urand(5000, 7000);
                    } else uiIcyTouchTimer -= uiDiff;
                    if (uiPlagueStrikeTimer <= uiDiff)
                    {
                        DoCastVictim(SPELL_ICY_TOUCH);
                        uiPlagueStrikeTimer = urand(12000, 15000);
                    } else uiPlagueStrikeTimer -= uiDiff;
                    if (uiObliterateTimer <= uiDiff)
                    {
                        DoCastVictim(SPELL_OBLITERATE);
                        uiObliterateTimer = urand(17000, 19000);
                    } else uiObliterateTimer -= uiDiff;
                    switch (uiPhase)
                    {
                        case PHASE_UNDEAD:
                        {
                            if (uiDeathRespiteTimer <= uiDiff)
                            {
                                if (Unit* target = SelectTarget(SelectTargetMethod::Random, 0, 100, true))
                                {
                                    if (target->IsAlive())
                                        DoCast(target, SPELL_DEATH_RESPITE);
                                }
                                uiDeathRespiteTimer = urand(15000, 16000);
                            } else uiDeathRespiteTimer -= uiDiff;
                            break;
                        }
                        case PHASE_SKELETON:
                        {
                            if (!bSummonArmy)
                            {
                                bSummonArmy = true;
                                me->AddUnitState(UNIT_STATE_ROOT | UNIT_STATE_STUNNED);
                                DoCast(me, SPELL_ARMY_DEAD);
                            }
                            if (!bDeathArmyDone)
                            {
                                if (uiDeathArmyCheckTimer <= uiDiff)
                                {
                                    me->ClearUnitState(UNIT_STATE_ROOT | UNIT_STATE_STUNNED);
                                    uiDeathArmyCheckTimer = 0;
                                    bDeathArmyDone = true;
                                } else uiDeathArmyCheckTimer -= uiDiff;
                            }
                            if (uiDesecration <= uiDiff)
                            {
                                if (Unit* target = SelectTarget(SelectTargetMethod::Random, 0, 100, true))
                                {
                                    if (target->IsAlive())
                                        DoCast(target, SPELL_DESECRATION);
                                }
                                uiDesecration = urand(15000, 16000);
                            } else uiDesecration -= uiDiff;
                            if (uiGhoulExplodeTimer <= uiDiff)
                            {
                                DoCast(me, SPELL_GHOUL_EXPLODE);
                                uiGhoulExplodeTimer = 8000;
                            } else uiGhoulExplodeTimer -= uiDiff;
                            break;
                        }
                        break;
                    }
                    break;
                }
                case PHASE_GHOST:
                {
                    if (uiDeathBiteTimer <= uiDiff)
                    {
                        DoCastAOE(SPELL_DEATH_BITE);
                        uiDeathBiteTimer = urand(2000, 4000);
                    } else uiDeathBiteTimer -= uiDiff;
                    if (uiMarkedDeathTimer <= uiDiff)
                    {
                        if (Unit* target = SelectTarget(SelectTargetMethod::Random, 0, 100, true))
                        {
                            if (target->IsAlive())
                                DoCast(target, SPELL_MARKED_DEATH);
                        }
                        uiMarkedDeathTimer = urand(5000, 7000);
                    } else uiMarkedDeathTimer -= uiDiff;
                    break;
                }
            }

            if (!me->HasUnitState(UNIT_STATE_ROOT) && !me->HealthBelowPct(1))
                DoMeleeAttackIfReady();
        }

        void DamageTaken(Unit* /*pDoneBy*/, uint32& uiDamage, DamageEffectType /*damageType*/, SpellInfo const* /*spellInfo = nullptr*/) override
        {
            if (uiDamage > me->GetHealth() && uiPhase <= PHASE_SKELETON)
            {
                uiDamage = 0;
                me->SetHealth(0);
                me->AddUnitState(UNIT_STATE_ROOT | UNIT_STATE_STUNNED);
                summons.DespawnAll();
                switch (uiPhase)
                {
                    case PHASE_UNDEAD:
                        me->SetDisplayId(MODEL_SKELETON);
                        break;
                    case PHASE_SKELETON:
                        me->SetDisplayId(MODEL_GHOST);
                        break;
                }
                bEventInProgress = true;
            }
        }

        void JustDied(Unit* /*killer*/) override
        {
            DoCast(me, SPELL_KILL_CREDIT);

            instance->SetBossState(BOSS_BLACK_KNIGHT, DONE);
        }
    };

    CreatureAI* GetAI(Creature* creature) const override
    {
        return GetTrialOfTheChampionAI<boss_black_knightAI>(creature);
    }
};

class npc_risen_ghoul : public CreatureScript
{
public:
    npc_risen_ghoul() : CreatureScript("npc_risen_ghoul") { }

    struct npc_risen_ghoulAI : public ScriptedAI
    {
        npc_risen_ghoulAI(Creature* creature) : ScriptedAI(creature)
        {
            Initialize();
        }

        void Initialize()
        {
            uiAttackTimer = 3500;
        }

        uint32 uiAttackTimer;

        void Reset() override
        {
            Initialize();
        }

        void UpdateAI(uint32 uiDiff) override
        {
            if (!UpdateVictim())
                return;

            if (uiAttackTimer <= uiDiff)
            {
                if (Unit* target = SelectTarget(SelectTargetMethod::Random, 1, 100, true))
                {
                    if (target->IsAlive())
                        DoCast(target, (SPELL_LEAP));
                }
                uiAttackTimer = 3500;
            } else uiAttackTimer -= uiDiff;

            DoMeleeAttackIfReady();
        }
    };

    CreatureAI* GetAI(Creature* creature) const override
    {
        return GetTrialOfTheChampionAI<npc_risen_ghoulAI>(creature);
    }
};

static constexpr uint32 PATH_ESCORT_GRYPHON = 283930;

class npc_black_knight_skeletal_gryphon : public CreatureScript
{
public:
    npc_black_knight_skeletal_gryphon() : CreatureScript("npc_black_knight_skeletal_gryphon") { }

    struct npc_black_knight_skeletal_gryphonAI : public EscortAI
    {
        npc_black_knight_skeletal_gryphonAI(Creature* creature) : EscortAI(creature)
        {
            LoadPath(PATH_ESCORT_GRYPHON);
            Start(false);
        }

        void UpdateAI(uint32 uiDiff) override
        {
            EscortAI::UpdateAI(uiDiff);

            UpdateVictim();
        }

    };

    CreatureAI* GetAI(Creature* creature) const override
    {
        return GetTrialOfTheChampionAI<npc_black_knight_skeletal_gryphonAI>(creature);
    }
};

// 67751 - Ghoul Explode
class spell_black_knight_ghoul_explode : public SpellScript
{
    bool Validate(SpellInfo const* spellInfo) override
    {
        return ValidateSpellInfo({ uint32(spellInfo->GetEffect(EFFECT_0).CalcValue()) });
    }

    void HandleScript(SpellEffIndex /*effIndex*/)
    {
        GetHitUnit()->CastSpell(GetHitUnit(), uint32(GetEffectInfo(EFFECT_0).CalcValue()));
    }

    void Register() override
    {
        OnEffectHitTarget += SpellEffectFn(spell_black_knight_ghoul_explode::HandleScript, EFFECT_1, SPELL_EFFECT_SCRIPT_EFFECT);
    }
};

// 67754 - Ghoul Explode
// 67889 - Ghoul Explode
class spell_black_knight_ghoul_explode_risen_ghoul : public SpellScript
{
    bool Validate(SpellInfo const* spellInfo) override
    {
        return ValidateSpellInfo({ uint32(spellInfo->GetEffect(EFFECT_1).CalcValue()) });
    }

    void HandleScript(SpellEffIndex /*effIndex*/)
    {
        GetCaster()->CastSpell(GetCaster(), uint32(GetEffectValue()));
    }

    void Register() override
    {
        OnEffectHit += SpellEffectFn(spell_black_knight_ghoul_explode_risen_ghoul::HandleScript, EFFECT_1, SPELL_EFFECT_SCRIPT_EFFECT);
    }
};

void AddSC_boss_black_knight()
{
    new boss_black_knight();
    new npc_risen_ghoul();
    new npc_black_knight_skeletal_gryphon();
    RegisterSpellScript(spell_black_knight_ghoul_explode);
    RegisterSpellScript(spell_black_knight_ghoul_explode_risen_ghoul);
}
