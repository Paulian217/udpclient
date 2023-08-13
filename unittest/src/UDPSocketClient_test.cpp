#include <Log.h>
#include <UDPSocketClient.h>
#include <arpa/inet.h>
#include <gmock/gmock.h>
#include <gtest/gtest.h>
#include <sys/socket.h>
#include <sys/types.h>

#include <thread>

constexpr const char* LOCALHOST = "127.0.0.1";
constexpr const int PORT = 8083;
constexpr const size_t MAX_MESSAGES = 5000;

class MockSocketListener : public ISocketListener {
public:
    MOCK_METHOD(void, onBufferReceived, (const struct sockaddr_in&, const std::vector<unsigned char>&), (override));
};

class UDPSocketClientTest : public ::testing::Test {
protected:
    void SetUp() override {
        // create udp server
        mSocketFD = socket(PF_INET, SOCK_DGRAM, IPPROTO_UDP);

        struct sockaddr_in sockaddrin;
        memset(&sockaddrin, sizeof(sockaddrin), 0);
        sockaddrin.sin_family = AF_INET;
        sockaddrin.sin_addr.s_addr = htonl(INADDR_ANY);
        sockaddrin.sin_port = htons(PORT);

        (void)bind(mSocketFD, (const sockaddr*)&sockaddrin, sizeof(sockaddrin));

        mSocketThread = std::thread([this]() {
            static int i = 0;
            while (!mQuit.load()) {
                std::vector<unsigned char> buffer(65535, 0);
                sockaddr_in sockaddrin;
                socklen_t sockaddrlen = sizeof(sockaddr_in);

                auto recvsize = recvfrom(mSocketFD, buffer.data(), buffer.size(), MSG_DONTWAIT, (sockaddr*)&sockaddrin, &sockaddrlen);
                if (recvsize > 0) {
                    buffer.resize(recvsize);
                    sendto(mSocketFD, buffer.data(), buffer.size(), SOCKET_NO_OPTION, (sockaddr*)&sockaddrin, sockaddrlen);
                } else {
                    uint8_t error = 0;
                    socklen_t socklen = sizeof(error);
                    getsockopt(mSocketFD, SOL_SOCKET, SO_ERROR, &error, &socklen);
                }
            }
        });
    }

    void TearDown() {
        mQuit.store(true);
        close(mSocketFD);
        if (mSocketThread.joinable()) {
            mSocketThread.join();
        }
    }

protected:
    int mSocketFD;
    std::thread mSocketThread;
    std::atomic_bool mQuit{false};
    std::mutex mMutex;
    std::condition_variable mCondition;
};

TEST_F(UDPSocketClientTest, Connect) {
    SocketFD socketfd = -1;
    UDPSocketClient client(socketfd);

    SocketAddress addr;
    memset(&addr, sizeof(addr), 0);
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = inet_addr(LOCALHOST);
    addr.sin_port = htons(PORT);

    EXPECT_EQ(SocketReturn::SUCCESS, client.Connect(addr));
    EXPECT_EQ(SocketReturn::SUCCESS, client.Disconnect());
}

TEST_F(UDPSocketClientTest, SendAndReceive) {
    SocketFD socketfd = -1;
    std::mutex mutex;
    std::condition_variable condition;
    ByteBuffer received;

    auto listener = std::make_shared<MockSocketListener>();
    ON_CALL(*listener, onBufferReceived(::testing::_, ::testing::_)).WillByDefault([&](const auto& addr, const auto& buffer) {
        std::unique_lock<std::mutex> lock(mutex);
        LOGV("socket(%d) Received: 0x%x, buffer.size(): %lu", socketfd, buffer[0], received.size());
        std::copy(buffer.begin(), buffer.end(), std::back_inserter(received));
        condition.notify_one();
    });
    EXPECT_CALL(*listener, onBufferReceived(::testing::_, ::testing::_)).Times(::testing::AtLeast(1));

    UDPSocketClient client(socketfd);

    SocketAddress addr;
    memset(&addr, sizeof(addr), 0);
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = inet_addr("127.0.0.1");
    addr.sin_port = htons(PORT);

    auto ret = client.Connect(addr, listener);
    EXPECT_EQ(ret, SocketReturn::SUCCESS);

    size_t totalsend = 0;
    LOGV("socket(%d) starts to send messsages: %lu", socketfd, MAX_MESSAGES);
    for (auto i = 0; i < MAX_MESSAGES; i++) {
        ByteBuffer buffer;
        buffer.push_back(i);
        totalsend += client.Send(buffer);
    }

    std::unique_lock<std::mutex> lock(mutex);
    auto success = condition.wait_for(lock, std::chrono::seconds(3), [&]() { return received.size() == MAX_MESSAGES; });
    LOGV("socket(%d) has received messsages: %lu", socketfd, received.size());
    EXPECT_EQ(received.size(), MAX_MESSAGES);
}
