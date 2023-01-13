cmake_minimum_required(VERSION 3.0.2)
project(AsyncECS C CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

if (CMAKE_BUILD_TYPE STREQUAL "Debug")
    set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} /MDd")
else()
    set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} /MD")
endif ()

set(FILES
    bitflag.h
    bitflag.cpp
    entity.h
    entity.cpp
    component.h
    component.cpp
    view.h
    notification.h
    registry.h
    registry.cpp
    reactive_system.h
    reactive_system.cpp
)

add_subdirectory(3rd_party)
add_subdirectory(tests)

add_library(${PROJECT_NAME} ${FILES})
