#pragma once
#include "Collider.h"

BEGIN(Engine)

class ENGINE_DLL CPhysicsCollider final : public CCollider
{
public:
    CPhysicsCollider(ComPtr<ID3D11Device> pDevice, ComPtr<ID3D11DeviceContext> pContext);
    CPhysicsCollider(const CPhysicsCollider& rhs);
    virtual ~CPhysicsCollider() = default;

public:
    virtual HRESULT Initialize_Prototype(COLLIDER eColliderType) override;
    virtual HRESULT Initialize(void* pArg) override;
    virtual void Update(_fmatrix WorldMatrix) override;

#ifdef _DEBUG
public:
    virtual HRESULT Render();
#endif

public:
    static shared_ptr<CPhysicsCollider> Create(ComPtr<ID3D11Device> pDevice, ComPtr<ID3D11DeviceContext> pContext, COLLIDER eColliderType);
    virtual shared_ptr<CComponent> Clone(void* pArg) override;
};

END 