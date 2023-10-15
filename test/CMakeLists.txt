include_directories("${CMAKE_CURRENT_SOURCE_DIR}/../src/include")

add_definitions(-Wwrite-strings -fprofile-arcs  -ftest-coverage)

add_executable(
    test
    main.cpp
    test_server_dtp.cpp
    test_server_pi.cpp
    ../src/utils.cpp
    ../src/server_dtp.cpp
    ../src/server_pi.cpp
)
target_link_libraries(test ACE gtest gcov)