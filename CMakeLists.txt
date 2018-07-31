cmake_minimum_required(VERSION 2.8)

project(SimPlusPlus)

add_definitions(-std=c++11)

add_executable(${PROJECT_NAME}
    sim_environment.cpp
    random.h
    test.cpp
    )

add_executable(coroutine_sample
    main.cpp
)

target_link_libraries(coroutine_sample pthread)
