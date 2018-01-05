cmake_minimum_required(VERSION 2.8)
project(SIGTRAN)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11 -Wwrite-strings -Wno-narrowing")

find_package( Boost COMPONENTS system thread filesystem regex system signals REQUIRED )
link_directories( ${Boost_LIBRARY_DIRS} )
include_directories( ${Boost_INCLUDE_DIRS} )
include_directories( ./ )
include_directories( ./dns )
set (CMAKE_CXX_FLAGS   "${CMAKE_CXX_FLAGS} -fpermissive")

set(SOURCE_FILES
        main.cpp
        sctp.cpp
     )

add_executable(sctp-cli ${SOURCE_FILES})
target_link_libraries(sctp-cli ${Boost_LIBRARIES} pthread)
