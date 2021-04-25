cmake_minimum_required(VERSION 3.13)
project(satisfactorygenerator)

set(CMAKE_CXX_STANDARD 17)

#set(CMAKE_CXX_FLAGS "-O3")

add_executable(satisfactorygenerator main.cpp)
target_link_libraries(satisfactorygenerator z)
