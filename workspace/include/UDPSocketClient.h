#ifndef DAY_1_WORKSPACE_INCLUDE_UDPSOCKETCLIENT_H
#define DAY_1_WORKSPACE_INCLUDE_UDPSOCKETCLIENT_H

#include <ISocketListener.h>

#include <atomic>
#include <functional>
#include <thread>

using Thread = std::thread;
using SocketFD = int;
using SocketSize = size_t;
using SocketAddress = struct sockaddr_in;
using ByteBuffer = std::vector<unsigned char>;
using SocketReceiver = std::function<void(const SocketAddress& addr, const ByteBuffer& buffer)>;

enum class SocketReturn { SUCCESS, FAILED, EOF, MAX };
enum class SocketStatus { IDLE, CONNECTING, CONNECTED, DISCONNECTING, MAX };

class UDPSocketClient {
public:
    explicit UDPSocketClient(SocketFD& socketfd);
    virtual ~UDPSocketClient();
    SocketReturn Connect(const SocketAddress& address, const std::shared_ptr<ISocketListener>& listener = nullptr);
    SocketSize Send(const ByteBuffer& buffer);
    SocketReturn Disconnect();

private:
    SocketReturn ReceiveAndForward(const SocketFD& socketfd);
    void onBufferReceived(const SocketAddress& address, const ByteBuffer& buffer);

private:
    SocketFD mSocketFD;
    std::atomic<SocketStatus> mStatus;
    SocketAddress mSocketAddress;
    std::shared_ptr<ISocketListener> mSocketListener;
    std::vector<ByteBuffer> mCacheBuffer;
    Thread mThread;
};

#endif  // DAY_1_WORKSPACE_INCLUDE_UDPSOCKETCLIENT_H