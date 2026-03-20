#include "Thread_Manager.h"

CThread_Manager::~CThread_Manager()
{
    Shutdown();
}

shared_ptr<CThread_Manager> CThread_Manager::Create()
{
    auto pInstance = shared_ptr<CThread_Manager>(new CThread_Manager());
    pInstance->Initialize(7); 

    return pInstance;
}

void CThread_Manager::Initialize(_int threadCount)
{
    m_Shutdown = false;

    for (size_t i = 0; i < threadCount; ++i)
    {
        m_Workers.emplace_back([this]()
            {
                WorkerThread();
            });
    }
}

void CThread_Manager::Add_Task(shared_ptr<CILoadTask> task)
{
    if (!task) 
        return;

    {
        unique_lock<mutex> lock(m_QueueMutex);
        m_TaskQueue.push(task);
    }

    m_TaskAvailable.notify_one();
}

void CThread_Manager::WorkerThread()
{
    while (true)
    {
        shared_ptr<CILoadTask> task;

        {
            unique_lock<mutex> lock(m_QueueMutex);
            m_TaskAvailable.wait(lock, [this]() 
            {
                return m_Shutdown || !m_TaskQueue.empty();
            });

            if (m_Shutdown && m_TaskQueue.empty())
                return;

            task = m_TaskQueue.front();
            m_TaskQueue.pop();
        }

        task->Execute();
    }
}

void CThread_Manager::Shutdown()
{
    {
        unique_lock<mutex> lock(m_QueueMutex);
        m_Shutdown = true;
    }

    m_TaskAvailable.notify_all();

    for (auto& thread : m_Workers)
    {
        if (thread.joinable())
            thread.join();
    }

    m_Workers.clear();
}
