#include "SimCPP/sim_resource.h"
#include <algorithm>
#include <exception>

namespace Sim{

Resource::Resource(Environment *env, int max_concurrent_request):
    _env(env),
    _max_slots( max_concurrent_request ),
    _used_slots_count(0)
{
    if( max_concurrent_request <= 0)
    {
        throw std::invalid_argument("Resource: max_concurrent_request must be >=1");
    }
}

Resource::~Resource()
{
    for(auto ev: _memory_pool)
    {
        delete ev;
    }
}

ResourceEvent* Resource::newEvent()
{
    // TODO: reuse instances with _memory_pool.

    auto ev = new ResourceEvent(this);
    return ev;
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

}
