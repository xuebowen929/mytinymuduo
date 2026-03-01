#ifndef EVENTLOOP_H
#define EVENTLOOP_H

#include "../base/Timestamp.h"
#include "../base/CurrentThread.h"
#include "../nocopyable.h"

#include <memory>
#include <functional>
#include <vector>
#include <atomic>
#include <mutex>

class Channel;
class Poller;

class EventLoop : nocopyable
{
public:
    using Functor = std::function<void()>;

    EventLoop();
    ~EventLoop();

    void loop();
    void quit();

    Timestamp pollReturnTime() const { return pollReturnTime_; }

    // 在当前线程执行回调
    void runInLoop(Functor cb);
    // 放进队列，在 EventLoop 所在线程执行回调
    void queueInLoop(Functor cb);

    void wakeup();

    // EventLoop 方法 =》 Poll 方法
    void updateChannel(Channel* channel);
    void removeChannel(Channel* channel);
    bool hasChannel(Channel* channel);

    // 判断 EventLoop 是否在当前线程
    bool isInLoopThread() const { return threadId_ == CurrentThread::tid(); }
private:
    void handleRead();
    void doPendingFunctors();

    using ChannelList = std::vector<Channel*>;
    ChannelList activateChannels_;

    std::atomic_bool looping_;
    std::atomic_bool quit_;

    const pid_t threadId_; // 当前 loop 所在线程

    Timestamp pollReturnTime_; // Poller 返回事件Channels的时间点
    std::unique_ptr<Poller> poller_;

    int wakeupFd_; // muduo库mainloop与subloop之间用wakeupfd, 没有用队列
    std::unique_ptr<Channel> wakeupChannel_;

    std::atomic_bool callingPendingFunctors_; // 标识当前 loop 是否有需要执行的回调操作
    std::vector<Functor> pendingFunctors_; // 存储 loop 所需要执行的所有回调操作

    std::mutex mutex_;
};

#endif