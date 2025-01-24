// Copyright dSPACE GmbH. All rights reserved.

#pragma once

#include <chrono>
#include <condition_variable>
#include <mutex>

namespace DsVeosCoSim {

class Event {
public:
    void Set() {
        std::lock_guard guard(_mutex);
        _signaled = true;
        _conditionVariable.notify_one();
    }

    [[nodiscard]] bool Wait(const uint32_t timeoutInMilliseconds) {
        std::unique_lock lock(_mutex);
        const bool result = _conditionVariable.wait_for(lock, std::chrono::milliseconds(timeoutInMilliseconds), [this] {
            return _signaled;
        });
        _signaled = false;
        return result;
    }

private:
    std::mutex _mutex;
    std::condition_variable _conditionVariable;
    bool _signaled = false;
};

}  // namespace DsVeosCoSim
