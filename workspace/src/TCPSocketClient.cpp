#include <Log.h>
#include <TCPSocketClient.h>
#include <unistd.h>

#include <future>

#ifdef EOF
#undef EOF
#endif  // EOF

TCPSocketClient::TCPSocketClient(SocketFD& socketfd)
    : mSocketFD(INVALID_SOCKET), mConnSocketFD(INVALID_SOCKET), mStatus(SocketStatus::IDLE), mSocketAddress(), mSocketListener(), mThread() {
    mSocketFD = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    socketfd = mSocketFD;
}

TCPSocketClient::~TCPSocketClient() { Disconnect(); }

SocketReturn TCPSocketClient::Connect(const SocketAddress& address, const std::shared_ptr<ISocketListener>& listener) {
    mSocketAddress = address;
    mSocketListener = listener;

    LOGT("socket(%d) Connecting...", mSocketFD);
    mStatus.store(SocketStatus::CONNECTING);
    if (INVALID_SOCKET == connect(mSocketFD, (const sockaddr*)&address, sizeof(address))) {
        LOGE("Failed to connect socket");
        mStatus.store(SocketStatus::IDLE);
        return SocketReturn::FAILED;
    }
    mStatus.store(SocketStatus::CONNECTED);
    LOGT("socket(%d) Connected!...", mSocketFD);

    mThread = std::thread(
        [&](const SocketFD socketfd) {
            mStatus.store(SocketStatus::RUNNING);
            LOGT("socket(%d) Running!", socketfd);
            do {
                SocketReturn result = ReceiveAndForward(socketfd);
                if (result == SocketReturn::EOF) {
                    break;
                }
            } while (mStatus.load() == SocketStatus::RUNNING);
            mStatus.store(SocketStatus::IDLE);
        },
        mSocketFD);

    /* Wait for Handler thread started ... */
    usleep(10);
    return SocketReturn::SUCCESS;
}

SocketSize TCPSocketClient::Send(const ByteBuffer& buffer) {
    if (SocketStatus::RUNNING != mStatus.load()) {
        return 0;
    }
    auto sendsize = std::async(std::launch::async, sendto, mSocketFD, buffer.data(), buffer.size(), MSG_CONFIRM,
                               (const struct sockaddr*)&mSocketAddress, sizeof(mSocketAddress))
                        .get();
    return static_cast<size_t>(sendsize < 0 ? 0 : sendsize);
}

SocketReturn TCPSocketClient::Disconnect() {
    switch (mStatus.load()) {
        case SocketStatus::CONNECTING:
        case SocketStatus::CONNECTED:
            close(mSocketFD);
            mSocketFD = -1;
            mStatus.store(SocketStatus::IDLE);
            break;
        case SocketStatus::RUNNING:
            mStatus.store(SocketStatus::DISCONNECTING);
            shutdown(mSocketFD, SHUT_RDWR);
            if (mThread.joinable()) {
                mThread.join();
            }
            LOGD("socket(%d) Disconnected!", mSocketFD);
            mSocketFD = -1;
            mStatus.store(SocketStatus::IDLE);
            break;
        case SocketStatus::DISCONNECTING:
            LOGD("socket(%d) Still disconnecting...", mSocketFD);
            break;
        default:
            break;
    }
    return SocketReturn::SUCCESS;
}

SocketReturn TCPSocketClient::ReceiveAndForward(const SocketFD& socketfd) {
    ByteBuffer buffer(MAX_BUFFER, 0);

    // LOGI("socket(%d) Wait for recvfrom", socketfd);
    auto readsize = read(socketfd, buffer.data(), buffer.size());
    // LOGI("socket(%d) Wakeup from recvfrom: %ld", socketfd, readsize);

    if (INVALID_SOCKET == readsize) {
        buffer.resize(0);
        return SocketReturn::FAILED;
    } else if (SOCKET_EOF == readsize) {
        buffer.resize(0);
        return SocketReturn::EOF;
    }

    buffer.resize(readsize);
    onBufferReceived(mSocketAddress, buffer);
    return SocketReturn::SUCCESS;
}

void TCPSocketClient::onBufferReceived(const SocketAddress& address, const ByteBuffer& buffer) {
    if (mSocketListener != nullptr) {
        mSocketListener->onBufferReceived(address, buffer);
    }
}
