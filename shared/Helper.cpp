// Copyright dSPACE GmbH. All rights reserved.

#include "Helper.h"

#include <stdexcept>
#include <string>
#include <string_view>

#include "CoSimHelper.h"
#include "LogHelper.h"
#include "Socket.h"
#include "SocketChannel.h"

#ifdef _WIN32
#include <Windows.h>
#include <conio.h>
#else
#include <termios.h>
#include <unistd.h>

#include <cstdlib>
#endif

using namespace DsVeosCoSim;

namespace {

[[nodiscard]] uint16_t GetNextFreeDynamicPort() {
    const Socket socket(AddressFamily::Ipv4);
    socket.Bind(0, false);
    return socket.GetLocalPort();
}

}  // namespace

[[nodiscard]] bool StartUp() {
    InitializeOutput();

    try {
        StartupNetwork();
    } catch (const std::exception& e) {
        LogError(e.what());
        return false;
    }

    try {
        const uint16_t portMapperPort = GetNextFreeDynamicPort();
        const std::string portMapperPortString = std::to_string(portMapperPort);
#ifdef _WIN32
        const std::string environmentString = "VEOS_COSIM_PORTMAPPER_PORT=" + portMapperPortString;
        (void)_putenv(environmentString.c_str());
#else
        (void)setenv("VEOS_COSIM_PORTMAPPER_PORT", portMapperPortString.c_str(), 1);
#endif
    } catch (const std::exception& e) {
        LogError(e.what());
        return false;
    }

    return true;
}

[[nodiscard]] Socket ConnectSocket(const std::string_view ipAddress, const uint16_t remotePort) {
    std::optional<Socket> connectedSocket = Socket::TryConnect(ipAddress, remotePort, 0, DefaultTimeout);
    if (connectedSocket) {
        return std::move(*connectedSocket);
    }

    throw std::runtime_error("Could not connect within timeout.");
}

[[nodiscard]] Socket ConnectSocket(const std::string& name) {
    Socket socket(AddressFamily::Uds);
    if (socket.TryConnect(name)) {
        return socket;
    }

    throw std::runtime_error("Could not connect.");
}

[[nodiscard]] Socket Accept(const Socket& serverSocket) {
    std::optional<Socket> acceptedSocket = serverSocket.TryAccept(DefaultTimeout);
    if (acceptedSocket) {
        return std::move(*acceptedSocket);
    }

    throw std::runtime_error("Could not accept within timeout.");
}

[[nodiscard]] SocketChannel ConnectToTcpChannel(const std::string_view ipAddress, const uint16_t remotePort) {
    std::optional<SocketChannel> channel = TryConnectToTcpChannel(ipAddress, remotePort, 0, DefaultTimeout);
    if (channel) {
        return std::move(*channel);
    }

    throw std::runtime_error("Could not connect within timeout.");
}

[[nodiscard]] SocketChannel Accept(const TcpChannelServer& server) {
    std::optional<SocketChannel> acceptedChannel = server.TryAccept(DefaultTimeout);
    if (acceptedChannel) {
        return std::move(*acceptedChannel);
    }

    throw std::runtime_error("Could not accept within timeout.");
}

[[nodiscard]] SocketChannel ConnectToUdsChannel(const std::string& name) {
    std::optional<SocketChannel> channel = TryConnectToUdsChannel(name);
    if (channel) {
        return std::move(*channel);
    }

    throw std::runtime_error("Could not connect.");
}

[[nodiscard]] SocketChannel Accept(const UdsChannelServer& server) {
    std::optional<SocketChannel> acceptedChannel = server.TryAccept(DefaultTimeout);
    if (acceptedChannel) {
        return std::move(*acceptedChannel);
    }

    throw std::runtime_error("Could not accept within timeout.");
}

#ifdef _WIN32

[[nodiscard]] LocalChannel ConnectToLocalChannel(const std::string& name) {
    std::optional<LocalChannel> channel = TryConnectToLocalChannel(name);
    if (channel) {
        return std::move(*channel);
    }

    throw std::runtime_error("Could not connect.");
}

[[nodiscard]] LocalChannel Accept(LocalChannelServer& server) {
    std::optional<LocalChannel> acceptedChannel = server.TryAccept();
    if (acceptedChannel) {
        return std::move(*acceptedChannel);
    }

    throw std::runtime_error("Could not accept.");
}

#endif

[[nodiscard]] std::string_view GetLoopBackAddress(const AddressFamily addressFamily) {
    if (addressFamily == AddressFamily::Ipv4) {
        return "127.0.0.1";
    }

    return "::1";
}

[[nodiscard]] bool SendComplete(const Socket& socket, const void* buffer, size_t length) {
    const auto* bufferPointer = static_cast<const uint8_t*>(buffer);
    while (length > 0) {
        int32_t sentSize{};
        CheckResult(socket.Send(bufferPointer, static_cast<int32_t>(length), sentSize));

        length -= sentSize;
        bufferPointer += sentSize;
    }

    return true;
}

[[nodiscard]] bool ReceiveComplete(const Socket& socket, void* buffer, size_t length) {
    auto* bufferPointer = static_cast<uint8_t*>(buffer);
    while (length > 0) {
        int32_t receivedSize{};
        CheckResult(socket.Receive(bufferPointer, static_cast<int32_t>(length), receivedSize));

        length -= receivedSize;
        bufferPointer += receivedSize;
    }

    return true;
}
