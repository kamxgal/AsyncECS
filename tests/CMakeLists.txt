project(ecs_testing)

find_package(Threads REQUIRED)

#set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS}")

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(TEST_FILES
    bitflagTests.cpp
    entityTests.cpp
	viewTests.cpp
	registryAsyncOperationsTests.cpp
	registrySyncOperationsTests.cpp
    TestComponents.h
    main.cpp
)

include_directories(${PROJECT_SOURCE_DIR}/..)

add_executable(${PROJECT_NAME} ${TEST_FILES})
target_link_libraries(${PROJECT_NAME} AsyncECS gtest gmock)