#include "common/stdafx.h"
#include "common/print.h"
#include "common/constant.h"
#include "common/fops.h"
#include "common/util.h"
#include "common/syzlang.h"
#include "usb/usb_driver.h"
#include "usb/usb_class_driver.h"

using namespace std;
using namespace llvm;

std::string usb_driver_st("struct.usb_driver");
std::string usb_class_driver_st("struct.usb_class_driver");
std::string file_op_st("struct.file_operations");
std::string dev_attr_st("struct.device_attribute");
std::string attr_st("struct.attribute");
std::string attr_group_st("struct.attribute_group");
std::string pci_driver_st("struct.pci_driver");


/*
static DEVICE_ATTR_WO(enable_compliance); -> struct device_attribute 

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

/*
struct usb_attribute_info : usb_driver_info{
	GlobalVariable* attr_group;

    vector<string> attr_name;
    usb_attribute_info(GlobalVariable* usb_driver_g ,GlobalVariable* attr_group_g, Module* mm) : usb_driver_info(usb_driver_g,mm) ,attr_group(attr_group_g){}
};*/





void process_usb(Module *m, FILE *infoFile, FILE *syzlangFile){	
	GlobalVariable* usb_driver=findStruct(usb_driver_st,m);
    //process usb_class_driver
    if (GlobalVariable* usb_class_driver=findStruct(usb_class_driver_st,m)){
    	usb_class_driver_info infostruct(usb_driver,usb_class_driver,m);
    	infostruct.process_usb_class_driver_st(infoFile);
		if (infostruct.gen_syzlang(syzlangFile)){
			cout<<"success to generate "<<infostruct.usb_driver_name<<endl;
		}
	//process attribute with attribute_group
    }else if (GlobalVariable* attr_group=findStruct(attr_group_st,m)){
    	//usb_attribute_info infostruct(usb_driver,attr_group,m);  	
    }
}

void process_pci(Module *m, FILE *infoFile, FILE *syzlangFile){
    GlobalVariable* pci_driver=findStruct(pci_driver_st,m);
}


int main(int argc, char **argv) {
    /*  Parse command line arguments
        argv[1] is driver's llvm bitcode as input
        argv[2] is driver's info file for symbolic execution
        argv[3] is driver's syzlang file for fuzz testing*/
	if (argc < 4) {
        std::cerr << "Usage: " << argv[0] << " <Bitcode file> <Info file> <Syzlang file>" << std::endl;
        exit(-1);
    }
  
    char* bitcode_file=argv[1];
    char* info_file=argv[2];
    char* syzlang_file=argv[3];
    FILE *infoFile = fopen(info_file, "w");
    FILE *syzlangFile = fopen(syzlang_file, "w");
  
    //parse bitcode to module
    LLVMContext context;
    ErrorOr<std::unique_ptr<MemoryBuffer>> fileOrErr = MemoryBuffer::getFileOrSTDIN(bitcode_file);

    Expected<std::unique_ptr<llvm::Module>> moduleOrErr = parseBitcodeFile(fileOrErr.get()->getMemBufferRef(), context);
    
    if (!moduleOrErr) {
        std::cerr << "[-] Error reading Module " << bitcode_file << std::endl;
        return 3;
    }

    Module *m = moduleOrErr.get().get();
    
    // call process func for USB/PCI driver
    if (findStruct(usb_driver_st,m)){
        process_usb(m, infoFile, syzlangFile);//USB driver
    }else if(findStruct(pci_driver_st,m)){
        process_pci(m, infoFile, syzlangFile);//PCI driver
    }else{
        cout<<"Can't identify driver!"<<endl;
    }
    

    return 0;
}

