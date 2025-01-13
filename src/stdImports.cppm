// std_imports.cppm

module; // Global fragment

// All system includes here
#include <algorithm>
#include <compare>
#include <cstddef> // for size_t
#include <cstdint>
#include <deque>
#include <iostream>
#include <memory>
#include <random>
#include <stdexcept>
#include <string>
#include <unistd.h>
#include <unordered_map>
#include <vector>

export module stdImports;

// Export everything needed
export {
  // Make std namespace contents visible
  namespace std {
  // Forward all the types and functions we need
  // Basic types
  using ::std::size_t;

  // Containers
  using ::std::string;
  using ::std::vector;

  // Utilities
  using ::std::cout;
  using ::std::endl;
  using ::std::make_shared;
  using ::std::mt19937;
  using ::std::out_of_range;
  using ::std::random_device;
  using ::std::shared_ptr;
  using ::std::shuffle;

  // Iterator support
  using ::std::begin;
  using ::std::end;
  } // namespace std
}
