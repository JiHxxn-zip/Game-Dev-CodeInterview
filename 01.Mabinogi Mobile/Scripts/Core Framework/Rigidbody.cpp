#include "Rigidbody.h"
#include "Transform.h"
#include "Navigation.h"

CRigidbody::CRigidbody(ComPtr<ID3D11Device> pDevice, ComPtr<ID3D11DeviceContext> pContext)
    : CComponent{ pDevice, pContext }
{
}

HRESULT CRigidbody::Initialize_Prototype()
{
    return S_OK;
}

HRESULT CRigidbody::Initialize(void* pArg)
{
    if (nullptr == pArg)
        return S_OK;

    RIGIDBODY_DESC* pDesc = static_cast<RIGIDBODY_DESC*>(pArg);
    if (nullptr == pDesc)
        return E_FAIL;

    m_pTransform = pDesc->pTransform;
    m_fMass = pDesc->mass;
    m_bUseGravity = pDesc->useGravity;
    m_fGravityScale = pDesc->gravityScale;
    m_fDrag = pDesc->drag;
    m_fMaxSpeed = pDesc->maxSpeed;

    return S_OK;
}

void CRigidbody::Update(_float fTimeDelta, shared_ptr<CNavigation> pNavigation)
{
    // 중력 적용
    if (m_bUseGravity)
    {
        _vector vGravity = XMVectorSet(0.f, -9.8f * m_fGravityScale, 0.f, 0.f);
        m_vVelocity += vGravity * fTimeDelta;
    }

    // 외부 힘 적용
    m_vVelocity += m_vExternalForce;
    m_vExternalForce = XMVectorZero(); // 한 번만 적용

    // 속도 제한
    _float speed = XMVectorGetX(XMVector3Length(m_vVelocity));
    if (speed > m_fMaxSpeed)
        m_vVelocity = XMVector3Normalize(m_vVelocity) * m_fMaxSpeed;

    // 감속 (x, z만)
    //_vector vHorizontal = XMVectorSet(m_vVelocity.m128_f32[0], 0.f, m_vVelocity.m128_f32[2], 0.f);
    //_float fHorizontalSpeed = XMVectorGetX(XMVector3Length(vHorizontal));
    //if (fHorizontalSpeed > 0.f)
    //{
    //    _float newSpeed = max(0.f, fHorizontalSpeed - m_fDrag * fTimeDelta);
    //    _vector dir = XMVector3Normalize(vHorizontal);
    //    vHorizontal = dir * newSpeed;
    //}

    //_float y = m_vVelocity.m128_f32[1];
    //m_vVelocity = XMVectorSet(XMVectorGetX(vHorizontal), y, XMVectorGetZ(vHorizontal), 0.f);

    // 위치 반영
    _vector vPos = m_pTransform->Get_State(CTransform::STATE_POSITION);
    _vector vOldPosition = vPos;
    vPos += m_vVelocity * fTimeDelta;

    if (pNavigation != nullptr)
    {
        if (pNavigation->isMove(vPos))
            m_pTransform->Set_State(CTransform::STATE_POSITION, vPos);
        else
        {
            if (pNavigation->GetCurrentCellIndex() != -1)
            {
                if (!pNavigation->NeedRefresh())
                {
                    vPos = pNavigation->Reflection(pNavigation->GetCurrentCellIndex(), vPos, vOldPosition);
                    int tryCount = 0;
                    while (!pNavigation->isMove(vPos) && tryCount < 10)
                    {
                        vPos = pNavigation->Reflection(pNavigation->GetCurrentCellIndex(), vPos, vOldPosition);
                        tryCount++;
                    }
                    if (tryCount != 10)
                        m_pTransform->Set_State(CTransform::STATE_POSITION, vPos);
                }
            }
        }
    }
}

void CRigidbody::Add_Force(_fvector vForce)
{
    m_vExternalForce += vForce;
}

void CRigidbody::Update_Velocity(_float fTimeDelta, shared_ptr<CNavigation> pNavigation)
{
    if (XMVector3LengthSq(m_vVelocity).m128_f32[0] <= 0.00001f)
        return;

    _vector vPosition = m_pTransform->Get_State(CTransform::STATE_POSITION);
    vPosition += m_vVelocity * fTimeDelta;
    m_vVelocity *= m_fVelocityDamping;

    if (pNavigation == nullptr || pNavigation->isMove(vPosition))
        m_pTransform->Set_State(CTransform::STATE_POSITION, vPosition);
}

