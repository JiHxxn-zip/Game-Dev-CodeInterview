#include "GameObject.h"
#include "GameInstance.h"
#include "Transform.h"
#include "Shader.h"

_uint CGameObject::g_guid = 0;

CGameObject::CGameObject()
{
    m_guid = ++g_guid;
    m_pGameInstance = CGameInstance::GetInstance();
    m_pContext = m_pGameInstance->Get_Context();
    m_pDevice = m_pGameInstance->Get_Device();
}

CGameObject::CGameObject(ComPtr<ID3D11Device> pDevice, ComPtr<ID3D11DeviceContext> pContext)
	: m_pDevice { pDevice }
	, m_pContext { pContext }
	, m_pGameInstance { CGameInstance::GetInstance() }
{
    m_guid = ++g_guid;
}

CGameObject::CGameObject(const CGameObject& Prototype)
	: m_pDevice{ Prototype.m_pDevice }
	, m_pContext{ Prototype.m_pContext }
	, m_pGameInstance{ Prototype.m_pGameInstance }
{
    m_guid = ++g_guid;
}

HRESULT CGameObject::Initialize_Prototype()
{

	return S_OK;
}

HRESULT CGameObject::Initialize(void* pArg)
{
	if (nullptr != Find_Component(g_strTransformTag))
		return E_FAIL;

	m_pTransformCom = CTransform::Create(m_pDevice, m_pContext);
	if (nullptr == m_pTransformCom)
		return E_FAIL;

	m_Components.emplace(g_strTransformTag, m_pTransformCom);

	if (FAILED(m_pTransformCom->Initialize(pArg)))
		return E_FAIL;

    if (nullptr != pArg)
    {
        GAMEOBJECT_DESC* pDesc = static_cast<GAMEOBJECT_DESC*>(pArg);

        if (pDesc->pGameObjectTag != nullptr)
            lstrcpy(m_szGameObjectTag, pDesc->pGameObjectTag);
        else
            lstrcpy(m_szGameObjectTag, L"Unnamed"); // 디폴트로 Unnamed 처리
    }
    else
    {
        lstrcpy(m_szGameObjectTag, L"Unnamed");
    }

    m_viewport = m_pGameInstance->Get_Viewport();

 	return S_OK;
}

void CGameObject::Priority_Update(_float fTimeDelta)
{
}

void CGameObject::Update(_float fTimeDelta)
{
}

void CGameObject::Last_Update(_float fTimeDelta)
{
}

HRESULT CGameObject::Render()
{
    return S_OK;
}

HRESULT CGameObject::Render_GlowMask()
{
    return S_OK;
}

_vector CGameObject::Get_Position()
{
    return m_pTransformCom->Get_State(CTransform::STATE_POSITION);
}

shared_ptr<CComponent> CGameObject::Find_Component(const _wstring& strComponentTag)
{
	auto	iter = m_Components.find(strComponentTag);

	if(iter == m_Components.end())
		return nullptr;

	return iter->second;
}

void CGameObject::Set_Active(_bool bActive)
{
    if (m_bActive == bActive)
        return;

    m_bActive = bActive;

    if (m_bActive)
        Active();
    else
        Deactive();
}

void CGameObject::Set_Visible(_bool bVisible)
{
    m_bVisible = bVisible;
}

shared_ptr<CComponent> CGameObject::Add_Component(_uint iPrototypeLevelIndex, const _wstring& strPrototypeTag, const _wstring& strComponentTag, void* pArg)
{
	if (nullptr != Find_Component(strComponentTag))
		return nullptr;

	shared_ptr<CComponent> pComponent = dynamic_pointer_cast<CComponent>(m_pGameInstance->Clone_Prototype(PROTOTYPE::PROTOTYPE_COMPONENT, iPrototypeLevelIndex, strPrototypeTag, pArg));
	if (nullptr == pComponent)
		return nullptr;

	m_Components.emplace(strComponentTag, pComponent);

	return pComponent;
}

