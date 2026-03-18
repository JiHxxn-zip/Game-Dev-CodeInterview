#include "Quest_Manager.h"
#include "Json_Manager.h"

#include "Objective_TalkToNPC.h"
#include "Objective_MonsterKill.h"
#include "Objective_ItemUse.h"

#include "JSON_Data.h"
#include <sstream>

namespace fs = std::filesystem;

IMPLEMENT_SINGLETON_NORELASE(CQuest_Manager)

using namespace Client;

CQuest_Manager::~CQuest_Manager()
{
    Release();
}

HRESULT CQuest_Manager::Initialize()
{
    m_pQuest = CQuest::Create();

    //Save_Json(); // 코드에서 Save 로직
    
    // 퀘스트 데이터 로드
    Load_Json(L"../Bin/DataFiles/Json/Quest");

    return S_OK;
}

void CQuest_Manager::Save_Json()
{
    QUEST_DESC quest_Desc{};
    quest_Desc.strQuestID = L"DuncanMale_Welcome";
    quest_Desc.strTitle = L"안녕! 티르코네일";
    quest_Desc.strDescription = L"나침반이 인도하는 곳으로 가기";
    quest_Desc.strNextQuestID = L"Quest002";
    quest_Desc.reward.iExp = 10;
    quest_Desc.reward.iGold = 100;

    // objective
    quest_Desc.objective.strQuestType = L"TalkToNPC";
    quest_Desc.objective.strNpcName = L"던컨";
    quest_Desc.objective.strMonsterName = L"";
    quest_Desc.objective.iTargetKillCount = 0;
    quest_Desc.objective.iCurrentKillCount = 0;
    quest_Desc.objective.strTargetItemTag = L"";

    // JSON으로 직렬화
    json jsonRoot;
    quest_Desc.ToJson(jsonRoot);

    // 경로 조합 및 저장
    wstring defaultPath = L"../Bin/DataFiles/Json/Quest/";
    wstring fullPath = defaultPath + quest_Desc.strQuestID + L".json";
    CJson_Manager::GetInstance()->Save_Json(fullPath, jsonRoot);
}

HRESULT CQuest_Manager::Load_Json(const wstring& folderPath)
{
    // 지정된 폴더 경로에서 모든 스크립트 파일 로드
    for (const auto& entry : fs::recursive_directory_iterator(folderPath))
    {
        if (entry.is_regular_file())
            Load_SingleScript(entry.path().wstring());
    }

    return S_OK;
}

HRESULT CQuest_Manager::Load_SingleScript(const wstring& filePath)
{
    json j;
    if (FAILED(CJson_Manager::GetInstance()->Load_Json(filePath, j)))
        return E_FAIL;

    QUEST_DESC desc;
    desc.FromJson(j);

    // Null 문자 제거
    CleanWString(desc.objective.strQuestType);
    //CleanWString(desc.objective.strNpcName);
    //CleanWString(desc.objective.strMonsterName);
    //CleanWString(desc.strQuestID);

    if (desc.objective.strQuestType == L"TalkToNPC")
    {
        auto pTalkObjective = make_shared<CObjective_TalkToNPC>();
        pTalkObjective->Set_QuestDesc(desc);

        m_pQuest->Add_Objective(pTalkObjective);
    }
    else if (desc.objective.strQuestType == L"MonsterKill")
    {
        auto pTalkObjective = make_shared<CObjective_MonsterKill>();
        pTalkObjective->Set_QuestDesc(desc);

        m_pQuest->Add_Objective(pTalkObjective);
    }
    else if (desc.objective.strQuestType == L"ItemUse")
    {
        auto pTalkObjective = make_shared<CObjective_ItemUse>();
        pTalkObjective->Set_QuestDesc(desc);

        m_pQuest->Add_Objective(pTalkObjective);
    }
    return S_OK;
}

void CQuest_Manager::Activate(const wstring& questID)
{
    if (m_pQuest)
    {
        m_CurrentObjective = m_pQuest->ActivateObjective(questID);
    }
}

void CQuest_Manager::NotifyEvent(const wstring& type, const wstring& value)
{
    if (m_pQuest)
        m_pQuest->NotifyEvent(type, value);
}

unsigned int CQuest_Manager::Release()
{
    m_pQuest.reset();
    return 0;
}
