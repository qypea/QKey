
project (test)

set (CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wall -Wextra -Werror -g")

add_executable (extract extract.cpp)
add_executable (test_huff test_huff.c)
