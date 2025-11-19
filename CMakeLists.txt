cmake_minimum_required(VERSION 3.10)
project(myclib VERSION 0.1.0 LANGUAGES C)

set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED ON)

option(BUILD_TESTS "Build tests" ON)

add_library(${PROJECT_NAME} STATIC
        src/aligned_malloc.c
        src/array.c
        src/hash.c
        src/list.c
        src/log.c
        src/map.c
        src/string.c
        src/test.c
        src/time.c
        src/type.c
)

target_include_directories(${PROJECT_NAME} PUBLIC include)

if (MSVC)
    target_compile_options(${PROJECT_NAME} PRIVATE /W4 /WX)
else ()
    target_compile_options(${PROJECT_NAME} PRIVATE -Wall -Wextra -Wpedantic -Werror)
endif ()

if (BUILD_TESTS)
    enable_testing()

    function(mc_add_test test_name test_source)
        add_executable(${test_name} ${test_source})
        target_link_libraries(${test_name} PRIVATE ${PROJECT_NAME})
        add_test(NAME ${test_name} COMMAND ${test_name})
    endfunction()

    mc_add_test(array_test tests/array_test.c)
    mc_add_test(list_test tests/list_test.c)
    mc_add_test(map_test tests/map_test.c)
    mc_add_test(string_test tests/string_test.c)
endif ()
