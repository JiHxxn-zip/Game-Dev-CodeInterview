#pragma once
#include "Base.h"
#include "JSON_Data.h"

class CObjective abstract : public CBase
{
public:
    CObjective() = default;
    virtual ~CObjective() = default;

public:
    virtual _bool MatchesEvent(const wstring& type, const wstring& value) = 0;
    virtual _bool OnEvent(const wstring& type, const wstring& value) = 0;
    virtual _bool IsCompleted() = 0;
    virtual void Activate() { m_bActived = true; }
    virtual _bool IsActivated() = 0;

public:
    void Set_QuestDesc(QUEST_DESC qestDesc) { m_QuestDesc = qestDesc; }
    QUEST_DESC Get_QuestDesc() { return m_QuestDesc; }
   
public:
    EQuestState Get_CurrentState() { return m_eState; }
    void Set_CurrentState(EQuestState state) { m_eState = state; }

protected:
    EQuestState m_eState = EQuestState::NOTSTARTED;

protected:
    QUEST_DESC m_QuestDesc{};

    _bool m_bCompleted = {};
    _bool m_bActived= {};
};


