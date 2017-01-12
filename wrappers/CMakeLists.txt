# These two wrappers are used to gather in-depth information about the
# hashing process. Using them does not yield proper timing information
# about the overall compilation process.
configure_file(clang-hash.in ${PROJECT_BINARY_DIR}/wrappers/clang-hash)
configure_file(clang-hash-collect.in ${PROJECT_BINARY_DIR}/wrappers/clang-hash-collect)

# These wrappers can be used for the actual (re)compilation of whole procects.
execute_process(
  COMMAND "${CMAKE_COMMAND}" "-E" "create_symlink"
    "${LLVM_C_COMPILER}" "${CMAKE_CURRENT_BINARY_DIR}/clang-normal"
)

configure_file(clang-hash-stop.in ${PROJECT_BINARY_DIR}/wrappers/clang-hash-stop)


# Symlink compiler wrappers
execute_process(
  COMMAND "${CMAKE_COMMAND}" "-E" "create_symlink"
    "${CMAKE_CURRENT_SOURCE_DIR}/gcc" "${CMAKE_CURRENT_BINARY_DIR}/gcc"
)

execute_process(
  COMMAND "${CMAKE_COMMAND}" "-E" "create_symlink"
    "${CMAKE_CURRENT_SOURCE_DIR}/compare-logs" "${CMAKE_CURRENT_BINARY_DIR}/compare-logs"
)

execute_process(
  COMMAND "${CMAKE_COMMAND}" "-E" "create_symlink"
    "${CMAKE_CURRENT_SOURCE_DIR}/hash-objectfile" "${CMAKE_CURRENT_BINARY_DIR}/hash-objectfile"
)
