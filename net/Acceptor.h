#ifndef ACCEPTOR_H
#define ACCEPTOR_H

#include "Socket.h"
#include "Channel.h"
#include "../nocopyable.h"

#include <functional>

class EventLoop;
class InetAddress;

class Acceptor : nocopyable
{
public:
    using NewConnectionCallback = std::function<void(int sockfd, const InetAddress&)>;
    
    Acceptor(EventLoop* loop, const InetAddress& listenAddr, bool reuseport);
    ~Acceptor();

    void setNewConnectionCallback(const NewConnectionCallback& cb)
    { newConnectionCallback_ = cb; }

    void listen();

    bool listening() const { return listening_; }
private:
    void handleRead();

    EventLoop* loop_; // baseLoop (main loop)
    Socket acceptSocket_;
    Channel acceptChannel_;
    NewConnectionCallback newConnectionCallback_;
    bool listening_;
    int idleFd_;
};

#endif