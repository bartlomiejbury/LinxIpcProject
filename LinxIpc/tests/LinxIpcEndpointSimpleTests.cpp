
#include <stdio.h>
#include "gtest/gtest.h"
#include "LinxIpcSocketMock.h"
#include "LinxIpcClientMock.h"
#include "LinxIpc.h"
#include "LinxIpcEndpointSimpleImpl.h"

using namespace ::testing;

class LinxIpcEndpointSimpleTests : public testing::Test {
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

TEST_F(LinxIpcEndpointSimpleTests, getPollFdCallSocketGetFd) {
    auto endpoint = std::make_shared<LinxIpcEndpointSimpleImpl>(socketMock);
    EXPECT_CALL(*socketMock, getFd());

    endpoint->getPollFd();
}

TEST_F(LinxIpcEndpointSimpleTests, getPollFdReturnSocketGetFdResult) {
    auto endpoint = std::make_shared<LinxIpcEndpointSimpleImpl>(socketMock);
    EXPECT_CALL(*socketMock, getFd()).WillOnce(Return(1));

    ASSERT_EQ(endpoint->getPollFd(), 1);
}

TEST_F(LinxIpcEndpointSimpleTests, sendCallSocketSend) {
    auto endpoint = std::make_shared<LinxIpcEndpointSimpleImpl>(socketMock);
    auto msg = LinxMessageIpc(10);
    auto client = std::make_shared<NiceMock<LinxIpcClientMock>>();

    ON_CALL(*(client.get()), getName()).WillByDefault(Return("TEST"));
    EXPECT_CALL(*socketMock, send(Ref(msg), "TEST"));

    endpoint->send(msg, client);
}

TEST_F(LinxIpcEndpointSimpleTests, sendReturnSocketSendResult) {
    auto endpoint = std::make_shared<LinxIpcEndpointSimpleImpl>(socketMock);
    auto msg = LinxMessageIpc(10);
    auto client = std::make_shared<NiceMock<LinxIpcClientMock>>();

    ON_CALL(*(client.get()), getName()).WillByDefault(Return("TEST"));
    EXPECT_CALL(*socketMock, send(_, _)).WillOnce(Return(1));

    ASSERT_EQ(endpoint->send(msg, client), 1);
}

TEST_F(LinxIpcEndpointSimpleTests, sendWithNoDestinationReturnError) {
    auto endpoint = std::make_shared<LinxIpcEndpointSimpleImpl>(socketMock);
    auto msg = LinxMessageIpc(10);

    ASSERT_EQ(endpoint->send(msg, nullptr), -1);
}

TEST_F(LinxIpcEndpointSimpleTests, start) {
    auto endpoint = std::make_shared<LinxIpcEndpointSimpleImpl>(socketMock);
    endpoint->start();
}

TEST_F(LinxIpcEndpointSimpleTests, stop) {
    auto endpoint = std::make_shared<LinxIpcEndpointSimpleImpl>(socketMock);
    endpoint->stop();
}

TEST_F(LinxIpcEndpointSimpleTests, endpoint_createClientHasCorrectName) {
    auto endpoint = std::make_shared<LinxIpcEndpointSimpleImpl>(socketMock);

    auto client = endpoint->createClient("TEST");
    ASSERT_STREQ(client->getName().c_str(), "TEST");
}

TEST_F(LinxIpcEndpointSimpleTests, receive_callSocketSend) {
    auto endpoint = std::make_shared<LinxIpcEndpointSimpleImpl>(socketMock);
    auto sigsel = std::initializer_list<uint32_t>{4};
    std::optional<std::string> fromOpt = std::nullopt;

    EXPECT_CALL(*socketMock, receive(_, _, 10000));
    endpoint->receive(10000, sigsel);
}

TEST_F(LinxIpcEndpointSimpleTests, receive_ReturnNullWhenSocketReturnError) {
    auto endpoint = std::make_shared<LinxIpcEndpointSimpleImpl>(socketMock);
    auto sigsel = std::initializer_list<uint32_t>{4};
    EXPECT_CALL(*socketMock, receive(_, _, _)).WillOnce(Return(-1));

    ASSERT_EQ(endpoint->receive(10000, sigsel), nullptr);
}

TEST_F(LinxIpcEndpointSimpleTests, receive_ReturnMsgWhenSignalMatchAny) {
    auto endpoint = std::make_shared<LinxIpcEndpointSimpleImpl>(socketMock);
    auto message = std::make_shared<LinxMessageIpc>(10);

    EXPECT_CALL(*socketMock, receive(_, _, _)).WillOnce(
        Invoke([&message](LinxMessageIpcPtr *msg, std::string *from, int timeout){
            *msg = message;
            *from = "TEST";
            return 0;
        }));


    ASSERT_EQ(endpoint->receive(10000, LINX_ANY_SIG, LINX_ANY_FROM), message);
}

TEST_F(LinxIpcEndpointSimpleTests, receive_ReturnNullWhenSignalNotMatch) {
    auto endpoint = std::make_shared<LinxIpcEndpointSimpleImpl>(socketMock);
    auto sigsel = std::initializer_list<uint32_t>{4};

    EXPECT_CALL(*socketMock, receive(_, _, _)).WillOnce(
        Invoke([](LinxMessageIpcPtr *msg, std::string *from, int timeout){
            *msg = std::make_shared<LinxMessageIpc>(10);
            *from = "TEST";
            return 0;
        }));


    ASSERT_EQ(endpoint->receive(10000, sigsel, LINX_ANY_FROM), nullptr);
}

TEST_F(LinxIpcEndpointSimpleTests, receive_ReturnMsgWhenSignalMatch) {
    auto endpoint = std::make_shared<LinxIpcEndpointSimpleImpl>(socketMock);
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

TEST_F(LinxIpcEndpointSimpleTests, receive_ReturnNullWhenClientNotMatch) {
    auto endpoint = std::make_shared<LinxIpcEndpointSimpleImpl>(socketMock);

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

TEST_F(LinxIpcEndpointSimpleTests, receive_ReturnMsgWhenClientMatch) {
    auto endpoint = std::make_shared<LinxIpcEndpointSimpleImpl>(socketMock);
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

TEST_F(LinxIpcEndpointSimpleTests, receive_ReturnMsgWhenSignalClientMatch) {
    auto endpoint = std::make_shared<LinxIpcEndpointSimpleImpl>(socketMock);
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
