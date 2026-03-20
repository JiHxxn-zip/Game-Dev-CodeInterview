#include "Collision_Manager.h"
#include "GameObject.h"
#include "Collider.h"
#include "Rigidbody.h"
#include "Bounding_AABB.h"

CCollision_Manager::~CCollision_Manager()
{
    m_Colliders.clear();
}

shared_ptr<CCollision_Manager> CCollision_Manager::Create()
{
    return shared_ptr<CCollision_Manager>(new CCollision_Manager());
}

void CCollision_Manager::CollisionUpdate(const _float& fTimeDelta)
{
    if (m_Colliders.size() <= 0)
        return;
    
    map<pair<weak_ptr<CCollider>, weak_ptr<CCollider>>, bool, ColliderPairLess> currFrameCollisions;
    
    for (size_t i = 0; i < m_Colliders.size(); ++i)
    {
        for (size_t j = i + 1; j < m_Colliders.size(); ++j)
        {
            auto pA = m_Colliders[i].lock();
            auto pB = m_Colliders[j].lock();
            
            if (!pA || !pB)
                continue;

            if (pA->is_Active() == false || pB->is_Active() == false)
                continue;
    
            auto pOwnerA = pA->Get_Owner().lock();
            auto pOwnerB = pB->Get_Owner().lock();
            
            if (!pOwnerA || !pOwnerB)
                continue;
    
            bool isColliding = pA->Intersect(pB.get());
            auto key = make_pair(weak_ptr<CCollider>(pA), weak_ptr<CCollider>(pB));
            currFrameCollisions[key] = isColliding;
    
            bool wasColliding = false;
            auto prevIt = m_PrevCollisions.find(key);
            if (prevIt != m_PrevCollisions.end())
                wasColliding = prevIt->second;

            if (pA->IsTrigger() && pB->IsTrigger())
                continue;
            
            if (isColliding)
            {
                if (pA->IsTrigger() || pB->IsTrigger())
                {
                    if (!wasColliding)
                    {
                        pOwnerA->OnTriggerEnter(pB);
                        pOwnerB->OnTriggerEnter(pA);
                    }
                    else
                    {
                        pOwnerA->OnTriggerStay(pB);
                        pOwnerB->OnTriggerStay(pA);
                    }
                    continue; // 트리거는 여기서 종료
                }


                if (!wasColliding)
                {
                    // OnTriggerEnter
                    pOwnerA->OnTriggerEnter(pB);
                    pOwnerB->OnTriggerEnter(pA);
                }
                else
                {
                    // OnTriggerStay
                    pOwnerA->OnTriggerStay(pB);
                    pOwnerB->OnTriggerStay(pA);
                }

                // Sphere 충돌 반응
                if (pA->Get_ColliderType() == COLLIDER::TYPE_SPHERE &&
                    pB->Get_ColliderType() == COLLIDER::TYPE_SPHERE)
                {
                    HandleSphereCollisionResponse(pA, pB, fTimeDelta);
                }

            }
            else if (wasColliding)
            {
                pOwnerA->OnTriggerExit(pB);
                pOwnerB->OnTriggerExit(pA);
            }
        }
    }

    m_PrevCollisions = currFrameCollisions;
}

