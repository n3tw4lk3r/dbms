cmake_minimum_required(VERSION 3.20)

project(DBMS LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

file(GLOB_RECURSE DBMS_SOURCES
    src/*.cpp
)

set(DBMS_MAIN_SOURCE src/main.cpp)
list(REMOVE_ITEM DBMS_SOURCES ${DBMS_MAIN_SOURCE})

add_library(dbms_lib STATIC ${DBMS_SOURCES})

target_include_directories(dbms_lib
    PUBLIC
        ${PROJECT_SOURCE_DIR}/include
)

target_compile_options(dbms_lib
    PRIVATE
        -Wall
        -Wextra
        -Wpedantic
)

target_compile_features(dbms_lib PUBLIC cxx_std_20)

add_executable(dbms ${DBMS_MAIN_SOURCE})

target_link_libraries(dbms PRIVATE dbms_lib)

target_include_directories(dbms
    PRIVATE
        ${PROJECT_SOURCE_DIR}/include
)

target_compile_options(dbms
    PRIVATE
        -Wall
        -Wextra
        -Wpedantic
)

target_compile_features(dbms PRIVATE cxx_std_20)

file(GLOB_RECURSE TEST_SOURCES
    tests/*.cpp
)

foreach(TEST_FILE ${TEST_SOURCES})
    get_filename_component(TEST_NAME ${TEST_FILE} NAME_WE)
    set(TEST_TARGET "test_${TEST_NAME}")
    
    add_executable(${TEST_TARGET} ${TEST_FILE})
    
    target_include_directories(${TEST_TARGET}
        PRIVATE
            ${PROJECT_SOURCE_DIR}/include
            ${PROJECT_SOURCE_DIR}/tests
    )
    
    target_link_libraries(${TEST_TARGET}
        PRIVATE
            dbms_lib
    )
    
    target_compile_options(${TEST_TARGET}
        PRIVATE
            -Wall
            -Wextra
            -Wpedantic
    )
    
    target_compile_features(${TEST_TARGET} PRIVATE cxx_std_20)
    
    add_test(
        NAME ${TEST_TARGET}
        COMMAND ${TEST_TARGET}
    )
    
    message(STATUS "Added test: ${TEST_TARGET}")
endforeach()

enable_testing()

