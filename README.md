# Keyval: In-Memory Key-Value Store (Valkey-Compatible Subset)

## Overview

Keyval is a simple local in-memory key-value database embedded in the process. The database is controlled via text commands whose syntax is a subset of the commands documented at valkey.io.

The program reads commands from standard input (one command per line), executes them, and writes results to standard output. Output format follows each command's description below.

## Supported Data Types and Commands

### String

Strings are the basic type. Each key holds a single string value.

| Command | Syntax | Description |
|---------|--------|-------------|
| `SET` | `SET key value` | Set a key's value |
| `GET` | `GET key` | Get a key's value |
| `STRLEN` | `STRLEN key` | Return the length of the string value |
| `APPEND` | `APPEND key value` | Append a string to the existing value |
| `EXPIRE` | `EXPIRE key seconds` | Set a key's TTL in seconds |
| `TTL` | `TTL key` | Return the remaining time to live in seconds |

Docs: https://valkey.io/commands/?group=string

---

### List

Lists are ordered sequences of strings. Elements can be pushed and popped from both ends.

| Command | Syntax | Description |
|---------|--------|-------------|
| `LPUSH` | `LPUSH key value [value ...]` | Push elements to the head of the list |
| `RPUSH` | `RPUSH key value [value ...]` | Push elements to the tail of the list |
| `LPOP` | `LPOP key [count]` | Pop and return elements from the head |
| `RPOP` | `RPOP key [count]` | Pop and return elements from the tail |
| `LLEN` | `LLEN key` | Return the length of the list |
| `LRANGE` | `LRANGE key start stop` | Return a sublist by indexes (negative indexes supported) |
| `LINDEX` | `LINDEX key index` | Return an element by index |
| `LSET` | `LSET key index value` | Set the element at index |
| `LINSERT` | `LINSERT key BEFORE\|AFTER pivot value` | Insert an element before or after the pivot |

Docs: https://valkey.io/commands/?group=list

---

### Set

Sets are unordered collections of unique strings.

| Command | Syntax | Description |
|---------|--------|-------------|
| `SADD` | `SADD key member [member ...]` | Add members to a set |
| `SREM` | `SREM key member [member ...]` | Remove members from a set |
| `SISMEMBER` | `SISMEMBER key member` | Test set membership |
| `SMEMBERS` | `SMEMBERS key` | Return all members of the set |
| `SCARD` | `SCARD key` | Return the number of members |
| `SUNION` | `SUNION key [key ...]` | Union of multiple sets |
| `SINTER` | `SINTER key [key ...]` | Intersection of multiple sets |
| `SDIFF` | `SDIFF key [key ...]` | Difference of sets (members of the first not present in the rest) |
| `SMOVE` | `SMOVE source destination member` | Move a member from one set to another |

Docs: https://valkey.io/commands/?group=set

---

### Geospatial Index

The geospatial index stores point coordinates (longitude, latitude) and supports distance-based queries.

| Command | Syntax | Description |
|---------|--------|-------------|
| `GEOADD` | `GEOADD key longitude latitude member [longitude latitude member ...]` | Add points with coordinates |
| `GEOPOS` | `GEOPOS key member [member ...]` | Get point coordinates |
| `GEODIST` | `GEODIST key member1 member2 [unit]` | Compute distance between two points. Units: `m`, `km`, `mi`, `ft` |
| `GEOSEARCH` | `GEOSEARCH key FROMLONLAT lon lat BYRADIUS radius unit [ASC\|DESC] [COUNT count]` | Find points within a radius from a coordinate |
| `GEOSEARCHSTORE` | `GEOSEARCHSTORE dest source FROMLONLAT lon lat BYRADIUS radius unit [ASC\|DESC] [COUNT count]` | Same as above, store the result in a destination key |

Note: Distances are computed using the Haversine formula. Earth radius is set to 6372.8 km.

Docs: https://valkey.io/commands/?group=geo

---

### General Commands

| Command | Syntax | Description |
|---------|--------|-------------|
| `TYPE` | `TYPE key` | Return the value type: `string`, `list`, `set`, `zset`, `hash`, `none` |
| `DEL` | `DEL key [key ...]` | Delete keys of any type |
| `EXISTS` | `EXISTS key [key ...]` | Check for key existence |
| `KEYS` | `KEYS pattern` | List keys by glob pattern |
| `FLUSHDB` | `FLUSHDB` | Delete all keys |
| `CONFIG SET` | `CONFIG SET maxmemory <bytes>` | Set memory limit in bytes (0 — no limit) |
| `CONFIG GET` | `CONFIG GET maxmemory` | Get current memory limit |
| `DBSIZE` | `DBSIZE` | Return the number of keys in the store |
| `MEMORY USAGE` | `MEMORY USAGE key` | Return an estimate of a key's memory usage in bytes |

---

## Behavior and I/O

- A library and a set of tests are provided.
- A separate executable offers an interactive mode. Commands are read from `stdin`; results are written to `stdout`.
- Unknown commands or wrong arity print an error to `stderr` and the program continues.
- Keys are case-sensitive; command names are case-insensitive (`SET` and `set` are equivalent).
- The `EXIT` command or end-of-input terminates the program.

## Memory Limit

The executable accepts an optional command-line argument:

```
./valkey [--maxmemory <bytes>]
```

Supported suffixes: `b`, `kb`, `mb`, `gb` (for example, `--maxmemory 64mb`). By default, no limit is set.

The limit can also be changed at runtime with `CONFIG SET maxmemory`.

The memory usage estimate accounts for the key, value, and internal overhead. Exactness is not critical, but it should increase as data is added and decrease when data is removed.

When the total size reaches the limit, any command that would increase storage must return an error:

```
(error) OOM command not allowed when used memory > 'maxmemory'
```

Docs: https://valkey.io/topics/memory-optimization/

## Build and Run

Requirements: CMake 3.10+, a C++23 compiler.

1. Configure and build:
   - `cmake -S . -B build`
   - `cmake --build build`
2. Run the interactive CLI:
   - `./build/src/valkey [--maxmemory <bytes>]`
