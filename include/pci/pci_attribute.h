#ifndef PCI_ATTRIBUTE_H
#define PCI_CLASS_DRIVER_H
#include "pci/pci_driver.h"
/*
static DEVICE_ATTR_WO(enable_compliance); -> struct device_attribute {struct attribute, func show, func store}

static struct attribute *lvs_attrs[] = {
	&dev_attr_get_dev_desc.attr, -> struct attribute 
	&dev_attr_u1_timeout.attr,
	&dev_attr_u2_timeout.attr,
	&dev_attr_hot_reset.attr,
	&dev_attr_warm_reset.attr,
	&dev_attr_u3_entry.attr,
	&dev_attr_u3_exit.attr,
	&dev_attr_enable_compliance.attr,
	NULL
};
ATTRIBUTE_GROUPS(lvs); -> struct attribute_group
*/


struct pci_attribute_info : pci_driver_info{
	std::vector<llvm::GlobalVariable*> pci_device_attribute;
    std::vector<std::tuple<std::string, std::string, std::string>> pci_attribute_rw;//attr's name,whether read(show),whether write(store)
    pci_attribute_info(llvm::GlobalVariable* pci_driver_g, llvm::Module* mm);

	void process_pci_attribute(FILE *outputFile);
	bool gen_syzlang(FILE* outputFile) override ;
};


#endif