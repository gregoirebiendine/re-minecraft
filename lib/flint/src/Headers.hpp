#ifndef INCLUDED_HEADERS_HPP
#define INCLUDED_HEADERS_HPP

#include <algorithm>
#include <cmath>
#include <cxxabi.h>
#include <exception>
#include <filesystem>
#include <initializer_list>
#include <iostream>
#include <map>
#include <memory>
#include <optional>
#include <ostream>
#include <sstream>
#include <string>
#include <unordered_map>
#include <unordered_set>
#include <utility>
#include <vector>

namespace fs = std::filesystem;

// get break trace
#ifdef _WIN32
    #include <windows.h>
#elif defined(__linux__) || defined(__APPLE__)
    #include <execinfo.h>
#endif

#endif
