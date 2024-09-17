// Copyright dSPACE GmbH. All rights reserved.

#include "Helper.h"

#include <stdexcept>

#include "LogHelper.h"
#include "Logger.h"
#include "Result.h"
#include "Socket.h"
#include "SocketChannel.h"

#ifdef _WIN32
#include <Windows.h>
#include <conio.h>
#else
#include <cstdlib>
#include <termios.h>
#include <unistd.h>
#endif

using namespace DsVeosCoSim;

namespace {

uint16_t GetNextFreeDynamicPort() {
    Socket socket(AddressFamily::Ipv4);
    socket.Bind(0, false);
    return socket.GetLocalPort();
}

}  // namespace

bool StartUp() {
    InitializeOutput();

    try {
        StartupNetwork();
    } catch (const std::exception& e) {
        LogError(e.what());
        return false;
    }

    try {
        uint16_t portMapperPort = GetNextFreeDynamicPort();
        std::string portMapperPortString = std::to_string(portMapperPort);
#ifdef _WIN32
        (void)::SetEnvironmentVariableA("VEOS_COSIM_PORTMAPPER_PORT", portMapperPortString.c_str());
#else
        (void)::setenv("VEOS_COSIM_PORTMAPPER_PORT", portMapperPortString.c_str(), 1);
#endif
    } catch (const std::exception& e) {
        LogError(e.what());
        return false;
    }

    return true;
}

int32_t GetChar() {
#ifdef _WIN32
    return _getch();
#else
    termios oldt;
    tcgetattr(STDIN_FILENO, &oldt);

    termios newt = oldt;
    newt.c_lflag &= ~(ICANON | ECHO);

    tcsetattr(STDIN_FILENO, TCSANOW, &newt);

    int32_t ch = getchar();

    tcsetattr(STDIN_FILENO, TCSANOW, &oldt);

    return ch;
#endif
}

Socket ConnectSocket(std::string_view ipAddress, uint16_t remotePort) {
    std::optional<Socket> connectedSocket = Socket::TryConnect(ipAddress, remotePort, 0, DefaultTimeout);
    if (connectedSocket) {
        return std::move(*connectedSocket);
    }

    throw std::runtime_error("Could not connect within timeout.");
}

Socket ConnectSocket(const std::string& name) {
    Socket socket(AddressFamily::Uds);
    if (socket.TryConnect(name)) {
        return socket;
    }

    throw std::runtime_error("Could not connect.");
}

Socket Accept(const Socket& serverSocket) {
    std::optional<Socket> acceptedSocket = serverSocket.TryAccept(DefaultTimeout);
    if (acceptedSocket) {
        return std::move(*acceptedSocket);
    }

    throw std::runtime_error("Could not accept within timeout.");
}

SocketChannel ConnectToTcpChannel(std::string_view ipAddress, uint16_t remotePort) {
    std::optional<SocketChannel> channel = TryConnectToTcpChannel(ipAddress, remotePort, 0, DefaultTimeout);
    if (channel) {
        return std::move(*channel);
    }

    throw std::runtime_error("Could not connect within timeout.");
}

SocketChannel Accept(const TcpChannelServer& server) {
    std::optional<SocketChannel> acceptedChannel = server.TryAccept(DefaultTimeout);
    if (acceptedChannel) {
        return std::move(*acceptedChannel);
    }

    throw std::runtime_error("Could not accept within timeout.");
}

SocketChannel ConnectToUdsChannel(const std::string& name) {
    std::optional<SocketChannel> channel = TryConnectToUdsChannel(name);
    if (channel) {
        return std::move(*channel);
    }

    throw std::runtime_error("Could not connect.");
}

SocketChannel Accept(const UdsChannelServer& server) {
    std::optional<SocketChannel> acceptedChannel = server.TryAccept(DefaultTimeout);
    if (acceptedChannel) {
        return std::move(*acceptedChannel);
    }

    throw std::runtime_error("Could not accept within timeout.");
}

#ifdef _WIN32

LocalChannel ConnectToLocalChannel(const std::string& name) {
    std::optional<LocalChannel> channel = TryConnectToLocalChannel(name);
    if (channel) {
        return std::move(*channel);
    }

    throw std::runtime_error("Could not connect.");
}

LocalChannel Accept(LocalChannelServer& server) {
    std::optional<LocalChannel> acceptedChannel = server.TryAccept();
    if (acceptedChannel) {
        return std::move(*acceptedChannel);
    }

    throw std::runtime_error("Could not accept.");
}

#endif

std::string GetLoopBackAddress(AddressFamily addressFamily) {
    if (addressFamily == AddressFamily::Ipv4) {
        return "127.0.0.1";
    }

    return "::1";
}

bool SendComplete(const Socket& socket, const void* buffer, size_t length) {
    const auto* bufferPointer = static_cast<const uint8_t*>(buffer);
    while (length > 0) {
        int32_t sentSize{};
        CheckResult(socket.Send(bufferPointer, static_cast<int32_t>(length), sentSize));

        length -= sentSize;
        bufferPointer += sentSize;
    }

    return true;
}

bool ReceiveComplete(const Socket& socket, void* buffer, size_t length) {
    auto* bufferPointer = static_cast<uint8_t*>(buffer);
    while (length > 0) {
        int32_t receivedSize{};
        CheckResult(socket.Receive(bufferPointer, static_cast<int32_t>(length), receivedSize));

        length -= receivedSize;
        bufferPointer += receivedSize;
    }

    return true;
}