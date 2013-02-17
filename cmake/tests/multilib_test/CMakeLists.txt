cmake_minimum_required(VERSION 2.8)
project(multilib_test)
set(CMAKE_C_FLAGS "-m32")
file(GLOB Sources *.c)

add_executable(multilib_test ${Sources})
