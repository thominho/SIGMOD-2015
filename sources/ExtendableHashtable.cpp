#include "../headers/ExtendableHashtable.hpp"
#include <stdlib.h>
#include <stdio.h>

unsigned int* ExtendableHashtable::values;

ExtendableHashtable::Bucket::Bucket(unsigned int d)
{
	depth=d;
	currentSize = 0;

	keys = (uint32_t*)malloc(BUCKET_SIZE*sizeof(uint32_t));
	if (keys == NULL)
	{
		perror("Failed to create bucket (keys malloc)");
		exit(-1);
	}

	trids = (TridsList*)malloc(BUCKET_SIZE*sizeof(TridsList));
	if (trids == NULL)
	{
		perror("Failed to create bucket (tids malloc)");
		exit(-2);
	}

	for (unsigned int i = 0;i<BUCKET_SIZE;i++)
	{
		trids[i].trid = (uint32_t*)malloc(TRID_BLOCK*sizeof(uint32_t));
		if (trids[i].trid == NULL)
		{
			perror("Failed to allocate memory for transaction ids while creating bucket");
			exit(-3);
		}

		trids[i].offsets = (unsigned int**)malloc(TRID_BLOCK*sizeof(unsigned int*));
		if (trids[i].trid == NULL)
		{
			perror("Failed to allocate memory for offsets while creating bucket");
			exit(-4);
		}
		for (int j = 0;j<TRID_BLOCK;j++)
			trids[i].offsets[j] = NULL;

		trids[i].totalSize = TRID_BLOCK;
		trids[i].size = 0;
	}
}

ExtendableHashtable::Bucket::~Bucket()
{
	if (keys != NULL) free(keys);
	if (trids == NULL) return;
	for (unsigned int i = 0;i<BUCKET_SIZE;i++)
	{
		if (trids[i].trid != NULL) free(trids[i].trid);
		if (trids[i].offsets != NULL)
		{
			for (unsigned int j = 0;j<trids[i].totalSize;j++)
				if (trids[i].offsets[j] != NULL) free(trids[i].offsets[j]);
			free(trids[i].offsets);
		}
	}
	free(trids);
}

/*Note that this function does not take into account the size of the bucket. The
  calling function has to make sure that things won't be inserted in a full bucket*/
void ExtendableHashtable::Bucket::insert(uint32_t key, uint32_t trid, unsigned int* offset)
{

	unsigned int size;

	for(unsigned int i = 0;i<currentSize;i++)
		if(keys[i]==key)
		{
			if(trids[i].size==trids[i].totalSize)
			{
				trids[i].trid=(uint32_t*)realloc(trids[i].trid,(trids[i].totalSize + TRID_BLOCK)*sizeof(uint32_t));
				if(trids[i].trid==NULL)
					exit(-10);

				trids[i].offsets=(unsigned int**)realloc(trids[i].offsets,(trids[i].totalSize + TRID_BLOCK)*sizeof(unsigned int*));
				if(trids[i].offsets==NULL)
					exit(-10);

				for(unsigned int j=trids[i].totalSize;j < trids[i].totalSize + TRID_BLOCK;j++)
				{
					trids[i].offsets[j]=NULL;
				}

				trids[i].totalSize += TRID_BLOCK;
			}

			size=trids[i].size;

			trids[i].trid[size]=trid;
			if(trids[i].offsets[size]!=NULL)
				free(trids[i].offsets[size]);
			trids[i].offsets[size]=(unsigned int*)malloc(2*sizeof(unsigned int));
			if(trids[i].offsets[size]==NULL)
				exit(-10);
			trids[i].offsets[size][0]=offset[0];
			trids[i].offsets[size][1]=offset[1];
			trids[i].size++;

			return;
		}

	keys[currentSize]=key;

	size=trids[currentSize].size;
	trids[currentSize].trid[size]=trid;
	if(trids[currentSize].offsets[size]!=NULL)
		free(trids[currentSize].offsets[size]);
	trids[currentSize].offsets[size]=(unsigned int*)malloc(2*sizeof(unsigned int));
	if(trids[currentSize].offsets[size]==NULL)
		exit(-10);
	trids[currentSize].offsets[size][0]=offset[0];
	trids[currentSize].offsets[size][1]=offset[1];
	trids[currentSize].size++;

	currentSize++;
}

ExtendableHashtable::ExtendableHashtable()
{
	bucketSize = BUCKET_SIZE;
	globalDepth = 0;
	totalSize = HASH_BLOCK;
	currentSize = 1;

	testVar=42;

	index = (Bucket**)malloc(HASH_BLOCK*sizeof(Bucket*));
	if (index == NULL)
	{
		perror("Failed to create Hashtable(index malloc)");
		exit(-21);
	}

	index[0] = new Bucket(0);
	if (index[0] == NULL)
	{
		perror("Failed to create Hashtable");
		exit(-22);
	}

	for (unsigned int i = 1;i<totalSize;i++)
		index[i] = NULL;
}

