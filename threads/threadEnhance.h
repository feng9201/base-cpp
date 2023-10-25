#ifndef		THREAD_ENHANCE_H_
#define		THREAD_ENHANCE_H_
/*
@author:hxf
@time:2023-10-18
@brief: 封装c++11 thread, 使用类似QT 线程继承QThread 重写run函数使用
@ 不要再本线程中调用stopThread
*/

#include <thread>
#include <mutex>
#include <condition_variable>
#include <atomic>


class threadEnhance {
    enum class ThreadState
    {
        Stoped = 0,
        Running = 1,
        Paused = 2
    };

public:
    threadEnhance();
    virtual ~threadEnhance();

    void StartThread();
    void PauseThread();
    void ResumeThread();
    void StopThread();

    int GetThreadState() const;

private:
    void Start();

protected:
    virtual void Run() {};

private:
    ThreadState m_Thread_State;
    std::shared_ptr<std::thread> m_pThread;
    std::mutex m_Mutex;
    std::condition_variable m_Condition_Variable;
    std::atomic<bool> m_Thread_Pause_Flag;
    std::atomic<bool> m_Thread_Stop_Flag;
};

#endif // !1
