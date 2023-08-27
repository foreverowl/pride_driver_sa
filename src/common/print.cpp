#include "common/print.h"

void visualizeControlFlow(std::string& functionname, llvm::Module* m) {
    llvm::Function* F = m->getFunction(functionname);
    std::error_code EC;
    std::string dotfile = functionname + ".dot";
    llvm::raw_fd_ostream file(dotfile, EC, llvm::sys::fs::OF_Text);
    if (EC) {
        llvm::errs() << "Error opening file: " << EC.message() << "\n";
        return;
    }

    // replace '.' to '-'
    std::regex dotRegex("\\.");
    //string modifiedFilename = std::regex_replace(dotfile, dotRegex, "D");

    file << "digraph CFG {\n";

    // add BB
    for (llvm::BasicBlock& BB : *F) {

        bool calltargetfunc = false;
        for (llvm::Instruction& I : BB) {
            if (llvm::CallInst* CI = llvm::dyn_cast<llvm::CallInst>(&I)) {
                if (llvm::Function* calledFunc = CI->getCalledFunction()) {
                    if (calledFunc->getName() == "__copy_from_user" || calledFunc->getName() == "_copy_from_user" ||
                        calledFunc->getName() == "__copy_to_user" || calledFunc->getName() == "_copy_to_user")
                        calltargetfunc = true;
                    break;
                }
            }
        }
        std::string modifiedBBName = std::regex_replace(BB.getName().str(), dotRegex, "D");
        //file << "  " << modifiedBBName << " [shape=box];\n";
        if (calltargetfunc) {
            file << "  " << modifiedBBName << " [shape=box, style=filled, fillcolor=yellow];\n";
        } else {
            file << "  " << modifiedBBName << " [shape=box];\n";
        }
    }

    // add edge
    for (llvm::BasicBlock& BB : *F) {
        std::string modifiedBBName = std::regex_replace(BB.getName().str(), dotRegex, "D");
        for (llvm::BasicBlock* succ : llvm::successors(&BB)) {
            std::string modifiedSuccName = std::regex_replace(succ->getName().str(), dotRegex, "D");
            file << "  " << modifiedBBName << " -> " << modifiedSuccName << ";\n";
        }
    }

    file << "}\n";
    file.close();

    char command[100];
    std::string pngfile = functionname + ".png";
    sprintf(command, "dot -Tpng %s -o %s", dotfile.c_str(), pngfile.c_str());
    system(command);
    //llvm::outs() << "Control flow graph DOT representation has been generated: " << dotfile << "\n";
}

void printValue(llvm::Value *V) {
    if (V) {
        llvm::outs() << "Value: ";
        V->print(llvm::outs());
        llvm::outs() << "\n";
    }
}

void printGlobalVariable(llvm::GlobalVariable *G) {
    if (G) {
        llvm::outs() << "GlobalVariable: ";
        G->print(llvm::outs());
        llvm::outs() << "\n";
    }
}

void printInst(llvm::Instruction *I) {
    if (I) {
        llvm::outs() << "Instruction: ";
        I->print(llvm::outs());
        llvm::outs() << "\n";
    }
}

void printChain(std::vector<llvm::Value*> chain) {
    for (auto V : chain) {
        printValue(V);
    }
}

void printPath(std::vector<llvm::BasicBlock*>& basicBlocks) {
    for (size_t i = 0; i < basicBlocks.size(); ++i) {
        if (i > 0) {
            llvm::outs() << " -> ";
        }
        llvm::outs() << basicBlocks[i]->getName();
    }
    llvm::outs() << "\n";
}

std::string printFuncVal(llvm::Value *currVal, FILE *outputFile, const char *hdr_str) {
    llvm::Function *targetFunction = llvm::dyn_cast<llvm::Function>(currVal->stripPointerCasts());

    if (targetFunction != nullptr && targetFunction->hasName()) {
        if (strcmp(NO_OUTPUT, hdr_str)) {
            fprintf(outputFile, "%s:%s\n", hdr_str, targetFunction->getName().str().c_str());
        }
        return targetFunction->getName().str();
    }
    return "";
}


std::string printStr(llvm::Value *currVal, FILE *outputFile, const char *hdr_str) {
    llvm::Value *array = currVal->stripPointerCasts();
    llvm::GlobalVariable *gv = llvm::cast<llvm::GlobalVariable>(array);
    const char *str = llvm::cast<llvm::ConstantDataArray>(gv->getInitializer())->getAsCString().data();

    if (strcmp(NO_OUTPUT, hdr_str)) {
        fprintf(outputFile, "%s:%s\n", hdr_str, str);
    }
    return str;
}

