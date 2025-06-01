cmake_minimum_required(VERSION 3.10)
project(cpp_rest)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_executable(cpp_rest src/main.cpp)

target_include_directories(cpp_rest PRIVATE include)

find_package(PkgConfig REQUIRED)
pkg_check_modules(PQXX REQUIRED libpqxx)
find_package(Crow REQUIRED)

target_link_libraries(cpp_rest PRIVATE ${PQXX_LIBRARIES} Crow::Crow)
target_include_directories(cpp_rest PRIVATE ${PQXX_INCLUDE_DIRS})