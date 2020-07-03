cmake_minimum_required(VERSION 3.0.0)
project(gdrpc VERSION 2.0.3)

add_definitions(-DUNICODE)
set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -D_DEBUG")

file(GLOB_RECURSE SOURCE_FILES src/*.cpp)
add_library(gdrpc SHARED ${SOURCE_FILES})

target_include_directories(gdrpc PRIVATE libraries/fmt/include libraries/toml11 libraries/discord-rpc/include libraries/minhook/include)
target_link_libraries(gdrpc wininet)

add_subdirectory(libraries/fmt)
target_link_libraries(gdrpc fmt)

add_subdirectory(libraries/minhook)
target_link_libraries(gdrpc minhook)

set(BUILD_SHARED_LIBS ON)
set(BUILD_EXAMPLES OFF CACHE BOOL "prevent building discord-rpc examples" FORCE)
include_directories(libraries/rapidjson/include) # discord compilation will fail without this
add_subdirectory(libraries/discord-rpc)
target_link_libraries(gdrpc discord-rpc)

set_target_properties(gdrpc PROPERTIES PREFIX "")
set(CPACK_PROJECT_NAME ${PROJECT_NAME})
set(CPACK_PROJECT_VERSION ${PROJECT_VERSION})