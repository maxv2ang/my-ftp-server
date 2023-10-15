cmake_minimum_required(VERSION 3.10)
project(ftpd)

set(CMAKE_CXX_STANDARD 17)

add_subdirectory(src)
add_subdirectory(test)

# Include the directory where FindACE.cmake is located
# set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_SOURCE_DIR}/CMakeModules/")

# Find the ACE library
# find_package(ACE REQUIRED)
# include_directories(${ACE_INCLUDE_DIRS})

# Add the executable and link with ACE library
add_executable(server main.cpp)
target_link_libraries(server ACE libftpd)
