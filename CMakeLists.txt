PROJECT(clang-hash C CXX)
cmake_minimum_required(VERSION 3.0)

# Generic system
# removes -rdynamic from the linker, which llvm-ld does not support.
set(CMAKE_SYSTEM_NAME Generic)

# LLVM version.
SET(LLVM_RECOMMENDED_VERSION 3.9)

if(NOT DEFINED ${LLVM_ROOT})
  # find llvm-config. prefers to the one with version suffix, Ex:llvm-config-3.4
    # find llvm-config. prefers to the one with version suffix, Ex:llvm-config-3.4
    find_program(LLVM_CONFIG_EXE NAMES "llvm-config-${LLVM_RECOMMENDED_VERSION}"
      HINTS ~/llvm-build/bin /proj/i4vamos/tools/llvm-3.7/llvm-build/bin)

  if (NOT LLVM_CONFIG_EXE)
    find_program(LLVM_CONFIG_EXE NAMES "llvm-config"
      HINTS ~/llvm-build/bin /proj/i4vamos/tools/llvm-3.7/llvm-build/bin)
  endif()

  # Get the directory of llvm by using llvm-config. also remove whitespaces.
  execute_process(COMMAND ${LLVM_CONFIG_EXE} --prefix OUTPUT_VARIABLE LLVM_ROOT
    OUTPUT_STRIP_TRAILING_WHITESPACE )
endif()

message(STATUS "LLVM root: ${LLVM_ROOT}")

# Find a compiler which compiles c source into llvm bitcode.
# It first finds clang, then it finds llvm-g++ if there is no clang.
find_program(LLVM_C_COMPILER "clang-${LLVM_RECOMMENDED_VERSION}"
                NAMES clang
                HINTS ${LLVM_ROOT}/bin )
# Find a compiler which compiles c++ source into llvm bitcode.
# It first finds clang, then it finds llvm-g++ if there is no clang.
find_program(LLVM_CXX_COMPILER "clang++-${LLVM_RECOMMENDED_VERSION}"
                NAMES clang++
                HINTS ${LLVM_ROOT}/bin )


# Checks whether a LLVM_COMPILER is found, give a warning if not found.
# A warning instread of error is beceuse that we don't need clang during
# building pinavm.
if(${LLVM_C_COMPILER} STREQUAL "LLVM_C_COMPILER-NOTFOUND")
  message(FATAL "Could not find clang")
endif()

message(STATUS "LLVM compiler: ${LLVM_C_COMPILER}")

# Use System C and CXX compiler
#SET(CMAKE_C_COMPILER "${LLVM_C_COMPILER}")
#SET(CMAKE_CXX_COMPILER "${LLVM_CXX_COMPILER}")

ADD_SUBDIRECTORY(src)
ADD_SUBDIRECTORY(wrappers)
