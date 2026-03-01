#include "EventLoopThreadPool.h"
#include "EventLoop.h"

#include <memory>

EventLoopThreadPool::EventLoopThreadPool(EventLoop* baseLoop, const std::string& nameArg)
    : baseLoop_(baseLoop)
    , name_(nameArg)
    , started_(false)
    , numThreads_(0)
    , next_(0)
{
}

EventLoopThreadPool::~EventLoopThreadPool()
{
}

void EventLoopThreadPool::start(const ThreadInitCallback& cb)
{
    started_ = true;

    for (int i = 0; i < numThreads_; i++)
    {
        char buf[name_.size() + 32];
        snprintf(buf, sizeof buf, "%s%d", name_.c_str(), i);
        EventLoopThread* t = new EventLoopThread(cb, buf);
        threads_.emplace_back(std::unique_ptr<EventLoopThread>(t));
        loops_.emplace_back(t->startLoop());
    }

    if (numThreads_ == 0)
    {
        cb(baseLoop_);
    }
}

EventLoop* EventLoopThreadPool::getNextLoop()
{
    EventLoop* loop = baseLoop_;

    // 轮询
    if (!threads_.empty())
    {
        loop = loops_[next_];
        next_ %= loops_.size();
    }

    return loop;
}

std::vector<EventLoop*> EventLoopThreadPool::getAllLoops()
{
    if (loops_.empty())
    {
        return std::vector<EventLoop*>(1, baseLoop_);
    }
    else
    {
        return loops_;
    }
}