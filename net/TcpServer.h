#ifndef TCPSERVER_H
#define TCPSERVER_H

#include "EventLoop.h"
#include "InetAddress.h"
#include "Acceptor.h"
#include "EventLoopThreadPool.h"
#include "TcpConnection.h"
#include "Buffer.h"
#include "Callbacks.h"
#include "../nocopyable.h"

#include <functional>
#include <memory>
#include <atomic>
#include <unordered_map>
#include <string>

class TcpServer : nocopyable
{
public:
    using ThreadInitCallback = std::function<void(EventLoop*)>;

    enum Option
    {
        kNoReusePort,
        kReusePort,
    };
    
    TcpServer(EventLoop* loop, 
                const InetAddress &listenAddr, 
                const std::string &nameArg,
                Option option = kNoReusePort);
    ~TcpServer();

    void setThreadInitCallback(const ThreadInitCallback &cb) { threadInitCallback_ = cb; }
    void setConnectionCallback(const ConnectionCallback &cb) { connectionCallback_ = cb; }
    void setMessageCallback(const MessageCallback &cb) { messageCallback_ = cb; }
    void setWriteComplateCallback(const WriteCompleteCallback &cb) { writeCompleteCallback_ = cb; }

    void setThreadNum(int numThreads); // subloop 数量

    void start();
private:
    void newConnection(int sockfd, const InetAddress &peerAddr);
    void removeConnection(const TcpConnectionPtr &conn);
    void removeConnectionInLoop(const TcpConnectionPtr &conn);

    using ConnectionMap = std::unordered_map<std::string, TcpConnectionPtr>;

    EventLoop* loop_; // client 定义的 baseLoop

    const std::string ipPort_;
    const std::string name_;

    std::unique_ptr<Acceptor> acceptor_; // 运行在 mainLoop

    std::unique_ptr<EventLoopThreadPool> threadPool_; // one loop peer thread

    ConnectionCallback connectionCallback_; // 有新连接时的回调
    MessageCallback messageCallback_; // 有读写消息时的回调
    WriteCompleteCallback writeCompleteCallback_; // 消息完成后的回调

    ThreadInitCallback threadInitCallback_;

    std::atomic_int started_;

    int nextConnId_;
    ConnectionMap connections_;
};

#endif