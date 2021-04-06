cmake_minimum_required(VERSION 3.0.0)
project(coin VERSION 0.1.0)

add_definitions("-DSELF_TEST_COIN")
add_executable(coin coin.c ../chain/chain.c ../bitblock/bitblock.c ../SHA256/mark2/sha256.c ../ecc/ecc.c)

set(CPACK_PROJECT_NAME ${PROJECT_NAME})
set(CPACK_PROJECT_VERSION ${PROJECT_VERSION})
include(CPack)
