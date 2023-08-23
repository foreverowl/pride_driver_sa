#include "common/fops.h"

void process_file_operations_st(llvm::GlobalVariable *currGlobal, FILE *outputFile, std::unordered_map<std::string,std::string>& entry) {
	//std::cout<<"entry"<<std::endl;
    if(currGlobal->hasInitializer()) {
		//std::cout<<"fops"<<std::endl;
        // get the initializer.
        llvm::Constant *targetConstant = currGlobal->getInitializer();
        llvm::ConstantStruct *actualStType = llvm::dyn_cast<llvm::ConstantStruct>(targetConstant);
        if(actualStType != nullptr) {
            // read: 2
            if (actualStType->getNumOperands() > 2) {
                entry.emplace(READ_HDR,printFuncVal(actualStType->getOperand(2), outputFile, READ_HDR));
            }
            // write: 3
            if (actualStType->getNumOperands() > 3) {
                entry.emplace(WRITE_HDR,printFuncVal(actualStType->getOperand(3), outputFile, WRITE_HDR));
            }
            // ioctl : 10 unlocked_ioctl
            if (actualStType->getNumOperands() > 10) {
                entry.emplace(UNLOCKED_IOCTL_HDR,printFuncVal(actualStType->getOperand(10), outputFile, UNLOCKED_IOCTL_HDR));
            }
            // ioctl : 11 compat_ioctl
            if (actualStType->getNumOperands() > 11) {
                entry.emplace(COMPAT_IOCTL_HDR,printFuncVal(actualStType->getOperand(11), outputFile, COMPAT_IOCTL_HDR));
            }
        }
    }
}

llvm::BasicBlock* getReturnBlock(llvm::Function *F){//assume that ioctl only return in one BB
	for (llvm::BasicBlock& BB : *F) {
		if (llvm::ReturnInst* retInst = llvm::dyn_cast<llvm::ReturnInst>(BB.getTerminator())) {
			// Get the exit basic block
			llvm::BasicBlock* retBB = retInst->getParent();
            //outs() << "Exit Basic Block: " << retBB->getName() << "\n";
			return retBB;
        }
    }
	return nullptr;
}

void bfs(llvm::BasicBlock* entry, llvm::BasicBlock* target, std::vector<std::vector<llvm::BasicBlock*>>& paths, llvm::BasicBlock* anchor) {
    std::queue<std::pair<std::vector<llvm::BasicBlock*>, bool>> queue;
    queue.push(std::make_pair(std::vector<llvm::BasicBlock*>{entry}, false)); // Initialize 'passedAnchor' as false

    while (!queue.empty()) {
        std::vector<llvm::BasicBlock*> path = queue.front().first;
        bool passedAnchor = queue.front().second;
        queue.pop();

        llvm::BasicBlock* current = path.back();

        // Check if the current basic block is the anchor
        if (current == anchor) {
            passedAnchor = true;
        }

        if (current == target && passedAnchor) {
            paths.push_back(path);
        } else {
            // Visit neighbors and add new paths to the queue
            for (llvm::succ_iterator succIt = llvm::succ_begin(current), succEnd = llvm::succ_end(current); succIt != succEnd; ++succIt) {
                llvm::BasicBlock* succ = *succIt;
                // Check if 'succ' is already in the current path to avoid cycles
                if (std::find(path.begin(), path.end(), succ) == path.end()) {
                    std::vector<llvm::BasicBlock*> newPath = path;
                    newPath.push_back(succ);
                    // Inherit 'passedAnchor' value for the new path
                    queue.push(std::make_pair(newPath, passedAnchor));
                }
            }
        }
    }
}

std::vector<std::vector<llvm::BasicBlock*>> getAllPaths(llvm::BasicBlock* entry, llvm::BasicBlock* target, llvm::BasicBlock* anchor) {
    std::vector<std::vector<llvm::BasicBlock*>> paths; 
    //vector<BasicBlock*> path;  

    bfs(entry, target, paths, anchor); 
    return paths;
}

