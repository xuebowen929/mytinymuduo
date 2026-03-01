#ifndef NOCOPYABLE_H
#define NOCOPYABLE_H

class nocopyable 
{
public:
    // nocopyable的派生类对象(TcpServer)不允许被复制
    nocopyable(const nocopyable&) = delete;
    void operator=(const nocopyable&) = delete;
protected:
    nocopyable() = default;
    ~nocopyable() = default;
};

#endif