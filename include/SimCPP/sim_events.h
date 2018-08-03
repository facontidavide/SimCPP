#ifndef SIM_EVENTS_H
#define SIM_EVENTS_H

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
    Event();
    virtual ~Event() = default;

    stCoRoutine_t* coro() const;

    virtual bool ready() const;

    bool cancelled() const;

    void cancel();

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

    bool ready() const override;
};

class AndEvent: public CompositeEvent
{
public:
    AndEvent( Event& a, Event& b): CompositeEvent(a,b) {}

    bool ready() const override;
};

inline OrEvent operator |( Event& a, Event& b ) { return OrEvent(a,b); }
inline OrEvent operator |( EventPtr& a, EventPtr& b ) { return OrEvent( *a.get(), *b.get()); }
inline OrEvent operator |( EventPtr& a, Event& b ) { return OrEvent(*a.get(),b); }
inline OrEvent operator |( Event& a, EventPtr& b ) { return OrEvent(a, *b.get()); }

inline AndEvent operator &( Event& a, Event& b ) { return AndEvent(a,b); }
inline AndEvent operator &( EventPtr& a, EventPtr& b ) { return AndEvent( *a.get(), *b.get()); }
inline AndEvent operator &( EventPtr& a, Event& b ) { return AndEvent(*a.get(),b); }
inline AndEvent operator &( Event& a, EventPtr& b ) { return AndEvent(a, *b.get()); }


}

#endif // SIM_ENVIRONMENT_H