llvm::Value* findValue_in_StoreInst(std::string& ptr, std::vector<llvm::BasicBlock*> path, std::vector<llvm::Value*>& chain){
    for (auto& BB : path){
        for (auto &I : *BB){
            if (I.getOpcode() == llvm::Instruction::Store){
                llvm::StoreInst* storeInst = llvm::dyn_cast<llvm::StoreInst>(&I);
                
                llvm::Value* valueOperand = storeInst->getValueOperand();
                llvm::Value* pointerOperand = storeInst->getPointerOperand();
                
                if(ptr == pointerOperand->getName().str()){
                    chain.push_back(static_cast<llvm::Value*>(storeInst));
                    chain.push_back(valueOperand);
                    return valueOperand;
                }  
            }
        }
    }
    return nullptr;
}

llvm::Type* copy_ud_chain_recovery(llvm::Value* V, std::vector<llvm::Value*>& chain, std::vector<std::vector<llvm::BasicBlock*>> paths){
    
    /*
	struct/i64*:copy->load->alloca->store->bitcast
		%call1.i80 = call i64 @_copy_from_user(i8* %86, i8* %87, i64 %88) #11, !dbg !5714
		%86 = load i8*, i8** %to.addr.i41, align 8, !dbg !5711
		%to.addr.i41 = alloca i8*, align 8
		store i8* %62, i8** %to.addr.i41, align 8
		%62 = bitcast %struct.sisusb_command* %y to i8*, !dbg !5679
    i8*:load->alloca->store->load
		%call2.i = call i64 @__copy_from_user(i8* %43, i8* %44, i64 %45) #5, !dbg !4179
		%43 = load i8*, i8** %to.addr.i, align 8, !dbg !4176
		%to.addr.i = alloca i8*, align 8
		store i8* %31, i8** %to.addr.i, align 8, !dbg !4163
		%31 = load i8*, i8** %buffer, align 8, !dbg !4159
	*/
    if (llvm::isa<llvm::LoadInst>(V)){//%127 = load i8*, i8** %to.addr.i119, align 8, !dbg !5918
        auto loadInst = llvm::dyn_cast<llvm::LoadInst>(V);
        llvm::Value* load_pointerOperand = loadInst->getPointerOperand();
        chain.push_back(load_pointerOperand);
		//printValue(load_pointerOperand);
        if (llvm::isa<llvm::AllocaInst>(load_pointerOperand)){
            std::string ptr = load_pointerOperand->getName().str();//ptr to find in store instruction
            llvm::Value* store_valueOperand = nullptr;
            for (auto& path : paths) {//define in only one path because SSA
                store_valueOperand=findValue_in_StoreInst(ptr,path,chain);
                if(store_valueOperand){
                    break;
                }
            }
			if (store_valueOperand){
				//printValue(store_valueOperand);
            	if (llvm::isa<llvm::BitCastInst>(store_valueOperand)){
                	auto bitcastInst = llvm::dyn_cast<llvm::BitCastInst>(store_valueOperand);
                	llvm::Type* sourceType=bitcastInst->getSrcTy(); 
                	return sourceType;
            	}else if(llvm::isa<llvm::LoadInst>(store_valueOperand)){
                	auto loadInst = llvm::dyn_cast<llvm::LoadInst>(store_valueOperand);
                	llvm::Type* pointerType = loadInst->getPointerOperandType()->getPointerElementType();
                	return pointerType;
            	}
			}
        }
    }
    return nullptr;
}

llvm::Type* findCopyType(llvm::Value* copy, llvm::Value* V, llvm::BasicBlock* begin, llvm::BasicBlock* end, llvm::Module *m){
    std::vector<llvm::Value*> use_def_chain;
	use_def_chain.push_back(copy);
    use_def_chain.push_back(V);

    std::vector<std::vector<llvm::BasicBlock*>> paths=getAllPaths(begin,end,begin);//all paths from begin to target and must pass through anchor
  
    llvm::Type* type = copy_ud_chain_recovery(V,use_def_chain,paths);

	if (type){
    	std::cout<<"chain"<<std::endl;
    	printChain(use_def_chain); 
    	type->dump();
		std::cout<<std::endl;
	}  
    return type;
}

llvm::Value* findPtr_in_StoreInst(std::string& val, std::vector<llvm::BasicBlock*> path, std::vector<llvm::Value*>& chain){
    for (auto& BB : path){
        for (auto &I : *BB){
            if (I.getOpcode() == llvm::Instruction::Store){
				
                llvm::StoreInst* storeInst = llvm::dyn_cast<llvm::StoreInst>(&I);
                //printInst(&I);
                llvm::Value* valueOperand = storeInst->getValueOperand();
                llvm::Value* pointerOperand = storeInst->getPointerOperand();

                if (val == valueOperand->getName().str()){
                    chain.push_back(static_cast<llvm::Value*>(storeInst));
                    chain.push_back(pointerOperand);
                    return pointerOperand;
                }  
            }
        }
    }
    return nullptr;
}

