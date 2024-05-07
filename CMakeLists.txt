cmake_minimum_required(VERSION 3.28)
project(harmony)

set(CMAKE_CXX_STANDARD 26)



set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Werror -fsanitize=address -fno-sanitize-recover=address -fno-omit-frame-pointer -fsanitize=undefined -fno-sanitize-recover=undefined -fno-omit-frame-pointer")
# set(CMAKE_CXX_CLANG_TIDY clang-tidy;--checks=*,-llvm-header-guard)

find_package(fmt REQUIRED)
find_package(PkgConfig)
pkg_check_modules(liburing REQUIRED IMPORTED_TARGET GLOBAL liburing>=2.0)
find_package(spdlog CONFIG REQUIRED)

add_executable(harmony server_test.cpp)
target_include_directories(harmony PUBLIC include)
target_link_libraries(harmony PRIVATE fmt::fmt PkgConfig::liburing)
