cmake_minimum_required(VERSION 2.8)

project(SimPlusPlus)


set (CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -fno-omit-frame-pointer -fsanitize=address")
set (CMAKE_LINKER_FLAGS_DEBUG "${CMAKE_STATIC_LINKER_FLAGS_DEBUG} -fno-omit-frame-pointer -fsanitize=address")


add_subdirectory(3rdparty/libco)

add_definitions(-std=c++11)

INCLUDE_DIRECTORIES( ./include ./3rdparty)

add_library(simpp
    ./include/SimCPP/coroutine.h
    ./include/SimCPP/random.h
    ./include/SimCPP/sim_environment.h
    ./include/SimCPP/sim_resource.h
    ./include/SimCPP/sim_events.h

    ./src/sim_events.cpp
    ./src/sim_resource.cpp
    ./src/sim_environment.cpp
    )
target_link_libraries(simpp colib_static pthread)

add_executable(bank_simple ./examples/bank_simple.cpp )
target_link_libraries(bank_simple simpp)

add_executable(bank_renege ./examples/bank_renege.cpp )
target_link_libraries(bank_renege simpp)


add_executable(simpp_test ./tests/test.cpp )
target_link_libraries(simpp_test simpp)

