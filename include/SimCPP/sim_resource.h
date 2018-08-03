#ifndef SIM_RESOURCE_H
#define SIM_RESOURCE_H

#include <SimCPP/sim_events.h>

namespace Sim{

class Environment;

class Resource
{
public:

    Resource(Environment* env, int max_concurrent_request);
    ~Resource();

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
