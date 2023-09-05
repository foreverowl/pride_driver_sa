#include "common/attribute.h"

//struct attribute, not GlobalVariable
/*
struct attribute {
	const char		*name;
	umode_t			mode;
};
*/
std::string process_attribute(llvm::Value* attribute){

    llvm::ConstantStruct *attribute_st = llvm::dyn_cast<llvm::ConstantStruct>(attribute);
    auto nameGEP = attribute_st->getOperand(0);
    llvm::Value *array = nameGEP->stripPointerCasts();
    llvm::GlobalVariable *gv = llvm::cast<llvm::GlobalVariable>(array);
    const char *str = llvm::cast<llvm::ConstantDataArray>(gv->getInitializer())->getAsCString().data();
    return std::string(str);
}

//struct device_attribute
/*
struct device_attribute {
	struct attribute	attr;
	ssize_t (*show)(struct device *dev, struct device_attribute *attr,
			char *buf);
	ssize_t (*store)(struct device *dev, struct device_attribute *attr,
			 const char *buf, size_t count);
};
*/
void process_device_attribute(llvm::GlobalVariable* device_attribute, std::vector<std::tuple<std::string, llvm::Value*, llvm::Value*>>& attribute_rw){

    if(device_attribute->hasInitializer()){
        llvm::ConstantStruct *device_attribute_st = llvm::dyn_cast<llvm::ConstantStruct>(device_attribute->getInitializer());

        std::string attr_name = process_attribute(device_attribute_st->getOperand(0));

        if(device_attribute_st){
            if(device_attribute_st->getNumOperands()<=1){
                return;
            }
            llvm::Value* attr_show = device_attribute_st->getOperand(1);
            if(device_attribute_st->getNumOperands()<=2){
                return;
            }
            llvm::Value* attr_store = device_attribute_st->getOperand(2);
            attribute_rw.push_back(std::make_tuple(attr_name,attr_show,attr_store));
        }
    }
}

//struct attribute_group
/*
struct attribute_group {
	const char		*name;
	umode_t			(*is_visible)(struct kobject *,
					      struct attribute *, int);
	umode_t			(*is_bin_visible)(struct kobject *,
						  struct bin_attribute *, int);
	struct attribute	**attrs;
	struct bin_attribute	**bin_attrs;
};
*/
void process_attribute_group(llvm::GlobalVariable* attribute_group, std::vector<std::tuple<std::string, llvm::Value*, llvm::Value*>>& attribute_rw){
    if(attribute_group->hasInitializer()){
        llvm::ConstantStruct *attribute_group_st = llvm::dyn_cast<llvm::ConstantStruct>(attribute_group->getInitializer());
        if(attribute_group_st) {
            //struct attribute	**attrs
            if (attribute_group_st->getNumOperands() > 3) {
                //%struct.attribute** getelementptr inbounds ([9 x %struct.attribute*], [9 x %struct.attribute*]* @lvs_attrs, i32 0, i32 0)
                auto attrsGEP= attribute_group_st->getOperand(3);
                if (attrsGEP->getNumOperands() <= 0) {
                    return;
                }
                //[9 x %struct.attribute*]* @lvs_attrs
                auto attrsArray = attrsGEP->getOperand(0);
                llvm::GlobalVariable *attrsArray_g = llvm::cast<llvm::GlobalVariable>(attrsArray);
                if(attrsArray_g->hasInitializer()){
                    if (llvm::ConstantArray *attrInitializer = llvm::dyn_cast<llvm::ConstantArray>(attrsArray_g->getInitializer())) {
                        for (unsigned i = 0; i < attrInitializer->getNumOperands(); ++i) {
                            auto attribute = attrInitializer->getOperand(i);
                            //%struct.attribute* getelementptr inbounds (%struct.device_attribute, %struct.device_attribute* @dev_attr_get_dev_desc, i32 0, i32 0)
                            if (attribute->getNumOperands() <= 0) {
                                return;
                            }
                            //%struct.device_attribute* @dev_attr_get_dev_desc
                            auto device_attribute = attribute->getOperand(0);
                            llvm::GlobalVariable *device_attribute_g = llvm::cast<llvm::GlobalVariable>(device_attribute);
                            process_device_attribute(device_attribute_g,attribute_rw);
                        }
                    }
                }
            }
        }
    }
}
