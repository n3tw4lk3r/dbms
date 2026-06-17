# Database Management System

A relational DBMS written in C++20.

It supports SQL-like queries, typed tables, persistent storage, indexed lookups, and JSON output.

## Features

- Persistent storage using the filesystem
- Interactive and batch execution modes
- SQL-like query language
- Typed columns:
  - `INT`
  - `STRING`
- Constraints:
  - `NOT_NULL`
  - `INDEXED` (unique indexed column)
- B-Tree indexes for optimized equality lookups
- JSON query output
- Semantic and syntax validation
- Multi-line query support
- Safe recovery from invalid queries and runtime errors

## Supported Queries

### Database Management

- `CREATE DATABASE`
- `DROP DATABASE`
- `USE`

### Table Management

- `CREATE TABLE`
- `DROP TABLE`

### Data Manipulation

- `INSERT`
- `SELECT`
- `UPDATE`
- `DELETE`

### Conditions

Supported operators:

- `==`
- `!=`
- `<`
- `>`
- `<=`
- `>=`
- `BETWEEN`
- `LIKE` (regex)

## Architecture

Project structure:

- `catalog/`
  - Database and table management
- `common/`
  - Shared types and utilities
- `exceptions/`
  - Custom exceptions
- `execution/`
  - Query execution engine
- `io/`
  - Input-output system
- `sqlparser/`
  - Query tokenizer and parser
- `storage/`
  - Rows, serialization, B-Tree index

## Storage Model

Each database is stored as a directory inside `data/`.

Each table uses two files:

- `.meta` — schema definition
- `.data` — append-only operation log

Indexes are rebuilt automatically during loading.

## Build

Requirements:

- CMake 3.20+
- C++20-compatible compiler

Build:
```bash
cmake -B build
cmake --build build
```

## Run

Run unit tests:
```bash
# inside build/tests/
ctest --rerun-failed --output-on-failure
```

Interactive mode:

```bash
# inside build/
./dbms
```

Batch mode:

```bash
# inside build/
./dbms script.txt
```

