#include "DamageObject.h"

#include "GameInstance.h"
#include "Game_Manager.h"
#include "SkillIndicator.h"
#include "SkillIndicator_Manager.h"

Client::CDamageObject::CDamageObject(ComPtr<ID3D11Device> pDevice, ComPtr<ID3D11DeviceContext> pContext)
    :CGameObject{pDevice, pContext}
{
}

Client::CDamageObject::CDamageObject(const CDamageObject& Prototype)
    :CGameObject{Prototype}
{
}

HRESULT Client::CDamageObject::Initialize_Prototype()
{

    return S_OK;
}

HRESULT Client::CDamageObject::Initialize(void* pArg)
{
    DAMAGE_OBJECT_DESC* desc = static_cast<DAMAGE_OBJECT_DESC*>(pArg);
    if (desc == nullptr)
        return E_FAIL;

    m_pOwner = desc->pOwner;
    m_strSkillName = desc->strSkillName;
    m_pIndicatorManager = CSkillIndicator_Manager::GetInstance();

    GAMEOBJECT_DESC objectDesc = {};
    objectDesc.pGameObjectTag = L"GameObject_DamageObject";
    objectDesc.fSpeedPerSec = 1.f;
    objectDesc.fRotationPerSec = 1.f;

    if (FAILED(CGameObject::Initialize(&objectDesc)))
        return E_FAIL;

    if (FAILED(Ready_Components()))
        return E_FAIL;

    Set_Active(false);

    return S_OK;
}

void Client::CDamageObject::Priority_Update(_float fTimeDelta)
{

}

void Client::CDamageObject::Update(_float fTimeDelta)
{
    m_fReadyTimer += fTimeDelta;

    if (m_IsSkillActive == false && m_fReadyTimer >= m_fReadyDuration)
    {
        Active_Skill();
        m_IsSkillActive = true;
    }

    if (m_pSkillIndicator)
    {
        //m_pSkillIndicator->Update_Ratio(m_fReadyTimer / m_fReadyDuration);
    }
}

void Client::CDamageObject::Last_Update(_float fTimeDelta)
{

}

HRESULT Client::CDamageObject::Render()
{
    return S_OK;
}

HRESULT Client::CDamageObject::Render_Shadow()
{
    return S_OK;

}

void Client::CDamageObject::Active()
{
    m_IsSkillActive = false;
    m_fReadyTimer = 0.f;
}

HRESULT Client::CDamageObject::Ready_Components()
{
    return S_OK;
}

_bool CDamageObject::Check_PlayerInRange()
{
    auto pPlayer = CGame_Manager::GetInstance()->Get_Player();
    if (pPlayer == nullptr)
        return false;

    _float distance = m_pTransformCom->Compute_DistanceTo(pPlayer->Get_Transform());

    if (distance < m_fRange)
        return true;

    return false;
}

