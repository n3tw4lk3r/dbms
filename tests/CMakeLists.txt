add_library(test_utils STATIC
    utils.cpp
)

target_include_directories(test_utils
    PUBLIC
        ${PROJECT_SOURCE_DIR}/include
        ${CMAKE_CURRENT_SOURCE_DIR}
)

target_compile_options(test_utils
    PRIVATE
        -Wall
        -Wextra
        -Wpedantic
)

add_executable(test_batch_mode
    test_batch_mode.cpp
)

add_executable(test_btree
    test_btree.cpp
)

add_executable(test_btree_node
    test_btree_node.cpp
)

add_executable(test_database
    test_database.cpp
)

add_executable(test_executor
    test_executor.cpp
)

add_executable(test_indexed_value
    test_indexed_value.cpp
)

add_executable(test_parser
    test_parser.cpp
)

add_executable(test_serializer
    test_serializer.cpp
)

add_executable(test_system
    test_system.cpp
)

add_executable(test_table
    test_table.cpp
)

add_executable(test_value
    test_value.cpp
)

add_executable(test_value_comparator
    test_value_comparator.cpp
)

set(TEST_TARGETS
    test_batch_mode
    test_btree
    test_btree_node
    test_database
    test_executor
    test_indexed_value
    test_parser
    test_serializer
    test_system
    test_table
    test_value
    test_value_comparator
)

foreach(TEST_TARGET ${TEST_TARGETS})
    target_include_directories(${TEST_TARGET}
        PRIVATE
            ${PROJECT_SOURCE_DIR}/include
            ${CMAKE_CURRENT_SOURCE_DIR}
    )

    target_link_libraries(${TEST_TARGET}
        PRIVATE
            dbms_lib
            test_utils
    )

    target_compile_options(${TEST_TARGET}
        PRIVATE
            -Wall
            -Wextra
        -Wpedantic
    )

    add_test(
        NAME ${TEST_TARGET}
        COMMAND ${TEST_TARGET}
    )

    message(STATUS "Added test: ${TEST_TARGET}")
endforeach()

enable_testing()

