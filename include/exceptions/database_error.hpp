#pragma once

#include <stdexcept>
#include <string>

namespace dbms {

class DatabaseError : public std::runtime_error {
public:
    explicit DatabaseError(const std::string& message) :
        std::runtime_error(message)
    {}
};

class ParserError : public DatabaseError {
public:
    explicit ParserError(const std::string& message) :
        DatabaseError("Parser error: " + message)
    {}
};

class DataCorruptionError : public DatabaseError {
public:
    explicit DataCorruptionError(const std::string& message) :
        DatabaseError("Data corruption: " + message)
    {}
};

class SchemaError : public DatabaseError {
public:
    explicit SchemaError(const std::string& message) :
        DatabaseError("Schema error: " + message)
    {}
};

class TableError : public DatabaseError {
public:
    explicit TableError(const std::string& message) :
        DatabaseError("Table error: " + message)
    {}
};

class SystemError : public DatabaseError {
public:
    explicit SystemError(const std::string& message) :
        DatabaseError("System error: " + message)
    {}
};

class StorageError : public DatabaseError {
public:
    explicit StorageError(const std::string& message) :
        DatabaseError("Storage error: " + message)
    {}
};

class TypeError : public DatabaseError {
public:
    explicit TypeError(const std::string& message) :
        DatabaseError("Type error: " + message)
    {}
};

class ConstraintViolationError : public DatabaseError {
public:
    explicit ConstraintViolationError(const std::string& message) :
        DatabaseError("Constraint violation: " + message)
    {}
};

class ExecutionError : public DatabaseError {
public:
    explicit ExecutionError(const std::string& message) :
        DatabaseError("Execution error: " + message)
    {}
};

class NotFoundError : public DatabaseError {
public:
    explicit NotFoundError(const std::string& message) :
        DatabaseError("Not found: " + message)
    {}
};

class DuplicateError : public DatabaseError {
public:
    explicit DuplicateError(const std::string& message) :
        DatabaseError("Duplicate: " + message)
    {}
};

} // namespace dbms
