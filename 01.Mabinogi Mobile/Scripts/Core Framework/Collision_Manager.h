#pragma once
#include "Engine_Defines.h"
#include "Collider.h"
#include <map>

BEGIN(Engine)

// weak_ptr<CCollider>에 대한 사용자 정의 비교자
struct ColliderWeakPtrLess
{
    bool operator()(const weak_ptr<CCollider>& lhs, const weak_ptr<CCollider>& rhs) const
    {
        return lhs.owner_before(rhs);
    }
};

// pair<weak_ptr<CCollider>, weak_ptr<CCollider>>에 대한 사용자 정의 비교자
struct ColliderPairLess
{
    bool operator()(const pair<weak_ptr<CCollider>, weak_ptr<CCollider>>& lhs,
        const pair<weak_ptr<CCollider>, weak_ptr<CCollider>>& rhs) const
    {
        // 첫 번째 weak_ptr를 비교하고, 같으면 두 번째 weak_ptr를 비교
        if (lhs.first.owner_before(rhs.first)) return true;
        if (rhs.first.owner_before(lhs.first)) return false;
        return lhs.second.owner_before(rhs.second);
    }
};

class CCollision_Manager final
{
private:
    CCollision_Manager() = default;
public:
    ~CCollision_Manager();
    static shared_ptr<CCollision_Manager> Create();

public:
    void CollisionUpdate(const _float& fTimeDelta);

public:
    vector<weak_ptr<CCollider>>& Get_ColliderList() { return m_Colliders; }

public:
    void HandleSphereCollisionResponse(shared_ptr<CCollider> pA, shared_ptr<CCollider> pB, _float fTimeDelta);
    bool CanMove_AABB(shared_ptr<CCollider> pMover, const _float3& nextPos);
    void AddCollider(shared_ptr<CCollider> pCollider);
    void RemoveCollider(shared_ptr<CCollider> pCollider);
    void RemoveAllCollider();

private:
    vector<weak_ptr<CCollider>> m_Colliders;
    map<pair<weak_ptr<CCollider>, weak_ptr<CCollider>>, bool, ColliderPairLess> m_PrevCollisions;
};

END
