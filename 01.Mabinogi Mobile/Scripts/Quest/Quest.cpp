#include "Quest.h"
#include "GameInstance.h"
#include "Client_Util.h"
#include "UI_Manager.h"
#include "Game_Manager.h"
#include "StatusCom.h"
#include "CombatComponent.h"

void CQuest::NotifyEvent(const wstring& type, const wstring& value)
{
    if (m_pCurrentObjective == nullptr)
        return;

    if (m_pCurrentObjective->Get_CurrentState() == EQuestState::REWARD)
        return;

    if (!m_pCurrentObjective->IsActivated())
        return;

    if (!m_pCurrentObjective->MatchesEvent(type, value))
        return;

    if (m_pCurrentObjective->OnEvent(type, value) && m_pCurrentObjective->IsCompleted())
    {   
        m_pCurrentObjective->Set_CurrentState(EQuestState::REWARD);

        // 퀘스트 완료 UI On
        if (m_pCurrentObjective->Get_QuestDesc().objective.strQuestType != L"TalkToNPC")
        {
            m_sNpcName = m_pCurrentObjective->Get_QuestDesc().objective.strNpcName;
            Client::CleanWString(m_sNpcName);

            if (m_pCurrentObjective->Get_QuestDesc().objective.strQuestType == L"MonsterKill")
            {
                CGame_Manager::GetInstance()->Get_Player()->Get_CombatCom()->Stop_Combat();
                
                CGameInstance::GetInstance()->Schedule(1.0f, [this]()
                {
                    CUI_Manager::GetInstance()->Activate_QuestNpcPicture(m_sNpcName, m_pCurrentObjective->Get_QuestDesc().strTitle);
                    CUI_Manager::GetInstance()->Set_NameDisplayTitle(L"10살에 곰 잡은");
                });
            }
            else
            {
                CUI_Manager::GetInstance()->Activate_QuestNpcPicture(m_sNpcName, m_pCurrentObjective->Get_QuestDesc().strTitle);
            }

            // 몬스터 처치 시 1초 뒤
            CGame_Manager::GetInstance()->Get_Player()->Get_CombatCom()->Stop_Combat();
        }

        // 보상
        auto player = CGame_Manager::GetInstance()->Get_Player();
        if (player)
        {
            //auto baseCom = player->Find_Component(TEXT("Com_Inventory"));
            auto pInventoryCom = CGame_Manager::GetInstance()->Get_InventoryCom();
            if (pInventoryCom)
            {
                const auto& reward = m_pCurrentObjective->Get_QuestDesc().reward;
                //==========경험치와 골드===========
                //player->Add_Exp(reward.iExp);
                pInventoryCom->Add_Gold(reward.iGold);
                //=================================

                //==========아이템 보상 처리===========
                size_t iCount = min(reward.iItemTypes.size(), reward.iItemCounts.size());

                for (size_t i = 0; i < iCount; ++i)
                {
                    _int iItemType = reward.iItemTypes[i];
                    _int iItemCount = reward.iItemCounts[i];

                    for (_int j = 0; j < iItemCount; ++j)
                    {
                        auto icon = CUI_Manager::GetInstance()->Add_ItemIcon(LEVEL_STATIC, L"LAYER_INVENTORY", iItemType);
                        if (icon)
                            pInventoryCom->Add_ItemIcon(icon);
                    }
                }
                //===================================
            }
        }

        //if (m_pCurrentObjective->Get_QuestDesc().activeNextQuest)
        //{
        //    wstring nextQuestID = m_pCurrentObjective->Get_QuestDesc().strNextQuestID;
        //    Client::CleanWString(nextQuestID);
        //    ActivateObjective(nextQuestID);
        //    CUI_Manager::GetInstance()->Start_Move_To_QuestHelper_Only();
        //}
    }
}

shared_ptr<CObjective> CQuest::ActivateObjective(const wstring& questID)
{
    // 1. 현재 진행 중이면 그냥 반환
    if (m_pCurrentObjective != nullptr &&
        m_pCurrentObjective->Get_CurrentState() == EQuestState::INPROGRESS)
    {
        return m_pCurrentObjective;
    }

    // 2. 보상 상태는 건너뛰고, questID에 해당하는 Objective 찾기
    for (auto& obj : m_Objectives)
    {
        if (obj->Get_CurrentState() == EQuestState::REWARD)
            continue;

        wstring _questID = obj->Get_QuestDesc().strQuestID;
         Client::CleanWString(_questID);

        if (_questID == questID)
        {
            m_pCurrentObjective = obj;
            m_pCurrentObjective->Activate();
            return m_pCurrentObjective;
        }
    }

    return m_pCurrentObjective;
}

shared_ptr<CQuest> CQuest::Create()
{
    return shared_ptr<CQuest>(new CQuest());
}
