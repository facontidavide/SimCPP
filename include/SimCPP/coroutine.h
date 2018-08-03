#ifndef COROUTINE_H
#define COROUTINE_H

#include <iostream>
#include <functional>
#include <unordered_map>
#include "libco/co_routine.h"
#include "libco/co_routine_inner.h"

// simple wrapper functions

namespace coro{

struct routine_t
{
    routine_t(): co(nullptr) {}

    stCoRoutine_t* co;

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
        std::function<void()> callable = *static_cast<std::function<void()>*>( arg );
        co_yield_ct();
        callable();
        return nullptr;
    }
}

inline void create(routine_t* coro, std::function<void()> callable )
{
    co_create( &coro->co, nullptr, details::co_function_wrapper, &callable );
    co_resume( coro->co );
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
