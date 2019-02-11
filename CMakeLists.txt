PROJECT(clang-hash C CXX)
cmake_minimum_required(VERSION 3.0)

# Generic system
# removes -rdynamic from the linker, which llvm-ld does not support.
set(CMAKE_SYSTEM_NAME Generic)
set(CMAKE_EXPORT_COMPILE_COMMANDS TRUE)

# LLVM version.
SET(LLVM_RECOMMENDED_VERSION 6.0)
SET(GCC_RECOMMENDED_VERSION 6)


if(NOT DEFINED ${LLVM_ROOT})
  # find llvm-config. prefers to the one with version suffix, Ex:llvm-config-3.4
    # find llvm-config. prefers to the one with version suffix, Ex:llvm-config-3.4
    find_program(LLVM_CONFIG_EXE NAMES "llvm-config-${LLVM_RECOMMENDED_VERSION}"
      HINTS ~/llvm-build/bin)

  if (NOT LLVM_CONFIG_EXE)
    find_program(LLVM_CONFIG_EXE NAMES "llvm-config"
      HINTS ~/llvm-build/bin)
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

# Find the GCC
find_program(GCC_C_COMPILER "gcc-${GCC_RECOMMENDED_VERSION}"
             NAMES gcc)
# Find the G++
find_program(GCC_CXX_COMPILER "g++-${GCC_RECOMMENDED_VERSION}"
             NAMES g++)


# Checks whether a LLVM_COMPILER is found, give a warning if not found.
# A warning instread of error is beceuse that we don't need clang during
# building pinavm.
if(${LLVM_C_COMPILER} STREQUAL "LLVM_C_COMPILER-NOTFOUND")
  message(FATAL "Could not find clang")
endif()
if(${GCC_C_COMPILER} STREQUAL "GCC_C_COMPILER-NOTFOUND")
  message(FATAL "Could not find gcc")
endif()

message(STATUS "LLVM compiler: ${LLVM_C_COMPILER}")
message(STATUS "GCC compiler: ${GCC_C_COMPILER}")
message(STATUS "GXX compiler: ${GCC_CXX_COMPILER}")


# Use System C and CXX compiler
#SET(CMAKE_C_COMPILER "${LLVM_C_COMPILER}")
#SET(CMAKE_CXX_COMPILER "${LLVM_CXX_COMPILER}")

ADD_SUBDIRECTORY(clang-plugin)
ADD_SUBDIRECTORY(gcc-plugin)
ADD_SUBDIRECTORY(wrappers)
