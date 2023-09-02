#include "pci/pci_driver.h"

pci_driver_info::pci_driver_info(llvm::GlobalVariable* pci_driver_g, llvm::Module* mm) {
    pci_driver = pci_driver_g;
    m = mm;
}

void pci_driver_info::process_pci_driver_st(FILE *outputFile) {
    if (pci_driver->hasInitializer()) {
        // get the initializer.
        llvm::Constant *targetConstant = pci_driver->getInitializer();
        llvm::ConstantStruct *actualStType = llvm::dyn_cast<llvm::ConstantStruct>(targetConstant);
        if (actualStType != nullptr) {
            if (actualStType->getNumOperands() > 1) {
                // name: 1
                pci_driver_name = printStr(actualStType->getOperand(1), outputFile, init==false?PCI_DRIVER_NAME:NO_OUTPUT);
                pci_driver_sysfs_dir = "/sys/bus/pci/drivers/" + pci_driver_name;
            }
            
            if (actualStType->getNumOperands() > 2) {
                // id_table: 2
                auto idTableGEP = actualStType->getOperand(2);
                if (idTableGEP->getNumOperands() <= 0) {
                    return;
                }
                auto idTable = idTableGEP->getOperand(0);
                llvm::GlobalVariable *idTableGlobal = llvm::dyn_cast<llvm::GlobalVariable>(idTable);
                //array* -> array -> element
                llvm::Type* elementType = idTableGlobal->getType()->getElementType()->getArrayElementType();
                //%struct.pci_device_id
                if (llvm::StructType* structType = llvm::dyn_cast<llvm::StructType>(elementType)) {
			        if(!structType->isLiteral()){
				        if (structType->getName() != "struct.pci_device_id") {
                            return;
            	        }
			        }
                }
                llvm::ConstantArray *idTableArray = llvm::dyn_cast<llvm::ConstantArray>(idTableGlobal->getInitializer());
                if (!idTableArray) {
                    return;
                }
                for (auto i = 0; i < idTableArray->getNumOperands() - 1; i++) {
                    llvm::ConstantStruct *idTableStruct = llvm::dyn_cast<llvm::ConstantStruct>(idTableArray->getOperand(i));
                    
                    uint32_t idVendor = llvm::dyn_cast<llvm::ConstantInt>(idTableStruct->getOperand(0))->getSExtValue();
                    uint32_t idDevice = llvm::dyn_cast<llvm::ConstantInt>(idTableStruct->getOperand(1))->getSExtValue();
                    //#define PCI_ANY_ID (~0)
                    uint32_t subIdVendor = llvm::dyn_cast<llvm::ConstantInt>(idTableStruct->getOperand(2))->getSExtValue();
                    uint32_t subIdDevice = llvm::dyn_cast<llvm::ConstantInt>(idTableStruct->getOperand(3))->getSExtValue();

                    vid_table[i] = idVendor;
                    did_table[i] = idDevice;
                    sub_vid_table[i] = subIdVendor;
                    sub_did_table[i] = subIdDevice;

                    if(!init){
                        fprintf(outputFile, "%s:0x%04x:0x%04x:0x%04x:0x%04x\n", PCI_ID, idVendor, idDevice, subIdVendor, subIdDevice);
                    }
                }
            }
            if (actualStType->getNumOperands() > 3) {
                // probe: 3
                pci_probe_func = printFuncVal(actualStType->getOperand(3), outputFile, init==false?PCI_PROBE:NO_OUTPUT);
            }
            init = true;
        }
    }
}