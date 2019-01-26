#pragma once

#include <functional>
#include <memory>
#include <atomic>

namespace async_ecs
{

struct command
{
    std::atomic<bool> is_stopped = false;
    virtual void execute() = 0;
};

class reactive_system
{
    struct reactive_system_data;

public: /* types definitions */
    enum class state_t : uint8_t
    {
        idle = 0,
        running,
        paused,
        stopped
    };

public: /* methods */
    reactive_system();
    virtual ~reactive_system();

    virtual void initialize() {}

    void start();
    void pause();
    void stop();
    void join();
    state_t state();

protected:
    void add_task(std::unique_ptr<command> cc);
    void waitForResume();

private:
    void main();

private:
    std::shared_ptr<reactive_system_data> mData;
};
} // namespace async_ecs