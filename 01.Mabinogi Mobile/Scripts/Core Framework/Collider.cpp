#include "Collider.h"

#include "Bounding_AABB.h"
#include "Bounding_OBB.h"
#include "Bounding_Sphere.h"
#include "Bounding_Ring.h"

#include "GameInstance.h"


CCollider::CCollider(ComPtr<ID3D11Device> pDevice, ComPtr<ID3D11DeviceContext> pContext)
	: CComponent { pDevice, pContext }
{
}

CCollider::CCollider(const CCollider& Prototype)
	: CComponent{ Prototype }
	, m_eColliderType { Prototype.m_eColliderType } 
#ifdef _DEBUG
	, m_pBatch { Prototype.m_pBatch }
	, m_pEffect { Prototype.m_pEffect }
	, m_pInputLayout { Prototype.m_pInputLayout } 
#endif
{

}

CCollider::~CCollider()
{
}

HRESULT CCollider::Initialize_Prototype(COLLIDER eColliderType)
{
	m_eColliderType = eColliderType;

#ifdef _DEBUG
	m_pBatch = make_shared<PrimitiveBatch<VertexPositionColor>>(m_pContext.Get());
	m_pEffect = make_shared<BasicEffect>(m_pDevice.Get());

	const void* pShaderByteCode = { nullptr };
	size_t	iShaderCodeLength = {};

	m_pEffect->SetVertexColorEnabled(true);

	m_pEffect->GetVertexShaderBytecode(&pShaderByteCode, &iShaderCodeLength);
	
	if (FAILED(m_pDevice->CreateInputLayout(VertexPositionColor::InputElements, VertexPositionColor::InputElementCount, pShaderByteCode, iShaderCodeLength, &m_pInputLayout)))
		return E_FAIL;
#endif

	return S_OK;
}

HRESULT CCollider::Initialize(void* pArg)
{
	CBounding::BOUNDING_DESC* pDesc = static_cast<CBounding::BOUNDING_DESC*>(pArg);

	switch (m_eColliderType)
	{
	case COLLIDER::TYPE_AABB:
		m_pBounding = CBounding_AABB::Create(m_pDevice, m_pContext, pDesc);
		break;
	case COLLIDER::TYPE_OBB:
		m_pBounding = CBounding_OBB::Create(m_pDevice, m_pContext, pDesc);
		break;
	case COLLIDER::TYPE_SPHERE:
		m_pBounding = CBounding_Sphere::Create(m_pDevice, m_pContext, pDesc);	
		break;
    case COLLIDER::TYPE_RING:
		m_pBounding = CBounding_Ring::Create(m_pDevice, m_pContext, pDesc);
		break;
	}

	return S_OK;
}

_bool CCollider::Intersect(const CCollider* pTargetCollider)
{	
	m_isCollision = m_pBounding->Intersect(pTargetCollider->m_eColliderType, pTargetCollider->m_pBounding.get());	

	return m_isCollision;
}

void CCollider::Update(_fmatrix WorldMatrix)
{
    if (m_Active == false)
        return;

	m_pBounding->Update(WorldMatrix);
}

#ifdef _DEBUG
HRESULT CCollider::Render()
{
    if (m_Active == false)
        return S_OK;

	m_pContext->GSSetShader(nullptr, nullptr, 0);

	m_pEffect->SetWorld(XMMatrixIdentity());
	m_pEffect->SetView(m_pGameInstance->Get_Transform_Matrix(CPipeLine::D3DTS_VIEW));
	m_pEffect->SetProjection(m_pGameInstance->Get_Transform_Matrix(CPipeLine::D3DTS_PROJ));
	m_pContext->IASetInputLayout(m_pInputLayout.Get());

	m_pEffect->Apply(m_pContext.Get());

	m_pBounding->Render(m_pBatch.get(), true == m_isCollision ? XMVectorSet(1.f, 0.f, 0.f, 1.f) : XMVectorSet(0.f, 1.f, 0.f, 1.f));

	return S_OK;
}
#endif

void CCollider::Set_RingSize(_float fInner, _float fOuter)
{
    if (m_eColliderType != COLLIDER::TYPE_RING)
        return;

    auto pRing = std::dynamic_pointer_cast<CBounding_Ring>(m_pBounding);
    if (pRing)
    {
        pRing->Set_Radii(fInner, fOuter);
    }
}
void CCollider::Set_RingHeight(_float fHeight)
{
    if (m_eColliderType != COLLIDER::TYPE_RING)
        return;

    auto pRing = std::dynamic_pointer_cast<CBounding_Ring>(m_pBounding);
    if (!pRing)
        return;

    pRing->Set_RingHeight(fHeight);
}

void CCollider::GrowRing(_float fDeltaTime, _float fGrowScale)
{
    if (m_eColliderType != COLLIDER::TYPE_RING)
        return;

    auto pRing = std::dynamic_pointer_cast<CBounding_Ring>(m_pBounding);
    if (!pRing)
        return;

    _float newOuter = pRing->Get_OuterRadius() + fDeltaTime * fGrowScale;
    _float newInner = pRing->Get_InnerRadius() + fDeltaTime * fGrowScale;
	
    pRing->Set_Radii(newInner, newOuter); // inner 고정
}

shared_ptr<CCollider> CCollider::Create(ComPtr<ID3D11Device> pDevice, ComPtr<ID3D11DeviceContext> pContext, COLLIDER eColliderType)
{
	auto pGameInstance = shared_ptr<CCollider>(new CCollider(pDevice, pContext));

	if (FAILED(pGameInstance->Initialize_Prototype(eColliderType)))
	{
		MSG_BOX("Failed to Create : CCollider");
	}

	return pGameInstance;
}


shared_ptr<CComponent> CCollider::Clone(void* pArg)
{
	auto pGameInstance = shared_ptr<CCollider>(new CCollider(*this));

	if (FAILED(pGameInstance->Initialize(pArg)))
	{
		MSG_BOX("Failed to Clone : CCollider");
	}

	return pGameInstance;
}
