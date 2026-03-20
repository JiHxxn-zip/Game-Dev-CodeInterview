## Overview
이 전투 판정 시스템은 공격 범위와 피격 처리를 단순 충돌 이벤트에 맡기지 않고,  
`TriggerCollider` 기반 공격 등록과 `Hit_Manager`의 공격 수명 관리를 통해  
일관된 전투 판정 흐름을 구성하도록 설계했습니다.

공격자는 자신의 `TriggerCollider`를 활성 공격으로 등록하고,  
`Hit_Manager`는 활성화된 공격 범위와 피격 대상의 충돌을 검사합니다.  
이후 실제 데미지 적용은 공격자의 `OnHit()`와 `Process_Hit()` 흐름을 통해 처리되며,  
동일 공격의 중복 피격은 별도로 방지합니다.

---

## Core Design
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

- Attacker (`Monster`, `DamageObject`)
  - `OnHit()`에서 데미지 정보 생성
  - `Process_Hit()` 호출

```mermaid
flowchart LR
    Attacker[Attacker / DamageObject] --> Trigger[TriggerCollider]
    Trigger --> StartAttack[GameInstance::Start_Attack]
    StartAttack --> HitManager[Hit_Manager]
    HitManager --> Check[Collision Check]
    Check --> Notify[Attacker::OnHit]
    Notify --> ProcessHit[GameInstance::Process_Hit]
    ProcessHit --> TakeDamage[Target::Take_Damage]
