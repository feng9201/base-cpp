#include "threadEnhance.h"


threadEnhance::threadEnhance()
    :m_pThread(nullptr),
    m_Thread_Pause_Flag(false),
    m_Thread_Stop_Flag(false),
    m_Thread_State(ThreadState::Stoped)
{
}

threadEnhance::~threadEnhance()
{
    StopThread();
}

void threadEnhance::StartThread()
{
    if (m_pThread == nullptr)
    {
        m_pThread = std::make_shared<std::thread>(&threadEnhance::Start, this);

        if (m_pThread != nullptr)
        {
            m_Thread_Pause_Flag = false;
            m_Thread_Stop_Flag = false;
            m_Thread_State = ThreadState::Running;
        }
    }
}

void threadEnhance::PauseThread()
{
    if (m_pThread != nullptr)
    {
        if (m_Thread_State == ThreadState::Running)
        {
            m_Thread_Pause_Flag = true;
            m_Thread_State = ThreadState::Paused;
        }
    }
}

void threadEnhance::ResumeThread()
{
    if (m_pThread != nullptr)
    {
        if (m_Thread_State == ThreadState::Paused)
        {
            m_Thread_Pause_Flag = false;
            m_Condition_Variable.notify_all();
            m_Thread_State = ThreadState::Running;
        }
    }
}

void threadEnhance::StopThread()
{
    if (m_pThread != nullptr)
    {
        m_Thread_Stop_Flag = true;
        m_Thread_Pause_Flag = false;

        m_Condition_Variable.notify_all();

        if (m_pThread->joinable())
        {
            m_pThread->join();
        }

        // 释放
        m_pThread.reset();

        if (m_pThread == nullptr)
        {
            m_Thread_State = ThreadState::Stoped;
        }
    }
}

int threadEnhance::GetThreadState() const
{
    return (int)m_Thread_State;
}

void threadEnhance::Start()
{
    while (!m_Thread_Stop_Flag)
    {
        try
        {
            Run();
        }
        catch (std::exception& e)
        {
            break;
        }

        if (m_Thread_Pause_Flag)
        {
            std::unique_lock<std::mutex> thread_locker(m_Mutex);
            if (m_Thread_Pause_Flag)
            {
                // 等待互斥锁
                m_Condition_Variable.wait(thread_locker);
            }
            thread_locker.unlock();
        }
    }

    m_Thread_Pause_Flag = false;
    m_Thread_Stop_Flag = false;
}