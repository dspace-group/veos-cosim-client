// Copyright dSPACE GmbH. All rights reserved.

#include <asio.hpp>
#include <string>

#include "CoSimHelper.h"
#include "LogHelper.h"
#include "PerformanceTestHelper.h"
#include "RunPerformanceTest.h"

using namespace DsVeosCoSim;
using namespace asio;
using ip::tcp;

namespace {

void AsioBlockingClientRun(std::string_view host, Event& connectedEvent, uint64_t& counter, const bool& isStopped) {
    try {
        io_context io_context;
        tcp::socket s(io_context);
        tcp::resolver resolver(io_context);
        connect(s, resolver.resolve(host, std::to_string(AsioBlockingPort)));

        char data[BufferSize]{};

        connectedEvent.Set();

        while (!isStopped) {
            std::error_code error;
            size_t length = write(s, buffer(data), error);
            if (error == error::eof) {
                throw std::runtime_error("Server disconnected.");
            }

            if (error || (length != BufferSize)) {
                throw std::system_error(error);
            }

            length = read(s, buffer(data));
            if (error == error::eof) {
                throw std::runtime_error("Server disconnected.");
            }

            if (error || (length != BufferSize)) {
                throw std::system_error(error);
            }

            counter++;
        }
    } catch (const std::exception& e) {
        LogError("Exception in ASIO blocking client thread: {}", e.what());
        connectedEvent.Set();
    }
}

}  // namespace

void RunAsioBlockingTest(std::string_view host) {
    LogTrace("ASIO blocking client:");
    RunPerformanceTest(AsioBlockingClientRun, host);
    LogTrace("");
}
