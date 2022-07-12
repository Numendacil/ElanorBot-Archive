add_library(Client STATIC)

target_link_libraries(Client PUBLIC mirai-cpp)

target_compile_definitions(Client PUBLIC MIRAICPP_STATICLIB)