cmake_minimum_required(VERSION 3.26)
project(sockets)

set(CMAKE_CXX_STANDARD 23)

# set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Werror -fsanitize=address -fno-sanitize-recover=address -fno-omit-frame-pointer -fsanitize=undefined -fno-sanitize-recover=undefined -fno-omit-frame-pointer")

find_package(fmt REQUIRED)
find_package(PkgConfig REQUIRED)
pkg_check_modules(PC_liburing REQUIRED liburing)

find_path(liburing_INCLUDE_DIR
        NAMES liburing.h
        PATHS ${PC_liburing_INCLUDE_DIRS}
)
find_library(liburing_LIBRARY
        NAMES uring
        PATHS ${PC_liburing_LIBRARY_DIRS}
)

set(liburing_VERSION ${PC_liburing_VERSION})

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(liburing
        FOUND_VAR liburing_FOUND
        REQUIRED_VARS
        liburing_LIBRARY
        liburing_INCLUDE_DIR
        VERSION_VAR liburing_VERSION
)

if(liburing_FOUND AND NOT TARGET liburing::liburing)
    add_library(liburing::liburing UNKNOWN IMPORTED)
    set_target_properties(liburing::liburing PROPERTIES
            IMPORTED_LOCATION "${liburing_LIBRARY}"
            INTERFACE_INCLUDE_DIRECTORIES "${liburing_INCLUDE_DIR}"
    )
endif()

mark_as_advanced(
        liburing_INCLUDE_DIR
        liburing_LIBRARY
)


add_executable(server
        src/server.cpp
        temp/server_client.hpp
        src/http/http_builder.hpp
        src/server/async_client_acceptor.hpp
        src/server/async_socket.hpp
        tests/simdjson.cpp
)

target_include_directories(server PUBLIC tests)
target_link_libraries(server PRIVATE fmt::fmt liburing::liburing)
