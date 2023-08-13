#include <Log.h>
#include <TCPSocketClient.h>
#include <arpa/inet.h>
#include <gmock/gmock.h>
#include <gtest/gtest.h>
#include <sys/socket.h>
#include <sys/types.h>

#include <thread>

constexpr const char* LOCALHOST = "127.0.0.1";
constexpr const int PORT = 8083;
constexpr const size_t MAX_MESSAGES = 100;

class MockSocketListener : public ISocketListener {
public:
    MOCK_METHOD(void, onBufferReceived, (const struct sockaddr_in&, const std::vector<unsigned char>&), (override));
};

class TCPSocketClientTest : public ::testing::Test {
protected:
    void SetUp() override {
        // create udp server
        mSocketFD = socket(PF_INET, SOCK_STREAM, IPPROTO_TCP);

        struct sockaddr_in sockaddrin;
        memset(&sockaddrin, sizeof(sockaddrin), 0);
        sockaddrin.sin_family = AF_INET;
        sockaddrin.sin_addr.s_addr = htonl(INADDR_ANY);
        sockaddrin.sin_port = htons(PORT);

        (void)bind(mSocketFD, (const sockaddr*)&sockaddrin, sizeof(sockaddrin));
        (void)listen(mSocketFD, UINT8_MAX);

        mSocketThread = std::thread([this]() {
            sockaddr_in sockaddrin;
            socklen_t sockaddrlen = sizeof(sockaddr_in);

            LOGV("socket(%d) Wait for accept...", mSocketFD);
            auto connsockfd = accept(mSocketFD, (sockaddr*)&sockaddrin, &sockaddrlen);
            if (connsockfd <= 0) {
                LOGV("socket(%d) Shutdown!", mSocketFD);
                return;
            }
            LOGV("socket(%d) Accepted! connsockfd: %d", mSocketFD, connsockfd);

            while (!mQuit.load()) {
                // LOGV("socket(%d) Wait for read from socketfd...", mSocketFD, connsockfd);
                std::vector<unsigned char> buffer(65535, 0);
                auto readsize = read(connsockfd, buffer.data(), buffer.size());
                if (SOCKET_EOF == readsize) {
                    LOGV("socket(%d) Awake! EOF", connsockfd);
                    break;
                } else if (SOCKET_ERROR == readsize) {
                    LOGW("socket(%d) Awake! Failed to read", connsockfd);
                    continue;
                }
                // LOGV("socket(%d) Awake! Read buffer: %lu, front: %x", connsockfd, readsize, buffer.front());
                buffer.resize(readsize);
                write(connsockfd, buffer.data(), buffer.size());
            }
            close(connsockfd);
        });
    }

    void TearDown() {
        shutdown(mSocketFD, SHUT_RDWR);
        mQuit.store(true);
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

TEST_F(TCPSocketClientTest, Connect) {
    SocketFD socketfd = -1;
    TCPSocketClient client(socketfd);

    SocketAddress addr;
    memset(&addr, sizeof(addr), 0);
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = inet_addr(LOCALHOST);
    addr.sin_port = htons(PORT);

    EXPECT_EQ(SocketReturn::SUCCESS, client.Connect(addr));
    EXPECT_EQ(SocketReturn::SUCCESS, client.Disconnect());
}

TEST_F(TCPSocketClientTest, SendAndReceive) {
    SocketFD socketfd = -1;
    std::mutex mutex;
    std::condition_variable condition;
    ByteBuffer received;

    auto listener = std::make_shared<MockSocketListener>();
    ON_CALL(*listener, onBufferReceived(::testing::_, ::testing::_)).WillByDefault([&](const auto& addr, const auto& buffer) {
        std::unique_lock<std::mutex> lock(mutex);
        LOGV("socket(%d) Received buffer.size(): %lu", socketfd, received.size());
        std::copy(buffer.begin(), buffer.end(), std::back_inserter(received));
        condition.notify_one();
    });
    EXPECT_CALL(*listener, onBufferReceived(::testing::_, ::testing::_)).Times(::testing::AtLeast(1));

    TCPSocketClient client(socketfd);

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
