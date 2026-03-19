#pragma once
#include "State.h"

namespace Client
{
    //class CMorrighanDefaultProjectile;
}

BEGIN(Client)
    class CGlasgavelen;
    class CGlasgavelenDefaultAttack;
class CGlasgavelenStateAttack : public CState
{

public:
    explicit CGlasgavelenStateAttack(weak_ptr<CGameObject> _pActor) : CState(_pActor)
    {
    }
    virtual ~CGlasgavelenStateAttack() = default;

public:
    virtual HRESULT Init_State() override;
    virtual void    State_Enter() override;
    virtual void    State_Update(_float fTimeDelta) override;
    virtual void    State_Exit() override;

private:
    void Spawn_DamageObject();

private:
    /* ==== Actor ==== */
    weak_ptr<CGlasgavelen> m_pGlasgavelen = {};
    shared_ptr<CModel> m_pBodyModel = {};

    shared_ptr<CGlasgavelenDefaultAttack> m_DefaultAttack = {};

    _int m_CurAttack = {};

public:
    static shared_ptr<CGlasgavelenStateAttack> Create(weak_ptr<CGameObject> _pActor);
};
END