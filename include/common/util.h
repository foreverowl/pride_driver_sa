#ifndef UTIL_H
#define UTIL_H
#include "common/stdafx.h"

llvm::GlobalVariable* findStruct(const std::string& structName, llvm::Module *m);

llvm::Function* findFunctionCallinFunction(const std::string& f, const std::string& targetf, llvm::Module* m);

#endif
