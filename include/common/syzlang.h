#ifndef SYZLANG_H
#define SYZLANG_H
#include "common/stdafx.h"

std::string gen_fd(std::string driver_name);
void gen_resource(std::string fd_name, FILE *syzlangFile);
void gen_syz_open_dev(std::string driver_name, std::string path, std::string fd_name ,FILE *syzlangFile);
void gen_syz_open_sysfs_attribute(std::string driver_name, std::string attr_name, std::string driver_path, std::string fd_name, FILE *syzlangFile);
void gen_write(std::string driver_name, std::string fd_name, FILE *syzlangFile);
void gen_read(std::string driver_name, std::string fd_name, FILE *syzlangFile);
bool gen_ioctl(std::string driver_name, std::string fd_name, int index, int64_t cmd, llvm::Type* type, std::string direct, FILE *syzlangFile);
bool generate_struct(llvm::StructType* structtype, FILE *syzlangFile);
#endif 
