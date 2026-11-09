# SPDX-License-Identifier: MIT
cmake_minimum_required(VERSION 3.20.0)
find_package(Zephyr REQUIRED HINTS $ENV{ZEPHYR_BASE})
project(icom_fw C)

set(ICOM_LIB ${CMAKE_CURRENT_SOURCE_DIR}/../lib/icom)
file(GLOB ICOM_SRC ${ICOM_LIB}/src/*.c)

target_sources(app PRIVATE src/main.c ${ICOM_SRC})
target_include_directories(app PRIVATE ${ICOM_LIB}/include)
target_compile_options(app PRIVATE -Wall -Wextra -Werror)
