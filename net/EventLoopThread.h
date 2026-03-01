#ifndef EVENTLOOPTHREAD_H
#define EVENTLOOPTHREAD_H

#include "../base/Thread.h"

#include <mutex>
#include <condition_variable>
#include <functional>
#include <string>

class EventLoop;

class EventLoopThread : nocopyable
{
public:
    using ThreadInitCallback = std::function<void(EventLoop*)>;
    EventLoopThread(const ThreadInitCallback &cb = ThreadInitCallback(), 
                    const std::string &name = std::string());
    ~EventLoopThread();

    EventLoop* startLoop();
private:
    void threadFunc();

    EventLoop* loop_;
    Thread thread_;
    bool exiting_;
    std::mutex mutex_;
    std::condition_variable cond_;
    ThreadInitCallback callback_;
};

#endif