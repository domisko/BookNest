cmake_minimum_required(VERSION 3.10)
project(BookNest)

set(CMAKE_CXX_STANDARD 17)

# Catch2 Integration
include(FetchContent)
FetchContent_Declare(
        Catch2
        GIT_REPOSITORY https://github.com/catchorg/Catch2.git
        GIT_TAG        v3.11.0
)
FetchContent_MakeAvailable(Catch2)

include_directories(include)

include_directories(include)

# Die Liste der Quelldateien (ohne main.cpp)
set(PROJECT_SOURCES
        src/Library.cpp
        src/Borrower.cpp
        src/Loan.cpp
        src/Book.cpp
        src/Utils.cpp
        src/Employee.cpp
)

# Main App
add_executable(BookNest
        src/main.cpp
        ${PROJECT_SOURCES}
)

# Test-Target
file(GLOB TEST_SOURCES tests/*.cpp)
add_executable(unit_tests
        ${PROJECT_SOURCES}
        ${TEST_SOURCES}
)

target_link_libraries(unit_tests PRIVATE Catch2::Catch2WithMain)
target_include_directories(unit_tests PRIVATE include)