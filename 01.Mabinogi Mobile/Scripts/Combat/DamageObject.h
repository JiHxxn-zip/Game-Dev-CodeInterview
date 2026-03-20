#pragma once
#include "GameObject.h"

namespace Engine
{
    class CTriggerCollider;
}

BEGIN(Client)
class CSkillIndicator;
class CSkillIndicator_Manager;

class CDamageObject: public CGameObject
{
public:
    typedef struct DamageObjectDesc : public GAMEOBJECT_DESC
    {
        weak_ptr<CGameObject> pOwner;
        string strSkillName;
        DamageObjectDesc(weak_ptr<CGameObject> owner, string skillName = {}) :pOwner(owner), strSkillName(skillName) {}
    }DAMAGE_OBJECT_DESC;


protected:
    CDamageObject(ComPtr<ID3D11Device> pDevice, ComPtr<ID3D11DeviceContext> pContext);
    CDamageObject(const CDamageObject& Prototype);

public:
    virtual ~CDamageObject() = default;

public:
    virtual HRESULT Initialize_Prototype() override;
    virtual HRESULT Initialize(void* pArg) override;
    virtual void	Priority_Update(_float fTimeDelta) override;
    virtual void	Update(_float fTimeDelta) override;
    virtual void	Last_Update(_float fTimeDelta) override;
    virtual HRESULT Render() override;
    virtual HRESULT Render_Shadow() override;

    virtual void Active() override;
    virtual void Active_Skill() = 0;

public:
    /* ======== Getter & Setter ======== */

protected:
    /* ==== Data & Resource Setting ==== */
    virtual HRESULT Ready_Components();
    _bool Check_PlayerInRange();

protected:
    /*==== Component ====*/
    shared_ptr<CTriggerCollider>	    m_pHitColliderCom = { nullptr };

    /*===== Object =====*/
    // 스킬 주체
    weak_ptr<CGameObject> m_pOwner;
    string m_strSkillName = {};

    // 인디케이터
    CSkillIndicator_Manager* m_pIndicatorManager = { nullptr };
    shared_ptr<CSkillIndicator> m_pSkillIndicator = { nullptr };

    /*======= Value =======*/
    _float m_fReadyTimer = {};
    _float m_fReadyDuration = {};

    _bool   m_IsSkillActive = { false };

    _float m_fRange = { 1.f };

    /*======= Shader Value =======*/
    _float4             m_fAttributes = {};

public:
    virtual shared_ptr<CGameObject> Clone(void* pArg) = 0;
};

END