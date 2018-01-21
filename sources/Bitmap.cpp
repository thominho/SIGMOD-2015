#include "../headers/Bitmap.hpp"

bitmap::bitmap(){
	bits=NULL;
}

void bitmap::allocation(int length){
	bits=(uint8_t*) malloc(sizeof(uint8_t)*length);
}

bitmap::~bitmap(){
	if(bits!=NULL){
		free(bits);
	}
}