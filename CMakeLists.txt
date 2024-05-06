#cmake_minimum_required(VERSION 3.20)
#project(qtkml VERSION 1.0 LANGUAGES CXX)

#set(CMAKE_CXX_STANDARD 17)
#set(CMAKE_CXX_STANDARD_REQUIRED ON)

#find_package(Qt5 COMPONENTS Core REQUIRED)
#
#add_library(qtkml)
#target_include_directories(libevents_generated INTERFACE ${CMAKE_CURRENT_SOURCE_DIR}/libevents/libs/cpp/generated)
#
#add_library(libevents_parser
#        definitions.cpp
#        libevents/libs/cpp/parse/parser.cpp
#        libevents/libs/cpp/protocol/receive.cpp
#)
#target_include_directories(libevents_parser PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
#target_link_libraries(libevents_parser PUBLIC Qt5::Core)
#target_link_libraries(libevents_parser PUBLIC comm)

if(NOT APPLE)
    set(CMAKE_POSITION_INDEPENDENT_CODE ON)
    add_compile_options(-fno-rtti)
endif()

add_subdirectory(kmllib)
#add_subdirectory(test)