llvm::Value* findLoadInst_by_Ptr(std::string& ptr, std::vector<llvm::BasicBlock*> path, std::vector<llvm::Value*>& chain){
    for (auto& BB : path){
        for (auto &I : *BB){
            if (I.getOpcode() == llvm::Instruction::Load){
				llvm::LoadInst* loadInst = llvm::dyn_cast<llvm::LoadInst>(&I);
				llvm::Value* pointerOperand = loadInst->getPointerOperand();
				if (ptr == pointerOperand->getName().str()){
					chain.push_back(static_cast<llvm::Value*>(loadInst));
					return static_cast<llvm::Value*>(loadInst);
				}
            }
        }
    }
    return nullptr;
}

llvm::Value* findInToPtrInst_by_src(std::string& src, std::vector<llvm::BasicBlock*> path, std::vector<llvm::Value*>& chain){
    for (auto& BB : path){
        for (auto &I : *BB){
            if (I.getOpcode() == llvm::Instruction::IntToPtr){
				llvm::IntToPtrInst* inttoptrInst = llvm::dyn_cast<llvm::IntToPtrInst>(&I);
				llvm::Value* srcOperand = inttoptrInst->getOperand(0);
				if (src == srcOperand->getName().str()){
					chain.push_back(static_cast<llvm::Value*>(inttoptrInst));
					return static_cast<llvm::Value*>(inttoptrInst);
				}
            }
        }
    }
    return nullptr;	
}

llvm::Type* cast_ud_chain_recovery(std::string& argname, std::vector<llvm::Value*>& chain, std::vector<std::vector<llvm::BasicBlock*>> paths){
    
	/*
		store i64 %arg, i64* %arg.addr, align 8
		%arg.addr = alloca i64, align 8
		%0 = load i64, i64* %arg.addr, align 8, !dbg !7222
		%1 = inttoptr i64 %0 to i32*, !dbg !7223
		store i32* %1, i32** %argp, align 8, !dbg !7221

		store->alloca->load->inttoptr
	*/
	llvm::Value* store_ptrOperand = nullptr;
	for (auto& path : paths) {//define in only one path because SSA
		//printPath(path);
		store_ptrOperand = findPtr_in_StoreInst(argname,path,chain);
		if (store_ptrOperand){
			break;
		}
	}
	if (store_ptrOperand){
		if (llvm::isa<llvm::AllocaInst>(store_ptrOperand)){
			std::string ptr = store_ptrOperand->getName().str();
			llvm::Value* loadInst = nullptr;
			for (auto& path : paths) {
				loadInst = findLoadInst_by_Ptr(ptr,path,chain);
				if (loadInst){
					break;
				}
			}
			if (loadInst){
				std::string loadResult = loadInst->getName().str();
				llvm::Value* inttoptrInst = nullptr;
				for (auto& path : paths) {
					inttoptrInst = findInToPtrInst_by_src(loadResult,path,chain);
					if (inttoptrInst){
						break;
					}
				}
				if (inttoptrInst){ 
					if (llvm::IntToPtrInst * intoptrinst = llvm::dyn_cast<llvm::IntToPtrInst>(inttoptrInst)){
						llvm::Type* destPointerType = intoptrinst->getDestTy();
						//destPointerType->dump();
						return destPointerType;
					}
				}
			}
		}
	}
    return nullptr;
}

llvm::Type* findCastType(std::string& argname, llvm::BasicBlock* begin, llvm::BasicBlock* end, llvm::BasicBlock* target, llvm::Module *m){
    std::vector<llvm::Value*> use_def_chain;

    std::vector<std::vector<llvm::BasicBlock*>> paths = getAllPaths(begin,end,target);//all paths from entry to target
	llvm::Type* type = cast_ud_chain_recovery(argname,use_def_chain,paths);
	
	if (type){
    	std::cout<<"chain"<<std::endl;
    	printChain(use_def_chain); 
    	type->dump();
		std::cout<<std::endl;
	}
    return type;
}

