#ifndef SIM_ENVIRONMENT_H
#define SIM_ENVIRONMENT_H

#include <functional>
#include <vector>
#include <queue>
#include <limits>
#include <list>
#include <memory>
#include "coroutine.h"

namespace Sim{

enum class EventType{
    TIMEOUT,
    RESOURCE_READY,
    COMPOSITE
};

class Event
{
public:
    Event(): _ready(false), _cancelled(false)
    {
        _coro = co_self();
    }
    virtual ~Event()
    {
        _cancelled = true;
    }

    stCoRoutine_t* coro() const{
        return _coro;
    }

    bool ready() const { return _ready; }
    bool cancelled() const { return _cancelled; }

    virtual EventType type() const = 0;

private:

    friend class Resource;
    friend class Environment;

    stCoRoutine_t* _coro;
    bool _ready;
    bool _cancelled;
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

class Resource;

class ResourceEvent: public Event
{
public:
    ResourceEvent(Resource* parent): _parent(parent) {}
    ~ResourceEvent() { release(); }

    void release();

    virtual EventType type() const { return EventType::RESOURCE_READY; }
private:
    Resource* _parent;
};

typedef std::unique_ptr<Event, void(*)(Event*)> EventPtr;


class OrEvent:  public Event
{
public:

    OrEvent( Event* a, Event* b);

    virtual EventType type() const { return EventType::COMPOSITE; }
};


class Environment
{
public:
    Environment();

    double now() const;

    using Callable = std::function<void()>;

    void addProcess(const Callable &function);

    EventPtr timeout(double time);

    void run(double timeout = std::numeric_limits<double>::infinity() );

    void wait(EventPtr& event);

    void wait_any(std::initializer_list<Event*> events);

private:
    double _now;

    struct Process
    {
        ulong ID;
        coro::routine_t coro;
    };

    std::list<Process> _process;

    ulong _last_UID;

    bool _running;

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

    EventPtr request();

private:

    friend class ResourceEvent;

    void release(ResourceEvent* event);

    Environment* _env;
    unsigned _max_slots;
    unsigned _used_slots_count;
    std::deque<ResourceEvent*> _pending_events;
    std::list<ResourceEvent*> _memory_pool;
    ResourceEvent* newEvent();
};

}

#endif // SIM_ENVIRONMENT_H
