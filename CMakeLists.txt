cmake_minimum_required(VERSION 3.20)

project(GeoKernelLite VERSION 1.0.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

include(FetchContent)
include(CTest)

add_library(geokernel INTERFACE)
target_include_directories(geokernel INTERFACE
    ${CMAKE_CURRENT_SOURCE_DIR}/core/include
)

FetchContent_Declare(
    nlohmann_json
    URL https://github.com/nlohmann/json/releases/download/v3.11.3/json.tar.xz
)

FetchContent_Declare(
    CLI11
    URL https://github.com/CLIUtils/CLI11/archive/refs/tags/v2.4.2.zip
)

FetchContent_MakeAvailable(nlohmann_json CLI11)

add_executable(geokernel_demo apps/geokernel_demo.cpp)
target_link_libraries(geokernel_demo PRIVATE geokernel nlohmann_json::nlohmann_json CLI11::CLI11)

add_executable(geokernel_benchmarks benchmarks/geokernel_benchmarks.cpp)
target_link_libraries(geokernel_benchmarks PRIVATE geokernel)

if(BUILD_TESTING)
    FetchContent_Declare(
        googletest
        URL https://github.com/google/googletest/archive/refs/tags/v1.14.0.zip
    )
    set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
    FetchContent_MakeAvailable(googletest)

    add_executable(geokernel_tests tests/cpp/test_geokernel.cpp)
    target_link_libraries(geokernel_tests PRIVATE geokernel GTest::gtest_main)

    include(GoogleTest)
    gtest_discover_tests(geokernel_tests)
endif()
