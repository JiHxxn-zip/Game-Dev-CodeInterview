## 개요
이 전투 판정 시스템은 공격 범위와 피격 처리를 단순 충돌 이벤트에 맡기지 않고,  
`TriggerCollider` 기반 공격 등록과 `Hit_Manager`의 공격 수명 관리를 통해  
일관된 전투 판정 흐름을 구성하도록 설계했습니다.

공격자는 자신의 `TriggerCollider`를 활성 공격으로 등록하고,  
`Hit_Manager`는 활성화된 공격 범위와 피격 대상의 충돌을 검사합니다.  
이후 실제 데미지 적용은 공격자의 `OnHit()`와 `Process_Hit()` 흐름을 통해 처리되며,  
동일 공격의 중복 피격은 별도로 방지합니다.

---

수정 중
[🔗 DamageObject.h](../Scripts/Glasgavelen%20Boss%20AI/DamageObject.h)  
[🔗 DamageObject.cpp](../Scripts/Glasgavelen%20Boss%20AI/DamageObject.cpp)

[🔗 GlasgavelenStateAttack.h](../Scripts/Glasgavelen%20Boss%20AIt/GlasgavelenStateAttack.h)  
[🔗 GlasgavelenStateIAttack.cpp](../Scripts/Glasgavelen%20Boss%20AI/GlasgavelenStateIAttack.cpp)

[🔗 GlasgavelenDefaultAttack.h](../Scripts/Glasgavelen%20Boss%20AI/GlasgavelenDefaultAttack.h)  
[🔗 GlasgavelenDefaultAttack.cpp](../Scripts/Glasgavelen%20Boss%20AI/GlasgavelenDefaultAttack.cpp)

---

## 핵심 구조
- `CTriggerCollider`
  - 공격 판정 범위를 표현
  - 공격 활성화 여부를 가짐

- `CHit_Manager`
  - 공격자 등록 / 해제
  - 공격 유지 시간 관리
  - 충돌 검사
  - 중복 피격 방지
  - 최종 데미지 처리

- `CGameInstance`
  - `Start_Attack`, `End_Attack`, `Process_Hit` 진입점 제공

- 공격 주체 (`Player`, `Monster`)
  - 충돌이 발생했을 때 `OnHit()` 콜백을 받음
  - 자신의 공격 정보로 `DamageInfo`를 구성함
  - `Process_Hit()`를 호출해 실제 피격 처리를 요청함

### 1. 공격 시작과 활성화
공격 객체는 자신의 TriggerCollider를 공격자로 등록해 일정 시간 동안 활성 판정을 수행합니다.
```cpp
void CHit_Manager::Start_Attack(const AttackerInfo& info)
{
    auto pTriggerCollider = info.pTriggerCollider;
    if (!pTriggerCollider)
        return;

    auto it = std::find_if(m_ActiveTriggerColliders.begin(), m_ActiveTriggerColliders.end(),
        [&](const weak_ptr<CTriggerCollider>& elem) {
            return !elem.expired() && elem.lock() == pTriggerCollider;
        });

    if (it == m_ActiveTriggerColliders.end())
    {
        m_ActiveTriggerColliders.push_back(pTriggerCollider);
    }

    pTriggerCollider->Set_Active(true);
    pTriggerCollider->Set_AttackActive(true);

    m_Attackers[pTriggerCollider] = info;
}
```
> 이 구조에서는 공격이 단순히 충돌체가 존재하는 상태가 아니라,<br>
> 명시적으로 시작되고 종료되는 활성 공격 단위로 관리됩니다.

---

