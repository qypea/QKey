cmake_minimum_required(VERSION 3.9)

project (test)

set (CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wall -Wextra -Werror -g")

find_library (SPRITZ_LIB spritz)
add_executable (extract extract.cpp "${SPRITZ_LIB}/SpritzCipher.c")
target_include_directories (extract SYSTEM PRIVATE "${SPRITZ_LIB}")

add_executable (test_huff test_huff.c)
