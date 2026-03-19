#include "GlasgavelenDefaultAttack.h"

#include <GameInstance.h>

#include "Client_Defines.h"
#include "Effect_Manager.h"
#include "PlayerCamera.h"
#include "TriggerCollider.h"
#include "SkillIndicator_Manager.h"
#include "Sound_Manager.h"

Client::CGlasgavelenDefaultAttack::CGlasgavelenDefaultAttack(ComPtr<ID3D11Device> pDevice, ComPtr<ID3D11DeviceContext> pContext)
    :CDamageObject{ pDevice, pContext }
{
}

Client::CGlasgavelenDefaultAttack::CGlasgavelenDefaultAttack(const CDamageObject& Prototype)
    : CDamageObject{ Prototype }
{
}

HRESULT Client::CGlasgavelenDefaultAttack::Initialize_Prototype()
{
    return S_OK;
}

HRESULT Client::CGlasgavelenDefaultAttack::Initialize(void* pArg)
{
    if (FAILED(CDamageObject::Initialize(pArg)))
        return E_FAIL;

    if (FAILED(Ready_Components()))
        return E_FAIL;

    /* ========== Member Data Setting ========== */
    m_fReadyDuration = 1.2f;
    m_IsSkillActive = false;
    m_fReadyTimer = 0;
    m_Length = 10.f;

    m_pGlasgavelen = dynamic_pointer_cast<CGlasgavelen>(m_pOwner.lock());

    m_pRigidBody = dynamic_pointer_cast<CRigidbody>(m_pGlasgavelen.lock()->Find_Component(L"Com_Rigidbody"));
    if (nullptr == m_pRigidBody)
        return E_FAIL;

    return S_OK;
}

void Client::CGlasgavelenDefaultAttack::Priority_Update(_float fTimeDelta)
{
    if (Is_Active() == false)
        return;

    CDamageObject::Priority_Update(fTimeDelta);
}

void Client::CGlasgavelenDefaultAttack::Update(_float fTimeDelta)
{
    if (!Is_Active())
        return;

    CDamageObject::Update(fTimeDelta);
}

void Client::CGlasgavelenDefaultAttack::Last_Update(_float fTimeDelta)
{
    if (Is_Active() == false)
        return;

    CDamageObject::Last_Update(fTimeDelta);

    m_fAttributes.y -= fTimeDelta * 0.1f; // 0.1초만에 0.1짜리 Outline이 사라지기

    m_pHitColliderCom->Update(XMLoadFloat4x4(m_pTransformCom->Get_WorldMatrix_Ptr()));
#ifdef _DEBUG
    m_pGameInstance->Add_RG_DebugComponent(m_pHitColliderCom);
#endif
}

void Client::CGlasgavelenDefaultAttack::Active_Skill()
{
    // TriggerCollider 등록
    auto pTrigger = dynamic_pointer_cast<CTriggerCollider>(m_pHitColliderCom);
    if (pTrigger)
    {
        AttackerInfo attackerInfo;
        attackerInfo.pTriggerCollider = pTrigger;
        attackerInfo.bIsAttacking = true;
        attackerInfo.fAttackDuration = 2.5f; // 유지시간

        m_pGameInstance->Start_Attack(attackerInfo);
    }

    static_pointer_cast<CPlayerCamera>(m_pGameInstance->Find_Camera("PLAYER_CAM"))->Active_Vibration(1.0f, 3.0f, 3.0f, 0.0f);
}

void CGlasgavelenDefaultAttack::OnHit(shared_ptr<CGameObject> pTarget)
{
    // 플레이어인지 확인
    if (pTarget->Get_GameObjectTag() != TEXT("GameObject_Player"))
        return;

    // TriggerCollider가 있다면
    auto pTrigger = std::dynamic_pointer_cast<CTriggerCollider>(m_pHitColliderCom);
    if (!pTrigger)
        return;

    // Damage Info
    _float damage = 100.f;

    DamageInfo damageInfo(m_pOwner.lock(), static_cast<_int>(damage));
    m_pGameInstance->Process_Hit(m_pHitColliderCom, pTarget, damageInfo);
}

void CGlasgavelenDefaultAttack::Spawn(_fvector spawnPos)
{
    m_pTransformCom->Set_State(CTransform::STATE_POSITION, spawnPos);
    m_pIndicatorManager->Request(CSkillIndicator::SHAPE_RECTANGLE_FORWARD, { 5.f, m_Length }, m_fReadyDuration, m_pOwner.lock()->Get_Transform()->Get_WorldMatrix_Ptr());

    m_fReadyTimer = 0;
    m_IsSkillActive = false; 

    Set_Active(true);

    CSound_Manager::GetInstance()->PlaySound(L"SFX_Monster_Skill_Indicator.wav", 0.5f);
}

HRESULT Client::CGlasgavelenDefaultAttack::Ready_Components()
{
    /* Hit Collider */
    CBounding_OBB::BOUNDING_OBB_DESC		CoilliderDesc{};
    CoilliderDesc.vRotation = { 0.f,0.f,0.f };
    CoilliderDesc.vExtents = { 5.f, 1.f, 10.f };
    CoilliderDesc.vCenter = _float3(0.f, CoilliderDesc.vExtents.y * 0.5f, 10.f);

    if (auto component = CGameObject::Add_Component(LEVEL_STATIC, TEXT("Prototype_Component_TriggerCollider_OBB"), TEXT("Com_AttackTriggerCollider"), &CoilliderDesc))
        m_pHitColliderCom = static_pointer_cast<CTriggerCollider>(component);

    auto self = dynamic_pointer_cast<CGameObject>(shared_from_this());
    m_pHitColliderCom->Set_Owner(self);

    return S_OK;
}

shared_ptr<Client::CGlasgavelenDefaultAttack> Client::CGlasgavelenDefaultAttack::Create(ComPtr<ID3D11Device> pDevice,
    ComPtr<ID3D11DeviceContext> pContext)
{
    shared_ptr<CGlasgavelenDefaultAttack> pGameInstance = shared_ptr<CGlasgavelenDefaultAttack>(new CGlasgavelenDefaultAttack(pDevice, pContext));

    if (FAILED(pGameInstance->Initialize_Prototype()))
    {
        MSG_BOX("Failed to Create : CMorrighanAbsorbField");
    }

    return pGameInstance;
}

shared_ptr<CGameObject> Client::CGlasgavelenDefaultAttack::Clone(void* pArg)
{
    auto pGameInstance = shared_ptr<CGlasgavelenDefaultAttack>(new CGlasgavelenDefaultAttack(*this));
    if (FAILED(pGameInstance->Initialize(pArg)))
    {
        MSG_BOX("Failed to Clone : CMorrighanAbsorbField");
    }

    return pGameInstance;
}