### 2. Hit_Manager의 공격 시간 관리와 충돌 검사
Hit_Manager는 공격 유지 시간을 갱신하고,  
활성 TriggerCollider와 잠재 타겟 사이의 충돌을 검사합니다.
```cpp
void CHit_Manager::Update(const _float& fTimeDelta)
{
    m_ActiveTriggerColliders.erase(
        remove_if(m_ActiveTriggerColliders.begin(), m_ActiveTriggerColliders.end(),
            [](const weak_ptr<CTriggerCollider>& c) { return c.expired(); }),
        m_ActiveTriggerColliders.end()
    );

    for (auto& [collider, info] : m_Attackers)
    {
        if (info.bIsAttacking)
        {
            info.fAttackStartTime += fTimeDelta;

            if (info.fAttackStartTime >= info.fAttackDuration)
                End_Attack(collider);
        }
    }

    for (auto& weakTrigger : m_ActiveTriggerColliders)
    {
        auto trigger = weakTrigger.lock();
        if (!trigger || !trigger->Is_AttackActive())
            continue;

        for (auto& pTarget : m_PotentialTargets)
        {
            if (!pTarget || !pTarget->Is_Active())
                continue;

            auto physicsCollider = dynamic_pointer_cast<CPhysicsCollider>(
                pTarget->Find_Component(L"Com_PhysicsCollider"));

            if (!physicsCollider || !physicsCollider->is_Active())
                continue;

            if (trigger->Intersect(physicsCollider.get()))
            {
                Notify_Hit(trigger, pTarget);                
            }
        }
    }
}
```
> 이 흐름에서 Hit_Manager는 다음을 담당합니다.
> - 공격 유지 시간 갱신
> - 활성 공격만 검사
> - 공격 범위와 피격 대상 충돌 검사
> - 충돌 발생 시 공격 주체에게 알림 전달  

> 즉, 전투 판정은 단순 충돌 이벤트가 아니라  
> 활성 공격 목록을 기준으로 제어되는 별도 흐름으로 관리됩니다.

---

### 3. 동일 공격의 중복 피격 방지
같은 공격이 같은 대상에게 여러 번 적용되지 않도록 피격 이력을 추적합니다.
```cpp
_bool CHit_Manager::Is_AlreadyHit(shared_ptr<CTriggerCollider> pTriggerCollider, shared_ptr<CGameObject> pTarget)
{
    auto it = m_Attackers.find(pTriggerCollider); 
    if (it == m_Attackers.end())
        return false;

    const auto& hitTargets = it->second.HitTargets;
    return find(hitTargets.begin(), hitTargets.end(), pTarget) != hitTargets.end();
}
```
```cpp
_bool CHit_Manager::Process_Hit(shared_ptr<CTriggerCollider> pTriggerCollider, shared_ptr<CGameObject> pTarget, DamageInfo damageInfo)
{
    if (pTriggerCollider->Is_AttackActive() == false)
        return false;

    if (!pTriggerCollider || !pTarget)
        return false;

    if (Is_AlreadyHit(pTriggerCollider, pTarget))
        return false;

    _vector vHitPosition = pTarget->Get_Transform()->Get_State(CTransform::STATE_POSITION);

    auto it = m_Attackers.find(pTriggerCollider);
    if (it != m_Attackers.end())
    {
        if (it->second.HitTargets.size() >= 1)
        {
            damageInfo.bIsMultiHit = true;
        }
    }

    return Process_Damage(pTriggerCollider, pTarget, vHitPosition, damageInfo);
}
```
> 이 방식 덕분에 지속형 공격, 확산형 공격, 넓은 범위 공격에서도  
> 한 번의 공격이 동일 대상에게 과도하게 중복 적용되는 문제를 줄일 수 있었습니다.

---

