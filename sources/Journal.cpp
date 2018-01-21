#include "../headers/Journal.hpp"
#define BLOCKS 100
#define JOURNAL_BLOCK 100


void Journal::Record::initialize(){
	tid=0;
	columns=NULL;
}

/*-------------------------------------------------------------------------------------------------*/


void Journal::Record::allocate(uint32_t threadId,uint32_t numOfCols,uint64_t* colValues,int flag){
	tid=threadId;
	columns=(uint64_t*) malloc(sizeof(uint64_t)*numOfCols);
	if(columns==NULL)
	{
		exit(1);
	}
	for(uint64_t i=0;i<numOfCols;i++)
	{
		columns[i]=colValues[i];
	}
	deletion=flag;
}


/*-------------------------------------------------------------------------------------------------*/



void Journal::Record::Columnsdeallocate(){
	tid=0;
	free(columns);
	columns=NULL;
}


/*-------------------------------------------------------------------------------------------------*/


void Journal::createJournal(uint32_t numOfColumns){
	numOfCols=numOfColumns;
	currentSize=0;
	totalSize=JOURNAL_BLOCK;
	records=(Record *) malloc(sizeof(Record)*JOURNAL_BLOCK);
	if(records==NULL)
	{
		exit(1);
	}
	for(int i=0;i<JOURNAL_BLOCK;i++)
	{
		records[i].initialize();
	}
}


/*-------------------------------------------------------------------------------------------------*/



void Journal::destroyJournal(){
	int i;
	for(i=0;i<currentSize;i++)
	{
		if(records[i].columns!=NULL)
		{records[i].Columnsdeallocate();}
	}
	if(records!=NULL)
	{free(records);}
}


/*-------------------------------------------------------------------------------------------------*/



void Journal::appendInsertion(uint32_t trId,uint64_t* colValues){
	if(currentSize<totalSize)
	{
		records[currentSize].allocate(trId,numOfCols,colValues,0);
		currentSize++;
	}
	else if(currentSize==totalSize)
	{
		totalSize=totalSize+BLOCKS;
		records=(Record *) realloc(records,totalSize*sizeof(Record));
		if(records==NULL)
		{
			cout << "Something went wrong and programm must terminate" << endl;
			exit(1);
		}
		records[currentSize].allocate(trId,numOfCols,colValues,0);
		currentSize++;
	}
}


/*-------------------------------------------------------------------------------------------------*/



void Journal::PrintRecords(){
	int i;
	for(i=0;i<currentSize;i++)
	{
	 	cout << records[i].tid << ",";
		for(int j=0;j<numOfCols;j++)
		{
				cout << records[i].columns[j] << ",";
		}
		cout << records[i].deletion << endl;
		cout << endl;
	}
}


/*-------------------------------------------------------------------------------------------------*/



void Journal::appendDeletion(uint32_t trId,uint32_t pos){
	if(currentSize<totalSize)
	{
		records[currentSize].allocate(trId,numOfCols,records[pos].columns,1);
		currentSize++;
	}

	else if(currentSize==totalSize)
	{
		totalSize=totalSize+BLOCKS;
		records=(Record *) realloc(records,totalSize*sizeof(Record));
		if(records==NULL)
		{
			cout << "Something went wrong and programm must terminate" << endl;
			exit(1);
		}
		records[currentSize].allocate(trId,numOfCols,records[pos].columns,1);
		currentSize++;
	}
}


/*-------------------------------------------------------------------------------------------------*/




unsigned int Journal::valid(uint64_t *type,int csize,int rid,uint32_t from,uint32_t to,ExtendableHashtable *h){
	int i=-1;
	int result_and,temp2=0;
	for(int k=from;k<=to;k++){
		if(i==-1){
			i=BinarySearch(k);
		}
	}
	if(i==-1){
		return 0;
	}
	while(records[i].tid<=to && i<currentSize){
		result_and=0;
		int j=0;
		while(j<csize){
			result_and=0;
			if(type[j]!=0 || type[j+1]!=0){
				if(type[j+1]==0){
					if(records[i].columns[type[j]]==type[j+2])
					{
						result_and=1;
					}
				}
				else if(type[j+1]==1){
					if(records[i].columns[type[j]]!=type[j+2])
					{
						result_and=1;
					}

				}
				else if(type[j+1]==2){
					if(records[i].columns[type[j]]<type[j+2])
					{
						result_and=1;
					}

				}
				else if(type[j+1]==3){
					if(records[i].columns[type[j]]<=type[j+2])
					{
						result_and=1;
					}
				}
				else if(type[j+1]==4){
					if(records[i].columns[type[j]]>type[j+2])
					{
						result_and=1;
					}
				}
				else if(type[j+1]==5){
					if(records[i].columns[type[j]]>=type[j+2])
					{
						result_and=1;
					}
				}
			}
			else{
					unsigned int *array2=h[rid].find(type[j+2],from,to);
					int op=0;
					if(array2!=NULL){
						if(array2[op]==0){
							result_and=0;
						}
						else{
							while(array2[op]!=0){
								if(i==array2[op]-1 && records[array2[op]-1].columns[0]==type[j+2]){
									result_and=1;
								}
								op++;
							}
						}
					}
			}
			j=j+3;
			if(result_and==0){
				break;
			}
		}
		if(result_and==1){
			return 1;
		}
		i++;
		if(i==currentSize){
			return 0;
		}
	}
}

/*----------------------------------------------------------------------------------------------------------------------------------------------------*/

