#ifndef HASH_H
#define HASH_H

#include <stdint.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <iostream>

uint64_t AwareHash(unsigned char* data, uint64_t n,
        uint64_t hash, uint64_t scale, uint64_t hardener);

uint64_t AwareHash_debug(unsigned char* data, uint64_t n,
        uint64_t hash, uint64_t scale, uint64_t hardener);

uint64_t GenHashSeed(uint64_t index);

uint64_t MurmurHash64A( const void * key, int len, uint64_t seed );
void MurmurHash3_x64_128 ( const void * key, const int len,
        uint32_t seed, void * out );

void mangle(const unsigned char* key, unsigned char* ret_key,
		int nbytes);

#endif /* __HASH_H__ */