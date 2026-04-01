// Copyright dSPACE SE & Co. KG. All rights reserved.

#pragma once

#include <string>

#include <gtest/gtest.h>

#include "Channel.hpp"
#include "CoSimTypes.hpp"
#include "Socket.hpp"

#ifdef _WIN32

#include "OsUtilities.hpp"

#endif

#define AssertOk(result) ASSERT_EQ(result, DsVeosCoSim::CreateOk())
#define AssertError(result) ASSERT_EQ(result, DsVeosCoSim::CreateError())
#define AssertTimeout(result) ASSERT_EQ(result, DsVeosCoSim::CreateTimeout())
#define AssertNotConnected(result) ASSERT_EQ(result, DsVeosCoSim::CreateNotConnected())
#define AssertFull(result) ASSERT_EQ(result, DsVeosCoSim::CreateFull())
#define AssertEmpty(result) ASSERT_EQ(result, DsVeosCoSim::CreateEmpty())

[[nodiscard]] DsVeosCoSim::CoSimType GetCounterPart(DsVeosCoSim::CoSimType coSimType);
[[nodiscard]] std::string GetCounterPart(const std::string& name, DsVeosCoSim::ConnectionKind connectionKind);

void TestSendAfterDisconnect(DsVeosCoSim::SocketClient& client);
void TestReceiveAfterDisconnect(DsVeosCoSim::SocketClient& client);
void TestReceiveAfterDisconnectOnRemoteClient(DsVeosCoSim::SocketClient& client1, DsVeosCoSim::SocketClient& client2);
void TestSendAndReceive(DsVeosCoSim::SocketClient& client1, DsVeosCoSim::SocketClient& client2);

void TestManyElements(DsVeosCoSim::SocketClient& client1, DsVeosCoSim::SocketClient& client2);
void TestBigElement(DsVeosCoSim::SocketClient& client1, DsVeosCoSim::SocketClient& client2);
void TestPingPong(DsVeosCoSim::SocketClient& client1, DsVeosCoSim::SocketClient& client2);

#ifdef _WIN32

void TestSendAfterDisconnect(DsVeosCoSim::ShmPipeClient& client);
void TestSendAfterDisconnectOnRemoteClient(DsVeosCoSim::ShmPipeClient& client1, DsVeosCoSim::ShmPipeClient& client2);
void TestReceiveAfterDisconnect(DsVeosCoSim::ShmPipeClient& client);
void TestReceiveAfterDisconnectOnRemoteClient(DsVeosCoSim::ShmPipeClient& client1, DsVeosCoSim::ShmPipeClient& client2);
void TestSendAndReceive(DsVeosCoSim::ShmPipeClient& client1, DsVeosCoSim::ShmPipeClient& client2);

void TestManyElements(DsVeosCoSim::ShmPipeClient& client1, DsVeosCoSim::ShmPipeClient& client2);
void TestBigElement(DsVeosCoSim::ShmPipeClient& client1, DsVeosCoSim::ShmPipeClient& client2);
void TestPingPong(DsVeosCoSim::ShmPipeClient& client1, DsVeosCoSim::ShmPipeClient& client2);

#endif

void TestWriteUInt16ToChannel(std::unique_ptr<DsVeosCoSim::Channel>& writeChannel);
void TestWriteUInt32ToChannel(std::unique_ptr<DsVeosCoSim::Channel>& writeChannel);
void TestWriteUInt64ToChannel(std::unique_ptr<DsVeosCoSim::Channel>& writeChannel);
void TestWriteBufferToChannel(std::unique_ptr<DsVeosCoSim::Channel>& writeChannel);

void TestReadUInt16FromChannel(std::unique_ptr<DsVeosCoSim::Channel>& writeChannel, std::unique_ptr<DsVeosCoSim::Channel>& readChannel);
void TestReadUInt32FromChannel(std::unique_ptr<DsVeosCoSim::Channel>& writeChannel, std::unique_ptr<DsVeosCoSim::Channel>& readChannel);
void TestReadUInt64FromChannel(std::unique_ptr<DsVeosCoSim::Channel>& writeChannel, std::unique_ptr<DsVeosCoSim::Channel>& readChannel);
void TestReadBufferFromChannel(std::unique_ptr<DsVeosCoSim::Channel>& writeChannel, std::unique_ptr<DsVeosCoSim::Channel>& readChannel);

void TestPingPong(std::unique_ptr<DsVeosCoSim::Channel>& firstChannel, std::unique_ptr<DsVeosCoSim::Channel>& secondChannel);

void TestSendTwoFramesAtOnce(std::unique_ptr<DsVeosCoSim::Channel>& writeChannel, std::unique_ptr<DsVeosCoSim::Channel>& readChannel);

void TestStream(std::unique_ptr<DsVeosCoSim::Channel>& writeChannel, std::unique_ptr<DsVeosCoSim::Channel>& readChannel);

void TestBigElement(std::unique_ptr<DsVeosCoSim::Channel>& writeChannel, std::unique_ptr<DsVeosCoSim::Channel>& readChannel);
