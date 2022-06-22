# CMake 版本要求 (最低 3.1)
cmake_minimum_required (VERSION 3.20)
# 项目名称（建议修改）
project(ElanorBot)
# 设置项目的 C++ 标准
set(CMAKE_CXX_STANDARD 20)

set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)

# 源码以 UTF-8 编码保存，使用 MSVC 编译器时需要加编译参数提供支持
if(MSVC)
	add_compile_options("$<$<C_COMPILER_ID:MSVC>:/utf-8>")
	add_compile_options("$<$<CXX_COMPILER_ID:MSVC>:/utf-8>")
	message(STATUS "optional: /UTF-8")   
endif(MSVC)

add_subdirectory(Elanor)