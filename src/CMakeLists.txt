set(SRCS Build.cpp AST.cpp Lexer.cpp Parser.cpp Analysis.cpp LLVMCodegen.cpp Diagnostic.cpp)

add_executable(bang ${SRCS})
target_link_libraries(bang ${LLVM_LIBS})
install(TARGETS bang DESTINATION /usr/bin)
