cmake_minimum_required(VERSION 2.8)

project(SimPlusPlus)


set (CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -fno-omit-frame-pointer -fsanitize=address")
set (CMAKE_LINKER_FLAGS_DEBUG "${CMAKE_STATIC_LINKER_FLAGS_DEBUG} -fno-omit-frame-pointer -fsanitize=address")


add_subdirectory(libco)

add_definitions(-std=c++11)

add_library(simpp
    coroutine.h
    sim_environment.cpp
    random.h
    )
target_link_libraries(simpp colib_static pthread)

add_executable(bank_simple bank_simple.cpp )
target_link_libraries(bank_simple simpp)

add_executable(bank_renege bank_renege.cpp )
target_link_libraries(bank_renege simpp)


add_executable(simpp_test test.cpp )
target_link_libraries(simpp_test simpp)


add_executable(coroutine_sample  main.cpp)
target_link_libraries(coroutine_sample simpp)
