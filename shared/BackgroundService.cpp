// Copyright dSPACE GmbH. All rights reserved.

#include "BackgroundService.h"

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
    _thread.join();
}
