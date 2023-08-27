#ifndef ATTRIBUTE_H
#define ATTRIBUTE_H
#include "common/stdafx.h"
#include "common/print.h"
//struct attribute
void process_attribute();
//struct device_attribute
void process_device_attribute();
//struct attribute_group
void process_attribute_group(llvm::GlobalVariable* attribute_group,std::vector<std::tuple<std::string, llvm::Value*, llvm::Value*>> &attribute_rw);

#endif 