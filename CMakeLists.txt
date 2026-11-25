cmake_minimum_required(VERSION 3.16)
project(MiniVectorForge VERSION 0.1.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

if(MSVC)
  set(VF_WARNINGS /W4)
else()
  set(VF_WARNINGS -Wall -Wextra -Wpedantic)
endif()

add_library(vf_core
  src/distance.cpp
  src/vector_store.cpp
)
target_include_directories(vf_core PUBLIC include)
target_compile_options(vf_core PRIVATE ${VF_WARNINGS})

include(FetchContent)
FetchContent_Declare(
  googletest
  URL https://github.com/google/googletest/archive/refs/tags/v1.14.0.zip
)
set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
FetchContent_MakeAvailable(googletest)

enable_testing()
add_executable(vf_tests
  tests/test_distance.cpp
  tests/test_vector_store.cpp
)
target_link_libraries(vf_tests PRIVATE vf_core GTest::gtest_main)
target_compile_options(vf_tests PRIVATE ${VF_WARNINGS})

include(GoogleTest)
gtest_discover_tests(vf_tests)