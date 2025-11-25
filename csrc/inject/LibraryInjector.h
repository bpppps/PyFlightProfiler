#ifndef LIBRARY_INJECTOR_H
#define LIBRARY_INJECTOR_H

#include "ExitCode.h"
#include "ProcessTracer.h"
#include "ProcessUtils.h"
#include <memory>
#include <string>
#include <sys/user.h>
#include <vector>

/**
 * @brief Manages the injection of shared libraries into target processes
 *
 * This class implements a comprehensive solution for injecting shared libraries
 * into target processes using advanced ptrace-based techniques. It handles
 * all aspects of the injection process including preparation, execution, and
 * verification.
 */
class LibraryInjector {
public:
  /**
   * @brief Constructs a LibraryInjector instance
   * @param target_process_id PID of the process to inject the library into
   * @param shared_library_file_path File path of the shared library to inject
   * @param debug_mode Enable debug logging
   */
  LibraryInjector(pid_t target_process_id,
                  const std::string &shared_library_file_path,
                  bool debug_mode = false);

  /**
   * @brief Cleans up resources used by the LibraryInjector
   */
  ~LibraryInjector();

  /**
   * @brief Performs the complete library injection process
   * @return true if the injection completed successfully, false otherwise
   */
  ExitCode performInjection();

private:
  // Core instance attributes
  pid_t target_process_id_;
  std::string library_file_path_;
  ProcessTracer process_tracer_;

  // Injection workflow methods
  ExitCode initializeInjectionEnvironment(long &code_injection_address,
                                          REG_TYPE *original_registers,
                                          REG_TYPE *working_registers);
  ExitCode orchestrateInjectionSequence(long injection_address,
                                        long malloc_function_address,
                                        long free_function_address,
                                        long dlopen_function_address,
                                        int library_path_string_length,
                                        REG_TYPE *initial_registers);
  ExitCode confirmInjectionSuccess(long injection_memory_location,
                                   const std::vector<char> &backup_memory_data,
                                   size_t shellcode_byte_size,
                                   REG_TYPE *original_register_state);

  // Shellcode generation methods
  std::vector<char> createShellcodePayload(size_t &payload_size,
                                           intptr_t &return_instruction_offset);

  // Path manipulation utilities
  void getParentDirectoryPath(std::string &file_path);
};

#endif // LIBRARY_INJECTOR_H
