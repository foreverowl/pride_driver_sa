#include "common/attribute.h"

//struct attribute
void process_attribute(){

}

//struct device_attribute
void process_device_attribute(){

}

//struct attribute_group
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
                            if(device_attribute_g->hasInitializer()){
                                llvm::ConstantStruct *device_attribute_st = llvm::dyn_cast<llvm::ConstantStruct>(device_attribute_g->getInitializer());
                                std::string prefix = "dev_attr_";
                                std::string attr_name;
                                size_t prefixPos = device_attribute_g->getName().str().find(prefix);
                                if (prefixPos != std::string::npos){
                                    attr_name = device_attribute_g->getName().str().substr(prefixPos + prefix.length());
                                }else{
                                    return;
                                }
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
                    }
                }
            }
        }
    }
}