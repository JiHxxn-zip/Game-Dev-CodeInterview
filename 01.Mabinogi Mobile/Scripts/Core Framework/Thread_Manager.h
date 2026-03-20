#pragma once
#include "Engine_Defines.h"
#include "ILoadTask.h"
#include <mutex>

BEGIN(Engine)

class CThread_Manager final
{
private:
    CThread_Manager() = default;
public:
    virtual ~CThread_Manager();
    static shared_ptr<CThread_Manager> Create();

private:
    void Initialize(_int threadCount);

public:
    void Add_Task(shared_ptr<CILoadTask> task);
    void Shutdown();

private:
    void WorkerThread();

private:
    vector<thread> m_Workers;
    queue<shared_ptr<CILoadTask>> m_TaskQueue;
    mutex m_QueueMutex;
    condition_variable m_TaskAvailable;
    _bool m_Shutdown = {};
};

END
