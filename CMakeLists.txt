cmake_minimum_required(VERSION 4.0.7)

include(include_sdk.cmake)

project(interchange C CXX ASM)

pico_sdk_init()

set(CMAKE_EXPORT_COMPILE_COMMANDS 1)
execute_process(
	COMMAND git rev-parse --short HEAD
  OUTPUT_VARIABLE GIT_REVISION
	OUTPUT_STRIP_TRAILING_WHITESPACE
)

add_subdirectory(prototype)
