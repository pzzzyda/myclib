# myclib

[English](README.md) | [中文](README_CN.md)

A lightweight, efficient, and easy-to-use C library providing essential data structures and utilities for C programmers.

## Overview

myclib is designed to fill the gap between the minimal standard C library and more heavyweight alternatives. It provides a set of well-tested, generic data structures and utilities that follow modern C programming practices while maintaining compatibility with C11 standard.

## Features

### Core Data Structures

- **Array**: Dynamic array implementation with support for generic types, automatic resizing, and various operations
- **List**: Doubly linked list with generic element support
- **Map**: Hash table-based key-value map with generic key and value support
- **String**: Dynamic string implementation with rich string manipulation functions

### Utilities

- **Type System**: Generic type metadata and operations framework
- **Logging**: Flexible logging system with multiple levels and formatting options
- **Time**: High-resolution time measurement utilities
- **Iterators**: Unified iterator interface for all data structures
- **Memory Management**: Aligned memory allocation functions
- **Hash Functions**: Efficient hash implementations for various data types
- **Attribute Support**: Cross-platform compiler attribute macros
- **Testing Framework**: Lightweight unit testing utilities

## Getting Started

### Prerequisites

- C11 compatible compiler (GCC, Clang, MSVC)
- CMake 3.10 or later

### Building the Library

```bash
# Create build directory
mkdir build

# Configure with CMake
cmake -B build -DCMAKE_BUILD_TYPE=Release

# Build the library
cmake --build build

# Optionally run tests
ctest --test-dir build
```

### Using the Library

1. **Include the header files**:

```c
#include <myclib/array.h>
#include <myclib/string.h>
#include <myclib/log.h>
```

2. **Link against the library**:

```bash
gcc your_program.c -lmyclib -I/path/to/myclib/include -L/path/to/myclib/lib
```

## Examples

### Using Dynamic Array

```c
#include <myclib/array.h>
#include <myclib/type.h>
#include <stdio.h>

int main() {
    struct mc_array array;
    mc_array_init(&array, int_get_mc_type());

    mc_array_push(&array, &(int){42});

    mc_array_push(&array, &(int){100});

    printf("Array length: %zu\n", mc_array_len(&array));
    printf("First element: %d\n", *(int *)mc_array_get_first(&array));
    printf("Second element: %d\n", *(int *)mc_array_get(&array, 1));

    mc_array_cleanup(&array);
    return 0;
}
```

### Using Dynamic String

```c
#include <myclib/string.h>
#include <stdio.h>

int main() {
    struct mc_string str;
    mc_string_from(&str, "Hello");

    mc_string_append(&str, " World");
    mc_string_append_format(&str, "! The answer is %d", 42);

    printf("String: %s\n", mc_string_c_str(&str));
    printf("Length: %zu\n", mc_string_len(&str));

    mc_string_cleanup(&str);
    return 0;
}
```

### Using Map

```c
#include <myclib/map.h>
#include <myclib/string.h>
#include <stdio.h>

int main() {
    struct mc_map map;
    mc_map_init(&map, mc_string_get_mc_type(), int_get_mc_type());

    struct mc_string key;
    mc_string_from(&key, "answer");

    int value = 42;
    mc_map_insert(&map, &key, &value); /* The key has been moved here. */

    mc_string_from(&key, "answer");
    int *result = mc_map_get(&map, &key);
    if (result) {
        printf("The answer is %d\n", *result);
    }

    mc_string_cleanup(&key);
    mc_map_cleanup(&map);
    return 0;
}
```

### Using Logging

```c
#include <myclib/log.h>

int main() {
    struct mc_logger_config config = MC_LOGGER_CONFIG_DEFAULT();
    config.level = MC_LOG_LEVEL_DEBUG;
    config.options = MC_LOG_OPTION_COLOR_OUTPUT;

    struct mc_logger *logger = mc_logger_new(&config);

    MC_LOG_DEBUG(logger, "This is a debug message");
    MC_LOG_INFO(logger, "This is an info message");
    MC_LOG_WARN(logger, "This is a warning message");
    MC_LOG_ERROR(logger, "This is an error message");

    mc_logger_free(logger);
    return 0;
}
```

### Using Testing Framework

```c
#include <myclib/test.h>

int add(int a, int b)
{
    return a + b;
}

int sub(int a, int b)
{
    return a - b;
}

MC_TEST_SUITE(math)

MC_TEST_IN_SUITE(math, add)
{
    MC_ASSERT_EQ_INT(add(1, 1), 2);
    MC_ASSERT_EQ_INT(add(-1, 1), 0);
}

MC_TEST_IN_SUITE(math, sub)
{
    MC_ASSERT_EQ_INT(sub(1, 1), 0);
    MC_ASSERT_EQ_INT(sub(-1, 1), -2);
}

int main(void)
{
#if !MC_COMPILER_SUPPORTS_ATTRIBUTE
    register_test_suite_math();
    register_test_math_add();
    register_test_math_sub();
#endif
    return mc_run_all_tests();
}
```

## Project Structure

```
myclib/
├── include/
│   └── myclib/
│       ├── aligned_malloc.h   # Aligned memory allocation
│       ├── array.h            # Dynamic array
│       ├── attribute.h        # Compiler attributes
│       ├── hash.h             # Hash functions
│       ├── iter.h             # Iterator interface
│       ├── list.h             # Linked list
│       ├── log.h              # Logging system
│       ├── map.h              # Hash map
│       ├── string.h           # Dynamic string
│       ├── test.h             # Testing framework
│       ├── time.h             # Time utilities
│       ├── type.h             # Type system
│       └── utils.h            # Utility functions
├── src/
│   ├── aligned_malloc.c
│   ├── array.c
│   ├── hash.c
│   ├── list.c
│   ├── log.c
│   ├── map.c
│   ├── string.c
│   ├── test.c
│   ├── time.c
│   └── type.c
├── tests/
│   ├── array_test.c
│   ├── list_test.c
│   ├── map_test.c
│   └── string_test.c
├── CMakeLists.txt
└── README.md
```

## Design Principles

1. **Generic Programming**: All data structures are designed to work with any C type through the type system framework.

2. **Memory Safety**: Careful memory management with clear ownership rules and cleanup functions.

3. **Performance**: Efficient implementations that minimize overhead and maximize speed.

4. **Portability**: Compatibility with C11 standard and major compilers (GCC, Clang, MSVC).

5. **Consistency**: Uniform API design across all modules for ease of use.

6. **Extensibility**: Easy to extend with new types and functionality.

## Testing

The library includes a comprehensive test suite that can be built and run with:

```bash
cmake -DBUILD_TESTS=ON ..
cmake --build .
ctest
```

## License

myclib is licensed under the MIT License. See the LICENSE file for details.

## Contributing

Contributions are welcome! Please feel free to submit issues, feature requests, or pull requests.

## TODO

- Add more data structures (e.g., set, queue, stack)
- Enhance string manipulation capabilities
- Add more utility functions
- Improve documentation
- Add more examples

## Contact

- Email: zhongyuan03@outlook.com

For questions or feedback, please open an issue on the project repository.
