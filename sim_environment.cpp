#include "sim_environment.h"

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

TimeoutEvent *Environment::timeout(double time)
{
    return new TimeoutEvent( _now + time );
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
            TimeoutEvent* tm = _timeout_queue.top();
            _timeout_queue.pop();
            _now = tm->deadline();
            co_resume( tm->coro() );
            delete tm;
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

void Environment::wait(Event* event)
{
    if( event->ready() )
    {
        return;
    }

    if(  event->type() == EventType::TIMEOUT )
    {
        auto ev = dynamic_cast<TimeoutEvent*>( event );
        _timeout_queue.push( ev );
    }
    coro::yield();
}

Resource::Resource(Environment *env, int max_concurrent_request):
    _env(env),
    _max_slots( max_concurrent_request ),
    _used_slots(0)
{

}

ResourceAvailableEvent *Resource::request()
{
    if( _used_slots < _max_slots )
    {
        _used_slots++;
        auto event = new ResourceAvailableEvent(this);
        event->setReady();
        return event;
    }
}

void Resource::release(ResourceAvailableEvent *event)
{
    _used_slots--;
}

void ResourceAvailableEvent::release()
{
    if (_parent)
    {
        _parent->release(this);
    }
}

}
