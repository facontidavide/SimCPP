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
    RESOURCE_READY
};

class Event
{
public:
    Event(): _ready(false)
    {
        _coro = co_self();
    }
    virtual ~Event() = default;

    stCoRoutine_t* coro() const{
        return _coro;
    }

    bool ready() const { return _ready; }


    virtual EventType type() const = 0;

private:

    friend class Resource;
    friend class Environment;

    stCoRoutine_t* _coro;
    bool _ready;
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

typedef std::unique_ptr<ResourceEvent, void(*)(ResourceEvent*)> ResourceEventPtr;



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

    template <typename Ev, typename T>
    void wait(std::unique_ptr<Ev, T>& event)
    {
        static_assert( std::is_base_of<Event,Ev>::value, "Not an Event");
        wait( event.get() );
    }

    void wait_any(std::initializer_list<Event> events);

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

    ResourceEventPtr request();

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
