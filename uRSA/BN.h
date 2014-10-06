#ifndef _BN_H_
#define _BN_H_

#include <stdint.h>


typedef struct{

	uint8_t nbytes;	//num bytes uzed - 1   (for 2048-bit value this will be 255)
	uint8_t* data;

}BN;

//externally needed

void* _bnRealloc(void* ptr, uint16_t newSz);
void* _bnMalloc(uint16_t t);
void _bnFree(void* ptr);



char bnExpMod(BN* dst, BN* a, BN* b, BN* n);	//a^b mod n		a and b are destroyed, n stays
char bnMul(BN* d, BN* a, BN* b);
void bnSub(BN* a, const BN* b);
char bnAdd(BN* r, const BN* a, const BN* b);


#endif
