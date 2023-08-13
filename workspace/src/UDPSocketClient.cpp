#include <Log.h>
#include <UDPSocketClient.h>
#include <sys/socket.h>
#include <unistd.h>

#include <future>
#include <iostream>

#ifdef EOF
#undef EOF
#endif

UDPSocketClient::UDPSocketClient(SocketFD& socketfd) : mSocketFD(-1), mStatus(SocketStatus::IDLE), mSocketAddress(), mSocketListener(), mThread() {
    mSocketFD = socket(PF_INET, SOCK_DGRAM, IPPROTO_UDP);
    socketfd = mSocketFD;
}

UDPSocketClient::~UDPSocketClient() { (void)Disconnect(); }

SocketReturn UDPSocketClient::Connect(const SocketAddress& address, const std::shared_ptr<ISocketListener>& listener) {
    mSocketAddress = address;
    mSocketListener = listener;

    LOGT("socket(%d) Connecting...", mSocketFD);
    mStatus.store(SocketStatus::CONNECTED);
    LOGD("socket(%d) Connected!", mSocketFD);
    mThread = std::thread(
        [&](const SocketFD socketfd) {
            mStatus.store(SocketStatus::RUNNING);
            do {
                SocketReturn result = ReceiveAndForward(socketfd);
                if (result == SocketReturn::EOF) {
                    break;
                }
            } while (mStatus.load() == SocketStatus::RUNNING);
            mStatus.store(SocketStatus::IDLE);
        },
        mSocketFD);

    /* Wait for Monitor thrad started ... */
    std::this_thread::sleep_for(std::chrono::microseconds(100));
    return SocketReturn::SUCCESS;
}

SocketSize UDPSocketClient::Send(const ByteBuffer& buffer) {
    if (SocketStatus::RUNNING != mStatus.load()) {
        return 0;
    }
    auto sendsize = std::async(std::launch::async, sendto, mSocketFD, buffer.data(), buffer.size(), MSG_CONFIRM,
                               (const struct sockaddr*)&mSocketAddress, sizeof(mSocketAddress))
                        .get();
    return static_cast<size_t>(sendsize < 0 ? 0 : sendsize);
}

SocketReturn UDPSocketClient::Disconnect() {
    switch (mStatus.load()) {
        case SocketStatus::CONNECTING:
        case SocketStatus::CONNECTED:
            close(mSocketFD);
            mStatus.store(SocketStatus::IDLE);
            mSocketFD = INVALID_SOCKET;
            break;
        case SocketStatus::RUNNING:
            LOGT("socket(%d) Disconnecting...", mSocketFD);
            shutdown(mSocketFD, SHUT_RDWR);
            mStatus.store(SocketStatus::DISCONNECTING);
            if (mThread.joinable()) {
                mThread.join();
            }
            mSocketFD = INVALID_SOCKET;
            mStatus.store(SocketStatus::IDLE);
            LOGD("socket(%d) Disconnected!", mSocketFD);
            break;
        case SocketStatus::DISCONNECTING:
            LOGT("socket(%d) Disconnecting...", mSocketFD);
            break;
    }
    return SocketReturn::SUCCESS;
}

SocketReturn UDPSocketClient::ReceiveAndForward(const SocketFD& socketfd) {
    SocketAddress addr;
    socklen_t addrlen = sizeof(addr);
    ByteBuffer buffer(MAX_BUFFER, 0);

    // LOGD("socket(%d) Wait for recvfrom", socketfd);
    auto recvsize = recvfrom(socketfd, buffer.data(), buffer.size(), 0, (struct sockaddr*)&addr, &addrlen);
    // LOGD("socket(%d) Wakeup from recvfrom: %ld", socketfd, recvsize);

    if (SOCKET_EOF == recvsize) {
        buffer.resize(0);
        return SocketReturn::EOF;
    } else if (SOCKET_ERROR == recvsize) {
        buffer.resize(0);
        return SocketReturn::FAILED;
    }

    buffer.resize(recvsize);
    if (mSocketListener != nullptr) {
        mSocketListener->onBufferReceived(addr, buffer);
    }
    return SocketReturn::SUCCESS;
}
