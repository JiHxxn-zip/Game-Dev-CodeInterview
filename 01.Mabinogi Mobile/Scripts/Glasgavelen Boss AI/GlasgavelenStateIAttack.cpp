#include "GlasgavelenStateAttack.h"

#include "Client_Defines.h"
#include "DamageObject.h"
#include "Glasgavelen.h"
#include "GlasgavelenDefaultAttack.h"

#include "UI_Manager.h"

HRESULT Client::CGlasgavelenStateAttack::Init_State()
{
    m_pGlasgavelen = dynamic_pointer_cast<CGlasgavelen>(m_pActor.lock());
    if (m_pGlasgavelen.expired())
        return E_FAIL;

    m_pBodyModel = m_pGlasgavelen.lock()->Get_Model();

    CDamageObject::DAMAGE_OBJECT_DESC desc = { m_pGlasgavelen };
    m_DefaultAttack = static_pointer_cast<CGlasgavelenDefaultAttack>(m_pGameInstance->Add_GameObject(LEVEL_DUNGEON, TEXT("Prototype_GameObject_GlasgavelenDefaultAttack"),
        LEVEL_DUNGEON, L"Layer_Object", &desc));

    if (m_DefaultAttack == nullptr)
        return E_FAIL;

    m_DefaultAttack->Set_Active(false);

    return S_OK;
}

void Client::CGlasgavelenStateAttack::State_Enter()
{
    shared_ptr<CGlasgavelen> lockGlasgavelen = m_pGlasgavelen.lock();

    //auto uiMgr = CUI_Manager::GetInstance();
    //uiMgr->Show_TextBallon3D(L"TextBallon_3D_Morrighan", 0, 3.f);

    if (m_CurAttack >= 4)
        m_CurAttack = 0;

    lockGlasgavelen->Look_Target();
    switch (m_CurAttack)
    {
        case 0:
            m_pBodyModel->Set_Animation(CGlasgavelen::DefaultAttack_01);
            break;
        case 1:
            m_pBodyModel->Set_Animation(CGlasgavelen::DefaultAttack_02);
            break;
        case 2:
            m_pBodyModel->Set_Animation(CGlasgavelen::DefaultAttack_03);
            break;
        case 3:
            m_pBodyModel->Set_Animation(CGlasgavelen::DefaultAttack_04);
            break;
    }
    ++m_CurAttack;
    Spawn_DamageObject();
}

void CGlasgavelenStateAttack::Spawn_DamageObject()
{
    shared_ptr<CGlasgavelen> lockGlasgavelen = m_pGlasgavelen.lock();
    //
    _vector centerPos = lockGlasgavelen->Get_Transform()->Get_State(CTransform::STATE_POSITION);
    _vector lookDir = XMVector3Normalize(lockGlasgavelen->Get_Transform()->Get_State(CTransform::STATE_LOOK));
    _vector fieldPos = centerPos + lookDir * 20.f;

    m_DefaultAttack->Spawn(fieldPos);
}

void Client::CGlasgavelenStateAttack::State_Update(_float fTimeDelta)
{
    shared_ptr<CGlasgavelen> lockGlasgavelen = m_pGlasgavelen.lock();

    if (m_pBodyModel->Is_Finished())
    {
        lockGlasgavelen->Change_State(CGlasgavelen::Glasgavelen_IDLE);
    }
}

void Client::CGlasgavelenStateAttack::State_Exit()
{
}

shared_ptr<Client::CGlasgavelenStateAttack> Client::CGlasgavelenStateAttack::Create(
    weak_ptr<CGameObject> _pActor)
{
    auto pGameInstance = shared_ptr<CGlasgavelenStateAttack>(new CGlasgavelenStateAttack(_pActor));

    if (FAILED(pGameInstance->Init_State()))
    {
        MSG_BOX("Failed to Create : CGlasgavelenStateDefaultAttack");
    }

    return pGameInstance;
}
