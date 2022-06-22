# 不要构建 mirai-cpp 自带的 examples
set(MIRAI_CPP_BUILD_EXAMPLES OFF CACHE BOOL "Build examples" FORCE)
# 添加子模块 mirai-cpp
add_subdirectory(mirai-cpp)

find_package(ZLIB REQUIRED)
find_package(OpenSSL REQUIRED)


add_executable(Elanor)

# 设置头文件目录
include_directories(.)
# 链接 mirai-cpp
target_link_libraries(Elanor PRIVATE mirai-cpp)

target_link_libraries(Elanor PRIVATE ZLIB::ZLIB)

target_link_libraries(Elanor PRIVATE OpenSSL::SSL)

target_link_libraries(Elanor PRIVATE OpenSSL::Crypto)

# target_link_libraries(Elanor PRIVATE -fsanitize=address -static-libasan)

# 使用 mirai-cpp 静态库
target_compile_definitions(Elanor PRIVATE MIRAICPP_STATICLIB)

if (CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
	# 链接 pthread
	target_link_libraries(Elanor PRIVATE pthread)
	# 静态链接 libgcc 和 libstdc++
	target_link_libraries(Elanor PRIVATE -static-libgcc -static-libstdc++)
endif()

add_subdirectory(app)
add_subdirectory(Command)
add_subdirectory(State)
add_subdirectory(Trigger)
add_subdirectory(third-party)
add_subdirectory(Utils)