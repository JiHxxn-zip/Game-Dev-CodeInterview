#pragma once
#include "DamageObject.h"
#include "Glasgavelen.h"

// attack => 직사각형 장판 생성 -> 휘두르는 칼날 2번 공격

namespace Engine
{
    class CEffectPresetObject;
}

BEGIN(Client)
class CGlasgavelenDefaultAttack : public CDamageObject
{
private:
    CGlasgavelenDefaultAttack(ComPtr<ID3D11Device> pDevice, ComPtr<ID3D11DeviceContext> pContext);
    CGlasgavelenDefaultAttack(const CDamageObject& Prototype);

public:
    virtual ~CGlasgavelenDefaultAttack() = default;

public:
    virtual HRESULT Initialize_Prototype() override;
    virtual HRESULT Initialize(void* pArg) override;
    virtual void	Priority_Update(_float fTimeDelta) override;
    virtual void	Update(_float fTimeDelta) override;
    virtual void	Last_Update(_float fTimeDelta) override;

    virtual void    Active_Skill() override;

private:
    virtual void    OnHit(shared_ptr<CGameObject> pTarget) override;

public:
    void Spawn(_fvector spawnPos);

private:
    /* ==== Data & Resource Setting ==== */
    HRESULT Ready_Components();

private:    
    /* ==== Actor ==== */
    weak_ptr<class CGlasgavelen> m_pGlasgavelen = {};

    /* ==== Component ==== */
    shared_ptr<CRigidbody> m_pRigidBody = { nullptr };

    weak_ptr<CGameObject> m_Effect;
    _float4x4 m_EffectOffset = {};

    _float m_Length = {};
public:
    static shared_ptr<CGlasgavelenDefaultAttack> Create(ComPtr<ID3D11Device> pDevice, ComPtr<ID3D11DeviceContext> pContext);
    virtual shared_ptr<CGameObject> Clone(void* pArg);
};

END