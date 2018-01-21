#include "../headers/ValidationPredicate.hpp"

uint64_t* ValidationPredicate::validay;

ValidationPredicate::ValidationPredicate(uint64_t *query,unsigned int from,unsigned int to)
{
	range_from=from;
	range_to=to;
	column=query[0];
	condition=query[1];
	cond_value=query[2];
	in_use_by=1;
	if(validay==NULL){
	validay=(uint64_t *) malloc(sizeof(uint64_t)*3);
	}
}


uint64_t* ValidationPredicate::getCond()
{
	validay[0]=column;
	validay[1]=condition;
	validay[2]=cond_value;
	return validay;
}

bool ValidationPredicate::isPredicate(unsigned int cfrom,unsigned int cto,uint64_t* tcondition)
{	
	if(cfrom==range_from && cto==range_to && tcondition[0]==column && tcondition[1]==condition && tcondition[2]==cond_value){
		return true;
	}
	return false;
}


ValidationPredicate::~ValidationPredicate()
{
 if(validay!=NULL){
 free(validay);
 validay=NULL;
 }
}

uint8_t * ValidationPredicate::getValue(Journal *j,ExtendableHashtable *h,TridHash *tid,int rid,int search_by,int until,int size)
{
	if(value.bits==NULL){
		value.allocation(size);
		for(int k=0;k<size;k++){
			value.bits[k]=0x00;
		}
		uint64_t temp[3];
		temp[0]=column;
		temp[1]=condition;
		temp[2]=cond_value;
		j[rid].valid2(temp,rid,h,value.bits,range_from,range_to,search_by,until,size);//pame edw {dramatic music plays}
	}
	return value.bits;
}