_bool CGameObject::IsInView(_float marginPx, _float maxDistance /*= 20.f*/) const
{
    if (!m_pTransformCom)
        return false;

    // 뷰/프로젝션 행렬은 GameInstance에서 그대로 가져옴
    _matrix viewMat = XMLoadFloat4x4(m_pGameInstance->Get_Transform_Float4x4(CPipeLine::D3DTS_VIEW));
    _matrix projMat = XMLoadFloat4x4(m_pGameInstance->Get_Transform_Float4x4(CPipeLine::D3DTS_PROJ));

    // 월드 위치
    _vector worldPos = m_pTransformCom->Get_State(CTransform::STATE_POSITION);

    // 거리 컬링: 카메라 위치(뷰 행렬 역행렬의 W 요소)
    _matrix invView = XMMatrixInverse(nullptr, viewMat);
    _vector camPos = invView.r[3];
    
    _float dist = XMVectorGetX(XMVector3Length(worldPos - camPos));
    if (dist > maxDistance)
        return false;

    // 프러스텀 컬링: 클립 공간 투영
    _vector clip = XMVector4Transform(worldPos, viewMat * projMat);
    _float  w = XMVectorGetW(clip);
    if (w <= 0.f)  // 카메라 뒤쪽
        return false;

    _vector ndc = clip / w;
    _float nx = XMVectorGetX(ndc);
    _float ny = XMVectorGetY(ndc);
    _float nz = XMVectorGetZ(ndc);
    if (nx < -1.f || nx > 1.f ||
        ny < -1.f || ny > 1.f ||
        nz < 0.f || nz > 1.f)
    {
        return false;
    }

    // 스크린-스페이스 컬링
    _float fWidth = _float(m_viewport.Width);
    _float fHeight = _float(m_viewport.Height);
    _float2 fScreenPos = m_pTransformCom->World_To_Screen(viewMat, projMat, fWidth, fHeight);

    if (fScreenPos.x < -marginPx || fScreenPos.x > fWidth + marginPx ||
        fScreenPos.y < -marginPx || fScreenPos.y > fHeight + marginPx)
    {
        return false;
    }

    return true;
}


_bool CGameObject::IsInFrustum(_float marginPx) const
{
    if (!m_pTransformCom)
        return false;

    // 뷰/프로젝션 행렬
    _matrix viewMat = XMLoadFloat4x4(m_pGameInstance->Get_Transform_Float4x4(CPipeLine::D3DTS_VIEW));
    _matrix projMat = XMLoadFloat4x4(m_pGameInstance->Get_Transform_Float4x4(CPipeLine::D3DTS_PROJ));

    // 월드 위치
    _vector worldPos = m_pTransformCom->Get_State(CTransform::STATE_POSITION);

    // --- 프러스텀 컬링 (클립 공간 투영) ---
    _vector clip = XMVector4Transform(worldPos, viewMat * projMat);
    _float  w = XMVectorGetW(clip);
    if (w <= 0.f)  // 카메라 뒤쪽
        return false;

    _vector ndc = clip / w;
    _float nx = XMVectorGetX(ndc);
    _float ny = XMVectorGetY(ndc);
    _float nz = XMVectorGetZ(ndc);
    if (nx < -1.f || nx > 1.f ||
        ny < -1.f || ny > 1.f ||
        nz < 0.f || nz > 1.f)
    {
        return false;
    }

    // --- 스크린-스페이스 컬링 ---
    _float fWidth = _float(m_viewport.Width);
    _float fHeight = _float(m_viewport.Height);
    _float2 fScreenPos = m_pTransformCom->World_To_Screen(viewMat, projMat, fWidth, fHeight);

    if (fScreenPos.x < -marginPx || fScreenPos.x > fWidth + marginPx ||
        fScreenPos.y < -marginPx || fScreenPos.y > fHeight + marginPx)
    {
        return false;
    }

    return true;
}

HRESULT CGameObject::Bind_TransformToShader(shared_ptr<CTransform> transform, shared_ptr<CShader> shader)
{
    if (FAILED(transform->Bind_ShaderResource(shader, "g_WorldMatrix")))
        return E_FAIL;

    if (FAILED(shader->Bind_Matrix("g_ViewMatrix", m_pGameInstance->Get_Transform_Float4x4(CPipeLine::D3DTS_VIEW))))
        return E_FAIL;

    if (FAILED(shader->Bind_Matrix("g_ProjMatrix", m_pGameInstance->Get_Transform_Float4x4(CPipeLine::D3DTS_PROJ))))
        return E_FAIL;

    return S_OK;
}

