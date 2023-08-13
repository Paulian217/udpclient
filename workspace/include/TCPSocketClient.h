#ifndef WORKSPACE_INCLUDE_TCPSOCKETCLIENT_H
#define WORKSPACE_INCLUDE_TCPSOCKETCLIENT_H

#include <ISocketListener.h>
#include <SocketTypes.h>

#include <atomic>
#include <functional>
#include <thread>

class TCPSocketClient {
public:
    explicit TCPSocketClient(SocketFD& socketfd);
    virtual ~TCPSocketClient();
    SocketReturn Connect(const SocketAddress& address, const std::shared_ptr<ISocketListener>& listener = nullptr);
    SocketSize Send(const ByteBuffer& buffer);
    SocketReturn Disconnect();

private:
    SocketReturn ReceiveAndForward(const SocketFD& socketfd);
    void onBufferReceived(const SocketAddress& address, const ByteBuffer& buffer);

private:
    SocketFD mSocketFD;
    SocketFD mConnSocketFD;
    std::atomic<SocketStatus> mStatus;
    SocketAddress mSocketAddress;
    std::shared_ptr<ISocketListener> mSocketListener;
    std::thread mThread;
};

#endif  // WORKSPACE_INCLUDE_TCPSOCKETCLIENT_H