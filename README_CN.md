# myclib

[English](README.md) | [中文](README_CN.md)

一个轻量级、高效且易于使用的 C 库，为 C 程序员提供必要的数据结构和实用工具。

## 概述

myclib 旨在填补标准 C 库的极简功能与更重量级替代方案之间的空白。它提供了一套经过充分测试的通用数据结构和实用工具，遵循现代 C 编程实践，同时保持与 C11 标准的兼容性。

## 特性

### 核心数据结构

- **Array**: 动态数组实现，支持泛型类型、自动调整大小和各种操作
- **List**: 双向链表，支持泛型元素
- **Map**: 基于哈希表的键值映射，支持泛型键和值
- **String**: 动态字符串实现，提供丰富的字符串操作函数

### 实用工具

- **Type System**: 泛型类型元数据和操作框架
- **Logging**: 灵活的日志系统，支持多个级别和格式化选项
- **Time**: 高分辨率时间测量工具
- **Iterators**: 所有数据结构的统一迭代器接口
- **Memory Management**: 对齐内存分配函数
- **Hash Functions**: 各种数据类型的高效哈希实现
- **Attribute Support**: 跨平台编译器属性宏
- **Testing Framework**: 轻量级单元测试工具

## 快速开始

### 前提条件

- 兼容 C11 的编译器（GCC、Clang、MSVC）
- CMake 3.10 或更高版本

### 构建库

```bash
# 创建构建目录
mkdir build

# 使用CMake配置
cmake -B build -DCMAKE_BUILD_TYPE=Release

# 构建库
cmake --build build

# 可选：运行测试
ctest --test-dir build
```

### 使用库

1. **包含头文件**：

```c
#include <myclib/array.h>
#include <myclib/string.h>
#include <myclib/log.h>
```

2. **链接库**：

```bash
gcc your_program.c -lmyclib -I/path/to/myclib/include -L/path/to/myclib/lib
```

## 示例

### 使用动态数组

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

### 使用动态字符串

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

### 使用映射

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
    mc_map_insert(&map, &key, &value); /* 键已移至此处 */

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

### 使用日志

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

### 使用测试框架

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

## 项目结构

```
myclib/
├── include/
│   └── myclib/
│       ├── aligned_malloc.h   # 对齐内存分配
│       ├── array.h            # 动态数组
│       ├── attribute.h        # 编译器属性
│       ├── hash.h             # 哈希函数
│       ├── iter.h             # 迭代器接口
│       ├── list.h             # 链表
│       ├── log.h              # 日志系统
│       ├── map.h              # 哈希映射
│       ├── string.h           # 动态字符串
│       ├── test.h             # 测试框架
│       ├── time.h             # 时间工具
│       ├── type.h             # 类型系统
│       └── utils.h            # 实用函数
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

## 设计原则

1. **泛型编程**：所有数据结构都设计为通过类型系统框架与任何 C 类型一起使用。

2. **内存安全**：仔细的内存管理，明确的所有权规则和清理函数。

3. **性能**：高效的实现，最小化开销并最大化速度。

4. **可移植性**：与 C11 标准和主要编译器（GCC、Clang、MSVC）兼容。

5. **一致性**：所有模块之间的统一 API 设计，易于使用。

6. **可扩展性**：易于扩展新类型和功能。

## 测试

库包含一个全面的测试套件，可以使用以下命令构建和运行：

```bash
cmake -DBUILD_TESTS=ON ..
cmake --build .
ctest
```

## 许可证

myclib 采用 MIT 许可证。有关详细信息，请参阅 LICENSE 文件。

## 贡献

欢迎贡献！请随时提交问题、功能请求或拉取请求。

## TODO

- 添加更多数据结构（例如，集合、队列、栈）
- 增强字符串操作能力
- 添加更多实用函数
- 改进文档
- 添加更多示例

## 联系方式

- 电子邮件：zhongyuan03@outlook.com

如有问题或反馈，请在项目仓库上打开一个 issue。
