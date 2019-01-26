#include "reactive_system.h"

#include <thread>
#include <condition_variable>
#include <queue>
#include <atomic>

namespace async_ecs
{
struct reactive_system::reactive_system_data
{
    state_t mState = state_t::idle;
    std::thread mSystemThread;
    std::queue<std::unique_ptr<command>> mTasksQueue;
    std::mutex mTasksQueueMutex;
    std::condition_variable mExpectantForTask;
};

reactive_system::reactive_system()
    : mData(std::make_shared<reactive_system_data>())
{
}

reactive_system::~reactive_system()
{
    if (state() == state_t::stopped) {
        return;
    }

    stop();
    mData->mExpectantForTask.notify_one();

    if (mData->mSystemThread.joinable()) {
        join();
    }
}

void reactive_system::start()
{
    mData->mSystemThread = std::thread([this] () {
        main();
    });
}
void reactive_system::pause()
{
    mData->mState = state_t::paused;
}
void reactive_system::stop()
{
    mData->mState = state_t::stopped;
    mData->mExpectantForTask.notify_one();
}
void reactive_system::join()
{
    mData->mSystemThread.join();
}

reactive_system::state_t reactive_system::state()
{
    return mData->mState;
}

void reactive_system::add_task(std::unique_ptr<command> cc)
{
    mData->mTasksQueueMutex.lock();
    mData->mTasksQueue.emplace(std::move(cc));
    mData->mTasksQueueMutex.unlock();
    mData->mExpectantForTask.notify_one();
}

void reactive_system::waitForResume()
{
}

void reactive_system::main()
{
    mData->mState = state_t::running;
    while (true)
    {
        {
            std::unique_lock<std::mutex> lk(mData->mTasksQueueMutex);
            mData->mExpectantForTask.wait(lk, [this] () -> bool {
                return !mData->mTasksQueue.empty() || state() == state_t::stopped;
            });
        }

        if (state() == state_t::stopped) { break; }

        mData->mTasksQueue.front()->execute();
        mData->mTasksQueue.pop();
    }
    mData->mState = state_t::idle;
}
}
