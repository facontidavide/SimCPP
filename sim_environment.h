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

    virtual bool ready() const { return _ready; }

    bool cancelled() const { return _cancelled; }

    void cancel() { _cancelled = true; }

    virtual EventType type() const = 0;

protected:

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

    EventType type() const override { return EventType::TIMEOUT; }

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

    EventType type() const override { return EventType::RESOURCE_READY; }
private:
    Resource* _parent;
};

typedef std::unique_ptr<Event, void(*)(Event*)> EventPtr;


class CompositeEvent:  public Event
{
public:
    CompositeEvent( Event& a, Event& b): _a(a), _b(b) {}
    ~CompositeEvent() = default;

    EventType type() const override { return EventType::COMPOSITE; }
protected:
    Event& _a;
    Event& _b;
};

class OrEvent: public CompositeEvent
{
public:
    OrEvent( Event& a, Event& b): CompositeEvent(a,b) {}

    bool ready() const override
    {
        bool done = _a.ready() | _b.ready();
        if( done )
        {
            if( !_a.ready() ) _a.cancel();
            if( !_b.ready() ) _b.cancel();
        }
        return done;
    }
};

class AndEvent: public CompositeEvent
{
public:
    AndEvent( Event& a, Event& b): CompositeEvent(a,b) {}

    bool ready() const override
    {
        return _a.ready() && _b.ready();
    }
};

inline OrEvent operator |( Event& a, Event& b ) { return OrEvent(a,b); }
inline OrEvent operator |( EventPtr& a, EventPtr& b ) { return OrEvent( *a.get(), *b.get()); }
inline OrEvent operator |( EventPtr& a, Event& b ) { return OrEvent(*a.get(),b); }
inline OrEvent operator |( Event& a, EventPtr& b ) { return OrEvent(a, *b.get()); }

inline AndEvent operator &( Event& a, Event& b ) { return AndEvent(a,b); }
inline AndEvent operator &( EventPtr& a, EventPtr& b ) { return AndEvent( *a.get(), *b.get()); }
inline AndEvent operator &( EventPtr& a, Event& b ) { return AndEvent(*a.get(),b); }
inline AndEvent operator &( Event& a, EventPtr& b ) { return AndEvent(a, *b.get()); }


//-----------------------------------------------------------
class Environment
{
public:
    Environment();

    double now() const;

    using Callable = std::function<void()>;

    void addProcess(const Callable &function);

    EventPtr timeout(double time);

    void run(double timeout = std::numeric_limits<double>::infinity() );

    void wait(const Event& event);

    void wait(const EventPtr& event)
    {
        wait( *(event.get()) );
    }

   // void wait_any(std::initializer_list<Event*> events);

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
