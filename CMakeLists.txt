cmake_minimum_required(VERSION 3.0.0)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

project(gdrpc VERSION 2.2.0)

add_definitions(-DUNICODE)

find_file(WINDOWS_HEADER windows.h)
if(NOT WINDOWS_HEADER)
  message(FATAL_ERROR "Can't find windows.h!")
endif()

file(GLOB_RECURSE SOURCE_FILES src/*.cpp)
add_library(gdrpc SHARED ${SOURCE_FILES})

target_include_directories(gdrpc PRIVATE
  libraries/fmt/include
  libraries/toml11
  libraries/discord-rpc/include
  libraries/minhook/include
  libraries/spdlog/include
  libraries/cpp-httplib
)
target_link_libraries(gdrpc wininet)

add_subdirectory(libraries/fmt)
target_link_libraries(gdrpc fmt)

add_subdirectory(libraries/minhook)
target_link_libraries(gdrpc minhook)

add_subdirectory(libraries/cpp-httplib)
target_link_libraries(gdrpc httplib)

add_definitions(-DSPDLOG_FMT_EXTERNAL)
set(SPDLOG_FMT_EXTERNAL OFF)
add_subdirectory(libraries/spdlog)
target_include_directories(spdlog PRIVATE libraries/fmt/include)
target_link_libraries(gdrpc spdlog)

set(BUILD_SHARED_LIBS ON)
set(BUILD_EXAMPLES OFF CACHE BOOL "prevent building discord-rpc examples" FORCE)
add_subdirectory(libraries/discord-rpc)
target_include_directories(discord-rpc PRIVATE libraries/rapidjson/include) # discord compilation will fail without this
target_link_libraries(gdrpc discord-rpc)

set_target_properties(gdrpc PROPERTIES PREFIX "")
set(CPACK_PROJECT_NAME ${PROJECT_NAME})
set(CPACK_PROJECT_VERSION ${PROJECT_VERSION})