ExtendableHashtable::~ExtendableHashtable()
{
	if(index == NULL)
	{
		if(values == NULL) return;
		free(values);
		return;
	}

	unsigned int power;
	for (unsigned int i = 0;i<currentSize;i++)
	{
		if (index[i] != NULL)
		{
			power = 1;
			for (unsigned int j = 0;j<index[i]->depth;j++) power *= 2;

			delete index[i];
			index[i] = NULL;
			for (unsigned int j = i + power;j<currentSize;j += power)
				index[j] = NULL;
		}
	}
	
	if(values!=NULL) {
		free(values);
		values=NULL;
	}
	free(index);
}

unsigned int ExtendableHashtable::hash(uint32_t key)
{
	if (globalDepth == 0) return 0;
	return key%currentSize;
}

void ExtendableHashtable::insert(uint32_t key, uint32_t trid, unsigned int* offset)
{
	unsigned int h = hash(key);

	/*if key is already inside the bucket, add the thread id into the list*/
	for (unsigned int i = 0;i < index[h]->currentSize;i++)
		if (index[h]->keys[i] == key)
		{
			index[h]->insert(key, trid, offset);
			return;
		}

	if (index[h]->currentSize == bucketSize) /*split*/
	{
		Bucket* oldBucket=index[h];
		index[h]=new Bucket(oldBucket->depth + 1);
		if(index[h]==NULL)
			exit(-12);

		if (oldBucket->depth == globalDepth) /*split index*/
		{
			/*if spliting the index requires more memory than already allocated, reallocate memory*/
			if (currentSize * 2 > totalSize)
			{
				totalSize *= 2;

				index = (Bucket**)realloc(index, totalSize*sizeof(Bucket*));
				if (index == NULL)
				{
					perror("Failed to reallocate hash index (insert)");
					exit(-12);
				}
			}

			unsigned int oldSize=currentSize;

			currentSize*=2;

			for(unsigned int i = 0;i<oldSize;i++)
			{
				if(i==h)
				{
					index[i+oldSize]=new Bucket(oldBucket->depth + 1);
					if(index[i+oldSize]==NULL)
						exit(-12);
				}
				else
					index[i+oldSize]=index[i];
			}

			globalDepth++;
		}
		else
		{
			/*calculate 2^d, where d is the depth of the old(full) bucket*/
			/*2^d is the distance between the pointers to the old(full) bucket*/
			unsigned int power = 1;
			for (unsigned int i = 0;i<oldBucket->depth;i++)
				power *= 2;

			/*we need the first pointer to the bucket we want to split*/
			unsigned int pot;
			for (int i = 0;i <= power;i++)
				if (index[i] == oldBucket || i == h)
				{
					pot = i;
					break;
				}

			if(((h-pot)/power)%2)
			{
				/*create a new bucket for the split*/
				index[pot] = new Bucket(oldBucket->depth + 1);
				if (index[pot] == NULL)
				{
					perror("Failed to create new bucket on split (hash insertion)");
					exit(-14);
				}

				/*make the pointer that were pointing on the old bucket, point to the
				  new buckets.*/
				for(unsigned int i = pot;i < currentSize;i += power*2)
				{
					index[i] = index[pot];
					index[i + power] = index[h];
				}
			}
			else
			{
				/*create a new bucket for the split*/
				index[pot + power] = new Bucket(oldBucket->depth + 1);
				if (index[pot + power] == NULL)
				{
					perror("Failed to create new bucket on split (hash insertion)");
					exit(-14);
				}

				/*make the pointer that were pointing on the old bucket, point to the
				  new buckets.*/
				for (int i = pot;i < currentSize;i+=2*power)
				{
					index[i]=index[h];
					index[i+power]=index[pot + power];
				}
			}
		}

		/*Reinsert the data of the old bucket to the hashtable*/
		for(int i=0;i<oldBucket->currentSize;i++)
		{
			unsigned int hashVal=hash(oldBucket->keys[i]);
			for(int j=0;j<oldBucket->trids[i].size;j++)
				index[hashVal]->insert(oldBucket->keys[i],oldBucket->trids[i].trid[j],oldBucket->trids[i].offsets[j]);
		}

		delete oldBucket;

		this->insert(key,trid,offset);
	}
	else
		index[h]->insert(key, trid, offset);
}

bool binary_search(uint32_t* array, uint32_t key, unsigned int size, unsigned int& result)
{
	result = size / 2;

	while (size != 0)
	{
		size /= 2;
		if (array[result] == key) return true;
		if (array[result]<key) result = size + size / 2;
		else result = size / 2;
	}

	return false;
}

