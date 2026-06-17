add_library(dbms_lib STATIC
    catalog/database.cpp
    catalog/system.cpp
    catalog/table.cpp

    common/value.cpp
    common/value_comparator.cpp

    execution/executor.cpp

    io/repl.cpp

    sqlparser/parser.cpp
    sqlparser/query_buffer.cpp

    storage/btree.cpp
    storage/btree_node.cpp
    storage/indexed_value.cpp
    storage/serializer.cpp
)

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

add_executable(dbms
    main.cpp
)

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

