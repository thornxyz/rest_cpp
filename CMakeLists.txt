cmake_minimum_required(VERSION 3.10)
project(cpp_rest)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Create executable
add_executable(cpp_rest main.cpp)

# Add include directory
target_include_directories(cpp_rest PRIVATE include)

# Find required packages
find_package(PkgConfig REQUIRED)

# Find libpqxx and jsoncpp
pkg_check_modules(PQXX REQUIRED libpqxx)
pkg_check_modules(JSONCPP REQUIRED jsoncpp)

# Link libraries
target_link_libraries(cpp_rest PRIVATE 
    ${PQXX_LIBRARIES}
    ${JSONCPP_LIBRARIES}
)

# Add include directories
target_include_directories(cpp_rest PRIVATE 
    ${PQXX_INCLUDE_DIRS}
    ${JSONCPP_INCLUDE_DIRS}
)

# Add compile flags if needed
target_compile_options(cpp_rest PRIVATE 
    ${PQXX_CFLAGS_OTHER}
    ${JSONCPP_CFLAGS_OTHER}
)