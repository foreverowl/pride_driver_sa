#include "pci/pci_attribute.h"

pci_attribute_info::pci_attribute_info(llvm::GlobalVariable* pci_driver_g ,llvm::Module* mm)
	: pci_driver_info(pci_driver_g, mm) {
}

void pci_attribute_info::process_pci_attribute(FILE *outputFile){
	process_pci_driver_st(outputFile);
	std::vector<std::tuple<std::string, llvm::Value*, llvm::Value*>> attribute_rw;
	for(auto i=0;i<pci_device_attribute.size();i++){
		process_device_attribute(pci_device_attribute[i],attribute_rw);
	}
	for(auto i=0; i<attribute_rw.size(); i++){
		auto attribute_rw_each = attribute_rw[i];
		std::string pci_attr_name = std::get<0>(attribute_rw_each);
		llvm::Value* pci_attr_show = std::get<1>(attribute_rw_each);
		llvm::Value* pci_attr_store = std::get<2>(attribute_rw_each);
		fprintf(outputFile, "%s:%s\n", PCI_ATTRIBUTE_NAME, pci_attr_name.c_str());
		pci_attribute_rw.push_back(std::make_tuple(pci_attr_name,printFuncVal(pci_attr_show,outputFile,PCI_ATTRIBUTE_SHOW),printFuncVal(pci_attr_store,outputFile,PCI_ATTRIBUTE_STORE)));
	}
}

bool pci_attribute_info::gen_syzlang(FILE* outputFile){
	for(auto i=0; i<pci_attribute_rw.size(); i++){
		auto pci_attribute_rw_each = pci_attribute_rw[i];
		std::string pci_attr_name = std::get<0>(pci_attribute_rw_each);
		std::string pci_attr_show_name = std::get<1>(pci_attribute_rw_each);
		std::string pci_attr_store_name = std::get<2>(pci_attribute_rw_each);
		std::string mix_name = pci_driver_name+"_"+pci_attr_name;
		std::string fd_name = gen_fd(mix_name); 
		gen_resource(fd_name,outputFile);
		gen_syz_open_sysfs_attribute(pci_driver_name,pci_attr_name,pci_driver_sysfs_dir,fd_name,outputFile);
		if(pci_attr_show_name!=""){
			gen_read(mix_name,fd_name,outputFile);
		}
		if(pci_attr_store_name!=""){
			gen_write(mix_name,fd_name,outputFile);
		}
	}
	return true;
}

