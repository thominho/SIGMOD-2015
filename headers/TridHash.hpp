#ifndef TRID_HASH_HPP
#define TRID_HASH_HPP
#define TRID_HASH_BUCKET_SIZE 1
#define TRID_HASH_STARTING_SIZE 512
#include <stdint.h>
class TridHash
{
	struct Bucket
	{
		uint32_t trids[TRID_HASH_BUCKET_SIZE];
		unsigned int offsets[TRID_HASH_BUCKET_SIZE];

		unsigned int currentSize;
		unsigned int depth;

		Bucket(unsigned int d);

		void insert(uint32_t trid,unsigned int offset);
	};

	Bucket** index;

	unsigned int currentSize;
	unsigned int totalSize;
	unsigned int globalDepth;

	unsigned int hash(uint32_t trid);

public:

	TridHash();
	~TridHash();

	void insert(uint32_t trid,unsigned int offset);
	unsigned int find(uint32_t trid);
};
#endif