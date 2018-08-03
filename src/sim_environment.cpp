#include "SimCPP/sim_environment.h"
#include <algorithm>

namespace Sim{

Environment::Environment():
    _now(0.0),
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
    _process.push_back( std::move(rt) );
}

void destroyTimeoutEvent(Event* ev)
{

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

    // Motivation: a coroutine may create another one, altering the
    // container _process.
    // Therefore we need to store the initial list in coroutines_to_run
    for(auto& coro: _process )
    {
        coroutines_to_run.push_back( coro );
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
            if( it->finished() )
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


}
