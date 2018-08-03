#include "SimCPP/sim_events.h"
#include "SimCPP/sim_environment.h"

namespace Sim{

Event::Event(): _ready(false), _cancelled(false)
{
    _coro = co_self();
}

bool OrEvent::ready() const
{
    bool done = _a.ready() | _b.ready();
    if( done )
    {
        if( !_a.ready() ) _a.cancel();
        if( !_b.ready() ) _b.cancel();
    }
    return done;
}

bool AndEvent::ready() const
{
    return _a.ready() && _b.ready();
}

stCoRoutine_t *Event::coro() const{
    return _coro;
}

bool Event::ready() const { return _ready; }

bool Event::cancelled() const { return _cancelled; }

void Event::cancel() { _cancelled = true; }


void ResourceEvent::release()
{
    if (_parent)
    {
        _parent->release(this);
        _parent = nullptr;
    }
}

} // end namespace
