#ifndef PCI_DRIVER_H
#define PCI_DRIVER_H

#include "common/stdafx.h"
#include "common/print.h"
#include "common/constant.h"
#include "common/fops.h"
#include "common/util.h"
#include "common/syzlang.h"
#include "common/attribute.h"
struct pci_driver_info {
    llvm::GlobalVariable* pci_driver;
    llvm::Module* m;
    static bool init;

    std::string pci_probe_func;  // name of probe function
    std::string pci_driver_name;  // name of driver
    std::string pci_driver_sysfs_dir;  // driver dir in sysfs
    uint32_t vid_table[1000];      // vendor id
    uint32_t did_table[1000];      // device pid
    uint32_t sub_vid_table[1000];      // sub vendor id
    uint32_t sub_did_table[1000];      // sub device pid

    std::string pci_register_func;
    std::string pci_dev_path;  // pci device node path

    pci_driver_info(llvm::GlobalVariable* pci_driver_g, llvm::Module* mm);
    
    void process_pci_driver_st(FILE* outputFile);
    
    virtual bool gen_syzlang(FILE* outputFile)=0;
};

#endif 