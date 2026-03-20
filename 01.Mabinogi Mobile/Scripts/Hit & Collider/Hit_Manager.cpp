#include "Hit_Manager.h"
#include "GameObject.h"
#include "map"
#include "PhysicsCollider.h"

CHit_Manager::~CHit_Manager()
{
    Clear();
}

shared_ptr<CHit_Manager> CHit_Manager::Create()
{
    return shared_ptr<CHit_Manager>(new CHit_Manager());
}

void CHit_Manager::Update(const _float& fTimeDelta)
{
    // 콜라이더 정리
    m_ActiveTriggerColliders.erase(
        remove_if(m_ActiveTriggerColliders.begin(), m_ActiveTriggerColliders.end(),
            [](const weak_ptr<CTriggerCollider>& c) { return c.expired(); }),
        m_ActiveTriggerColliders.end()
    );

    // 공격 시간 업데이트
    for (auto& [collider, info] : m_Attackers)
    {
        if (info.bIsAttacking)
        {
            info.fAttackStartTime += fTimeDelta;

            if (info.fAttackStartTime >= info.fAttackDuration) // 공격 한번 유지시간
                End_Attack(collider);
        }
    }

    // 충돌 검사
    for (auto& weakTrigger : m_ActiveTriggerColliders)
    {
        auto trigger = weakTrigger.lock();
        if (!trigger || !trigger->Is_AttackActive())
            continue;

        for (auto& pTarget : m_PotentialTargets)
        {
            if (!pTarget || !pTarget->Is_Active())
                continue;

            // PhysicsCollider 가져오기
            auto physicsCollider = dynamic_pointer_cast<CPhysicsCollider>(pTarget->Find_Component(L"Com_PhysicsCollider"));
            if (!physicsCollider || !physicsCollider->is_Active())
                continue;

            if (trigger->Intersect(physicsCollider.get()))
            {
                Notify_Hit(trigger, pTarget);                
            }
        }
    }
}

void CHit_Manager::Notify_Hit(shared_ptr<CTriggerCollider> pTrigger, shared_ptr<CGameObject> pTarget)
{
    if (!pTrigger || !pTarget)
        return;

    auto pAttacker = pTrigger->Get_Owner().lock();
    if (!pAttacker)
        return;

    // 공격자에게 피격 발생 알림
    pAttacker->OnHit(pTarget);
}

void CHit_Manager::Start_Attack(const AttackerInfo& info)
{
    auto pTriggerCollider = info.pTriggerCollider;
    if (!pTriggerCollider)
        return;

    // 중복된 트리거 콜라이더가 있는지 검사하고 없을 때만 추가
    auto it = std::find_if(m_ActiveTriggerColliders.begin(), m_ActiveTriggerColliders.end(),
        [&](const weak_ptr<CTriggerCollider>& elem) {
            return !elem.expired() && elem.lock() == pTriggerCollider;
        });

    if (it == m_ActiveTriggerColliders.end())
    {
        m_ActiveTriggerColliders.push_back(pTriggerCollider);
    }

    // 트리거 콜라이더 활성화
    pTriggerCollider->Set_Active(true);
    pTriggerCollider->Set_AttackActive(true);

    // 공격자 정보 등록
    m_Attackers[pTriggerCollider] = info;
}

void CHit_Manager::End_Attack(shared_ptr<CTriggerCollider> pTriggerCollider)
{
    if (!pTriggerCollider)
        return;

    auto it = m_Attackers.find(pTriggerCollider);
    if (it != m_Attackers.end())
    {
        it->second.bIsAttacking = false;
        it->second.HitTargets.clear();
    }

    // 트리거 콜라이더 비활성화
    pTriggerCollider->Set_Active(false);
    pTriggerCollider->Set_AttackActive(false);
}

_bool CHit_Manager::Process_Hit(shared_ptr<CTriggerCollider> pTriggerCollider, shared_ptr<CGameObject> pTarget, DamageInfo damageInfo)
{
    if (pTriggerCollider->Is_AttackActive() == false)
        return false;

    if (!pTriggerCollider || !pTarget)
        return false;

    // 이미 맞은 타겟인지 확인
    if (Is_AlreadyHit(pTriggerCollider, pTarget))
        return false;

    // 충돌 위치 계산
    _vector vHitPosition = pTarget->Get_Transform()->Get_State(CTransform::STATE_POSITION);

    auto it = m_Attackers.find(pTriggerCollider);
    if (it != m_Attackers.end())
    {
        if (it->second.HitTargets.size() >= 1)
        {
            damageInfo.bIsMultiHit = true;
        }
    }

    // 데미지 처리
    return Process_Damage(pTriggerCollider, pTarget, vHitPosition, damageInfo);
}

_bool CHit_Manager::Process_Damage(shared_ptr<CTriggerCollider> pTriggerCollider, shared_ptr<CGameObject> pTarget,
    const _vector& vHitPosition, DamageInfo damageInfo)
{
    if (!pTriggerCollider || !pTarget)
        return false;

    // 이미 맞은 타겟인지 확인
    if (Is_AlreadyHit(pTriggerCollider, pTarget))
        return false;

    auto it = m_Attackers.find(pTriggerCollider);
    if (it == m_Attackers.end() || !it->second.bIsAttacking)
        return false;

    // 데미지 정보 생성
    //damageInfo.fDamage = it->second.fDamage;
    //damageInfo.bIsCritical = true;
    //damageInfo.vHitPosition = vHitPosition;
    damageInfo.vColliderPosition = pTriggerCollider->Get_Center();

    // 타겟에 데미지 적용
    pTarget->Take_Damage(damageInfo);

    // 이미 맞은 타겟으로 추가
    it->second.HitTargets.push_back(pTarget);

    if (it->second.bSingleTarget)
    {
        it->second.bIsAttacking = false;
    }

    return true;
}

_bool CHit_Manager::Is_AlreadyHit(shared_ptr<CTriggerCollider> pTriggerCollider, shared_ptr<CGameObject> pTarget)
{
    auto it = m_Attackers.find(pTriggerCollider); 
    if (it == m_Attackers.end())
        return false;

    const auto& hitTargets = it->second.HitTargets;
    return find(hitTargets.begin(), hitTargets.end(), pTarget) != hitTargets.end();
}

void CHit_Manager::Register_PotentialTarget(shared_ptr<CGameObject> pTarget)
{
    m_PotentialTargets.push_back(pTarget);
}

void CHit_Manager::Clear_PotentialTargets()
{
    m_PotentialTargets.clear();
}

void CHit_Manager::Clear()
{
    m_Attackers.clear();
    m_ActiveTriggerColliders.clear();
}
