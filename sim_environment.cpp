#include "sim_environment.h"
#include <algorithm>

namespace Sim{

Environment::Environment():
    _now(0.0),
    _last_UID(0),
    _running(false)
{

}

double Environment::now() const
{
    return _now;
}

void Environment::addProcess(const Environment::Callable& function)
{
    coro::routine_t rt;
    coro::create(&rt, function );

    if( _running )
    {
        bool finished = coro::resume( rt );

        if( finished )
        {
            return;
        }
    }
    _process.push_back( { _last_UID++, std::move(rt) } );
}

void destroyTimeoutEvent(Event* ev)
{
   // delete ev;
}

EventPtr Environment::timeout(double time)
{
    auto ev = new TimeoutEvent( _now + time );
    _timeout_queue.push( ev );
    return EventPtr(ev, destroyTimeoutEvent);
}

void Environment::run(double timeout)
{
    _running = true;

    std::vector<coro::routine_t> coroutines_to_run;
    coroutines_to_run.reserve( _process.size() );

    // trigger the initial processes
    for(auto it = _process.begin(); it != _process.end(); it++)
    {
        coroutines_to_run.push_back( it->coro );
    }
    for(auto co: coroutines_to_run)
    {
        coro::resume(co);
    }

    while( _now <= timeout && ! _process.empty() )
    {
        if( !_timeout_queue.empty() )
        {
            TimeoutEvent* ev = _timeout_queue.top();
            _timeout_queue.pop();
            if( !ev->cancelled() )
            {
                _now = ev->deadline();

                ev->_ready = true;
                if( ev->coro()->cEnd == 0){
                    co_resume( ev->coro() );
                }
            }
        }

        for(auto it = _process.begin(); it != _process.end(); /* no increment */)
        {
            if( it->coro.finished() )
            {
                it = _process.erase(it);
            }
            else{
                it++;
            }
        }
    }
    _running = false;
}

void Environment::wait(const Event& event)
{
    while( !event.ready() )
    {
        coro::yield();
    }
}


Resource::Resource(Environment *env, int max_concurrent_request):
    _env(env),
    _max_slots( max_concurrent_request ),
    _used_slots_count(0)
{

}

ResourceEvent* Resource::newEvent()
{
    return new ResourceEvent(this);
//    if( _memory_pool.empty())
//    {
//    }
//    else{
//        auto event = _memory_pool.back();
//        _memory_pool.pop_back();
//        event->_ready = false;
//        return event;
//    }
}

void destroyResourceEvent(Event* ev)
{
    dynamic_cast<ResourceEvent*>(ev)->release();
    delete ev;
}

EventPtr Resource::request()
{
    ResourceEvent* event = newEvent();

    if( _used_slots_count < _max_slots )
    {
        _used_slots_count++;
        event->_ready = true;
    }
    else{
        event = newEvent();
       _pending_events.push_back( event );
    }
    return EventPtr( event, destroyResourceEvent );
}

void Resource::release(ResourceEvent *event)
{
    if( event->cancelled() )
    {
        auto it = std::find( _pending_events.begin(), _pending_events.end(), event );
        if( it != _pending_events.end())
        {
            _pending_events.erase(it);
        }
    }
    else if( event->ready() ) // the event has been used. A new slot is free
    {
       _used_slots_count--;

        while( !_pending_events.empty() && _used_slots_count < _max_slots )
        {
            auto ev = _pending_events.front();
            _pending_events.pop_front();

            if( !ev->cancelled() )
            {
                ev->_ready = true;
                _used_slots_count++;

                if( ev->coro()->cEnd == 0){
                    co_resume( ev->coro() );
                }
            }
        }
    }
    else{
        throw std::logic_error("Unexpected ResourceEvent state");
    }
}

void ResourceEvent::release()
{
    if (_parent)
    {
        _parent->release(this);
        _parent = nullptr;
    }
}

}