void CRigidbody::Add_Impulse(_fvector vImpulse, class shared_ptr<CNavigation> pNavigation)
{
    if (XMVector3LengthSq(vImpulse).m128_f32[0] <= 0.00001f)
        return;

    _vector vPosition = m_pTransform->Get_State(CTransform::STATE_POSITION);
    vPosition += vImpulse;

    if (nullptr == pNavigation || pNavigation->isMove(vPosition))
        m_pTransform->Set_State(CTransform::STATE_POSITION, vPosition);
}

void CRigidbody::Apply_Drag(_float fDragForce, _float fTimeDelta)
{
    if (XMVector3LengthSq(m_vVelocity).m128_f32[0] <= 0.00001f)
        return;

    // x, z 감속
    _vector vHorizontalVelocity = XMVectorSet(m_vVelocity.m128_f32[0], 0.f, m_vVelocity.m128_f32[2], 0.f);
    _float fHorizontalSpeed = XMVectorGetX(XMVector3Length(vHorizontalVelocity));

    if (fHorizontalSpeed > 0.f)
    {
        _float fNewHorizontalSpeed = max(0.f, fHorizontalSpeed - fDragForce * fTimeDelta);
        _vector vDir = XMVector3Normalize(vHorizontalVelocity);
        vHorizontalVelocity = vDir * fNewHorizontalSpeed;
    }

    // y 감속
    _float fNewY = m_vVelocity.m128_f32[1];
    if (fabsf(fNewY) > 0.0001f)
    {
        if (fNewY > 0)
            fNewY = max(0.f, fNewY - fDragForce * fTimeDelta);
        else
            fNewY = min(0.f, fNewY + fDragForce * fTimeDelta);
    }

    // 최종 속도 갱신
    m_vVelocity = XMVectorSet(
        XMVectorGetX(vHorizontalVelocity),
        fNewY,
        XMVectorGetZ(vHorizontalVelocity),
        0.f
    );
}

void CRigidbody::Apply_DragWithoutY(_float fDragForce, _float fTimeDelta)
{
    if (XMVector3LengthSq(m_vVelocity).m128_f32[0] <= 0.00001f)
        return;

    // x, z 감속
    _vector vHorizontalVelocity = XMVectorSet(m_vVelocity.m128_f32[0], 0.f, m_vVelocity.m128_f32[2], 0.f);
    _float fHorizontalSpeed = XMVectorGetX(XMVector3Length(vHorizontalVelocity));

    if (fHorizontalSpeed > 0.f)
    {
        _float fNewHorizontalSpeed = max(0.f, fHorizontalSpeed - fDragForce * fTimeDelta);
        _vector vDir = XMVector3Normalize(vHorizontalVelocity);
        vHorizontalVelocity = vDir * fNewHorizontalSpeed;
    }

    // 최종 속도 갱신
    m_vVelocity = XMVectorSet(
        XMVectorGetX(vHorizontalVelocity),
        m_vVelocity.m128_f32[1],
        XMVectorGetZ(vHorizontalVelocity),
        0.f
    );
}

void CRigidbody::Apply_Gravity(_float fGravity, _float fTimeDelta)
{
    m_vVelocity.m128_f32[1] -= fGravity * fTimeDelta;
}

shared_ptr<CRigidbody> CRigidbody::Create(ComPtr<ID3D11Device> pDevice, ComPtr<ID3D11DeviceContext> pContext)
{
    auto pGameInstance = shared_ptr<CRigidbody>(new CRigidbody(pDevice, pContext));

    if (FAILED(pGameInstance->Initialize_Prototype()))
    {
        MSG_BOX("Failed to Create : CRigidbody");
    }

    return pGameInstance;
}

shared_ptr<CComponent> CRigidbody::Clone(void* pArg)
{
    auto pGameInstance = shared_ptr<CRigidbody>(new CRigidbody(*this));

    if (FAILED(pGameInstance->Initialize(pArg)))
    {
        MSG_BOX("Failed to Clone : CRigidbody");
    }

    return pGameInstance;
}