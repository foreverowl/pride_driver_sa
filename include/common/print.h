#ifndef PRINT_H
#define PRINT_H

#include "stdafx.h"
#include "constant.h"

void visualizeControlFlow(std::string& functionname, llvm::Module* m);

void printValue(llvm::Value *V);

void printInst(llvm::Instruction *I);

void printChain(std::vector<llvm::Value*> chain);

void printPath(std::vector<llvm::BasicBlock*>& basicBlocks);

std::string printFuncVal(llvm::Value *currVal, FILE *outputFile, const char *hdr_str);

std::string printStr(llvm::Value *currVal, FILE *outputFile, const char *hdr_str);

#endif 
