#pragma once

#include "Component.h"
#include "Bounding.h"
#include "GameObject.h"

BEGIN(Engine)

class ENGINE_DLL CCollider : public CComponent
{
protected:
	CCollider(ComPtr<ID3D11Device> pDevice, ComPtr<ID3D11DeviceContext> pContext);
public:
	CCollider(const CCollider& Prototype);
	virtual ~CCollider();

public:
	virtual HRESULT Initialize_Prototype(COLLIDER eColliderType);
	virtual HRESULT Initialize(void* pArg);
public:
	_bool           Intersect(const CCollider* pTargetCollider);
    virtual void    Update(_fmatrix WorldMatrix);

#ifdef _DEBUG
public:
	virtual HRESULT Render();
#endif

public:
    void Set_RingHeight(_float fHeight);
    void GrowRing(_float fDeltaTime, _float fGrowScale = 3.5f);
    void Set_RingSize(_float fInner, _float fOuter);

    _float3 Get_Center() { return m_pBounding->Get_Center(); }
public:
    shared_ptr<CBounding>   Get_WorldBoundingBox() { return m_pBounding; }

public:
    class shared_ptr<CBounding> Get_Bounding() { return m_pBounding; }
    COLLIDER			    Get_ColliderType() { return m_eColliderType; }

public:
    weak_ptr<CGameObject>   Get_Owner() const { return m_pOwner; }
    void				    Set_Owner(shared_ptr<CGameObject> pOwner) { m_pOwner = pOwner; }

public:
    void                    Set_Active(_bool active) { m_Active = active; }
    _bool                   is_Active() { return m_Active; }

public:
    _int Get_TranslationPriority() { return static_cast<_int>(m_eTranslateType); }
    void Set_TranslationType(COLLISION_TRANSLATE_TYPE eType) {  m_eTranslateType = eType; }


public:
    void Set_IsTrigger(bool bTrigger) { m_bIsTrigger = bTrigger; }
    bool IsTrigger() const { return m_bIsTrigger; }

protected:
	COLLIDER				m_eColliderType = { COLLIDER::TYPE_END };
	shared_ptr<CBounding>	m_pBounding = { nullptr };
	_bool					m_isCollision = { false };
    weak_ptr<CGameObject>   m_pOwner;
    COLLISION_TRANSLATE_TYPE m_eTranslateType = COLLISION_TRANSLATE_TYPE::TYPE_STATIC;

    _bool                   m_Active = { true };
    _float                  m_fGrowScale = { };

    _bool m_bIsTrigger = { false };
    
#ifdef _DEBUG
private:
	shared_ptr<PrimitiveBatch<VertexPositionColor>>	m_pBatch = { nullptr };
	shared_ptr<BasicEffect>							m_pEffect = { nullptr };
	ComPtr<ID3D11InputLayout>						m_pInputLayout = { nullptr };

#endif

public:
    static shared_ptr<CCollider> Create(ComPtr<ID3D11Device> pDevice, ComPtr<ID3D11DeviceContext> pContext, COLLIDER eColliderType);
	virtual shared_ptr<CComponent> Clone(void* pArg) override;

};

END
