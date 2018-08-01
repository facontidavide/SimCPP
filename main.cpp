#include "libco/co_routine.h"
#include <iostream>
#include <chrono>
#include <thread>

struct Coroutine_t
{
    stCoRoutine_t* co;
    std::function<void()> callable;
    bool finished;
};

void MyFunc()
{
    std::cout << ">> H" << std::endl;
    co_yield_ct();

    std::cout << ">> e" << std::endl;
    co_yield_ct();

    std::cout << ">> y" << std::endl;
    co_yield_ct();

     std::cout << ">> done" << std::endl;
}


void* co_function_wrapper(void* arg)
{
    Coroutine_t* coro = static_cast<Coroutine_t*>( arg );
    coro->callable();
    coro->finished = true;
    return nullptr;
}

void co_create(Coroutine_t* coro, std::function<void()> callable )
{
    coro->callable = std::move(callable);
    coro->finished = false;
    co_create( &coro->co, nullptr, co_function_wrapper, coro );
}

bool co_resume(const Coroutine_t& coro)
{
    if( coro.finished ) return true;
    co_resume( coro.co );
    return coro.finished;
}

void co_yield()
{
    co_yield_ct();
}

int main()
{
    Coroutine_t rt3;
    co_create( &rt3, MyFunc );

    while( ! co_resume(rt3) )
    {}

    return 0;
}
