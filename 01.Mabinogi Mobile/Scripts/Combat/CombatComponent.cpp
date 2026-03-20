#include "CombatComponent.h"
#include "GameInstance.h"

#include <GameObject.h>
#include <Client_Util.h>

#include "Effect_Manager.h"
#include "StatusCom.h"
#include "MonsterBase.h"

Client::CCombatComponent::CCombatComponent(ComPtr<ID3D11Device> pDevice, ComPtr<ID3D11DeviceContext> pContext)
    : CComponent{pDevice, pContext}
{
}

Client::CCombatComponent::CCombatComponent(const CCombatComponent& Prototype)
    : CComponent{Prototype}
{
}

shared_ptr<Client::CCombatComponent> Client::CCombatComponent::Create(ComPtr<ID3D11Device> pDevice, ComPtr<ID3D11DeviceContext> pContext)
{
    auto pGameInstance = shared_ptr<CCombatComponent>(new CCombatComponent(pDevice, pContext));

    if (FAILED(pGameInstance->Initialize_Prototype()))
    {
        MSG_BOX("Failed to Create : CCombatComponent");
    }

    return pGameInstance;
}

shared_ptr<CComponent> Client::CCombatComponent::Clone(void* pArg)
{
    auto pGameInstance = shared_ptr<CCombatComponent>(new CCombatComponent(*this));

    if (FAILED(pGameInstance->Initialize(pArg)))
    {
        MSG_BOX("Failed to Clone : CCombatComponent");
    }

    return pGameInstance;
}

HRESULT Client::CCombatComponent::Initialize_Prototype()
{
    return S_OK;
}

HRESULT Client::CCombatComponent::Initialize(void* pArg)
{
    COMBAT_DESC* pDesc = static_cast<COMBAT_DESC*>(pArg);
    if (pDesc == nullptr)
        return E_FAIL;

    m_pOwner = pDesc->pOwner;
    m_fAttackDistance = pDesc->fAttackDistance;
    return S_OK;
}

DamageInfo Client::CCombatComponent::GenerateDamage(weak_ptr<CMainPlayer> pPlayer, weak_ptr<CMonsterBase> pMonster, const DamageParams& params)
{
    // 플레이어 스탯
    const STATUSCOM_DESC playerStatus = pPlayer.lock()->Get_StatusCom()->Get_StatusDesc();

    // 몬스터 스탯
    auto pMonsterStatus = pMonster.lock()->Get_MonsterStatus();

    // 공격력 및 방어력
    _int baseAtk = pPlayer.lock()->Get_StatusCom()->Update_Atk();
    _int targetArmor = pMonsterStatus->iAmor;

    _bool isCritical = false;

    _int baseDamage = (params.fBaseDamage > 0.f)
        ? static_cast<_int>(params.fBaseDamage) : static_cast<_int>(baseAtk * params.fAtkRatio + params.iFixedDamage);

    _int finalDamage = CalculateFinalDamage(baseDamage, targetArmor, isCritical,
        params.bCanCrit ? (_float)playerStatus.tStatus_Meta.iLuck : 0.f);

    _int brakePower = params.iBrakePower;

    _vector vHitPos = pMonster.lock()->Get_Transform()->Get_State(CTransform::STATE_POSITION);

    _bool isMultiHit = false;
    _bool isCounter = pMonster.lock()->Get_CombatState() == ECombatState::COMBAT_ATTACK;
    _bool isVulnerable = pMonsterStatus->bBraking;
    _bool isFirstAttack = CombatUtils::IsFullHP(pMonsterStatus);
    _bool isBreak = pMonsterStatus->bActiveBrake;
    _bool isFinish = CombatUtils::IsFinish(pMonsterStatus, finalDamage);

    return DamageInfo(pPlayer.lock(), finalDamage, brakePower, vHitPos, isCritical, isMultiHit, isCounter, isVulnerable, isFirstAttack, isBreak, isFinish);
}

_int CCombatComponent::CalculateFinalDamage(_int baseAtk, _int targetAmor, _bool& isCritical, _float critRate)
{
    isCritical = CombatUtils::IsCritical(critRate);

    // 크리티컬 보정
    _float damage = RandomRange(baseAtk * 0.9f, baseAtk * 1.1f);
    if (isCritical)
        damage *= RandomRange(1.5f, 2.0f);

    // 방어력 보정 (간단 예시: 선형 감소)
    damage = max(1.f, damage - (_float)targetAmor * 0.5f);

    return static_cast<_int>(damage);
}


#pragma region Skill Cool Time

void CCombatComponent::Update_Cooldowns(_float fTimeDelta)
{
    for (auto& [name, slot] : m_CooldownSlots)
        slot.Update(fTimeDelta);
}

bool CCombatComponent::Can_UseSkill(const string& name)
{
    return m_CooldownSlots[name].CanUse();
}

void CCombatComponent::Start_Cooldown(const string& name)
{
    const auto& info = m_SkillCooldownData[name];
    auto& slot = m_CooldownSlots[name];

    // 한번만 초기화
    slot.iMaxCount = info.iMaxCount;
    slot.fCooldownPerStack = info.fCooldownPerStack;

    slot.Use(); // 스택 하나 사용하고, 회복 타이머 돌림
}

void CCombatComponent::Register_SkillCooldown(string skillTag, _int maxCount, _float cooldownPerStack)
{
    m_SkillCooldownData[skillTag] = { maxCount, cooldownPerStack };
}

void CCombatComponent::Set_IsUsingSkill(_bool isUsingSkill)
{
    m_bIsUsingSkill = isUsingSkill;
    if (isUsingSkill == false)
        CGameInstance::GetInstance()->Add_DebugMessage("UsingSkillOff");
}

_int CCombatComponent::Get_SkillUITimer(string skillName)
{
    return static_cast<_int>(m_CooldownSlots[skillName].fUITimer);
}

#pragma endregion


_float Client::CCombatComponent::Compute_DamageByCombatEffect(_float baseDamage)
{

    return 0.f;
}
