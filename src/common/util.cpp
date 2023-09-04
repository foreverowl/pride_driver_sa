#include "common/util.h"
llvm::GlobalVariable* findStruct(const std::string& structName, llvm::Module *m) {
    llvm::GlobalVariable* structVariable = nullptr;

    for (llvm::GlobalVariable& global : m->getGlobalList()) {
        llvm::Type* type = global.getType()->getElementType();
        if (llvm::StructType* structType = llvm::dyn_cast<llvm::StructType>(type)) {
			if(!structType->isLiteral()){
				if (structType->getName() == structName) {
                	structVariable = &global;
                	break;
            	}
			}
        }
    }
    return structVariable;
}

std::vector<llvm::GlobalVariable*> findAllStruct(const std::string& structName, llvm::Module *m){
    std::vector<llvm::GlobalVariable*> structVector;

    for (llvm::GlobalVariable& global : m->getGlobalList()) {
        llvm::Type* type = global.getType()->getElementType();
        if (llvm::StructType* structType = llvm::dyn_cast<llvm::StructType>(type)) {
			if(!structType->isLiteral()){
				if (structType->getName() == structName) {
                    structVector.push_back(&global);
            	}
			}
        }
    }
    return structVector;   
}

llvm::Function* findFunctionCallinFunction(const std::string& f, const std::string& targetf, llvm::Module* m) {//find function call in function
    llvm::Function* F = m->getFunction(f);
    if (!F) {
        return nullptr;
    }
    for (auto& BB : *F) {
        for (auto& I : BB) {
            if (auto* callInst = llvm::dyn_cast<llvm::CallInst>(&I)) {
                llvm::Function* calledFunction = callInst->getCalledFunction();
                if (calledFunction && calledFunction->getName().str() == targetf) {
                    return calledFunction;
                }
            }
        }
    }
    return nullptr;
}
