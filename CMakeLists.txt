cmake_minimum_required(VERSION 3.13)
project(satisfactory-sav-analyser)

set(CMAKE_CXX_STANDARD 20)

#set(CMAKE_CXX_FLAGS "-O3")

add_executable(satisfactory-sav-analyzer main.cpp)
target_link_libraries(satisfactory-sav-analyzer z)


#add_custom_command(TARGET satisfactory-sav-analyzer POST_BUILD
#        COMMAND /usr/bin/beep -f 2000 -l 50 -n -f 500 -l 50 -n -f 2500 -l 50 || true
#        )
