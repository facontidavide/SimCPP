#include "coroutine.h"
#include <iostream>
#include <chrono>

coroutine::Channel<int> channel;

string async_func()
{
    std::this_thread::sleep_for(std::chrono::milliseconds(3000));
    return "after sleep: --22--";
}

void routine_func1()
{
    int i = channel.pop();
    std::cout << "Rt1 (channel): "<< i << std::endl;
    i = channel.pop();
    std::cout << "Rt1 (channel): "<< i << std::endl;
}

void routine_func2()
{
    std::cout << "rt2: 20" << std::endl;
    coroutine::yield();

    std::cout << "rt2: 21" << std::endl;
    //run function async
    //yield current routine if result not returned
    string str = coroutine::await(async_func);
    std::cout << "Rt2:: " << str << std::endl;
}

void routine_func3()
{
    std::cout << "H" << std::endl;
    coroutine::yield();

    std::cout << "e" << std::endl;
    coroutine::yield();

    std::cout << "y" << std::endl;
    coroutine::yield();

     std::cout << "done" << std::endl;
}

int main()
{

    //create routine with callback like std::function<void()>

    coroutine::routine_t rt3 = coroutine::create(routine_func3);

    auto ret = coroutine::resume(rt3);
    std::cout << (ret) << std::endl;

    ret = coroutine::resume(rt3);
    std::cout << (ret) << std::endl;

    ret = coroutine::resume(rt3);
    std::cout << (ret) << std::endl;

    ret = coroutine::resume(rt3);
    std::cout << (ret) << std::endl;

    ret = coroutine::resume(rt3);
    std::cout << (ret) << std::endl;

    //    coroutine::routine_t rt1 = coroutine::create(routine_func1);
    //    coroutine::routine_t rt2 = coroutine::create(routine_func2);
    //    std::cout << "resume rt1" << std::endl;
    //    auto ret = coroutine::resume(rt1);

    //    std::cout << "resume rt2" << std::endl;
    //    coroutine::resume(rt2);

    //    std::cout << "push 10 into rt1" << std::endl;
    //    channel.push(10);

    //    std::cout << "resume rt2" << std::endl;
    //    coroutine::resume(rt2);

    //    std::cout << "push 10 into rt1" << std::endl;
    //    channel.push(11);

    //    std::cout << "sleep and resume" << std::endl;

    //    std::this_thread::sleep_for(std::chrono::milliseconds(4000));
    //    std::cout << "end of main sleep" << std::endl;
    //    coroutine::resume(rt2);

    //    //destroy routine, free resouce allocated
    //    //Warning: don't destroy routine by itself
    //    coroutine::destroy(rt1);
    //    coroutine::destroy(rt2);

    return 0;
}
