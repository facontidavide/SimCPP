#ifndef COROUTINE_H
#define COROUTINE_H

#include <functional>
#include <unordered_map>
#include "libco/co_routine.h"
#include "libco/co_routine_inner.h"

// simple wrapper functions

namespace coro{

struct routine_t
{
    stCoRoutine_t* co;
    std::function<void()> callable;
    bool started() const
    {
        return ( co && co->cStart);
    }
    bool finished() const
    {
        return ( co && co->cEnd);
    }
};

namespace details{

    inline void* co_function_wrapper(void* arg)
    {
        routine_t* coro = static_cast<routine_t*>( arg );
        coro->callable();
        return nullptr;
    }
}

inline void create(routine_t* coro, std::function<void()> callable )
{
    coro->callable = std::move(callable);
    co_create( &coro->co, nullptr, details::co_function_wrapper, coro );
}

inline bool resume(const routine_t& coro)
{
    if( coro.finished() )
    {
        return true;
    }
    co_resume( coro.co );
    return coro.finished();
}

inline void yield()
{
    co_yield_ct();
}

inline void current()
{
    co_yield_ct();
}

inline void destroy(const routine_t& coro)
{
    co_release( coro.co );
}


}
#endif // COROUTINE_H
