// Copyright dSPACE GmbH. All rights reserved.

#include "BackgroundService.h"

#include <thread>

using namespace DsVeosCoSim;

BackgroundService::BackgroundService(CoSimServer& coSimServer) : _coSimServer(coSimServer) {
    _thread = std::thread([this] {
        while (!_stopEvent.Wait(1)) {
            try {
                _coSimServer.BackgroundService();
            } catch (const std::exception& e) {
                LogError(e.what());
            }
        }
    });
}

BackgroundService::~BackgroundService() noexcept {
    _stopEvent.Set();
    if (std::this_thread::get_id() == _thread.get_id()) {
        _thread.detach();
    } else {
        _thread.join();
    }
}
