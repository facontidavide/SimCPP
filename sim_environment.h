#ifndef SIM_ENVIRONMENT_H
#define SIM_ENVIRONMENT_H

#include <functional>
#include <vector>
#include <queue>
#include <limits>
#include "coroutine.h"

namespace Sim{

enum class EventType{
    TIMEOUT,
    RESOURCE_READY
};

class Event
{
public:
    Event(){
     _coro = co_self();
    }
    virtual ~Event() = default;

    stCoRoutine_t* coro() const{
        return _coro;
    }

    bool is_ready() const { return 0; }

    virtual EventType type() const = 0;

private:
    stCoRoutine_t* _coro;
};

class TimeoutEvent: public Event
{
public:
    TimeoutEvent(double timeout): _deadline(timeout){}

    virtual EventType type() const { return EventType::TIMEOUT; }

    double deadline() const { return _deadline; }

private:

    double _deadline;
};

class ResourceAvailableEvent: public Event
{
public:
    virtual EventType type() const { return EventType::RESOURCE_READY; }

};


class Environment
{
public:
    Environment();

    double now() const;

    using Callable = std::function<void()>;

    void addProcess(const Callable &function);

    TimeoutEvent* timeout(double time);

    void run(double timeout = std::numeric_limits<double>::infinity() );

    void wait(Event *event);

    void wait_any(std::initializer_list<Event> events);

private:
    double _now;

    struct Process
    {
        ulong ID;
        coro::routine_t coro;
    };

    std::vector<Process> _process;

    ulong _last_UID;

    struct CompareTimeout
    {
      bool operator() (const TimeoutEvent* a,
                       const TimeoutEvent* b)
        {
          return a->deadline() > b->deadline();
        }
    };

    std::priority_queue<TimeoutEvent*, std::vector<TimeoutEvent*>, CompareTimeout> _timeout_queue;
};

class Resource
{
public:
    Resource(Environment* env, int max_concurrent_request);

    ResourceAvailableEvent request();

};

}

#endif // SIM_ENVIRONMENT_H
