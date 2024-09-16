// Copyright dSPACE GmbH. All rights reserved.

#pragma once

#include <thread>

#include "CoSimServer.h"
#include "Event.h"

class BackgroundService final {
public:
    explicit BackgroundService(DsVeosCoSim::CoSimServer& coSimServer);
    ~BackgroundService() noexcept;

    BackgroundService(const BackgroundService&) = delete;
    BackgroundService& operator=(BackgroundService const&) = delete;

    BackgroundService(BackgroundService&&) = delete;
    BackgroundService& operator=(BackgroundService&&) = delete;

private:
    DsVeosCoSim::CoSimServer& _coSimServer;
    DsVeosCoSim::Event _stopEvent;
    std::thread _thread;
};
