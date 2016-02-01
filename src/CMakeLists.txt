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

execute_process(COMMAND ${LLVM_CONFIG_EXE} --libs --system-libs
  OUTPUT_VARIABLE LLVM_LIBS
  OUTPUT_STRIP_TRAILING_WHITESPACE)

add_definitions(${LLVM_CXXFLAGS})

include_directories(${LLVM_OBJROOT}/tools/clang/include)
include_directories(${LLVM_SRCROOT}/tools/clang/include)

add_executable(clang-hash clang-hash.cc)

SET_TARGET_PROPERTIES(clang-hash PROPERTIES LINK_FLAGS ${LLVM_LDFLAGS})

message(${LLVM_LIBS})
# Link against LLVM libraries
target_link_libraries(clang-hash
  -Wl,--start-group
  clangAST
  clangAnalysis
  clangBasic
  clangDriver
  clangEdit
  clangFrontend
  clangFrontendTool
  clangLex
  clangParse
  #clangSema
  #clangEdit
  clangASTMatchers
  #clangRewrite
  #clangRewriteFrontend
  #clangStaticAnalyzerFrontend
  #clangStaticAnalyzerCheckers
  #clangStaticAnalyzerCore
  #clangSerialization
  clangToolingCore
  clangTooling
  # System Libs
  tinfo
  pthread
  dl m z
  # LLVM
  LLVMSupport
  )
