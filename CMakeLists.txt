cmake_minimum_required(VERSION 3.14)

project(httplib_example)

include(FetchContent)

find_package(OpenSSL REQUIRED)
find_package(Threads REQUIRED)

FetchContent_Declare(
    cpp-httplib
    GIT_REPOSITORY https://github.com/yhirose/cpp-httplib.git
    GIT_TAG        v0.18.3  # Укажите нужную версию
)
FetchContent_MakeAvailable(cpp-httplib)

FetchContent_Declare(
    json
    GIT_REPOSITORY https://github.com/nlohmann/json.git
    GIT_TAG v3.11.2
)

FetchContent_MakeAvailable(json)



if(OPENSSL_FOUND)
    set(HTTPLIB_IS_USING_OPENSSL TRUE)
endif()

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(CLIENT_NAME "client")
set(SERVER_NAME "server")

add_executable(${CLIENT_NAME} client_main.cpp)
add_executable(${SERVER_NAME} server_main.cpp)

target_include_directories(${CLIENT_NAME} PRIVATE ${cpp-httplib_SOURCE_DIR})
target_include_directories(${SERVER_NAME} PRIVATE ${cpp-httplib_SOURCE_DIR})

target_link_libraries(${CLIENT_NAME} ${CMAKE_THREAD_LIBS_INIT}
                                     OpenSSL::SSL
                                     OpenSSL::Crypto
                                     nlohmann_json::nlohmann_json)
target_link_libraries(${SERVER_NAME} ${CMAKE_THREAD_LIBS_INIT}
                                     OpenSSL::SSL
                                     OpenSSL::Crypto
                                     nlohmann_json::nlohmann_json)