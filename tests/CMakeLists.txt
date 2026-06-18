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

add_executable(test_BTree
    test_BTree.cpp
)

add_executable(test_BTreeNode
    test_BTreeNode.cpp
)

add_executable(test_Database
    test_Database.cpp
)

add_executable(test_Executor
    test_Executor.cpp
)

add_executable(test_IndexedValue
    test_IndexedValue.cpp
)

add_executable(test_Parser
    test_Parser.cpp
)

add_executable(test_Serializer
    test_Serializer.cpp
)

add_executable(test_System
    test_System.cpp
)

add_executable(test_Table
    test_Table.cpp
)

add_executable(test_Value
    test_Value.cpp
)

add_executable(test_ValueComparator
    test_ValueComparator.cpp
)

set(TEST_TARGETS
    test_batch_mode
    test_BTree
    test_BTreeNode
    test_Database
    test_Executor
    test_IndexedValue
    test_Parser
    test_Serializer
    test_System
    test_Table
    test_Value
    test_ValueComparator
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

