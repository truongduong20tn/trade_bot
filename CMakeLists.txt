cmake_minimum_required(VERSION 3.14)
project(TradeBot)
set(CMAKE_CXX_STANDARD 17)

find_package(OpenSSL REQUIRED)
find_package(CURL REQUIRED)
find_package(Threads REQUIRED)

if(MINGW OR WIN32)
    set(PLATFORM_LIBS ws2_32)
endif()

include(FetchContent)

FetchContent_Declare(json
  GIT_REPOSITORY https://github.com/nlohmann/json.git
  GIT_TAG v3.11.3
)
FetchContent_MakeAvailable(json)

include_directories(
  external/boost_1_74_0
  external/websocketpp
  ${json_SOURCE_DIR}/single_include
  include
)

file(GLOB SOURCES src/*.cpp)
add_executable(TradeBot ${SOURCES})

target_link_libraries(TradeBot
  OpenSSL::SSL
  OpenSSL::Crypto
  CURL::libcurl
  Threads::Threads
  ${PLATFORM_LIBS}
)
configure_file(${CMAKE_SOURCE_DIR}/config.json ${CMAKE_BINARY_DIR}/config.json COPYONLY)
