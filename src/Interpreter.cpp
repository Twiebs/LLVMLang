#include "llvm/ExecutionEngine/SectionMemoryManager.h"
#include "llvm/ExecutionEngine/ExecutionEngine.h"
#include "llvm/ExecutionEngine/GenericValue.h"
#include "llvm/ExecutionEngine/MCJIT.h"

#include "llvm/Support/TargetSelect.h"

#include "llvm/IR/LLVMContext.h"
#include "llvm/IR/Module.h"

#include "Common.hpp"
#include "Build.hpp"
#include "Lexer.hpp"

extern "C" void InterpTest() {
	printf("Interpreter Test was sucuessfull\n");
	printf("Hell yeah!!!  Calling a native function in bitcode!\n");
}

// For now this takes a single package until better packaing is
// TODO DOUBLE POINTER
void Print(ASTFunction* function) {
    printf("%s :: (", function->name);
    for (U32 i = 0; i < function->args.size(); i++) {
        ASTVariable* arg = function->args[i];
        if (arg->isPointer) {
            printf("%s : @%s", arg->name, arg->type->name);
        } else {
            printf("%s : %s", arg->name, arg->type->name);
        }
    }
    printf(") ");
    if (function->returnType != nullptr) {
        printf("-> %s", function->returnType->name);
    }

    // TODO functions are designated foregin if they do not contain a body
    // This may be foolish it would probably be better to include a flag that sets it as foreign
    // because it may be a good idea to allow function declerations without definitions
    // however, it may prove to be completly unessecary with the other faciltys in the language
    if (function->members.size() == 0) {
        printf(" FOREIGN");
    }

    printf("\n");
}

// TODO there is no way to declare double pointers in the language!

// TODO @Incomplete does not understand double pointers because they are currently not implemented
void Print(ASTStruct* structDefn) {
    printf("%s :: STRUCT\n", structDefn->name);
    for (U32 i = 0; i < structDefn->memberCount; i++) {
        auto structMember = &structDefn->members[i];
        if (structMember->isPointer) {
            printf("\t%s : @%s\n", structMember->name, structMember->type->name);
        } else {
            printf("\t%s : %s\n", structMember->name, structMember->type->name);
        }
    }
	printf("\n");
}

internal void PrintDefn(ASTNode* node) {
	switch (node->nodeType) {
	case AST_FUNCTION: Print((ASTFunction*)node); break;
	case AST_STRUCT: Print((ASTStruct*)node); break;
	default: assert(false); break;
	}
}


global_variable Package* global_package;
// HACK this is a quick hack for fun
extern "C" void ListAll() {
	Package* package = global_package;
	for (U32 i = 0; i < package->globalBlock.members.size(); i++) {
		auto node = package->globalBlock.members[i];
		PrintDefn(node);
	}
}


void RunInterp (Package* package) {
	global_package = package;
	llvm::InitializeAllTargets();
	llvm::InitializeAllTargetMCs();

	auto memoryManager = std::make_unique<llvm::SectionMemoryManager>();
	auto unique_module = std::make_unique<llvm::Module>("Bang Interpreter", llvm::getGlobalContext());
	auto module = unique_module.get();

	std::vector<llvm::Type*> args;
    args.push_back(llvm::PointerType::getInt8PtrTy(llvm::getGlobalContext(), 0));
	auto funcType = llvm::FunctionType::get(llvm::Type::getVoidTy(llvm::getGlobalContext()), args, false);
	auto testFunc = llvm::Function::Create(funcType, llvm::Function::ExternalLinkage, "Build", module);

	std::string errorStr;
	auto engine = llvm::EngineBuilder(std::move(unique_module)).setMCJITMemoryManager(std::move(memoryManager)).setErrorStr(&errorStr).create();
	if (engine == nullptr) {
		LOG_ERROR("Failed to create ExecutionEngine: " << errorStr);
	}

    // For now the interpreter will just tell you what things are you wont be able to call things
    // It will do its parsing of whatever you give it and then you can determine infromation about the program
    // by typing in the name of an identifer and it will give you static information about what it actualy is

    bool isRunning = true;
    std::string inputBuffer;
    InterpLexer lex;
    while (isRunning) {
        printf("> ");

        std::cin >> lex.buffer;
        lex.begin();
        lex.nextToken();

        if (lex.token.type == TOKEN_IDENTIFIER) {
            Token identToken = lex.token;
            lex.nextToken();
            if (lex.token.type == TOKEN_PAREN_OPEN) {
                lex.nextToken();
				// TODO parse the arguments correctly here!
				lex.nextToken();

                auto node = FindNodeWithIdent(&package->globalBlock, identToken.string);
                if (node == nullptr) {
                    printf("could not find a function named %s", identToken.string.c_str());
                } else {
                    auto funcSet = (ASTFunctionSet*)node;
                    auto function = funcSet->functions[0];
                    std::vector<llvm::GenericValue> args;
                    engine->runFunction((llvm::Function *) function->llvmFunction, args);
                }
            } else {

                auto node = FindNodeWithIdent(&package->globalBlock, identToken.string);

                if (node == nullptr) {
                    printf("That doesnt name anything\n");
                    continue;
                }

                switch (node->nodeType) {
                    case AST_FUNCTION: {
                        auto funcSet = (ASTFunctionSet *) node;
                        printf("Found %d functions named %s\n", funcSet->functions.size(), inputBuffer.c_str());
                        for (U32 i = 0; i < funcSet->functions.size(); i++) {
                            Print((ASTFunction *) funcSet->functions[i]);
                        }
						printf("\n");
                    }
                        break;

                    case AST_STRUCT: {
                        auto structDefn = (ASTStruct *) node;
                        Print(structDefn);
                    }
                        break;

                    case AST_DEFINITION: {
                        printf("Really? You already know what that is.\n");
                    }
                        break;

                    default:
                        printf("Dont know what that is yet but it exists!");
                }
            }

        }
    }
}
