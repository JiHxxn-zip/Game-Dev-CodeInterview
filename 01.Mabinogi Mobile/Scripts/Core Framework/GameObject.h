#pragma once

#include "Transform.h"
#include "Component.h"

BEGIN(Engine)

class CGameInstance;
class CCollider;

class ENGINE_DLL CGameObject abstract : public CBase
{
public:
	typedef struct tagGameObjectDesc : public CTransform::TRANSFORM_DESC
	{
        _tchar m_szTagBuffer[MAX_PATH] = TEXT("");
		const _tchar* pGameObjectTag;
	}GAMEOBJECT_DESC;

private:
    static _uint g_guid;
    _uint m_guid;

protected:
	CGameObject();
	CGameObject(ComPtr<ID3D11Device> pDevice, ComPtr<ID3D11DeviceContext> pContext);
	CGameObject(const CGameObject& Prototype);
public:
	virtual ~CGameObject() = default;

public:
    _uint Get_GUID() const { return m_guid; }
    const _uint& GetMyGUID(void) const { return m_guid; }
	virtual HRESULT Initialize_Prototype();
	virtual HRESULT Initialize(void* pArg);
	virtual void    Priority_Update(_float fTimeDelta);
	virtual void    Update(_float fTimeDelta);
	virtual void    Last_Update(_float fTimeDelta);
	virtual HRESULT Render();
	virtual HRESULT Render_Shadow() { return S_OK; }
	virtual HRESULT Render_Outline() { return S_OK; }
    virtual HRESULT Render_GlowMask();
    virtual _vector Get_Position();

public:
    virtual	void	OnCollision(shared_ptr<CCollider> pOtherCollider) {}
    virtual void    OnTriggerEnter(shared_ptr<CCollider> pOtherCollider) {}
    virtual void    OnTriggerStay(shared_ptr<CCollider> pOtherCollider) {}
    virtual void    OnTriggerExit(shared_ptr<CCollider> pOtherCollider) {}

public:
    HRESULT Bind_TransformToShader(shared_ptr<class CTransform> transform, shared_ptr<class CShader> shader);

    // 공격자가 상대방에게 충돌했을 때 호출됨 (공격자 입장)
    virtual void    OnHit(shared_ptr<CGameObject> pTarget) {}

    // 피격자 입장
    virtual void    Take_Damage(DamageInfo& info) {}

public:
    virtual void    Active() {}
    virtual void    Deactive() {}

    bool Is_GlowEffect() const { return m_bIsGlowEffect; }
    shared_ptr<CTransform>  Get_Transform() { return m_pTransformCom; }
    _wstring	            Get_GameObjectTag() { return m_szGameObjectTag; }
    shared_ptr<CComponent>  Find_Component(const _wstring& strComponentTag);
    const std::wstring& Get_GameObjectSubTag(void) const { return m_szGameObjectName; }

    void Set_GameObjectTag(const std::wstring& value) { wcscpy_s(m_szGameObjectTag, MAX_PATH, value.c_str()); }
    void Set_GameObjectSubTag(const std::wstring& value) { m_szGameObjectName = value; }

    CGameInstance* Get_MyGameInstance() { return m_pGameInstance; }

    GAMEOBJECT_TYPE Get_ObjectType() const { return m_eObjectType; }
    void Set_ObjectType(GAMEOBJECT_TYPE eType) { m_eObjectType = eType; }

public:
    void    Set_Active(_bool bActive);
    void    Set_Visible(_bool bVisible);

    _bool   Is_Active() const { return m_bActive; }
    _bool   Is_Visible() const { return m_bVisible; }

protected:
	ComPtr<ID3D11Device>			    m_pDevice = { nullptr };
	ComPtr<ID3D11DeviceContext>	        m_pContext = { nullptr };
	CGameInstance*                      m_pGameInstance = { nullptr };
	shared_ptr<CTransform>		        m_pTransformCom = { nullptr };

    _bool                               m_bActive = { true };
    _bool                               m_bVisible = { true };

    _bool                               m_bIsGlowEffect = { false }; // Glow 효과가 있는지 여부

protected:
    GAMEOBJECT_TYPE                     m_eObjectType = GAMEOBJECT_TYPE::UNKNOWN;

	_tchar								m_szGameObjectTag[MAX_PATH] = {};
    std::wstring                        m_szGameObjectName = L"Unnamed"; // 맵툴에서 설정해놓은 이름
	map<const _wstring, shared_ptr<CComponent>>	m_Components;

#pragma region 텍스트 떠야하는 객체들만 사용하세요.
public:
    _bool IsInView(_float marginPx, _float maxDistance = 20.f) const;
    _bool IsInFrustum(_float marginPx) const;
protected:
    // 게임인스탠스로부터 받아올 뷰포트(스크린 좌표로 변환하기 위함)
    D3D11_VIEWPORT m_viewport{};
    // 거리기반으로 사라지는데 걸리는 시간이 얼마나 걸릴건지
    _float m_fNameFadeDuration = 0.2f;
    // 폰트의 알파값
    _float m_fTextAlpha = 1.0f;
#pragma endregion

// 말풍선용
protected:
    _bool    m_bBalloonCreated = false;   // 한 번만 호출용 플래그

protected:

	shared_ptr<CComponent> Add_Component(_uint iPrototypeLevelIndex, const _wstring& strPrototypeTag, const _wstring& strComponentTag, void* pArg = nullptr);

public:
	virtual shared_ptr<CGameObject> Clone(void* pArg) = 0;	

};

END
