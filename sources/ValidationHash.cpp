#include "../headers/ValidationHash.hpp"
#include <stdlib.h>

/*this is used in hash function*/
#define get16bits(d) ((((uint32_t)(((const uint8_t *)(d))[1])) << 8)\
                       +(uint32_t)(((const uint8_t *)(d))[0]) )

ValidationHash::Bucket::Bucket(unsigned int d)
{
	currentSize = 0;
	depth = d;
}

ValidationHash::Bucket::~Bucket()
{
	for(unsigned int i = 0;i < currentSize;i++)
		if(valPreds[i] != NULL) delete valPreds[i];
}

ValidationPredicate* ValidationHash::Bucket::insert(unsigned int from, unsigned int to, uint64_t* condition)
{
	valPreds[currentSize] = new ValidationPredicate(condition,from,to);
	if(valPreds[currentSize] == NULL)
	{
		perror("Failed to create new Predicate on ValidationHash(ValidationHash.cpp:15)");
		exit(10);
	}

	return valPreds[currentSize++];
}

void ValidationHash::Bucket::insert(ValidationPredicate* pred)
{
	valPreds[currentSize] = pred;
	currentSize++;
}

ValidationHash::ValidationHash()
{
	index = (Bucket**)malloc(VAL_HASH_STARTING_SIZE*sizeof(Bucket*));
	if(index == NULL)
	{
		perror("Failed to allocate memory for ValidationHash index (ValidationHash.cpp:27)");
		exit(11);
	}

	index[0] = new Bucket(0);
	if(index[0] == NULL)
	{
		perror("Failed to allocate memory for ValidationHash starting Bucket (ValidationHash.cpp:34)");
		exit(12);
	}

	currentSize = 1;
	totalSize = VAL_HASH_STARTING_SIZE;
	globalDepth = 0;

	overflow = NULL;
	overflowCurrentSize = 0;
	overflowTotalSize = 0;
}

ValidationHash::~ValidationHash()
{
	if(index == NULL) return;

	for(unsigned int i = 0;i < currentSize;i++)
	{
		if(index[i] != NULL)
		{
			unsigned int power = 1;
			for(unsigned int j = 0;j < index[i]->depth;j++)
				power*=2;

			delete index[i];

			for(unsigned int j = i;j < currentSize;j+=power)
				index[j]=NULL;
		}
	}

	free(index);

	if(overflow == NULL) return;

	for(unsigned int i = 0;i < overflowCurrentSize;i++)
		if(overflow[i] != NULL)
			delete overflow[i];

	free(overflow);
}

/*hash function reference link: http://www.azillionmonkeys.com/qed/hash.html*/
uint32_t ValidationHash::hash(unsigned int from, unsigned int to, uint64_t* condition)
{
	if(globalDepth == 0) return 0;

	char* data = (char*)malloc(11);
	if(data == NULL)
	{
		perror("Failed to allocate temporary memory for data on Validation Hashtable hash function(ValidationHash.cpp:)");
		exit(13);
	}

	data[0] = condition[0] << 3 | condition[1];
	data[1] = condition[2] & 0xFF;
	data[2] = from & 0xFF;
	data[3] = to & 0xFF;
	data[4] = (condition[2] & 0xFF00) >> 8;
	data[5] = (from & 0xFF00) >> 8;
	data[6] = (to & 0xFF00) >> 8;
	data[7] = (condition[2] & 0xFF0000) >> 16;
	data[8] = (from & 0xFF0000) >> 16;
	data[9] = (to & 0xFF0000) >> 16;
	data[10] = (condition[2] & 0xFF000000) >> 24;

	uint32_t hash = 11; /*11 is the length of data*/
	uint32_t tmp;
	int rem;
	unsigned int len = 11;

    rem = len & 3;
    len >>= 2;

    /* Main loop */
    for (;len > 0; len--) {
        hash  += get16bits (data);
        tmp    = (get16bits (data+2) << 11) ^ hash;
        hash   = (hash << 16) ^ tmp;
        data  += 2*sizeof (uint16_t);
        hash  += hash >> 11;
    }

    /* Handle end cases */
    switch (rem) {
        case 3: hash += get16bits (data);
                hash ^= hash << 16;
                hash ^= ((signed char)data[sizeof (uint16_t)]) << 18;
                hash += hash >> 11;
                break;
        case 2: hash += get16bits (data);
                hash ^= hash << 11;
                hash += hash >> 17;
                break;
        case 1: hash += (signed char)*data;
                hash ^= hash << 10;
                hash += hash >> 1;
    }

    /* Force "avalanching" of final 127 bits */
    hash ^= hash << 3;
    hash += hash >> 5;
    hash ^= hash << 4;
    hash += hash >> 17;
    hash ^= hash << 25;
    hash += hash >> 6;

    free(data);

    return hash;
}

