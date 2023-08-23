#ifndef USB_DRIVER_H
#define USB_DRIVER_H

#include "common/stdafx.h"
#include "common/print.h"
#include "common/constant.h"
#include "common/fops.h"
#include "common/util.h"
#include "common/syzlang.h"
struct usb_driver_info {
    llvm::GlobalVariable* usb_driver;
    llvm::Module* m;

    std::string usb_probe_func;  // name of probe function
    std::string usb_driver_name;  // name of driver
    uint16_t vid_table[100];      // vid
    uint16_t pid_table[100];      // pid

    std::string usb_register_func;

    uint32_t id_table[100];  // vid&pid, higher 16 bits are vid, lower 16 bits are pid
    std::string driver_sysfs_dir;  // driver dir in sysfs
    std::string usb_dev_path;  // usb device node path in /dev

    // entrypoints in file_operations
    std::unordered_map<std::string, std::string> entry;  // entrypoints
    std::vector<std::tuple<int64_t, llvm::Type*, std::string>> ioctl_info;
    // unordered_map<int64_t, Type*> ioctl_map;

    usb_driver_info(llvm::GlobalVariable* usb_driver_g, llvm::Module* mm);
    
    void process_usb_driver_st(FILE* outputFile);
    
    virtual bool gen_syzlang(FILE* outputFile)=0;
};

#endif 
