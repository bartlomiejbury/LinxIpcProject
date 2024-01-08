
#include <stdio.h>
#include "gtest/gtest.h"
#include "LinxIpcSocketMock.h"
#include "LinxIpcClientMock.h"
#include "LinxIpc.h"
#include "LinxIpcEndpointImpl.h"

using namespace ::testing;

class LinxIpcEndpointTests : public testing::Test {
  public:
    NiceMock<LinxIpcSocketMock> *socketMock;

    void* (*callback)(void *arg){};
    void *data{};

    void SetUp() {
        socketMock = new NiceMock<LinxIpcSocketMock>();
        ON_CALL(*socketMock, receive(_, _, _)).WillByDefault(
        Invoke([](LinxMessageIpcPtr *msg, std::string *from, int timeout){
            *msg = std::make_shared<LinxMessageIpc>(10);
            *from = "TEST";
            return 0;
        }));
    }
};

TEST_F(LinxIpcEndpointTests, getPollFdCallSocketGetFd) {
    auto endpoint = std::make_shared<LinxIpcEndpointImpl>(socketMock);
    EXPECT_CALL(*socketMock, getFd());

    endpoint->getPollFd();
}

TEST_F(LinxIpcEndpointTests, getPollFdReturnSocketGetFdResult) {
    auto endpoint = std::make_shared<LinxIpcEndpointImpl>(socketMock);
    EXPECT_CALL(*socketMock, getFd()).WillOnce(Return(1));

    ASSERT_EQ(endpoint->getPollFd(), 1);
}

TEST_F(LinxIpcEndpointTests, sendCallSocketSend) {
    auto endpoint = std::make_shared<LinxIpcEndpointImpl>(socketMock);
    auto msg = LinxMessageIpc(10);
    auto client = std::make_shared<NiceMock<LinxIpcClientMock>>();

    ON_CALL(*(client.get()), getName()).WillByDefault(Return("TEST"));
    EXPECT_CALL(*socketMock, send(Ref(msg), "TEST"));

    endpoint->send(msg, client);
}

TEST_F(LinxIpcEndpointTests, sendReturnSocketSendResult) {
    auto endpoint = std::make_shared<LinxIpcEndpointImpl>(socketMock);
    auto msg = LinxMessageIpc(10);
    auto client = std::make_shared<NiceMock<LinxIpcClientMock>>();

    ON_CALL(*(client.get()), getName()).WillByDefault(Return("TEST"));
    EXPECT_CALL(*socketMock, send(_, _)).WillOnce(Return(1));

    ASSERT_EQ(endpoint->send(msg, client), 1);
}

TEST_F(LinxIpcEndpointTests, sendWithNoDestinationReturnError) {
    auto endpoint = std::make_shared<LinxIpcEndpointImpl>(socketMock);
    auto msg = LinxMessageIpc(10);

    ASSERT_EQ(endpoint->send(msg, nullptr), -1);
}

TEST_F(LinxIpcEndpointTests, endpoint_createClientHasCorrectName) {
    auto endpoint = std::make_shared<LinxIpcEndpointImpl>(socketMock);

    auto client = endpoint->createClient("TEST");
    ASSERT_STREQ(client->getName().c_str(), "TEST");
}

TEST_F(LinxIpcEndpointTests, receive_callSocketSend) {
    auto endpoint = std::make_shared<LinxIpcEndpointImpl>(socketMock);
    auto sigsel = std::initializer_list<uint32_t>{4};
    std::optional<std::string> fromOpt = std::nullopt;

    EXPECT_CALL(*socketMock, receive(_, _, 10000));
    endpoint->receive(10000, sigsel);
}

TEST_F(LinxIpcEndpointTests, receive_ReturnNullWhenSocketReturnError) {
    auto endpoint = std::make_shared<LinxIpcEndpointImpl>(socketMock);
    auto sigsel = std::initializer_list<uint32_t>{4};
    EXPECT_CALL(*socketMock, receive(_, _, _)).WillOnce(Return(-1));

    ASSERT_EQ(endpoint->receive(10000, sigsel), nullptr);
}

