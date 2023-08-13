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
    mStatus.store(SocketStatus::CONNECTING);
    mThread = std::thread(
        [&](const SocketFD socketfd) {
            do {
                SocketReturn result = ReceiveAndForward(socketfd);
                if (result == SocketReturn::EOF) {
                    break;
                }
            } while (mStatus.load() == SocketStatus::CONNECTED);
            mStatus.store(SocketStatus::IDLE);
        },
        mSocketFD);

    /* Wait for Monitor thrad started ... */
    std::this_thread::sleep_for(std::chrono::microseconds(100));

    mStatus.store(SocketStatus::CONNECTED);
    LOGD("socket(%d) Connected!", mSocketFD);
    return SocketReturn::SUCCESS;
}

SocketSize UDPSocketClient::Send(const ByteBuffer& buffer) {
    if (SocketStatus::CONNECTED != mStatus.load()) {
        return 0;
    }
    auto sendsize = std::async(std::launch::async, sendto, mSocketFD, buffer.data(), buffer.size(), MSG_CONFIRM,
                               (const struct sockaddr*)&mSocketAddress, sizeof(mSocketAddress))
                        .get();
    return static_cast<size_t>(sendsize < 0 ? 0 : sendsize);
}

SocketReturn UDPSocketClient::Disconnect() {
    if (mStatus.load() != SocketStatus::CONNECTED) {
        close(mSocketFD);
        mStatus.store(SocketStatus::IDLE);
        return SocketReturn::SUCCESS;
    }

    LOGT("socket(%d) Disconnecting...", mSocketFD);
    mStatus.store(SocketStatus::DISCONNECTING);
    shutdown(mSocketFD, SHUT_RDWR);
    if (mThread.joinable()) {
        mThread.join();
    }
    LOGD("socket(%d) Disconnected!", mSocketFD);
    mSocketFD = -1;
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
    onBufferReceived(addr, buffer);
    return SocketReturn::SUCCESS;
}

void UDPSocketClient::onBufferReceived(const SocketAddress& address, const ByteBuffer& buffer) {
    if (mSocketListener != nullptr) {
        mSocketListener->onBufferReceived(address, buffer);
    }
}
