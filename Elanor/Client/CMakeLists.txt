
add_library(Client STATIC
		Client.hpp
		Client.cpp)

target_link_libraries(Client PUBLIC mirai-cpp)

target_compile_definitions(Client PUBLIC MIRAICPP_STATICLIB)

add_subdirectory(mirai-cpp)