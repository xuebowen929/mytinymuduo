#ifndef EVENTLOOPTHREADPOOL_H
#define EVENTLOOPTHREADPOOL_H

#include "EventLoop.h"
#include "EventLoopThread.h"
#include "../nocopyable.h"

#include <functional>
#include <vector>
#include <memory>
#include <string>

class EventLoopThreadPool : nocopyable
{
public:
    using ThreadInitCallback = std::function<void(EventLoop* loop)>;

    EventLoopThreadPool(EventLoop* baseLoop, const std::string& nameArg);
    ~EventLoopThreadPool();

    void setThreadNum(int numThreads) { numThreads_ = numThreads; }
    void start(const ThreadInitCallback& cb = ThreadInitCallback());

    EventLoop* getNextLoop();
    std::vector<EventLoop*> getAllLoops();

    EventLoop* getLoopForHash(size_t hashCode);

    bool started() const { return started_; }
    const std::string& name() const { return name_; }
private:
    EventLoop* baseLoop_;
    std::string name_;
    bool started_;
    int numThreads_;
    int next_;
    std::vector<std::unique_ptr<EventLoopThread>> threads_;
    std::vector<EventLoop*> loops_;
};

#endif