add_library(libftpd STATIC utils.cpp server_pi.cpp server_dtp.cpp)

target_include_directories(libftpd PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include)