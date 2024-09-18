// Copyright dSPACE GmbH. All rights reserved.

#include <fmt/format.h>
#include <asio.hpp>
#include <thread>

#include "CoSimHelper.h"
#include "PerformanceTestHelper.h"

using namespace DsVeosCoSim;
using namespace asio;
using ip::tcp;

namespace {

void Session(tcp::socket s) {
    LogTrace("ASIO blocking server: Client connected.");
    try {
        for (;;) {
            char data[BufferSize]{};

            std::error_code error;
            size_t length = read(s, buffer(data), error);
            if (error == error::eof) {
                LogTrace("ASIO blocking server: Client disconnected.");
                break;
            }

            if (error || (length != BufferSize)) {
                throw std::system_error(error);
            }

            length = write(s, buffer(data, length));
            if (error == error::eof) {
                LogTrace("ASIO blocking server: Client disconnected.");
                break;
            }

            if (error || (length != BufferSize)) {
                throw std::system_error(error);
            }
        }
    } catch (const std::exception& e) {
        LogError("Exception in ASIO blocking server thread: {}", e.what());
    }
}

[[noreturn]] void AsioBlockingServerRun() {
    LogTrace("ASIO blocking server is listening on port {} ...", AsioBlockingPort);

    io_context ioContext;
    tcp::acceptor a(ioContext, tcp::endpoint(tcp::v4(), AsioBlockingPort));

    for (;;) {
        std::thread(Session, a.accept()).detach();
    }
}

}  // namespace

void StartAsioBlockingServer() {
    std::thread thread(AsioBlockingServerRun);
    thread.detach();
}
