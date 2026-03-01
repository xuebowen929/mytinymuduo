#ifndef POLLER_H
#define POLLER_H

#include "../base/Timestamp.h"

#include <vector>
#include <unordered_map>

class EventLoop;
class Channel;

class Poller : nocopyable
{
public:
    using ChannelList = std::vector<Channel*>;

    Poller(EventLoop* loop);
    virtual ~Poller();

    // 保留给IO复用的统一接口
    virtual Timestamp poll(int timeoutMs, ChannelList* activateChannels) = 0;
    virtual void updateChannel(Channel* channel) = 0;
    virtual void removeChannel(Channel* channel) = 0;
    // Channel 是否在当前 Poller 中
    virtual bool hasChannel(Channel* channel) const;

    static Poller* newDefaultPoller(EventLoop* loop);
protected:
    // key: sockfd, value: sockfd 所属的 Channel
    using ChannelMap = std::unordered_map<int, Channel*>;
    ChannelMap channels_;
private:
    EventLoop* ownerLoop_; // 当前 Poller 所属的 EventLoop
};

#endif