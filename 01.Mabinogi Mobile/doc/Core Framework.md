## 개요
이 프레임워크는 `GameObject`를 중심으로  
객체 구성, 충돌 처리, 이동, 작업 실행을 각각 분리하여  
런타임 구조가 기능별로 독립적으로 동작하도록 설계했습니다.

각 시스템은 다음과 같이 역할을 나눕니다.

- 객체 구성 → `GameObject + Component`
- 충돌 처리 → `Collider`
- 이동 처리 → `Rigidbody`
- 작업 실행 → `ThreadPool`

이를 통해 특정 기능을 수정하거나 확장할 때  
다른 시스템에 영향을 최소화할 수 있도록 구성했습니다.

---
수정 중  
### 객체 구성과 컴포넌트 구조
- [🔗 GameObject.h](../Scripts/Core%20Framework/GameObject.h)
- [🔗 GameObject.cpp](../Scripts/Core%20Framework/GameObject.cpp)

### 충돌 판정 
- [🔗 Collider.h](../Scripts/Core%20Framework/Collider.h)  
- [🔗 Collider.cpp](../Scripts/Core%20Framework/Collider.cpp)  
- [🔗 PhysicsCollider.h](../Scripts/Core%20Framework/PhysicsCollider.h)  
- [🔗 TriggerCollider.h](../Scripts/Core%20Framework/TriggerCollider.h)

### 충돌 처리 흐름
- [🔗 Collision_Manager.h](../Scripts/Core%20Framework/Collision_Manager.h)  
- [🔗 Collision_Manager.cpp](../Scripts/Core%20Framework/Collision_Manager.cpp)

### 이동 및 물리 처리
- [🔗 Rigidbody.h](../Scripts/Core%20Framework/Rigidbody.h)  
- [🔗 Rigidbody.cpp](../Scripts/Core%20Framework/Rigidbody.cpp)

### Thread Manager
- [🔗 Thread_Manager.h](../Scripts/Core%20Framework/Thread_Manager.h)  
- [🔗 Thread_Manager.cpp](../Scripts/Core%20Framework/Thread_Manager.cpp)

---

## 핵심 구조
- GameObject와 Component 구조
  - 모든 객체는 `CGameObject`를 기반으로 생성
  - 기능은 `Component`를 통해 확장
  - Transform, Collider, Rigidbody 등을 조합하여 객체 구성

---

### 1. GameObject와 Component 구조
```cpp
shared_ptr<CComponent> CGameObject::Add_Component(...)
{
    if (nullptr != Find_Component(strComponentTag))
        return nullptr;

    auto pComponent = m_pGameInstance->Clone_Prototype(...);

    m_Components.emplace(strComponentTag, pComponent);
    return pComponent;
}

shared_ptr<CComponent> CGameObject::Find_Component(const _wstring& strComponentTag)
{
    auto iter = m_Components.find(strComponentTag);
    return (iter == m_Components.end()) ? nullptr : iter->second;
}
```
> GameObject는 기능을 직접 가지지 않고,  
> Component를 조합하는 방식으로 객체를 구성합니다.  
> 이를 통해 객체 구조를 수정하지 않고 기능을 확장할 수 있도록 했습니다.

---

### 2. Collider (충돌 판정)
```cpp
_bool CCollider::Intersect(const CCollider* pTargetCollider)
{
    m_isCollision = m_pBounding->Intersect(
        pTargetCollider->m_eColliderType,
        pTargetCollider->m_pBounding.get()
    );
    return m_isCollision;
}
```
> 돌 판정은 Collider가 담당하고,  
> GameObject는 이벤트(OnTriggerEnter 등)만 처리하도록 분리했습니다.  
> 또한 Trigger 여부를 통해 전투 판정과 일반 충돌을 구분합니다.

---

### 3. Rigidbody
```cpp
void CRigidbody::Update(_float fTimeDelta, shared_ptr<CNavigation> pNavigation)
{
    if (m_bUseGravity)
    {
        _vector vGravity = XMVectorSet(0.f, -9.8f * m_fGravityScale, 0.f, 0.f);
        m_vVelocity += vGravity * fTimeDelta;
    }

    m_vVelocity += m_vExternalForce;
    m_vExternalForce = XMVectorZero();

    _vector vPos = m_pTransform->Get_State(CTransform::STATE_POSITION);
    vPos += m_vVelocity * fTimeDelta;
}
```
> 위치를 직접 수정하는 대신 속도를 누적하여 이동을 처리합니다.  
> 이를 통해 중력, 외력, 감속 등을 하나의 흐름으로 통합했습니다.

---

### 4. ThreadPool
```cpp
void CThread_Manager::WorkerThread()
{
    while (true)
    {
        unique_lock<mutex> lock(m_QueueMutex);

        m_TaskAvailable.wait(lock, [this]()
        {
            return m_Shutdown || !m_TaskQueue.empty();
        });

        auto task = m_TaskQueue.front();
        m_TaskQueue.pop();

        task->Execute();
    }
}
```
> 작업 큐 기반으로 비동기 실행을 처리하며,  
> Worker Thread가 대기 후 Task를 가져와 실행하는 구조입니다.

---

## Design Notes

### 핵심

> 객체는 기능을 직접 수행하지 않고,  
> 기능은 Component와 시스템으로 분산시키는 구조로 설계했습니다.

이를 통해
- 객체는 상태와 조합만 담당하고
- 실제 로직은 각 시스템이 책임지도록 분리했습니다.

또한 충돌, 이동, 작업 실행을 각각 독립된 시스템으로 나누어  
서로의 변경이 다른 시스템에 영향을 주지 않도록 구성했습니다.

### 장점
- 객체 구조 변경 없이 기능 확장 가능  
- 시스템 간 결합도가 낮아 유지보수 용이  
- 충돌, 이동, 실행 흐름이 명확하게 분리됨  
- 다양한 게임 로직(전투, 퀘스트 등)에 재사용 가능  
- 특정 시스템만 교체하거나 개선하기 쉬움  


### 보완할 점
- Component 접근이 문자열 기반으로 동작 (Tag 의존)
  - → 타입 기반 접근 또는 ID 매핑 구조로 개선 가능

- GameObject가 이벤트 인터페이스를 많이 보유
  - → 이벤트 시스템(Event Bus)으로 분리 가능

- ThreadPool에 작업 우선순위 및 취소 기능 없음
  - → 우선순위 큐 및 Task 상태 관리 추가 필요

- Rigidbody와 Collider 간 물리 반응이 제한적
  - → 충돌 응답(Impulse, Bounce 등) 확장 가능
