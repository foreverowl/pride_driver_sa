#ifndef USB_ATTRIBUTE_H
#define USB_CLASS_DRIVER_H
#include "usb/usb_driver.h"
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


struct usb_attribute_info : usb_driver_info{
	std::vector<llvm::GlobalVariable*> usb_device_attribute;
    std::vector<std::tuple<std::string, std::string, std::string>> usb_attribute_rw;//attr's name,whether read(show),whether write(store)
    usb_attribute_info(llvm::GlobalVariable* usb_driver_g ,llvm::Module* mm);

	void process_usb_attribute(FILE *outputFile);
	bool gen_syzlang(FILE* outputFile) override ;
};


#endif