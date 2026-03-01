#include "EventLoop.h"
#include "../base/Logger.h"
#include "Poller.h"
#include "Channel.h"

#include <sys/eventfd.h>
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>
#include <memory>

// 防止一个线程创建多个 EventLoop
__thread EventLoop* t_loopInThisThread = 0;

// 默认 Poll 接口超时时间
const int kPollTimeMs = 10000;

// 创建 wakefd, 通知 subReactor 处理新来的 Channel
int createEventfd() 
{
    int evtfd = ::eventfd(0, EFD_NONBLOCK | EFD_CLOEXEC);
    if (evtfd < 0) 
    {
        LOG_FATAL("evtfd error:%d \n", errno);
    }
    return evtfd;
}

EventLoop::EventLoop()
    : looping_(false)
    , quit_(false)
    , threadId_(CurrentThread::tid())
    , poller_(Poller::newDefaultPoller(this))
    , wakeupFd_(createEventfd())
    , wakeupChannel_(new Channel(this, wakeupFd_))
    , callingPendingFunctors_(false)
{
    LOG_DEBUG("EventLoop created %p in thread %d \n", this, threadId_);
    if (t_loopInThisThread == 0)
    {
        LOG_FATAL("Another EventLoop %p exists in thread %d \n", t_loopInThisThread, threadId_);
    } 
    else 
    {
        t_loopInThisThread = this;
    }

    // main Reactor 通过通知各个 EventLoop 的 wakeupFd_ 来唤醒各个 EventLoop
    wakeupChannel_->setReadCallback(std::bind(&EventLoop::handleRead, this));
    wakeupChannel_->enableReading();
}

EventLoop::~EventLoop()
{
    wakeupChannel_->disableAll();
    wakeupChannel_->remove();
    ::close(wakeupFd_);
    t_loopInThisThread = nullptr;
}

void EventLoop::loop()
{
    looping_ = true;
    quit_ = false;
    LOG_INFO("EventLoop %p start looping", this);

    while(!quit_)
    {
        activateChannels_.clear();
        pollReturnTime_ = poller_->poll(kPollTimeMs, &activateChannels_);
        for (Channel* channel : activateChannels_)
        {
            // Poll 监听到哪些 channel 发生事件了，上报给EventLoop, 通知 channel 处理相应的事件
            channel->handleEvent(pollReturnTime_);
        }
        // 执行当前 EventLoop 所需执行的回调操作
        doPendingFunctors();
    }

    LOG_INFO("EventLoop %p stop looping", this);
    quit_ = false;
}

void EventLoop::quit()
{
    quit_ = true;
    // 在别的loop中调用当前loop的quit时
    if (!isInLoopThread()) 
    {
        wakeup();
    }
}

void EventLoop::runInLoop(Functor cb)
{
    if (isInLoopThread())
    {
        cb();
    }
    else 
    {
        queueInLoop(cb);
    }
}
   
void EventLoop::queueInLoop(Functor cb)
{
    {
        std::unique_lock<std::mutex> lock(mutex_);
        pendingFunctors_.emplace_back(cb);
    }
    if (!isInLoopThread() || callingPendingFunctors_)
    {
        wakeup();
    }
}

// 用来唤醒当前loop所在线程，向wakeupfd_写一个数据，wakeupChannel就会发生读事件，当前loop就会被唤醒
void EventLoop::wakeup()
{
    uint64_t one = 1;
    ssize_t n = write(wakeupFd_, &one, sizeof one);
    if (n != sizeof one)
    {
        LOG_ERROR("EventLoop::wakeup() writes %lu bytes instead of 8", n);
    }
}

void EventLoop::updateChannel(Channel* channel)
{
    poller_->updateChannel(channel);
}

void EventLoop::removeChannel(Channel* channel)
{
    poller_->removeChannel(channel);
}

bool EventLoop::hasChannel(Channel* channel)
{
    return poller_->hasChannel(channel);
}

void EventLoop::handleRead()
{
    uint64_t one = 1;
    ssize_t n = read(wakeupFd_, &one, sizeof one);
    if (n != sizeof one)
    {
        LOG_ERROR("EventLoop::handleRead() reads %lu bytes instead of 8 \n", n);
    }
}

void EventLoop::doPendingFunctors()
{
    std::vector<Functor> functors;
    callingPendingFunctors_ = true;

    // 减小锁的粒度，在subReactor执行回调操作时，不影响mainReactor继续添加需要执行的回调操作
    {
        std::unique_lock<std::mutex> lock(mutex_);
        functors.swap(pendingFunctors_);
    }
    for (const Functor& functor : functors)
    {
        functor();
    }
    callingPendingFunctors_ = false;
}