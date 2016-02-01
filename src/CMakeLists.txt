# Use llvm-config to get all necessary compile options
execute_process(COMMAND ${LLVM_CONFIG_EXE} --cxxflags
  OUTPUT_VARIABLE LLVM_CXXFLAGS
  OUTPUT_STRIP_TRAILING_WHITESPACE)

execute_process(COMMAND ${LLVM_CONFIG_EXE} --obj-root
  OUTPUT_VARIABLE LLVM_OBJROOT
  OUTPUT_STRIP_TRAILING_WHITESPACE)

execute_process(COMMAND ${LLVM_CONFIG_EXE} --src-root
  OUTPUT_VARIABLE LLVM_SRCROOT
  OUTPUT_STRIP_TRAILING_WHITESPACE)

execute_process(COMMAND ${LLVM_CONFIG_EXE} --ldflags
  OUTPUT_VARIABLE LLVM_LDFLAGS
  OUTPUT_STRIP_TRAILING_WHITESPACE)

add_definitions(${LLVM_CXXFLAGS} -std=c++11)
include_directories(${LLVM_OBJROOT}/tools/clang/include)
include_directories(${LLVM_SRCROOT}/tools/clang/include)

add_library(clang-hash SHARED
  clang-hash.cc
  hash-visitor.cc)

SET_TARGET_PROPERTIES(clang-hash PROPERTIES LINK_FLAGS ${LLVM_LDFLAGS})

configure_file(clang-hash.in ${PROJECT_BINARY_DIR}/clang-hash)
