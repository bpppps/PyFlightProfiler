#include "ProcessUtils.h"
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <dirent.h>
#include <dlfcn.h>
#include <errno.h>
#include <fstream>
#include <iostream>
#include <limits.h>
#include <sstream>
#include <unistd.h>
#include <vector>

/**
 * @brief Find a free memory address in the target process
 *
 * Parses /proc/[pid]/maps to find a memory region with execute permissions.
 *
 * @param process_id PID of the target process
 * @return Address of free memory, or 0 on failure
 */
long ProcessUtils::findFreeMemoryAddress(pid_t process_id) {
  std::string filename = "/proc/" + std::to_string(process_id) + "/maps";
  std::ifstream maps_file(filename);

  if (!maps_file.is_open()) {
    std::cerr << "Failed to open " << filename << std::endl;
    exit(1);
  }

  std::string line;
  long address = 0;

  while (std::getline(maps_file, line)) {
    std::istringstream iss(line);
    std::string range, permissions, offset, device, inode, path;

    if (iss >> range >> permissions >> offset >> device >> inode) {
      // Extract address from range (format: address1-address2)
      size_t dash_pos = range.find('-');
      if (dash_pos != std::string::npos) {
        std::string address_str = range.substr(0, dash_pos);
        address = std::stol(address_str, nullptr, 16);
      }

      if (permissions.find('x') != std::string::npos) {
        break;
      }
    }
  }

  return address;
}

/**
 * @brief Get the base address of libc in the target process
 *
 * Parses /proc/[pid]/maps to find the base address of libc or similar
 * libraries.
 *
 * @param process_id PID of the target process
 * @return Base address of libc, or 0 on failure
 */
long ProcessUtils::getLibcBaseAddress(pid_t process_id) {
  std::string filename = "/proc/" + std::to_string(process_id) + "/maps";
  std::ifstream maps_file(filename);

  if (!maps_file.is_open()) {
    std::cerr << "Failed to open " << filename << std::endl;
    exit(1);
  }

  std::string line;
  long address = 0;

  const std::vector<std::string> libc_patterns = {"libc-", "libc.so.",
                                                  "libc.musl-"};

  while (std::getline(maps_file, line)) {
    std::istringstream iss(line);
    std::string range, permissions, offset, device, inode, path;

    if (iss >> range >> permissions >> offset >> device >> inode) {
      // Extract address from range (format: address1-address2)
      size_t dash_pos = range.find('-');
      if (dash_pos != std::string::npos) {
        std::string address_str = range.substr(0, dash_pos);
        address = std::stol(address_str, nullptr, 16);
      }

      bool found = false;
      for (const auto &pattern : libc_patterns) {
        if (line.find(pattern) != std::string::npos) {
          found = true;
          break;
        }
      }

      if (found) {
        break;
      }
    }
  }

  return address;
}

/**
 * @brief Check if a library is loaded in the target process
 *
 * Parses /proc/[pid]/maps to check if a library with the specified name is
 * loaded.
 *
 * @param process_id PID of the target process
 * @param library_name Name of the library to check
 * @return true if the library is loaded, false otherwise
 */
bool ProcessUtils::isLibraryLoaded(pid_t process_id,
                                   const std::string &library_name) {
  std::string filename = "/proc/" + std::to_string(process_id) + "/maps";
  std::ifstream maps_file(filename);

  if (!maps_file.is_open()) {
    std::cerr << "Failed to open " << filename << std::endl;
    exit(1);
  }

  std::string line;
  bool loaded = false;

  while (std::getline(maps_file, line)) {
    if (line.find(library_name) != std::string::npos) {
      loaded = true;
      break;
    }
  }

  return loaded;
}

/**
 * @brief Resolve the address of a function in the current process
 *
 * Uses dlopen and dlsym to resolve the address of a function in libc.so.6.
 *
 * @param function_name Name of the function to resolve
 * @return Address of the function, or 0 on failure
 */
long ProcessUtils::resolveFunctionAddress(const std::string &function_name) {
  void *self = dlopen("libc.so.6", RTLD_LAZY);
  if (!self) {
    return 0;
  }

  void *function_address = dlsym(self, function_name.c_str());
  if (!function_address) {
    dlclose(self);
    return 0;
  }

  long address = reinterpret_cast<long>(function_address);
  dlclose(self);

  return address;
}

/**
 * @brief Locate the return instruction in a function
 *
 * Searches backwards from the end address to find the return instruction
 * (0xc3).
 *
 * @param end_address End address of the function
 * @return Pointer to the return instruction
 */
unsigned char *ProcessUtils::locateReturnInstruction(void *end_address) {
  unsigned char *return_instruction_address =
      static_cast<unsigned char *>(end_address);
  while (*return_instruction_address != 0xc3) { // 0xc3 is the opcode for 'ret'
    return_instruction_address--;
  }
  return return_instruction_address;
}
