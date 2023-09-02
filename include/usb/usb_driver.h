#ifndef USB_DRIVER_H
#define USB_DRIVER_H

#include "common/stdafx.h"
#include "common/print.h"
#include "common/constant.h"
#include "common/fops.h"
#include "common/util.h"
#include "common/syzlang.h"
#include "common/attribute.h"
struct usb_driver_info {
    llvm::GlobalVariable* usb_driver;
    llvm::Module* m;
    static bool init;

    std::string usb_probe_func;  // name of probe function
    std::string usb_driver_name;  // name of driver
    std::string usb_driver_sysfs_dir;  // driver dir in sysfs
    uint16_t vid_table[1000];      // vid
    uint16_t pid_table[1000];      // pid
    uint32_t id_table[1000];  // vid&pid, higher 16 bits are vid, lower 16 bits are pid

    std::string usb_register_func;
    std::string usb_dev_path;  // usb device node path in /dev

    usb_driver_info(llvm::GlobalVariable* usb_driver_g, llvm::Module* mm);
    
    void process_usb_driver_st(FILE* outputFile);
    
    virtual bool gen_syzlang(FILE* outputFile)=0;
};

#endif 