TEST_F(LinxIpcEndpointTests, receive_ReturnMsgWhenSignalMatchAny) {
    auto endpoint = std::make_shared<LinxIpcEndpointImpl>(socketMock);
    auto message = std::make_shared<LinxMessageIpc>(10);

    EXPECT_CALL(*socketMock, receive(_, _, _)).WillOnce(
        Invoke([&message](LinxMessageIpcPtr *msg, std::string *from, int timeout){
            *msg = message;
            *from = "TEST";
            return 0;
        }));


    ASSERT_EQ(endpoint->receive(10000, LINX_ANY_SIG, LINX_ANY_FROM), message);
}

TEST_F(LinxIpcEndpointTests, receive_ReturnNullWhenSignalNotMatch) {
    auto endpoint = std::make_shared<LinxIpcEndpointImpl>(socketMock);
    auto sigsel = std::initializer_list<uint32_t>{4};

    EXPECT_CALL(*socketMock, receive(_, _, _)).WillOnce(
        Invoke([](LinxMessageIpcPtr *msg, std::string *from, int timeout){
            *msg = std::make_shared<LinxMessageIpc>(10);
            *from = "TEST";
            return 0;
        }));


    ASSERT_EQ(endpoint->receive(10000, sigsel, LINX_ANY_FROM), nullptr);
}

TEST_F(LinxIpcEndpointTests, receive_ReturnMsgWhenSignalMatch) {
    auto endpoint = std::make_shared<LinxIpcEndpointImpl>(socketMock);
    auto sigsel = std::initializer_list<uint32_t>{10};
    auto message = std::make_shared<LinxMessageIpc>(10);

    EXPECT_CALL(*socketMock, receive(_, _, _)).WillOnce(
        Invoke([&message](LinxMessageIpcPtr *msg, std::string *from, int timeout){
            *msg = message;
            *from = "TEST";
            return 0;
        }));


    ASSERT_EQ(endpoint->receive(10000, sigsel), message);
}

TEST_F(LinxIpcEndpointTests, receive_ReturnNullWhenClientNotMatch) {
    auto endpoint = std::make_shared<LinxIpcEndpointImpl>(socketMock);

    auto clientMock = std::make_shared<NiceMock<LinxIpcClientMock>>();
    EXPECT_CALL(*socketMock, receive(_, _, _)).WillOnce(
        Invoke([](LinxMessageIpcPtr *msg, std::string *from, int timeout){
            *msg = std::make_shared<LinxMessageIpc>(10);
            *from = "TEST";
            return 0;
        }));

    EXPECT_CALL(*(clientMock.get()), getName()).WillOnce(Return("TEST2"));
    ASSERT_EQ(endpoint->receive(10000, LINX_ANY_SIG, clientMock), nullptr);
}

TEST_F(LinxIpcEndpointTests, receive_ReturnMsgWhenClientMatch) {
    auto endpoint = std::make_shared<LinxIpcEndpointImpl>(socketMock);
    auto message = std::make_shared<LinxMessageIpc>(10);

    auto clientMock = std::make_shared<NiceMock<LinxIpcClientMock>>();
    EXPECT_CALL(*socketMock, receive(_, _, _)).WillOnce(
        Invoke([&message](LinxMessageIpcPtr *msg, std::string *from, int timeout){
            *msg = message;
            *from = "TEST";
            return 0;
        }));

    EXPECT_CALL(*(clientMock.get()), getName()).WillOnce(Return("TEST"));
    ASSERT_EQ(endpoint->receive(10000, LINX_ANY_SIG, clientMock), message);
}

TEST_F(LinxIpcEndpointTests, receive_ReturnMsgWhenSignalClientMatch) {
    auto endpoint = std::make_shared<LinxIpcEndpointImpl>(socketMock);
    auto message = std::make_shared<LinxMessageIpc>(10);
    auto sigsel = std::initializer_list<uint32_t>{10};
    auto clientMock = std::make_shared<NiceMock<LinxIpcClientMock>>();

    EXPECT_CALL(*socketMock, receive(_, _, _)).WillOnce(
        Invoke([&message](LinxMessageIpcPtr *msg, std::string *from, int timeout){
            *msg = message;
            *from = "TEST";
            return 0;
        }));

    EXPECT_CALL(*(clientMock.get()), getName()).WillOnce(Return("TEST"));
    ASSERT_EQ(endpoint->receive(10000, sigsel, clientMock), message);
}
