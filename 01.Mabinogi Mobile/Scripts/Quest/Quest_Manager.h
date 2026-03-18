 #pragma once
#include "Base.h"
#include "Client_Defines.h"
#include "JSON_Data.h"
#include "Quest.h"

BEGIN(Client)

class CQuest_Manager final : public CBase
{
    DECLARE_SINGLETON(CQuest_Manager)

private:
    CQuest_Manager() = default;
public:
    virtual ~CQuest_Manager();

public:
    HRESULT   Initialize();
              
private:      
    void Save_Json();
    HRESULT Load_Json(const wstring& folderPath);
    HRESULT Load_SingleScript(const wstring& filePath);

public:      
    shared_ptr<CObjective> Get_CurrentObjective() { return m_CurrentObjective; }

public:       
    void Activate(const wstring& questID);
    void NotifyEvent(const wstring& type, const wstring& value);

private:
    shared_ptr<CQuest> m_pQuest = { nullptr };
    shared_ptr<CObjective> m_CurrentObjective = { nullptr };

public:
    unsigned int Release();
};

END
