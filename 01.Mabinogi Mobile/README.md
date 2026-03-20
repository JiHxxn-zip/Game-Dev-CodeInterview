# 01. Mabinogi Mobile Clone (DirectX 11)

> 2025.06 — 2025.08  
> Team Leader (6인) | Framework Design / Boss AI / Gameplay Systems

마비노기 모바일의 핵심인 전투와 상호작용 구조를 DirectX 11 기반으로 구현하며,  
**확장 가능한 게임플레이 프레임워크 설계**에 집중한 프로젝트입니다.

---

## 핵심 키워드
- Data-Driven Quest System
- Event-Driven Objective Processing
- Trigger-Based Hit Detection
- Phase-Based Boss FSM
- Collider / Rigidbody Framework
- NPC Script Orchestration

---

[🔗 1. Quest System](./doc/Quest_System.md)
> JSON 기반 데이터 드리븐 구조와 이벤트 기반 Objective 갱신 구조를 통해<br>
> 새로운 퀘스트 타입을 코드 수정 최소화로 확장할 수 있도록 설계했습니다.

[🔗 2. Glasgavelen Boss AI](./doc/Glasgavelen_Boss_AI.md)
> 다수의 패턴을 FSM 상태 객체로 분리하고,<br>
> 공격 판정은 `DamageObject`로 독립시켜 전투 흐름과 히트 처리를 분리했습니다.

[🔗 3. Hit / Collider Framework](./doc/Hit_Collider_Framework.md)
> `TriggerCollider`, `PhysicsCollider`, `Rigidbody`를 역할별로 분리하고,<br>
> `Hit_Manager`와 `Collision_Manager`를 나눠 전투 판정과 일반 충돌 처리를 분리했습니다.
