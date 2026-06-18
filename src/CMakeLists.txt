add_library(dbms_lib STATIC
    catalog/Database.cpp
    catalog/System.cpp
    catalog/Table.cpp

    common/Value.cpp
    common/ValueComparator.cpp

    execution/Executor.cpp

    io/repl.cpp

    sqlparser/Parser.cpp
    sqlparser/QueryBuffer.cpp

    storage/BTree.cpp
    storage/BTreeNode.cpp
    storage/IndexedValue.cpp
    storage/Serializer.cpp
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

