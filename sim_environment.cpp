#include "sim_environment.h"

Sim::Environment::Environment():
    _now(0.0),
    _last_UID(0)
{

}

double Sim::Environment::now() const
{
    return _now;
}

void Sim::Environment::addProcess(const Sim::Environment::Callable& function)
{
    coro::routine_t rt;
    coro::create(&rt, function );

    bool finished = coro::resume( rt );

    if( !finished )
    {
        _process.push_back( { _last_UID++, rt } );
    }
}

Sim::TimeoutEvent *Sim::Environment::timeout(double time)
{
    return new Sim::TimeoutEvent( _now + time );
}

void Sim::Environment::run(double timeout)
{
    while( _now <= timeout && ! _timeout_queue.empty() )
    {
        TimeoutEvent* tm = _timeout_queue.top();
        _timeout_queue.pop();
        _now = tm->deadline();
        co_resume( tm->coro() );
        delete tm;
    }
}

void Sim::Environment::wait(Sim::Event* event)
{
    if(  event->type() == EventType::TIMEOUT )
    {
        auto ev = dynamic_cast<TimeoutEvent*>( event );
        _timeout_queue.push( ev );
    }
    coro::yield();
}
