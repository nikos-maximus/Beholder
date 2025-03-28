#pragma once

namespace bhThreads
{
    struct Task
    {
        enum State
        {
            TASK_DONE,
            TASK_NEW,
            TASK_WAIT,
            TASK_MOVED,

            NUM_TASK_STATES
        };

        bool (*jobFunc)(void* data) = nullptr;
        void* data = nullptr;
        State state = TASK_NEW;
    };

    bool Init();
    void Destroy();
    bool SpawnJob(bool (*jobFunc)(void* data), void* data);

    void Test();
}
