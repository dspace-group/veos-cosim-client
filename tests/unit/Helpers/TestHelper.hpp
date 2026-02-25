// Copyright dSPACE SE & Co. KG. All rights reserved.

#pragma once

#include <gtest/gtest.h>

#include <string>

#include "Channel.hpp"
#include "CoSimTypes.hpp"
#include "Socket.hpp"

#ifdef _WIN32

#include "OsUtilities.hpp"

#endif

namespace DsVeosCoSim {

#define AssertOk(result) ASSERT_EQ(result.kind, ResultKind::Ok)
#define AssertError(result) ASSERT_EQ(result.kind, ResultKind::Error)
#define AssertTimeout(result) ASSERT_EQ(result.kind, ResultKind::Timeout)
#define AssertNotConnected(result) ASSERT_EQ(result.kind, ResultKind::NotConnected)
#define AssertFull(result) ASSERT_EQ(result.kind, ResultKind::Full)
#define AssertEmpty(result) ASSERT_EQ(result.kind, ResultKind::Empty)

[[nodiscard]] CoSimType GetCounterPart(CoSimType coSimType);
[[nodiscard]] std::string GetCounterPart(const std::string& name, ConnectionKind connectionKind);

void TestSendAfterDisconnect(SocketClient& client);
void TestSendAfterDisconnectOnRemoteClient(SocketClient& client1, SocketClient& client2);
void TestReceiveAfterDisconnect(SocketClient& client);
void TestReceiveAfterDisconnectOnRemoteClient(SocketClient& client1, SocketClient& client2);
void TestSendAndReceive(SocketClient& client1, SocketClient& client2);

void TestManyElements(SocketClient& client1, SocketClient& client2);
void TestBigElement(SocketClient& client1, SocketClient& client2);
void TestPingPong(SocketClient& client1, SocketClient& client2);

#ifdef _WIN32

void TestSendAfterDisconnect(ShmPipeClient& client);
void TestSendAfterDisconnectOnRemoteClient(ShmPipeClient& client1, ShmPipeClient& client2);
void TestReceiveAfterDisconnect(ShmPipeClient& client);
void TestReceiveAfterDisconnectOnRemoteClient(ShmPipeClient& client1, ShmPipeClient& client2);
void TestSendAndReceive(ShmPipeClient& client1, ShmPipeClient& client2);

void TestManyElements(ShmPipeClient& client1, ShmPipeClient& client2);
void TestBigElement(ShmPipeClient& client1, ShmPipeClient& client2);
void TestPingPong(ShmPipeClient& client1, ShmPipeClient& client2);

#endif

void TestWriteUInt16ToChannel(std::unique_ptr<Channel>& writeChannel);
void TestWriteUInt32ToChannel(std::unique_ptr<Channel>& writeChannel);
void TestWriteUInt64ToChannel(std::unique_ptr<Channel>& writeChannel);
void TestWriteBufferToChannel(std::unique_ptr<Channel>& writeChannel);

void TestReadUInt16FromChannel(std::unique_ptr<Channel>& writeChannel, std::unique_ptr<Channel>& readChannel);
void TestReadUInt32FromChannel(std::unique_ptr<Channel>& writeChannel, std::unique_ptr<Channel>& readChannel);
void TestReadUInt64FromChannel(std::unique_ptr<Channel>& writeChannel, std::unique_ptr<Channel>& readChannel);
void TestReadBufferFromChannel(std::unique_ptr<Channel>& writeChannel, std::unique_ptr<Channel>& readChannel);

void TestPingPong(std::unique_ptr<Channel>& firstChannel, std::unique_ptr<Channel>& secondChannel);

void TestSendTwoFramesAtOnce(std::unique_ptr<Channel>& writeChannel, std::unique_ptr<Channel>& readChannel);

void TestStream(std::unique_ptr<Channel>& writeChannel, std::unique_ptr<Channel>& readChannel);

void TestBigElement(std::unique_ptr<Channel>& writeChannel, std::unique_ptr<Channel>& readChannel);

}  // namespace DsVeosCoSim
