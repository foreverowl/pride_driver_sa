#include "usb/usb_driver.h"

usb_driver_info::usb_driver_info(llvm::GlobalVariable* usb_driver_g, llvm::Module* mm) {
    usb_driver = usb_driver_g;
    m = mm;
}

void usb_driver_info::process_usb_driver_st(FILE *outputFile) {
    if (usb_driver->hasInitializer()) {
        // get the initializer.
        llvm::Constant *targetConstant = usb_driver->getInitializer();
        llvm::ConstantStruct *actualStType = llvm::dyn_cast<llvm::ConstantStruct>(targetConstant);
        if (actualStType != nullptr) {
            if (actualStType->getNumOperands() > 0) {
                // name: 0
                usb_driver_name = printStr(actualStType->getOperand(0), outputFile, USB_DRIVER_NAME);
                usb_driver_sysfs_dir = "/sys/bus/usb/drivers/" + usb_driver_name;
            }
            if (actualStType->getNumOperands() > 1) {
                // probe: 1
                usb_probe_func = printFuncVal(actualStType->getOperand(1), outputFile, USB_PROBE);
            }
            if (actualStType->getNumOperands() > 9) {
                // id_table: 9
                auto idTableGEP = actualStType->getOperand(9);
                if (idTableGEP->getNumOperands() <= 0) {
                    return;
                }
                auto idTable = idTableGEP->getOperand(0);
                llvm::GlobalVariable *idTableConstant = llvm::dyn_cast<llvm::GlobalVariable>(idTable);
                llvm::ConstantArray *idTableArray = llvm::dyn_cast<llvm::ConstantArray>(idTableConstant->getInitializer());
                if (!idTableArray) {
                    return;
                }
                for (auto i = 0; i < idTableArray->getNumOperands() - 1; i++) {
                    llvm::ConstantStruct *idTableStruct = llvm::dyn_cast<llvm::ConstantStruct>(idTableArray->getOperand(i));
                    uint16_t idVendor = llvm::dyn_cast<llvm::ConstantInt>(idTableStruct->getOperand(1))->getSExtValue();
                    uint16_t idProduct = llvm::dyn_cast<llvm::ConstantInt>(idTableStruct->getOperand(2))->getSExtValue();
                    uint32_t result = (static_cast<uint32_t>(idVendor) << 16) | idProduct;

                    vid_table[i] = idVendor;
                    pid_table[i] = idProduct;
                    id_table[i] = result;

                    fprintf(outputFile, "%s:0x%04x:0x%04x\n", USB_ID, idVendor, idProduct);
                }
            }
        }
    }
}
