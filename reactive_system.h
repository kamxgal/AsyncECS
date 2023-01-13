/*
 * MIT License
 * Copyright (c) 2018 kamxgal Kamil Galant kamil.galant@gmail.com
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
*/

#pragma once

#include <functional>
#include <memory>
#include <atomic>

namespace ecs
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

struct stop : public command
{
    stop(reactive_system& sys) : sys(sys) {}
    void execute() override {
        sys.stop();
    }

private:
    reactive_system& sys;
};

} // namespace ecs
