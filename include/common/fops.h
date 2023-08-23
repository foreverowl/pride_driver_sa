#ifndef FILE_OPERATIONS_H
#define FILE_OPERATIONS_H

#include "common/stdafx.h"
#include "common/print.h"
#include "common/constant.h"

void process_file_operations_st(llvm::GlobalVariable *currGlobal, FILE *outputFile, std::unordered_map<std::string,std::string>& entry);

llvm::BasicBlock* getReturnBlock(llvm::Function *F);

void bfs(llvm::BasicBlock* entry, llvm::BasicBlock* target, std::vector<std::vector<llvm::BasicBlock*>>& paths, llvm::BasicBlock* anchor);

std::vector<std::vector<llvm::BasicBlock*>> getAllPaths(llvm::BasicBlock* entry, llvm::BasicBlock* target, llvm::BasicBlock* anchor);

//ioctl arg recovery in copy_to_user/copy_from_user
llvm::Value* findValue_in_StoreInst(std::string& ptr, std::vector<llvm::BasicBlock*> path, std::vector<llvm::Value*>& chain);
llvm::Type* copy_ud_chain_recovery(llvm::Value* V, std::vector<llvm::Value*>& chain, std::vector<std::vector<llvm::BasicBlock*>> paths);
llvm::Type* findCopyType(llvm::Value* copy, llvm::Value* V, llvm::BasicBlock* begin, llvm::BasicBlock* end, llvm::Module *m);

//ioctl arg recovery when cast
llvm::Value* findPtr_in_StoreInst(std::string& val, std::vector<llvm::BasicBlock*> path, std::vector<llvm::Value*>& chain);
llvm::Value* findLoadInst_by_Ptr(std::string& ptr, std::vector<llvm::BasicBlock*> path, std::vector<llvm::Value*>& chain);
llvm::Value* findInToPtrInst_by_src(std::string& src, std::vector<llvm::BasicBlock*> path, std::vector<llvm::Value*>& chain);
llvm::Type* cast_ud_chain_recovery(std::string& argname, std::vector<llvm::Value*>& chain, std::vector<std::vector<llvm::BasicBlock*>> paths);
llvm::Type* findCastType(std::string& argname, llvm::BasicBlock* begin, llvm::BasicBlock* end, llvm::BasicBlock* target, llvm::Module *m);

void process_ioctl(std::string& ioctl_func, llvm::Module *m, std::vector<std::tuple<int64_t,llvm::Type*,std::string>>& ioctl_info);

#endif 