void process_ioctl(std::string& ioctl_func, llvm::Module *m, std::vector<std::tuple<int64_t,llvm::Type*,std::string>>& ioctl_info){
    llvm::Function *F = m->getFunction(ioctl_func.c_str());
	
	//name of variadic argument
	llvm::Argument* vararg = &*(F->arg_begin() + 2);
    std::string varargname = vararg->getName().str();

    llvm::BasicBlock* entry = &F->getEntryBlock();
	llvm::BasicBlock* retBB = getReturnBlock(F);
	
    visualizeControlFlow(ioctl_func,m);
    for (llvm::BasicBlock &BB : *F) {
        for (llvm::Instruction &I : BB) {
            if (I.getOpcode() == llvm::Instruction::Switch) {
                llvm::SwitchInst *SI = llvm::cast<llvm::SwitchInst>(&I);
                //std::cout << BB.getName().str() << std::endl;//BB's name
                //SI->getCondition()->print(outs());//condition station
                //std::cout << SI->getDefaultDest()->getName().str() << std::endl;//default dest
                std::unordered_set<llvm::BasicBlock*> visited;
                for (auto it = SI->case_begin(), e = SI->case_end(); it != e; ++it) {
                   	std::cout << "Case value: " << uint32_t(it->getCaseValue()->getSExtValue()) << std::endl;//case value
                    std::cout << "Destination: " << it->getCaseSuccessor()->getName().str() << std::endl;//dest BB
					std::cout << std::endl;
                    std::queue<llvm::BasicBlock*> q;
                    visited.insert(it->getCaseSuccessor());
                    q.push(it->getCaseSuccessor());
					bool process = 0;
                    while (!q.empty()){
                        llvm::BasicBlock *currBB = q.front();
                        q.pop();

                        for (llvm::Instruction &I : *currBB) {
                            if (llvm::CallInst *CI = llvm::dyn_cast<llvm::CallInst>(&I)) {
                                if (llvm::Function *calledFunc = CI->getCalledFunction()) {
                                    if (calledFunc->getName() == "__copy_from_user" || calledFunc->getName() == "_copy_from_user") {
                                        if (CI->getNumArgOperands() >= 3) {
											if(llvm::Value* copy = llvm::dyn_cast<llvm::Value>(&I)){
                                            	llvm::Value *to = CI->getArgOperand(0);//kernelspace
                                            	std::cout<<"copy_from_user in: "<<currBB->getName().str()<<std::endl;       		
                                            	//ioctl_map.emplace(uint32_t(it->getCaseValue()->getSExtValue()),findType(to,entry,currBB,m));
												ioctl_info.push_back(std::make_tuple(uint32_t(it->getCaseValue()->getSExtValue()),findCopyType(copy,to,entry,currBB,m),"in"));//in
												process = 1;
												break;											
											}
                                        }
                                        //std::cout<<currBB->getName().str()<<std::endl;
                                    }
                                    else if (calledFunc->getName() == "__copy_to_user" || calledFunc->getName() == "_copy_to_user") {
                                        if (CI->getNumArgOperands() >= 3) {
											if(llvm::Value* copy = llvm::dyn_cast<llvm::Value>(&I)){
                                            	llvm::Value *from = CI->getArgOperand(1);//kernelspace
                                            	std::cout<<"copy_to_user in: "<<currBB->getName().str()<<std::endl;
                                            	//ioctl_map.emplace(uint32_t(it->getCaseValue()->getSExtValue()),findType(from,entry,currBB,m));
												ioctl_info.push_back(std::make_tuple(uint32_t(it->getCaseValue()->getSExtValue()),findCopyType(copy,from,entry,currBB,m),"out"));//out
												process = 1;
												break;
											}
                                        }
                                        //std::cout<<currBB->getName().str()<<std::endl;
                                    }
                                }
                            }
                        }
						if (process == 1){
							break;
						}
                        
                        for (llvm::BasicBlock *succBB : successors(currBB)) {
                            if (visited.find(succBB) == visited.end()) {
                                q.push(succBB);
                                visited.insert(succBB);
                            }
                        }
                    }
					if (process == 0){
						ioctl_info.push_back(std::make_tuple(uint32_t(it->getCaseValue()->getSExtValue()),findCastType(varargname,entry,retBB,it->getCaseSuccessor(),m),"inout"));
					}
                }
            }
        }
    }
}
