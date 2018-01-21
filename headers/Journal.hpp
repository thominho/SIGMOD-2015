#ifndef JOURNAL_HPP
#define JOURNAL_HPP
#include <iostream>
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <unistd.h>
#include "../headers/ExtendableHashtable.hpp"

using namespace std;

class Journal
{
	struct Record
	{
		uint32_t tid; /*transaction id*/
		uint64_t* columns;
		int deletion;

		void allocate(uint32_t threadId,uint32_t cols,uint64_t* colValues,int flag);/*allocate the space when needed*/
		void Columnsdeallocate();/*deallocate the space when needed*/
		void initialize();/*initialiaze the records*/
	};
	
	uint32_t numOfCols;
	unsigned int currentSize;
	unsigned int totalSize;
	Record* records; /*must be single pointer because of cache management*/
	
	public:
		void createJournal(uint32_t numOfColumns);
		void destroyJournal();
		/*type=0 means binary search in tid, type=1 means binary search in columns[0], type=2 means binary search in columns[1] etc
		   RETURNS: the position of the record in the records array*/
		unsigned int valid(uint64_t *type,int csize,int rid,uint32_t from,uint32_t to,ExtendableHashtable *h);
		unsigned int valid2(uint64_t *type,int rid,ExtendableHashtable *h,uint8_t* bitmap,int from,int to,int search_by,int until,int size);

		/*valid3 is thes same as valid but instead of calling function find from hashtable, it call the thread
		  safe version of it: find2.*/
		unsigned int valid3(uint64_t *type,int csize,int rid,uint32_t from,uint32_t to,ExtendableHashtable *h);
		
		void appendInsertion(uint32_t trId,uint64_t* colValues);
		
		/*append deletion with key being the primary key*/
		void appendDeletion(uint32_t trId,uint32_t key);
		
		/*debugging factions may be needed at another part*/
		uint32_t getCurrentPos();
		void PrintRecords();
		/*if needed for validations*/
		uint32_t BinarySearch(uint32_t key);
		int return_deletion(int pos);
		int return_reg(int start,int end);
};

#endif
