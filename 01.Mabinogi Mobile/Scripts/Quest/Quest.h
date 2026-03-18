#pragma once
#include "Base.h"
#include "Client_Defines.h"
#include "Objective.h"

class CQuest final : public  CBase
{
private:
    CQuest() = default;
public:
    ~CQuest() = default;

public:
    void Add_Objective(shared_ptr<CObjective> pObj) { m_Objectives.push_back(pObj); }

    void NotifyEvent(const wstring& type, const wstring& value);

    shared_ptr<CObjective> ActivateObjective(const wstring& questID);

private:
    vector<shared_ptr<CObjective>> m_Objectives;

    shared_ptr<CObjective> m_pCurrentObjective;

    wstring m_sNpcName = {};

public:
    static shared_ptr<CQuest> Create();
};

