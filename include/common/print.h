#ifndef PRINT_H
#define PRINT_H

#include "common/stdafx.h"
#include "common/constant.h"

void debug(std::string s);

void visualizeControlFlow(std::string& functionname, llvm::Module* m);

void printValue(llvm::Value *V);

void printGlobalVariable(llvm::GlobalVariable *G);

void printInst(llvm::Instruction *I);

void printAttribute(std::vector<std::tuple<std::string, llvm::Value*, llvm::Value*>> attribute_rw,FILE *outputFile);

void printChain(std::vector<llvm::Value*> chain);

void printPath(std::vector<llvm::BasicBlock*>& basicBlocks);

std::string printFuncVal(llvm::Value *currVal, FILE *outputFile, const char *hdr_str);

std::string printStr(llvm::Value *currVal, FILE *outputFile, const char *hdr_str);

#endif 
