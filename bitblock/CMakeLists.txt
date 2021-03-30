cmake_minimum_required(VERSION 3.0.0)
project(bitblock VERSION 0.1.0)

add_definitions("-DSELF_TEST_BITBLK")
add_executable(bitblock bitblock.c ../SHA256/mark2/sha256.c)

set(CPACK_PROJECT_NAME ${PROJECT_NAME})
set(CPACK_PROJECT_VERSION ${PROJECT_VERSION})
include(CPack)
