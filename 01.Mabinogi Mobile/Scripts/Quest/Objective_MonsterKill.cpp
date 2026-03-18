#include "Objective_MonsterKill.h"

_bool CObjective_MonsterKill::MatchesEvent(const wstring& type, const wstring& value)
{
    wstring monsterName = m_QuestDesc.objective.strMonsterName;
    Client::CleanWString(monsterName);
    return type == L"MonsterKill" && value == monsterName;
}

_bool CObjective_MonsterKill::OnEvent(const wstring& type, const wstring& value)
{
    ++m_CurrentCount;
    m_QuestDesc.objective.iCurrentKillCount = m_CurrentCount;

    if(m_QuestDesc.objective.iTargetKillCount == m_CurrentCount)
        m_bCompleted = true;

    return m_bCompleted;
}

_bool CObjective_MonsterKill::IsCompleted() 
{
    m_eState = EQuestState::COMPLETED;
    return m_bCompleted;
}

_bool CObjective_MonsterKill::IsActivated()
{
    //m_CurrentCount = 0;

    m_eState = EQuestState::INPROGRESS;
    return m_bActived;
}

shared_ptr<CObjective_MonsterKill> CObjective_MonsterKill::Create()
{
    return shared_ptr<CObjective_MonsterKill>(new CObjective_MonsterKill());
}


