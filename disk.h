// REVIEWED
#pragma once
#include <string>
#include <stdint.h>
#include <iostream>
#include <tr1/memory>
#include <iosfwd>
#include <vector>
#include "logger_decls.h"

using std::tr1::shared_ptr;

bool folder_exists(const std::string &path);
bool file_exists(const std::string &file_path);
bool get_line_col(const std::string &file_path, size_t offset, size_t &line, size_t &col);
off_t file_size(const char *filename);
std::string directory_from_file_path(const std::string &file_path);
std::string leaf_from_file_path(const std::string &file_path);
bool ensure_directory_exists(const std::string &name);
bool move_files(const std::string &source, const std::string &dest);
bool list_files(const std::string &folder, const std::string &match, std::vector<std::string> &leaf_names);
