# 不要构建 mirai-cpp 自带的 examples
set(MIRAI_CPP_BUILD_EXAMPLES OFF CACHE BOOL "Build examples" FORCE)
# 添加子模块 mirai-cpp
add_subdirectory(mirai-cpp)

find_package(ZLIB REQUIRED)
find_package(OpenSSL REQUIRED)


add_executable(ElanorBot)

# 设置头文件目录
include_directories(.)
# # 链接 mirai-cpp
# target_link_libraries(ElanorBot PRIVATE mirai-cpp)
target_link_libraries(ElanorBot PRIVATE Client)

target_link_libraries(ElanorBot PRIVATE ZLIB::ZLIB)

target_link_libraries(ElanorBot PRIVATE OpenSSL::SSL)

target_link_libraries(ElanorBot PRIVATE OpenSSL::Crypto)

# target_link_libraries(ElanorBot PRIVATE -fsanitize=address -static-libasan)

# target_compile_options(ElanorBot PRIVATE -fsanitize=address)
# target_link_options(ElanorBot PRIVATE -fsanitize=address)

# 使用 mirai-cpp 静态库
target_compile_definitions(ElanorBot PRIVATE MIRAICPP_STATICLIB)

if (CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
	# 链接 pthread
	target_link_libraries(ElanorBot PRIVATE pthread)
	# 静态链接 libgcc 和 libstdc++
	target_link_libraries(ElanorBot PRIVATE -static-libgcc -static-libstdc++)
endif()

add_subdirectory(Client)
# add_subdirectory(Command)
# add_subdirectory(State)
# add_subdirectory(Trigger)
add_subdirectory(ThirdParty)
# add_subdirectory(Utils)