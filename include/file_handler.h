#pragma once

#include <filesystem>
#include <string>
#include <fstream>

bool readfiletoString(const std::filesystem::path &p, std::string &out);