ValidationPredicate* ValidationHash::insert(unsigned int from, unsigned int to, uint64_t* condition)
{
	uint32_t hash_ = hash(from,to,condition);
	unsigned int h = hash_ % currentSize;

	if(index[h]->currentSize == VAL_HASH_BUCKET_SIZE)
	{
		/*check to see if any of the values inside the old Bucket have the same hash function value as the current element*/
		for(unsigned int i = 0;i < VAL_HASH_BUCKET_SIZE;i++)
			if(hash_ == hash(index[h]->valPreds[i]->range_from,index[h]->valPreds[i]->range_to,index[h]->valPreds[i]->getCond()))
				return this->insertToOverflow(from,to,condition);

		Bucket* oldBucket = index[h];
		index[h] = new Bucket(oldBucket->depth + 1);
		if(index[h] == NULL)
		{
			perror("Failed to create new Bucket on Validation Hashtable(ValidationHash.cpp:)");
			exit(14);
		}

		if(oldBucket->depth == globalDepth)
		{
			if(currentSize * 2 >= totalSize)
			{
				index = (Bucket**)realloc(index,totalSize * sizeof(Bucket*));
				if(index == NULL)
				{
					perror("Failed to reallocate Validation Hashtable index(ValidationHash.cpp:)");
					exit(15);
				}

				totalSize *= 2;
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
						perror("Failed to create new Bucket on Validation Hashtable(ValidationHash.cpp:)");
						exit(16);
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
			unsigned int hashVal=hash(oldBucket->valPreds[i]->range_from,oldBucket->valPreds[i]->range_to,oldBucket->valPreds[i]->getCond()) % currentSize;
			index[hashVal]->insert(oldBucket->valPreds[i]);
		}

		delete oldBucket;

		return this->insert(from,to,condition);
	}
	else
		return index[h]->insert(from,to,condition);
}

ValidationPredicate* ValidationHash::find(unsigned int from, unsigned int to, uint64_t* condition)
{
	unsigned int h = hash(from,to,condition) % currentSize;

	for(unsigned int i = 0;i < index[h]->currentSize;i++)
		if(index[h]->valPreds[i]->isPredicate(from,to,condition)) return index[h]->valPreds[i];

	if(overflow == NULL) return NULL;

	for(unsigned int i = 0;i < overflowCurrentSize;i++)
		for(unsigned int j = 0;j < overflow[i]->currentSize;j++)
			if(overflow[i]->valPreds[j]->isPredicate(from,to,condition)) return overflow[i]->valPreds[j];

	return NULL;
}

unsigned int ValidationHash::getOffset(ValidationPredicate* vp)
{
	unsigned int h = hash(vp->range_from,vp->range_to,vp->getCond()) % currentSize;

	for(unsigned int i = 0;i < index[h]->currentSize;i++)
		if(index[h]->valPreds[i] == vp) return h + 2;

	if(overflow == NULL) return 0;

	for(unsigned int i = 0;i < overflowCurrentSize;i++)
		for(unsigned int j = 0;j < overflow[i]->currentSize;j++)
			if(overflow[i]->valPreds[j] == vp) return 1;

	return 0;
}

/*works only for bucket size == 1 for now*/
void ValidationHash::remove(unsigned int offset)
{
	//fprintf(stderr,"Now entering ValidationHash::remove\n");
	if(offset != 0)
	{
		offset--;
		if(index[offset]->depth <= 1)
		{
			delete index[offset]->valPreds[0];
			index[offset]->valPreds[0] = NULL;
			index[offset]->currentSize = 0;
			return;
		}

		unsigned int power = 1;
		unsigned int pot;

		for(unsigned int i = 0;i < index[offset]->depth;i++)
			power *= 2;

		for(unsigned int i = 0;i <= offset;i++)
			if(index[i] == index[offset])
			{
				pot = i;
				break;
			}

		delete index[pot];

		fprintf(stderr,"currentSize: %d\n",currentSize);
		fprintf(stderr, "power: %d\n", power);
		fprintf(stderr, "pot: %d\n", pot);

		if(power/2 + pot >= currentSize && pot - power/2 < 0) return;
		if(currentSize == 0) return;

		for(int i = pot;i < currentSize;i += power)
			if(i - power/2 < 0)
				index[i] = index[i + power/2];
			else
				index[i] = index[i - power/2];


		index[pot]->depth--;
	}
}

ValidationPredicate* ValidationHash::insertToOverflow(unsigned int from, unsigned int to, uint64_t* condition)
{
	if(overflow == NULL)
	{
		overflow = (Bucket**)malloc(VAL_HASH_OVERFLOW_BLOCK * sizeof(Bucket**));
		if(overflow == NULL)
		{
			perror("Failed to allocate memory for overflow (ValidationHash.cpp:)");
			exit(15);
		}

		overflowTotalSize = VAL_HASH_OVERFLOW_BLOCK;
	}
	else if(overflowTotalSize <= overflowCurrentSize)
	{
		overflow = (Bucket**)realloc(overflow,(overflowTotalSize + VAL_HASH_OVERFLOW_BLOCK) * sizeof(Bucket**));
		if(overflow == NULL)
		{
			perror("Failed to reallocate memory for overflow (ValidationHash.cpp:)");
			exit(16);
		}

		overflowTotalSize += VAL_HASH_OVERFLOW_BLOCK;
	}

	if(overflowCurrentSize == 0)
	{
		overflow[0] = new Bucket(0);
		if(overflow[0] == NULL)
		{
			perror("Failed create overflow Bucket on Validation Hashtable (ValidationHash.cpp:346");
			exit(17);
		}
		overflowCurrentSize++;
	}
	else if(overflow[overflowCurrentSize - 1]->currentSize == VAL_HASH_BUCKET_SIZE)
	{
		/*every bucket inside overflow has depth 0*/
		overflow[overflowCurrentSize] = new Bucket(0);
		if(overflow[overflowCurrentSize] == NULL)
		{
			perror("Failed create overflow Bucket on Validation Hashtable (ValidationHash.cpp:357");
			exit(18);
		}
		overflowCurrentSize++;
	}

	return overflow[overflowCurrentSize - 1]->insert(from,to,condition);
}