#include "../headers/TridHash.hpp"
#include <stdlib.h>
#include <stdio.h>

TridHash::Bucket::Bucket(unsigned int d)
{
	currentSize = 0;
	depth = d;
}

void TridHash::Bucket::insert(uint32_t trid,unsigned int offset)
{
	trids[currentSize] = trid;
	offsets[currentSize] = offset;
	currentSize++;
}

TridHash::TridHash()
{
	index = (Bucket**)malloc(TRID_HASH_STARTING_SIZE * sizeof(Bucket*));
	if(index == NULL)
	{
		perror("Failed to allocate memory for Trid Hashtable(TridHash.cpp:22)");
		exit(10);
	}

	index[0] = new Bucket(0);
	if(index[0] == NULL)
	{
		perror("Failed to create starting Bucket on Trid Hashtable(TridHash.cpp:29)");
		exit(11);
	}

	for(unsigned int i = 1;i < TRID_HASH_STARTING_SIZE;i++)
		index[i] = NULL;

	currentSize = 1;
	totalSize = TRID_HASH_STARTING_SIZE;
	globalDepth = 0;
}

TridHash::~TridHash()
{
	if(index == NULL) return;

	for(unsigned int i = 0;i < currentSize;i++)
		if(index[i] != NULL)
		{
			unsigned int power = 1;
			for(unsigned int j = 0;j < index[i]->depth;j++)
				power *= 2;

			delete index[i];

			for(unsigned int j = i;j < currentSize;j += power)
				index[j] = NULL;
		}

	free(index);
}

unsigned int TridHash::hash(uint32_t trid)
{
	if(globalDepth == 0) return 0;
	return trid % currentSize;
}

void TridHash::insert(uint32_t trid,unsigned int offset)
{
	unsigned int h = hash(trid);

	if(index[h]->currentSize == TRID_HASH_BUCKET_SIZE)
	{
		Bucket* oldBucket = index[h];
		index[h] = new Bucket(oldBucket->depth + 1);
		if(index[h] == NULL)
		{
			perror("Failed to create new Bucket for split in Trid hash insert(TridHash.cpp:67)");
			exit(11);
		}

		if(oldBucket->depth == globalDepth)
		{
			if(currentSize * 2 >= totalSize)
			{
				totalSize *=2;
				index = (Bucket**)realloc(index,totalSize * sizeof(Bucket*));
				if(index == NULL)
				{
					perror("Failed to reallocate memory on Trid hashtable(TridHash.cpp:)");
					exit(12);
				}
			}

			unsigned int oldSize = currentSize;
			currentSize *= 2;

			for(unsigned int i = 0;i < oldSize;i++)
			{
				if(i == h)
				{
					index[i + oldSize] = new Bucket(oldBucket->depth + 1);
					if(index[i + oldSize] == NULL)
					{
						perror("Failed to create new Bucket for split in Trid hash insert(TridHash.cpp:)");
						exit(13);
					}
				}
				else
					index[i + oldSize] = index[i];
			}

			globalDepth++;
		}
		else
		{
			unsigned int power = 1;
			for(unsigned int i = 0;i < oldBucket->depth;i++)
				power *= 2;

			unsigned int pot;
			for(unsigned int i = 0;i <= power;i++)
				if(index[i] == oldBucket || i == h)
				{
					pot = i;
					break;
				}

			/*pot is the first pointer to the old bucket. So h - pot is the distance of h from the first pointer
			  inside the array. Since every pointer has power distance with it's next, we have (h - pot) / power
			  pointers. If that number is even then pot and h should be pointing in the same bucket and thus the
			  split bucket should be created in pot + power. If it's odd then pot is pointing in a different
			  Bucket than h and the new bucket is created in position pot.*/
			if(((h - pot) / power) % 2)
			{
				index[pot] = new Bucket(oldBucket->depth + 1);
				if(index[pot] == NULL)
				{
					perror("Failed to create new bucket on Trid hash insert(TridHash.cpp:)");
					exit(14);
				}

				for(unsigned int i = pot;i < currentSize;i += power * 2)
				{
					index[i] = index[pot];
					index[i + power] = index[h];
				}
			}
			else
			{
				index[pot + power] = new Bucket(oldBucket->depth + 1);
				if(index[pot + power] == NULL)
				{
					perror("Failed to create new bucket on Trid hash insert(TridHash.cpp:)");
					exit(15);
				}

				for(unsigned int i = pot;i < currentSize;i += power * 2)
				{
					index[i] = index[h];
					index[i + power] = index[pot + power];
				}
			}
		}

		/*Reinsert the data of the old bucket to the hashtable*/
		for(unsigned int i = 0;i < oldBucket->currentSize;i++)
			index[hash(oldBucket->trids[i])]->insert(oldBucket->trids[i],oldBucket->offsets[i]);

		delete oldBucket;

		this->insert(trid,offset);
	}
	else
		index[h]->insert(trid,offset);
}

unsigned int TridHash::find(uint32_t trid)
{
	unsigned int h = hash(trid);

	for(unsigned int i = 0;i < index[h]->currentSize;i++)
		if(index[h]->trids[i] == trid) return index[h]->offsets[i];

	return 0;
}