#ifndef USB_CLASS_DRIVER_H
#define USB_CLASS_DRIVER_H

#include "usb/usb_driver.h"
struct usb_class_driver_info : usb_driver_info{
    llvm::GlobalVariable* usb_class_driver;
    
    std::string usb_node_name;//name of usb device node, always with device number 
    std::string usb_devnode_func;//name of devnode function
    std::string usb_kasprintf_str;//format string used by devnode function

    // entrypoints in file_operations
    std::unordered_map<std::string, std::string> entry;  // entrypoints
    std::vector<std::tuple<int64_t, llvm::Type*, std::string>> ioctl_info;
    // unordered_map<int64_t, Type*> ioctl_map;
  
    usb_class_driver_info(llvm::GlobalVariable* usb_driver_g ,llvm::GlobalVariable* usb_class_driver_g, llvm::Module* mm);
    
    std::string process_usb_devnode(FILE *outputFile);
    
    void process_usb_class_driver_st(FILE *outputFile);
    
    bool gen_syzlang(FILE* outputFile) override ;
};
#endif
