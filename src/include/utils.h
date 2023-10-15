#ifndef COMMON_H
#define COMMON_H

#include "ace/OS.h"
#include <string>
#include <vector>

std::string get_realpath(const std::string& path);
bool is_dir_exist(const std::string& path);
bool startsWith(const std::string& str, const std::string prefix);
bool get_size(const std::string file_path, size_t& size);
int rand_port();
void get_mode(mode_t mode, char* buf);
void get_ls_line(const char* filename, struct stat* st, char* buf, char* tmp);
int get_ls_data(const char* path, std::vector<std::string>& list);

#endif // COMMON_H