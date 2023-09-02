#include "usb/usb_attribute.h"

usb_attribute_info::usb_attribute_info(llvm::GlobalVariable* usb_driver_g ,llvm::GlobalVariable* usb_attribute_group_g, llvm::Module* mm)
	: usb_driver_info(usb_driver_g, mm), usb_attribute_group(usb_attribute_group_g) {
}

void usb_attribute_info::process_usb_attribute(FILE *outputFile){
	process_usb_driver_st(outputFile);
	std::vector<std::tuple<std::string, llvm::Value*, llvm::Value*>> attribute_rw;
	process_attribute_group(usb_attribute_group,attribute_rw);
	for(auto i=0; i<attribute_rw.size(); i++){
		auto attribute_rw_each = attribute_rw[i];
		std::string usb_attr_name = std::get<0>(attribute_rw_each);
		llvm::Value* usb_attr_show = std::get<1>(attribute_rw_each);
		llvm::Value* usb_attr_store = std::get<2>(attribute_rw_each);
		fprintf(outputFile, "%s:%s\n", USB_ATTRIBUTE_NAME, usb_attr_name.c_str());
		usb_attribute_rw.push_back(std::make_tuple(usb_attr_name,printFuncVal(usb_attr_show,outputFile,USB_ATTRIBUTE_SHOW),printFuncVal(usb_attr_store,outputFile,USB_ATTRIBUTE_STORE)));
	}
}

bool usb_attribute_info::gen_syzlang(FILE* outputFile){
	for(auto i=0; i<usb_attribute_rw.size(); i++){
		auto usb_attribute_rw_each = usb_attribute_rw[i];
		std::string usb_attr_name = std::get<0>(usb_attribute_rw_each);
		std::string usb_attr_show_name = std::get<1>(usb_attribute_rw_each);
		std::string usb_attr_store_name = std::get<2>(usb_attribute_rw_each);
		std::string mix_name = usb_driver_name+"_"+usb_attr_name;
		std::string fd_name = gen_fd(mix_name); 
		gen_resource(fd_name,outputFile);
		gen_syz_open_sysfs_attribute(usb_driver_name,usb_attr_name,usb_driver_sysfs_dir,fd_name,outputFile);
		if(usb_attr_show_name!=""){
			gen_read(mix_name,fd_name,outputFile);
		}
		if(usb_attr_store_name!=""){
			gen_write(mix_name,fd_name,outputFile);
		}
	}
	return true;
}