unsigned int Journal::valid2(uint64_t *type,int rid,ExtendableHashtable *h,uint8_t* bitmap,int from,int to,int search_by,int until,int size){
	if(type[0]!=0 || type[1]!=0){
		int temp3=1;
		int i=search_by;
		int counter=0;
		while(records[i].tid<=to && i<currentSize){
			counter++;
				if(type[1]==0){
					if(records[i].columns[type[0]]==type[2])
					{	
						uint8_t temp=0x01;
						temp=temp << (i-search_by)%8;
						//printf("%luaaaaaa4\n",temp);
						bitmap[temp3-1]=bitmap[temp3-1] | temp;
					}
				}
				else if(type[1]==1){
					if(records[i].columns[type[0]]!=type[2])
					{
						uint8_t temp=0x01;
						temp=temp << (i-search_by)%8;
						//printf("%luaaaaaa4\n",temp);
						bitmap[temp3-1]=bitmap[temp3-1] | temp;
					}
				}
				else if(type[1]==2){
					if(records[i].columns[type[0]]<type[2])
					{
						uint8_t temp=0x01;
						temp=temp << (i-search_by)%8;
						bitmap[temp3-1]=bitmap[temp3-1] | temp;
					}
				}
				else if(type[1]==3){
					if(records[i].columns[type[0]]<=type[2])
					{
						uint8_t temp=0x01;
						temp=temp << (i-search_by)%8;
						bitmap[temp3-1]=bitmap[temp3-1] | temp;
					}
				}
				else if(type[1]==4){
					if(records[i].columns[type[0]]>type[2])
					{
						uint8_t temp=0x01;
						temp=temp << (i-search_by)%8;
						bitmap[temp3-1]=bitmap[temp3-1] | temp;
					}
				}
				else if(type[1]==5){
					if(records[i].columns[type[0]]>=type[2])
					{
						uint8_t temp=0x01;
						temp=temp << (i-search_by)%8;
						bitmap[temp3-1]=bitmap[temp3-1] | temp;
					}
				}
				if(counter%8==0){
					temp3=temp3+1;
				}
				i++;
				if(i==currentSize){
					return 0;
				}
			}
		}
			else if(type[0]==type[1]){
					unsigned int *array2=h[rid].find(type[2],from,to);
					int op=0;
					if(array2!=NULL){
							while(array2[op]!=0){
								if(records[array2[op]-1].columns[0]==type[2]){
									uint8_t temp=0x01;
									temp=temp << ((array2[op]-1)-search_by)%8;
									int temp3 = (array2[op]-1-search_by)/8;
									bitmap[temp3]=bitmap[temp3] | temp;
								}
								op++;
							}
					}
			}
			//cout << endl;
}

/*--------------------------------------------------------------------------------------------------*/

unsigned int Journal::valid3(uint64_t *type,int csize,int rid,uint32_t from,uint32_t to,ExtendableHashtable *h){
	int i=-1;
	int result_and,temp2=0;
	for(int k=from;k<=to;k++){
		if(i==-1){
			i=BinarySearch(k);
		}
	}
	if(i==-1){
		return 0;
	}
	while(records[i].tid<=to && i<currentSize){
		result_and=0;
		int j=0;
		while(j<csize){
			result_and=0;
			if(type[j]!=0 || type[j+1]!=0){
				if(type[j+1]==0){
					if(records[i].columns[type[j]]==type[j+2])
					{
						result_and=1;
					}
				}
				else if(type[j+1]==1){
					if(records[i].columns[type[j]]!=type[j+2])
					{
						result_and=1;
					}

				}
				else if(type[j+1]==2){
					if(records[i].columns[type[j]]<type[j+2])
					{
						result_and=1;
					}

				}
				else if(type[j+1]==3){
					if(records[i].columns[type[j]]<=type[j+2])
					{
						result_and=1;
					}
				}
				else if(type[j+1]==4){
					if(records[i].columns[type[j]]>type[j+2])
					{
						result_and=1;
					}
				}
				else if(type[j+1]==5){
					if(records[i].columns[type[j]]>=type[j+2])
					{
						result_and=1;
					}
				}
			}
			else{
					unsigned int *array2=h[rid].find2(type[j+2],from,to);
					int op=0;
					if(array2!=NULL){
						if(array2[op]==0){
							result_and=0;
						}
						else{
							while(array2[op]!=0){
								if(i==array2[op]-1 && records[array2[op]-1].columns[0]==type[j+2]){
									result_and=1;
								}
								op++;
							}
						}
					}
					free(array2);
			}
			j=j+3;
			if(result_and==0){
				break;
			}
		}
		if(result_and==1){
			return 1;
		}
		i++;
		if(i==currentSize){
			return 0;
		}
	}
}

/*--------------------------------------------------------------------------------------------------*/

uint32_t Journal::BinarySearch(uint32_t key){
	int first=0;
	int pos=currentSize-1;
	int middle=(first+pos)/2;
	while(first<=pos)
	{
		//cout << middle << endl;
		if(records[middle].tid<key)
		{
			first=middle+1;
		}
		else if(records[middle].tid==key)
		{
			if(middle>0)//search where the tids begins from
			{
				while(records[middle].tid==key && middle>0)	
				{
					middle=middle-1;
				}
			}
			//cout << middle << endl;
			middle=middle+1;
			return middle;
		}
		else
		{
			pos=middle-1;
		}
		middle=(first+pos)/2;
	}
	return -1;
}

/*--------------------------------------------------------------------------------------------------*/

uint32_t Journal::getCurrentPos(){
	return currentSize;
}

/*-------------------------------------------------------------------------------------------------*/

int Journal::return_deletion(int pos){
	return records[pos].deletion;
}

/*-------------------------------------------------------------------------------------------------*/

int Journal::return_reg(int start,int end){
	int i=start;
	int counter=0;
	while(records[i].tid<=end && i<currentSize){
		counter=counter+1;
		i++;
		if(i==currentSize){
			return counter;
		}
	}
	return counter;
}


