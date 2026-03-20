#pragma once
#include "Collider.h"

BEGIN(Engine)

class ENGINE_DLL CTriggerCollider : public CCollider
{
public:
    CTriggerCollider(ComPtr<ID3D11Device> pDevice, ComPtr<ID3D11DeviceContext> pContext);
    CTriggerCollider(const CTriggerCollider& Prototype);
    virtual ~CTriggerCollider() = default;

public:
    virtual HRESULT Initialize_Prototype(COLLIDER eColliderType) override;
    virtual HRESULT Initialize(void* pArg) override;
    virtual void Update(_fmatrix WorldMatrix) override;

public:
    // 트리거 콜라이더 특화 기능
    void Set_AttackRange(_float3 vExtents) { m_vAttackRange = vExtents; }
    void Set_AttackOffset(_float3 vOffset) { m_vAttackOffset = vOffset; }
    void Set_AttackAngle(_float fAngle) { m_fAttackAngle = fAngle; }

    // 공격 판정 관련
    void Set_AttackActive(_bool bActive) { m_bIsAttackActive = bActive; }
    _bool Is_AttackActive() const { return m_bIsAttackActive; }

    // 공격력 설정
    void Set_AttackPower(_float fPower) { m_fAttackPower = fPower; }
    _float Get_AttackPower() const { return m_fAttackPower; }

private:
    _float3     m_vAttackRange = { _float3(1.0f, 1.0f, 1.0f) };     // 공격 범위
    _float3     m_vAttackOffset = { _float3(0.0f, 0.0f, 0.0f) };    // 공격 위치 오프셋
    _float      m_fAttackAngle = {};                                // 공격 각도
    _bool       m_bIsAttackActive = { false };                      // 공격 활성화 여부
    _float      m_fAttackPower = { 10.0f };                         // 공격력

public:
    static shared_ptr<CTriggerCollider> Create(ComPtr<ID3D11Device> pDevice,
        ComPtr<ID3D11DeviceContext> pContext, COLLIDER eColliderType);
    virtual shared_ptr<CComponent> Clone(void* pArg) override;
};

END
