configure_file(clang-hash.in ${PROJECT_BINARY_DIR}/wrappers/clang-hash)

# Symlink compiler wrappers
execute_process(
  COMMAND "${CMAKE_COMMAND}" "-E" "create_symlink"
    "${CMAKE_CURRENT_SOURCE_DIR}/gcc" "${CMAKE_CURRENT_BINARY_DIR}/gcc"
)

execute_process(
  COMMAND "${CMAKE_COMMAND}" "-E" "create_symlink"
    "${CMAKE_CURRENT_SOURCE_DIR}/compare-logs" "${CMAKE_CURRENT_BINARY_DIR}/compare-logs"
)
