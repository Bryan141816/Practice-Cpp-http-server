cmake_minimum_required(VERSION 3.10.0)
project(Server VERSION 0.1.0 LANGUAGES C CXX)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
include_directories(include)

add_executable(Server 
            src/main.cpp 
            src/mimetype_utils.cpp
            src/url_utils.cpp
            src/http_request.cpp
            src/http_response.cpp
            src/file_handler.cpp
            src/server_handler.cpp
            src/global_router.cpp
            src/dynamic_router_handler.cpp
)

target_link_libraries(Server PRIVATE Ws2_32)
include(CTest)
enable_testing()