void CCollision_Manager::HandleSphereCollisionResponse(shared_ptr<CCollider> pA, shared_ptr<CCollider> pB, _float fTimeDelta)
{
    if (pA->Get_ColliderType() != COLLIDER::TYPE_SPHERE || pB->Get_ColliderType() != COLLIDER::TYPE_SPHERE)
        return;

    auto pOwnerA = pA->Get_Owner().lock();
    auto pOwnerB = pB->Get_Owner().lock();
    if (!pOwnerA || !pOwnerB)
        return;

    auto pTransformA = pOwnerA->Get_Transform();
    auto pTransformB = pOwnerB->Get_Transform();
    if (!pTransformA || !pTransformB)
        return;

    _vector vPosA = XMVectorSetY(pTransformA->Get_State(CTransform::STATE_POSITION), 0.f);
    _vector vPosB = XMVectorSetY(pTransformB->Get_State(CTransform::STATE_POSITION), 0.f);

    _vector vDir = XMVector3Normalize(vPosB - vPosA);
    _float dist = XMVectorGetX(XMVector3Length(vPosB - vPosA));

    void* pADesc = pA->Get_Bounding()->Get_Desc();
    void* pBDesc = pB->Get_Bounding()->Get_Desc();
    _float rA = static_cast<BoundingSphere*>(pADesc)->Radius;
    _float rB = static_cast<BoundingSphere*>(pBDesc)->Radius;

    _float penetration = rA + rB - dist;
    if (penetration <= 0.f)
        return;

    _float ratioA = rB / (rA + rB);
    _float ratioB = rA / (rA + rB);
    _vector vOffset = XMVectorScale(vDir, penetration * 0.05f);
    _float pushForce = min(penetration * 150.f * fTimeDelta, 5.f);

    auto pRigidA = dynamic_pointer_cast<CRigidbody>(pOwnerA->Find_Component(TEXT("Com_Rigidbody")));
    auto pRigidB = dynamic_pointer_cast<CRigidbody>(pOwnerB->Find_Component(TEXT("Com_Rigidbody")));

    int priorityA = pA->Get_TranslationPriority();
    int priorityB = pB->Get_TranslationPriority();

    // 플레이어보다 우선순위 낮은 객체만 밀 수 있음
    if (priorityA > priorityB)
    {
        // A가 우선순위 낮으므로, A만 밀린다
        if (pRigidA)
        {
            auto pNavA = dynamic_pointer_cast<CNavigation>(pOwnerA->Find_Component(TEXT("Com_Navigation")));
            pRigidA->Add_Impulse(-vOffset * pushForce, pNavA);
        }
    }
    else if (priorityB > priorityA)
    {
        // B가 우선순위 낮으므로, B만 밀린다
        if (pRigidB)
        {
            auto pNavB = dynamic_pointer_cast<CNavigation>(pOwnerB->Find_Component(TEXT("Com_Navigation")));
            pRigidB->Add_Impulse(+vOffset * pushForce, pNavB);
        }
    }
    else
    {
        // 우선순위 같을 경우: 위치만 균등하게 보정 (둘 다 움직이되, 매우 조금씩만)
        _float3 posA, posB;
        XMStoreFloat3(&posA, pTransformA->Get_State(CTransform::STATE_POSITION) - vOffset * 0.1f);
        XMStoreFloat3(&posB, pTransformB->Get_State(CTransform::STATE_POSITION) + vOffset * 0.1f);

        pTransformA->Set_Position(posA);
        pTransformB->Set_Position(posB);
    }
}

bool CCollision_Manager::CanMove_AABB(shared_ptr<CCollider> pMover, const _float3& nextPos)
{
    if (pMover->Get_ColliderType() != COLLIDER::TYPE_AABB)
        return true;

    auto pBounding = dynamic_pointer_cast<CBounding_AABB>(pMover->Get_Bounding());
    if (!pBounding)
        return true;

    // 이동 후 AABB 생성
    _float3 extents = pBounding->Get_Extents();
    BoundingBox movedBox(nextPos, extents);

    for (auto& collider : m_Colliders)
    {
        auto pTarget = collider.lock();
        if (!pTarget || pTarget == pMover)
            continue;

        if (pTarget->Get_ColliderType() != COLLIDER::TYPE_AABB)
            continue;

        auto pTargetAABB = dynamic_pointer_cast<CBounding_AABB>(pTarget->Get_Bounding());
        if (!pTargetAABB)
            continue;

        BoundingBox targetBox = *static_cast<BoundingBox*>(pTargetAABB->Get_Desc());

        if (movedBox.Intersects(targetBox))
            return false; // 충돌 예상됨
    }

    return true; // 이동 가능
}

void CCollision_Manager::AddCollider(shared_ptr<CCollider> pCollider)
{
    if (!pCollider)
        return;

    m_Colliders.push_back(pCollider);
}

void CCollision_Manager::RemoveCollider(shared_ptr<CCollider> pCollider)
{
    if (!pCollider)
        return;

    for (auto iter = m_Colliders.begin(); iter != m_Colliders.end(); ++iter)
    {
        auto locked = iter->lock();
        if (locked == pCollider)
        {
            m_Colliders.erase(iter);
            break;
        }
    }
}

void CCollision_Manager::RemoveAllCollider()
{
    m_Colliders.clear();
}