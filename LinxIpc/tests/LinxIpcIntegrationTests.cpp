#include <thread>
#include "gtest/gtest.h"
#include "AfUnixClient.h"
#include "AfUnixServer.h"

using namespace ::testing;

class LinxIpcIntegrationTests : public testing::Test {
  public:
};

static const uint32_t IPC_SIG1_REQ = IPC_SIG_BASE + 1;
static const uint32_t IPC_SIG1_RSP = IPC_SIG_BASE + 2;
static const uint32_t IPC_SIG2_REQ = IPC_SIG_BASE + 3;
static const uint32_t IPC_SIG2_RSP = IPC_SIG_BASE + 4;

TEST_F(LinxIpcIntegrationTests, testHandleMessage) {

    std::atomic<bool> running{true};
    std::thread handlerThread([&]() {
        auto server = AfUnixServer::create("TestService", 10);
        auto handler = LinxIpcHandler(server);
        handler.registerCallback(IPC_SIG1_REQ, [](const LinxReceivedMessageSharedPtr &msg, void *data) {
            LinxMessage rsp(IPC_SIG1_RSP);
            msg->context->send(rsp);
            return 0;
        }, nullptr)
        .registerCallback(IPC_SIG2_REQ, [](const LinxReceivedMessageSharedPtr &msg, void *data) {
            LinxMessage rsp(IPC_SIG2_RSP);
            msg->context->send(rsp);
            return 0;
        }, nullptr);

        ASSERT_TRUE(handler.start());
        while (running) {
            handler.handleMessage(100);
        }
        handler.stop();
    });

    // Ensure thread is always joined, even if test fails
    auto threadGuard = [&]() {
        running = false;
        if (handlerThread.joinable()) {
            handlerThread.join();
        }
    };
    std::shared_ptr<void> guard(nullptr, [&](void*) { threadGuard(); });

    auto client = AfUnixClient::create("TestService");
    ASSERT_TRUE(client->connect(1000));

    auto startTime = std::chrono::steady_clock::now();
    auto rsp = client->sendReceive(LinxMessage(IPC_SIG1_REQ));
    auto endTime = std::chrono::steady_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(endTime - startTime);

    running = false;
    handlerThread.join();

    ASSERT_NE(rsp, nullptr);
    ASSERT_EQ(rsp->getReqId(), IPC_SIG1_RSP);
    ASSERT_LE(duration.count(), 10) << "Get should not take more than " << 10 <<" ms";
}

TEST_F(LinxIpcIntegrationTests, testConnectTwoServers) {

    std::atomic<bool> running{true};
    std::thread handlerThread([&]() {
        auto server = AfUnixServer::create("TestService", 10);
        auto handler = LinxIpcHandler(server);
        handler.registerCallback(IPC_SIG1_REQ, [](const LinxReceivedMessageSharedPtr &msg, void *data) {
            LinxMessage rsp(IPC_SIG1_RSP);
            msg->context->send(rsp);
            return 0;
        }, nullptr)
        .registerCallback(IPC_SIG2_REQ, [](const LinxReceivedMessageSharedPtr &msg, void *data) {
            LinxMessage rsp(IPC_SIG2_RSP);
            msg->context->send(rsp);
            return 0;
        }, nullptr);

        ASSERT_TRUE(handler.start());
        while (running) {
            handler.handleMessage(100);
        }
        handler.stop();
    });

    // Ensure thread is always joined, even if test fails
    auto threadGuard = [&]() {
        running = false;
        if (handlerThread.joinable()) {
            handlerThread.join();
        }
    };
    std::shared_ptr<void> guard(nullptr, [&](void*) { threadGuard(); });

    auto secondServer = AfUnixServer::create("TestClient");
    ASSERT_TRUE(secondServer->start());
    auto client = secondServer->createContext("TestService");

    ASSERT_TRUE(client->connect(1000));

    auto ret = client->send(LinxMessage(IPC_SIG1_REQ));
    ASSERT_EQ(ret, 0);

    auto startTime = std::chrono::steady_clock::now();
    auto rsp1 = secondServer->receive(1000, {IPC_SIG1_RSP}, LINX_ANY_FROM);
    auto endTime = std::chrono::steady_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(endTime - startTime);

    ASSERT_NE(rsp1, nullptr);
    ASSERT_EQ(rsp1->message->getReqId(), IPC_SIG1_RSP);
    ASSERT_EQ(*rsp1->context, *client);
    ASSERT_LE(duration.count(), 10) << "Get should not take more than " << 10 <<" ms";

    auto rsp2 = client->sendReceive(LinxMessage(IPC_SIG1_REQ));
    ASSERT_NE(rsp2, nullptr);
    ASSERT_EQ(rsp2->getReqId(), IPC_SIG1_RSP);

    secondServer->stop();
}

TEST_F(LinxIpcIntegrationTests, TestFailToCreateTwoServewWithSameName) {

    auto server1 = AfUnixServer::create("TestService", 10);
    ASSERT_TRUE(server1->start());

    auto server2 = AfUnixServer::create("TestService", 10);
    ASSERT_FALSE(server2->start()) << "Should not be able to create two servers with the same name";
}

TEST_F(LinxIpcIntegrationTests, TestFailToSendMessageToNonExistingServer) {

    auto client = AfUnixClient::create("TestService");

    auto rsp = client->sendReceive(LinxMessage(IPC_SIG1_REQ));
    ASSERT_EQ(rsp, nullptr);
}

TEST_F(LinxIpcIntegrationTests, TestFailToConnectToNonExistingServer) {

    auto client = AfUnixClient::create("TestService");

    auto rsp = client->connect(100);
    ASSERT_FALSE(rsp);
}
