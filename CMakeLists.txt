cmake_minimum_required(VERSION 3.0.2)
project(AsyncECS C CXX)

#set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS}")

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

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
)

add_subdirectory(3rd_party)
add_subdirectory(tests)

add_library(${PROJECT_NAME} ${FILES} ${TEST_FILES})
