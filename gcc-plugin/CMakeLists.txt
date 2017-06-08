execute_process(COMMAND ${GCC_CXX_COMPILER} -print-file-name=plugin
  OUTPUT_VARIABLE GCCPLUGINS_DIR
  OUTPUT_STRIP_TRAILING_WHITESPACE)

add_definitions(-std=c++11 -Wno-strict-aliasing -Wall -Wextra -fno-rtti)
include_directories(${GCCPLUGINS_DIR}/include)

add_library(gcc-hash SHARED
  chash.cc
  )