### 4. 공격 주체가 데미지를 결정하는 흐름
충돌 검사는 공통으로 처리하되, 최종 데미지 정보는 공격 주체가 결정합니다.
```cpp
void CHit_Manager::Notify_Hit(shared_ptr<CTriggerCollider> pTrigger, shared_ptr<CGameObject> pTarget)
{
    if (!pTrigger || !pTarget)
        return;

    auto pAttacker = pTrigger->Get_Owner().lock();
    if (!pAttacker)
        return;

    pAttacker->OnHit(pTarget);
}
```
```cpp
void CAnimalMonster::OnHit(shared_ptr<CGameObject> pTarget)
{
    if (pTarget->Get_GameObjectTag() != TEXT("GameObject_Player"))
        return;

    auto pTrigger = std::dynamic_pointer_cast<CTriggerCollider>(m_pHitColliderCom);
    if (!pTrigger)
        return;

    DamageInfo damageInfo(static_pointer_cast<CGameObject>(shared_from_this()), m_Monster_Status->iDamage);
    m_pGameInstance->Process_Hit(m_pHitColliderCom, pTarget, damageInfo);
}
```
> 흐름은 이렇게 이어집니다.
> 1. Hit_Manager가 충돌을 감지함
> 2. 공격 주체의 OnHit()를 호출함
> 3. 공격 주체가 자신의 공격 정보로 DamageInfo를 구성함
> 4. Process_Hit()를 통해 실제 피격 처리를 요청함

> 이 구조의 장점은 충돌 판정은 공통으로 유지하면서도,  
> 데미지 내용은 공격 종류에 따라 다르게 구성할 수 있다는 점입니다.

---

### 5. DamageObject의 공격 처리 예시
DamageObject는 공격 준비가 끝난 뒤   
자신의 TriggerCollider를 공격자로 등록하고 이후 히트 처리를 수행합니다.
```cpp
void Client::CBardBardsTail::Update(_float fTimeDelta)
{
    CDamageObject::Update(fTimeDelta);

    m_ActiveTimer += fTimeDelta;
    m_pHitColliderCom->GrowRing(fTimeDelta);

    if (m_ActiveTimer >= m_ActiveDuration[m_ActiveCount])
    {
        if (++m_ActiveCount >= 3)
        {
            Set_Active(false);
            return;
        }

        m_pHitColliderCom->Set_RingSize(0.2f, 0.7f);

        if (auto pTrigger = dynamic_pointer_cast<CTriggerCollider>(m_pHitColliderCom))
        {
            AttackerInfo attackerInfo;
            attackerInfo.pTriggerCollider = pTrigger;
            attackerInfo.bIsAttacking = true;
            attackerInfo.fAttackDuration = 1.0f;

            m_pGameInstance->Start_Attack(attackerInfo);
        }
    }
}
```
> 이 예시는 공격 오브젝트가 다음 흐름을 스스로 수행한다는 점을 보여줍니다.
> - 공격 범위 변화
> - 공격 활성 시점 결정
> - TriggerCollider 등록
> - Hit_Manager에 판정 위임

> 즉 공격 오브젝트는 “언제 공격을 시작할지”를 관리하고,
> 실제 판정의 일관성은 Hit_Manager가 담당합니다.

---

## Design Notes

### 핵심
이 시스템에서 가장 중요하게 본 부분은 공격 판정을 단순 Trigger 이벤트가 아니라,<br>
수명을 가진 공격 단위로 관리하는 것 이었습니다.

즉 공격은
- 시작 시 등록되고
- 유지 시간 동안만 유효하며
- 같은 대상에 대한 중복 피격을 막고
- 최종 데미지 적용은 공격 주체가 결정하는

하나의 명시적 전투 판정 흐름으로 구성했습니다.

### 장점
- 공격 활성 시간과 종료 시점을 일관되게 관리 가능
- 같은 공격의 중복 피격 방지 가능
- 충돌 검사와 데미지 계산 책임을 분리 가능
- 몬스터 / 플레이어 / 스킬 오브젝트가 같은 판정 흐름을 공유 가능
- TriggerCollider를 이용한 범위 공격 확장이 쉬움

### 보완할 점  
- 현재는 잠재 타겟 전체를 순회하므로, 공간 분할 구조를 추가하면 더 효율적일 수 있음
- Hit_Manager가 공격 시간 관리와 충돌 검사를 모두 담당하므로 장기적으로 역할을 더 나눌 수 있음
- 공격 판정 관련 메타데이터를 더 데이터화하면 스킬별 재사용성과 밸런싱 효율이 올라갈 수 있음
