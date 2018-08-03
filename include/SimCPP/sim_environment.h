#ifndef SIM_ENVIRONMENT_H
#define SIM_ENVIRONMENT_H

#include <functional>
#include <vector>
#include <queue>
#include <limits>
#include <list>
#include <memory>
#include <SimCPP/sim_events.h>

namespace Sim{

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

private:
    double _now;

    std::list<coro::routine_t> _process;

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
