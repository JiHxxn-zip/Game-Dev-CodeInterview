#pragma once
#include "GameObject.h"
#include "TriggerCollider.h"

BEGIN(Engine)

class CHit_Manager final
{
private:
    CHit_Manager() = default;
public:
    ~CHit_Manager();
    static shared_ptr<CHit_Manager> Create();

public:
    void Update(const _float& fTimeDelta);


public:
    void Start_Attack(const AttackerInfo& info);
    void End_Attack(shared_ptr<CTriggerCollider> pTriggerCollider);

    // 공격 판정 처리
    _bool Process_Hit(shared_ptr<CTriggerCollider> pTriggerCollider, shared_ptr<CGameObject> pTarget, DamageInfo damageInfo);    void Register_PotentialTarget(shared_ptr<CGameObject> pTarget);

    void Clear_PotentialTargets();

private:
    void Notify_Hit(shared_ptr<CTriggerCollider> pTrigger, shared_ptr<CGameObject> pTarget);
    _bool Is_AlreadyHit(shared_ptr<CTriggerCollider> pTriggerCollider, shared_ptr<CGameObject> pTarget);  // 이미 맞은 타겟인지 확인
    _bool Process_Damage(shared_ptr<CTriggerCollider> pTriggerCollider, shared_ptr<CGameObject> pTarget, const _vector& vHitPosition, DamageInfo damageInfo);    void Clear();

private:
    // unordered_map 사용 (키: CTriggerCollider, 값: AttackerInfo)
    unordered_map<shared_ptr<CTriggerCollider>, AttackerInfo> m_Attackers;

    // 활성화된 트리거 콜라이더들
    vector<weak_ptr<CTriggerCollider>> m_ActiveTriggerColliders;

    // 물리 대상 (몬스터 등)
    vector<shared_ptr<CGameObject>> m_PotentialTargets;
};

END