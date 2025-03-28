#include "bhThreads.h"
#include "bhDefines.h"
#include "bhConfig.h"
#include <thread>
#include <mutex>
#include <condition_variable>
#include <vector>

namespace bhThreads
{
    enum ThreadState
    {
        THREAD_QUIT,
        THREAD_ACTIVE,

        NUM_THREAD_STATES
    };

    static unsigned int g_numThreads = 0;
    static std::thread** g_threads = nullptr;
    static ThreadState* g_threadFlags = nullptr;
    
    static std::vector<Task> g_tasks;

    static std::mutex g_syncMutex, g_dataAccessMutex;
    static std::condition_variable g_signal;
    static uint32_t g_firstTask = 0, g_lastTask = 0;
    static uint32_t g_pendingTasks = 0;

    void WorkerThreadFunc(int threadIndex)
    {
        while (g_threadFlags[threadIndex] != ThreadState::THREAD_QUIT)
        {
            if (g_dataAccessMutex.try_lock())
            {
                if (g_pendingTasks > 0)
                {
                    Task* jobInQueue = &(g_tasks[g_firstTask]);
                    g_firstTask = (++g_firstTask) % g_tasks.size();
                    --g_pendingTasks;
                    g_dataAccessMutex.unlock();
                    Task myJob = *jobInQueue;
                    jobInQueue->state = Task::TASK_MOVED;
                    myJob.jobFunc(myJob.data);
                }
                else
                {
                    g_dataAccessMutex.unlock();
                    std::unique_lock<std::mutex> lock(g_syncMutex);
                    g_signal.wait(lock);
                }
            }
        }
    }

    bool Init()
    {
        // std::thread::hardware_concurrency()
        bhConfig::SystemSettings::Threads const& ts = bhConfig::GetSystemSettings().threads;
        g_numThreads = ts.num_threads;
        g_tasks.resize(ts.tasks_per_thread);

        g_threadFlags = static_cast<ThreadState*>(calloc(g_numThreads, sizeof(ThreadState)));
        g_threads = static_cast<std::thread**>(calloc(g_numThreads, sizeof(std::thread*)));
        for (unsigned int t = 0; t < g_numThreads; ++t)
        {
            g_threadFlags[t] = ThreadState::THREAD_ACTIVE;
            g_threads[t] = new std::thread(WorkerThreadFunc, t);
        }
        return true;
    }

    void Destroy()
    {
        while (g_pendingTasks > 0)
        {}

        for (unsigned int t = 0; t < g_numThreads; ++t)
        {
            g_threadFlags[t] = ThreadState::THREAD_QUIT;
        }
        g_signal.notify_all();
        for (unsigned int t = 0; t < g_numThreads; ++t)
        {
            g_threads[t]->join();
        }
        free(g_threads);
        free(g_threadFlags);
    }

    bool SpawnJob(bool (*jobFunc)(void* data), void* data)
    {
        if (g_tasks[g_lastTask].state == Task::TASK_WAIT)
        {
            // Cyclic buffer is full
            return false;
        }
        g_tasks[g_lastTask].jobFunc = jobFunc;
        g_tasks[g_lastTask].data = data;
        if (g_dataAccessMutex.try_lock())
        {
            ++g_pendingTasks;
            g_lastTask = (++g_lastTask) % g_tasks.size();
            g_dataAccessMutex.unlock();
        }
        g_signal.notify_all();
        return true;
    }

    ////////////////////////////////////////////////////////////////////////////////
    // Testing
    bool JobFunc(void*)// data)
    {
        //printf("Thread %d - Task %d\n", std::this_thread::get_id(), *reinterpret_cast<uint32_t*>(data));
        return true;
    }
    
    void Test()
    {
        for (uint32_t j = 0; j < 1000; ++j)
        {
            bhThreads::SpawnJob(JobFunc, &j);
        }
    }
}