unsigned int* ExtendableHashtable::find(uint32_t key, uint32_t from, uint32_t to)
{
	unsigned int h = hash(key);

	unsigned int pos;
	if (!binary_search(index[h]->keys, key, index[h]->currentSize, pos)) return NULL;

	unsigned int offsetPos;
	bool found = binary_search(index[h]->trids[pos].trid, from, index[h]->trids[pos].size, offsetPos);

	if (found == false)
	{
		if(index[h]->trids[pos].trid[offsetPos] > from)
		{
			while(1)
			{
				if(offsetPos == 0) break;
				if(index[h]->trids[pos].trid[offsetPos - 1] <= from) break;
				offsetPos--;
			}
			if(index[h]->trids[pos].trid[offsetPos] > to) return NULL;
		}
		else
		{
			while(1)
			{
				if(offsetPos == index[h]->trids[pos].size) return NULL;
				if(index[h]->trids[pos].trid[offsetPos] > from) break;
				offsetPos++;
			}
		}
	}

	if (values != NULL) free(values);
	values = (unsigned int*)malloc((index[h]->trids[pos].size)*sizeof(unsigned int) * 2 + sizeof(unsigned int));
	if (values == NULL)
	{
		perror("Failed to allocate memory for return values in hash function find");
		return NULL;
	}

	unsigned int counter = 0;
	for (unsigned int i = 0;i<index[h]->trids[pos].size;i++)
	{
		if (index[h]->trids[pos].trid[i] >= from && index[h]->trids[pos].trid[i] <= to)
		{
			if (index[h]->trids[pos].offsets[offsetPos][0] != 0)
			{
				values[counter] = index[h]->trids[pos].offsets[offsetPos][0];
				counter++;
			}
			if (index[h]->trids[pos].offsets[offsetPos][1] != 0)
			{
				values[counter] = index[h]->trids[pos].offsets[offsetPos][1];
				counter++;
			}
		}
	}

	values[counter] = 0;

	return values;
}

unsigned int* ExtendableHashtable::find2(uint32_t key, uint32_t from, uint32_t to)
{
	unsigned int h = hash(key);

	unsigned int pos;

	unsigned int* retVal;

	if (!binary_search(index[h]->keys, key, index[h]->currentSize, pos)) return NULL;

	unsigned int offsetPos;
	bool found = binary_search(index[h]->trids[pos].trid, from, index[h]->trids[pos].size, offsetPos);

	if (found == false)
	{
		if(index[h]->trids[pos].trid[offsetPos] > from)
		{
			while(1)
			{
				if(offsetPos == 0) break;
				if(index[h]->trids[pos].trid[offsetPos - 1] <= from) break;
				offsetPos--;
			}
			if(index[h]->trids[pos].trid[offsetPos] > to) return NULL;
		}
		else
		{
			while(1)
			{
				if(offsetPos == index[h]->trids[pos].size) return NULL;
				if(index[h]->trids[pos].trid[offsetPos] > from) break;
				offsetPos++;
			}
		}
	}

	retVal = (unsigned int*)malloc((index[h]->trids[pos].size)*sizeof(unsigned int) * 2 + sizeof(unsigned int));
	if (retVal == NULL)
	{
		perror("Failed to allocate memory for return values in hash function find");
		return NULL;
	}

	unsigned int counter = 0;
	for (unsigned int i = 0;i<index[h]->trids[pos].size;i++)
	{
		if (index[h]->trids[pos].trid[i] >= from && index[h]->trids[pos].trid[i] <= to)
		{
			if (index[h]->trids[pos].offsets[offsetPos][0] != 0)
			{
				retVal[counter] = index[h]->trids[pos].offsets[offsetPos][0];
				counter++;
			}
			if (index[h]->trids[pos].offsets[offsetPos][1] != 0)
			{
				retVal[counter] = index[h]->trids[pos].offsets[offsetPos][1];
				counter++;
			}
		}
	}

	retVal[counter] = 0;

	return retVal;
}

unsigned int ExtendableHashtable::findLast(uint32_t key)
{
	unsigned int h = hash(key);

	unsigned int pos;
	if (!binary_search(index[h]->keys, key, index[h]->currentSize, pos)) return 0;

	TridsList* temp = &(index[h]->trids[pos]);

	if (temp->offsets[temp->size - 1][1] == 0)return temp->offsets[temp->size - 1][0];
	else return temp->offsets[temp->size - 1][1];
}

void ExtendableHashtable::printData()
{
	for(unsigned int i = 0;i < currentSize;i++)
	{
		fprintf(stderr,"index[%u]\n",i);
		for(unsigned int j = 0;j < index[i]->currentSize;j++)
		{
			fprintf(stderr,"\tkeys[%u]=%u\n",j,index[i]->keys[j]);
			fprintf(stderr, "\ttrids[%u]\n", j);
			for(unsigned int k = 0;k < index[i]->trids[j].size;k++)
			{
				fprintf(stderr,"\t\ttrid[%u]=%u\n",k,index[i]->trids[j].trid[k]);
				fprintf(stderr,"\t\t\toffset[0]=%u\n",index[i]->trids[j].offsets[k][0]);
				fprintf(stderr,"\t\t\toffset[1]=%u\n",index[i]->trids[j].offsets[k][1]);
			}
		}
	}
	fprintf(stderr,"\n");
}