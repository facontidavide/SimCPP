cmake_minimum_required(VERSION 2.8)

project(SimPlusPlus)

add_subdirectory(libco)

add_definitions(-std=c++11)

add_executable(${PROJECT_NAME}
    coroutine.h
    sim_environment.cpp
    random.h
    test.cpp
    )
target_link_libraries(${PROJECT_NAME} pthread colib_static)


add_executable(coroutine_sample
    main.cpp
)

target_link_libraries(coroutine_sample pthread colib_static)
