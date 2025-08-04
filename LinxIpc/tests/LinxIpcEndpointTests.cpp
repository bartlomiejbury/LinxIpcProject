
#include <stdio.h>
#include "gtest/gtest.h"
#include "LinxIpcQueueMock.h"
#include "LinxIpcSocketMock.h"
#include "LinxIpcClientMock.h"
#include "LinxIpc.h"
#include "LinxIpcServerImpl.h"
#include "LinxIpcPrivate.h"

using namespace ::testing;

class LinxIpcEndpointTests : public testing::Test {
  public:
    NiceMock<LinxIpcSocketMock> *socketMock;
    std::shared_ptr<NiceMock<LinxIpcClientMock>> clientMock = std::make_shared<NiceMock<LinxIpcClientMock>>();

    void SetUp() {
        socketMock = new NiceMock<LinxIpcSocketMock>();

        ON_CALL(*socketMock, getName()).WillByDefault(Return("TEST"));
        ON_CALL(*(clientMock.get()), getName()).WillByDefault(Return("TEST"));

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

TEST_F(LinxIpcEndpointTests, sendReturnSocketSendResult) {
    auto endpoint = std::make_shared<LinxIpcEndpointImpl>(socketMock);
    auto msg = LinxMessageIpc(10);

    EXPECT_CALL(*socketMock, send(_, _)).WillOnce(Return(1));
    ASSERT_EQ(endpoint->send(msg, clientMock), 1);
}

TEST_F(LinxIpcEndpointTests, receive_callSocketSend) {
    auto endpoint = std::make_shared<LinxIpcEndpointImpl>(socketMock);
    auto sigsel = std::initializer_list<uint32_t>{4};

    EXPECT_CALL(*socketMock, receive(_, _, 10000));
    endpoint->receive(10000, sigsel, LINX_ANY_FROM);
}

TEST_F(LinxIpcEndpointTests, receive_ReturnNullWhenSocketReturnError) {
    auto endpoint = std::make_shared<LinxIpcEndpointImpl>(socketMock);
    auto sigsel = std::initializer_list<uint32_t>{4};
    EXPECT_CALL(*socketMock, receive(_, _, _)).WillOnce(Return(-1));

    ASSERT_EQ(endpoint->receive(10000, sigsel, LINX_ANY_FROM), nullptr);
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


    ASSERT_EQ(endpoint->receive(10000, sigsel, LINX_ANY_FROM), message);
}

TEST_F(LinxIpcEndpointTests, receive_ReturnNullWhenClientNotMatch) {
    auto endpoint = std::make_shared<LinxIpcEndpointImpl>(socketMock);

    EXPECT_CALL(*socketMock, receive(_, _, _)).WillOnce(
        Invoke([](LinxMessageIpcPtr *msg, std::string *from, int timeout){
            *msg = std::make_shared<LinxMessageIpc>(10);
            *from = "TEST2";
            return 0;
        }));

    ASSERT_EQ(endpoint->receive(10000, LINX_ANY_SIG, clientMock), nullptr);
}

TEST_F(LinxIpcEndpointTests, receive_ReturnMsgWhenClientMatch) {
    auto endpoint = std::make_shared<LinxIpcEndpointImpl>(socketMock);
    auto message = std::make_shared<LinxMessageIpc>(10);

    EXPECT_CALL(*socketMock, receive(_, _, _)).WillOnce(
        Invoke([&message](LinxMessageIpcPtr *msg, std::string *from, int timeout){
            *msg = message;
            *from = "TEST";
            return 0;
        }));

    ASSERT_EQ(endpoint->receive(10000, LINX_ANY_SIG, clientMock), message);
}

TEST_F(LinxIpcEndpointTests, receive_ReturnMsgWhenSignalClientMatch) {
    auto endpoint = std::make_shared<LinxIpcEndpointImpl>(socketMock);
    auto message = std::make_shared<LinxMessageIpc>(10);
    auto sigsel = std::initializer_list<uint32_t>{10};

    EXPECT_CALL(*socketMock, receive(_, _, _)).WillOnce(
        Invoke([&message](LinxMessageIpcPtr *msg, std::string *from, int timeout){
            *msg = message;
            *from = "TEST";
            return 0;
        }));

    ASSERT_EQ(endpoint->receive(10000, sigsel, clientMock), message);
    ASSERT_NE(message->getClient(), nullptr);
    ASSERT_EQ(message->getClient()->getName(), "TEST");
}
