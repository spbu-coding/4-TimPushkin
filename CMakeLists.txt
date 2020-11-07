cmake_minimum_required(VERSION 3.7)
project(tests LANGUAGES C)

set(CMAKE_C_STANDARD 99)

add_executable(converter src/converter.c src/message_handler.c src/bmp_handler.c src/str_ending_comparer.c)
add_executable(comparer src/comparer.c src/message_handler.c src/bmp_handler.c src/str_ending_comparer.c)
