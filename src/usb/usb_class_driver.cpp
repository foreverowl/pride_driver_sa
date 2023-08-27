#include "usb/usb_class_driver.h"

usb_class_driver_info::usb_class_driver_info(llvm::GlobalVariable* usb_driver_g, llvm::GlobalVariable* usb_class_driver_g, llvm::Module* mm)
	: usb_driver_info(usb_driver_g, mm), usb_class_driver(usb_class_driver_g) {
	usb_register_func = "usb_register_dev";
}

std::string usb_class_driver_info::process_usb_devnode(FILE *outputFile){
    llvm::Function *F = m->getFunction(usb_devnode_func.c_str());
    for (llvm::BasicBlock &BB : *F) {
        for (llvm::Instruction &I : BB) {
            if (llvm::CallInst *CI = llvm::dyn_cast<llvm::CallInst>(&I)) {
                if (llvm::Function *calledFunc = CI->getCalledFunction()) {
                    if (calledFunc->getName() == "kasprintf") {
                        if (CI->getNumArgOperands() >= 2) {
                            llvm::Value *secondArg = CI->getArgOperand(1);
                            llvm::Value* array=secondArg->stripPointerCasts();
                            llvm::GlobalVariable *gv = llvm::cast<llvm::GlobalVariable>(array);
                            return llvm::cast<llvm::ConstantDataArray>(gv->getInitializer())->getAsCString().str();
                        }
                    }
                }
            }
        }
    }
    return "";
}

void usb_class_driver_info::process_usb_class_driver_st(FILE *outputFile) {
	process_usb_driver_st(outputFile);
	
    if(usb_class_driver->hasInitializer()) {
        // get the initializer.
        llvm::Constant *targetConstant = usb_class_driver->getInitializer();
        llvm::ConstantStruct *actualStType = llvm::dyn_cast<llvm::ConstantStruct>(targetConstant);
        if(actualStType != nullptr) {
            //name:0
            if (actualStType->getNumOperands() > 0) {
                usb_node_name = printStr(actualStType->getOperand(0), outputFile, NO_OUTPUT);
            }
            //devnode:1
            if (actualStType->getNumOperands() > 1) {
                usb_devnode_func = printFuncVal(actualStType->getOperand(1), outputFile, NO_OUTPUT);
            }
			if (usb_node_name!=""){
        		//use '#' as device number
        		size_t pos = usb_node_name.find("%d");
        		if (pos!=std::string::npos){
        			usb_node_name.replace(pos, 2, "#");
        		}
        		//process devnode and generate dev path
        		if (usb_devnode_func!=""){
            		usb_kasprintf_str=process_usb_devnode(outputFile);//process devnode func
        		}
        		if (usb_kasprintf_str!=""){
       				char t[100];
           			sprintf(t,usb_kasprintf_str.c_str(),usb_node_name.c_str());
            		usb_dev_path=t;
            		usb_dev_path="/dev/"+usb_dev_path;
        		}else{
            		usb_dev_path="/dev/"+usb_node_name;
        		}
        		fprintf(outputFile, "%s:%s\n", USB_DEVPATH, usb_dev_path.c_str());				
			}
			//fileoperations:2
            if (actualStType->getNumOperands() > 2) {
                auto fops = actualStType->getOperand(2);
                //printValue(fops);
				//fops->print(outs());
                //Type* t=fops->getType();
                //t->dump();
                if (llvm::GlobalVariable* g = llvm::dyn_cast<llvm::GlobalVariable>(fops)) {	
                	//cout<<"transfer"<<endl;
  					process_file_operations_st(g,outputFile,entry);
            		if (entry.find(UNLOCKED_IOCTL_HDR)!=entry.end() && entry.find(UNLOCKED_IOCTL_HDR)->second!=""){//unlocked_ioctl defined
                		process_ioctl(entry.find(UNLOCKED_IOCTL_HDR)->second,m,ioctl_info);
           			}
            		if (entry.find(COMPAT_IOCTL_HDR)!=entry.end()&&entry.find(COMPAT_IOCTL_HDR)->second!=""){//compat_ioctl defined
                		process_ioctl(entry.find(COMPAT_IOCTL_HDR)->second,m,ioctl_info);
           			}
  				}else if(llvm::isa<llvm::PointerType>(fops->getType())){
  					//Type *containedType = pointerType->getContainedType(0);
  					//Type* sourceType=fops->getSrcTy();
  					//containedType->dump();
					llvm::Value* v=fops->getOperand(0);
					if (llvm::GlobalVariable* g = llvm::dyn_cast<llvm::GlobalVariable>(v)){
  						process_file_operations_st(g,outputFile,entry);
            			if (entry.find(UNLOCKED_IOCTL_HDR)!=entry.end() && entry.find(UNLOCKED_IOCTL_HDR)->second!=""){//ioctl defined
                			process_ioctl(entry.find(UNLOCKED_IOCTL_HDR)->second,m,ioctl_info);
           				}
            			if (entry.find(COMPAT_IOCTL_HDR)!=entry.end()&&entry.find(COMPAT_IOCTL_HDR)->second!=""){//compat_ioctl defined
                			process_ioctl(entry.find(COMPAT_IOCTL_HDR)->second,m,ioctl_info);
           				}
					}
  				}	
            }           
        }
    }
}

bool usb_class_driver_info::gen_syzlang(FILE* outputFile){

	//fd
	std::string fd_name = gen_fd(usb_driver_name);
	//resource
	gen_resource(fd_name,outputFile);
	//open
	gen_syz_open_dev(usb_driver_name,usb_dev_path,fd_name,outputFile);
	//write
	if (entry.find(WRITE_HDR) != entry.end()){
		gen_write(usb_driver_name,fd_name,outputFile);
	}
	//read
	if (entry.find(READ_HDR) != entry.end()){
		gen_read(usb_driver_name,fd_name,outputFile);
	}
	//ioctl
	int valid_cmd_index = -1;
	std::set<uint32_t> valid_cmd;
	for (auto i = 0;i < ioctl_info.size();i++){
		auto ioctl_info_each = ioctl_info[i];
		uint64_t cmd = std::get<0>(ioctl_info_each);
		llvm::Type* type = std::get<1>(ioctl_info_each);
		std::string direct = std::get<2>(ioctl_info_each);

		if (valid_cmd.find(cmd)!=valid_cmd.end()){
			break;
		}else{
			valid_cmd_index++;
			valid_cmd.insert(cmd);
		}
		
		if(!gen_ioctl(usb_driver_name,fd_name,valid_cmd_index,cmd,type,direct,outputFile)){
			return false;
		}
	}
	//struct
	for (auto i = 0;i < ioctl_info.size();i++){
		auto ioctl_info_each = ioctl_info[i];
		llvm::Type* type = std::get<1>(ioctl_info_each);
		if (type!=nullptr){
			if (llvm::PointerType* ptrType = llvm::dyn_cast<llvm::PointerType>(type)){
    			if (ptrType->getPointerElementType()->isStructTy()){
					if (llvm::StructType * structtype = llvm::dyn_cast<llvm::StructType>(ptrType->getPointerElementType())){
						if (!generate_struct(structtype,outputFile)){
							return false;
						}
					}
    			}
			}	
		}
	}
	return true;	
}
