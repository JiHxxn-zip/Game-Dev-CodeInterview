#pragma once
#include "Component.h"


BEGIN(Engine)

class ENGINE_DLL CRigidbody final : public CComponent
{
public:
    typedef struct tagRigidbodyDesc
    {
        shared_ptr<class CTransform> pTransform;
        _float mass = 1.0f;
        _bool useGravity = true;
        _float gravityScale = 1.0f;
        _float drag = 5.0f;
        _float maxSpeed = 20.f;
    }RIGIDBODY_DESC;

private:
    CRigidbody(ComPtr<ID3D11Device> pDevice, ComPtr<ID3D11DeviceContext> pContext);
public:
    virtual ~CRigidbody() = default;

public:
    virtual HRESULT Initialize_Prototype() override;
    virtual HRESULT Initialize(void* pArg);
    void Update(_float fTimeDelta, shared_ptr<class CNavigation> pNavigation = nullptr);

public:
    void Add_Force(_fvector vForce);
    void Set_UseGravity(_bool bUse) { m_bUseGravity = bUse; }
    void Set_Damping(_float fDamping) { m_fVelocityDamping = fDamping; }
public:
    void    Set_Velocity(_fvector vVelocity) { m_vVelocity = vVelocity; }
    _vector Get_Velocity() { return m_vVelocity; }

public:
    void Update_Velocity(_float fTimeDelta, class shared_ptr<CNavigation> pNavigation = nullptr);
    void Add_Impulse(_fvector vImpulse, class shared_ptr<CNavigation> pNavigation = nullptr);
    void Apply_Drag(_float fDragForce, _float fTimeDelta);
    void Apply_DragWithoutY(_float fDragForce, _float fTimeDelta);
    void Apply_Gravity(_float fGravity, _float fTimeDelta);

private:
    _vector m_vVelocity = XMVectorZero();
    _vector m_vExternalForce = XMVectorZero();

    _bool m_bUseGravity = { false };
    _float m_fGravityScale = { 1.f };
    _float m_fMass = { 1.f };
    _float m_fDrag = { 5.f };
    _float m_fMaxSpeed = { 20.f };

    shared_ptr<class CTransform> m_pTransform = { nullptr };

    _float m_fVelocityDamping = { 1.f }; // 1.0이면 감쇠 없음

public:
    static shared_ptr<CRigidbody> Create(ComPtr<ID3D11Device> pDevice, ComPtr<ID3D11DeviceContext> pContext);
    virtual shared_ptr<CComponent> Clone(void* pArg) override;
};

END
