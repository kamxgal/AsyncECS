cmake_minimum_required(VERSION 2.8)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++14 -pthread")
#find_package(Threads REQUIRED)
#SET(CMAKE_CXX_FLAGS -pthread)

set(FILES
    main.cpp
    bitflag.h
    bitflag.cpp
    entity.h
    entity.cpp
    component.h
    component.cpp
    view.h
    registry.h
    registry.cpp
)

project(AsyncECS C CXX)
add_executable(${PROJECT_NAME} ${FILES})

target_link_libraries (${PROJECT_NAME}
    ${CMAKE_THREAD_LIBS_INIT}
)
