# 01. Mabinogi Mobile Clone (DirectX 11)

> 2025.06 — 2025.08  
> Team Leader (6인) | Framework Design / Boss AI / Gameplay Systems

마비노기 모바일의 핵심 전투·상호작용 구조를 DirectX 11 기반으로 구현하며,  
단순 기능 복제가 아니라 **확장 가능한 게임플레이 프레임워크 설계**에 집중한 프로젝트입니다.

---

## 핵심 키워드
- Data-Driven Quest System
- Event-Driven Objective Processing
- Trigger-Based Hit Detection
- Phase-Based Boss FSM
- Collider / Rigidbody Framework
- NPC Script Orchestration

---

### [1. Quest System](./Quest_System.md)
<sub>
JSON 기반 데이터 드리븐 구조와 이벤트 기반 Objective 갱신 구조를 통해  
새로운 퀘스트 타입을 코드 수정 최소화로 확장할 수 있도록 설계했습니다.
</sub>
### [2. Glasgavelen Boss AI](./Glasgavelen_Boss_AI.md)
<sub>
20개 이상의 상태 클래스로 FSM을 구성하고,  
페이즈 전환 시 패턴 큐를 교체하는 방식으로 보스 전투 흐름을 구조화했습니다.
</sub>
### [3. Hit / Collider Framework](./Hit_Collider_Framework.md)
<sub>
TriggerCollider, PhysicsCollider, Rigidbody를 분리하여  
전투 판정과 물리 충돌 책임을 명확히 분리했습니다.
</sub>
