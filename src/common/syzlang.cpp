#include "common/syzlang.h"

std::string gen_fd(std::string driver_name){
	if(driver_name == ""){
		return "";
	}
	std::string fd_name = "fd_"+driver_name;
	return fd_name;
}

void gen_resource(std::string fd_name, FILE *syzlangFile){
	std::string out = "";
	out = "resource "+fd_name+"[fd]";
	fprintf(syzlangFile,"%s\n",out.c_str());
}

void gen_syz_open_dev(std::string driver_name, std::string path, std::string fd_name, FILE *syzlangFile){
	std::string out = "";
	out = "syz_open_dev$"+driver_name+
		  "(dev ptr[in, string[\""+ path +"\"]], "+
		  "id intptr, flags flags[open_flags]) "+ fd_name;
	fprintf(syzlangFile,"%s\n",out.c_str());
}

void gen_syz_open_sysfs_attribute(std::string driver_name, std::string attr_name, std::string driver_path, std::string fd_name, FILE *syzlangFile){
	std::string out = "";
	out = "syz_open_sysfs_attribute$"+driver_name+"-"+attr_name+
		  "(dev ptr[in, string[\""+ driver_path +"\"]], "+
		  "attr ptr[in, string[\""+attr_name+"\"]], "+
		  "flags flags[open_flags]) "+ fd_name;
	fprintf(syzlangFile,"%s\n",out.c_str());
}

void gen_write(std::string driver_name, std::string fd_name, FILE *syzlangFile){
	std::string out = "";
	out = "read$"+driver_name+"(fd "+fd_name+", data buffer[out], len bytesize[data])";
	fprintf(syzlangFile,"%s\n",out.c_str());
}

void gen_read(std::string driver_name, std::string fd_name, FILE *syzlangFile){
	std::string out = "";
	out = "write$"+driver_name+"(fd "+fd_name+", data buffer[in], len bytesize[data])";
	fprintf(syzlangFile,"%s\n",out.c_str());
}

bool gen_ioctl(std::string driver_name, std::string fd_name, int index, int64_t cmd, llvm::Type* type, std::string direct, FILE *syzlangFile){
	std::string out = "";
	if (type == nullptr){
		out = "ioctl$"+driver_name+std::to_string(index)+
				  "(fd "+fd_name+", cmd const["+std::to_string(cmd)+
				  "], arg ptr["+"todo"+"])";
		fprintf(syzlangFile,"%s\n",out.c_str());
		//todo_count++;
		return true;
	}else if (llvm::PointerType* ptrType = llvm::dyn_cast<llvm::PointerType>(type)){
		if (ptrType->getPointerElementType()->isStructTy()){
			if (llvm::StructType * structtype = llvm::dyn_cast<llvm::StructType>(ptrType->getPointerElementType())){
				std::string structname = structtype->getStructName().str(); 
				out = "ioctl$"+driver_name+std::to_string(index)+
				  	  	 "(fd "+fd_name+", cmd const["+std::to_string(cmd)+
				      	 "], arg ptr["+direct+","+structname.substr(structname.find('.')+1)+"])";
			}
    	}else if (ptrType->getPointerElementType()->isIntegerTy()){
			if (llvm::IntegerType * integertype = llvm::dyn_cast<llvm::IntegerType>(ptrType->getPointerElementType())){
				uint32_t width= integertype->getIntegerBitWidth();
				out = "ioctl$"+driver_name+std::to_string(index)+
				  	  	 "(fd "+fd_name+", cmd const["+std::to_string(cmd)+
				      	 "], arg ptr["+direct+","+ "array[int"+ std::to_string(width) +"]])";
    		}	
		}
		fprintf(syzlangFile,"%s\n",out.c_str());
		return true;
	}
	return false;//type unsupported besides nullptr
}

bool generate_struct(llvm::StructType* structtype, FILE *syzlangFile){
	std::string structname = structtype->getStructName().str();
	structname = structname.substr(structname.find('.')+1);
	std::string out = "";
	out += structname+"{\n";
	std::string tab = "	";
	for (auto i = 0;i < structtype->getStructNumElements();i++){
		llvm::Type* field = structtype->getStructElementType(i);
		if (field->isIntegerTy()){//int
			if (llvm::IntegerType * integertype = llvm::dyn_cast<llvm::IntegerType>(field)){
				uint32_t width = integertype->getIntegerBitWidth();
				out += tab+"arg"+std::to_string(i)+"	"+"int"+std::to_string(width)+"\n";
			}
		}else if (field->isArrayTy()){//array
			if(llvm::ArrayType* arraytype = llvm::dyn_cast<llvm::ArrayType>(field)){
				uint64_t num = arraytype->getArrayNumElements();
				llvm::Type* eletype = arraytype->getArrayElementType();
				if (eletype->isIntegerTy()){
					out += tab+"arg"+std::to_string(i)+"	"+"array[int"+
					   	   std::to_string(eletype->getIntegerBitWidth())+","+
					       std::to_string(num)+"]\n";
				}else{
					return false;
				}
			}
		}else{
			return false;
		}
	}
	out += "}";
	fprintf(syzlangFile,"%s\n",out.c_str());
	return true;
}
