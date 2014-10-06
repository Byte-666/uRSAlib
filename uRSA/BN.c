#include "BN.h"



static char bnSet(BN* bn, uint8_t val){
	
	bn->nbytes = 0;
	bn->data = _bnMalloc(1);
	if(!bn->data) return 0;
	
	bn->data[0] = val;
	return 1;
}

static void bnFree(BN* bn){
	
	_bnFree(bn->data);
}

static char bnIsNonzero(const BN* bn){
	
	uint8_t i = bn->nbytes;
	
	do{
		
		if(bn->data[i]) return 1;
		
	}while(i--);
	
	return 0;
}

static void bnNormalize(BN* bn){
		
	while(bn->nbytes && !bn->data[bn->nbytes]) bn->nbytes--;
}

static char bnIsGE(const BN* a, const BN* b){	//is a >= b ?
	
	uint8_t an = a->nbytes, bn = b->nbytes;
	
	while(an > bn) if(a->data[an--]) return 1;
	while(an < bn) if(b->data[bn--]) return 0;
	
	//now an == bn
	do{
		
		if(a->data[an] > b->data[an]) return 1;
		else if(a->data[an] < b->data[an]) return 0;
	}while(an--);
	
	//equal
	return 1;
}

void bnSub(BN* a, const BN* b){	//a -= b. precondition: a >= b	[this is assumed and not checked]
	
	uint8_t i, o, c = 0;
	
	i = 0;
	do{
		o = a->data[i];
		a->data[i] -= c;
		c = 0;
		if(a->data[i] > o) c = 1;
		o = a->data[i];
		a->data[i] -= b->data[i];
		if(a->data[i] > o) c = 1;
	}while(i++ != a->nbytes);
}

char bnAdd(BN* d, const BN* a, const BN* b){
	
	uint8_t i, c = 0, o;
	
	if(a->nbytes < b->nbytes){	//make sure A is longer than B
		
		const BN* t;
		
		t = a;
		a = b;
		b = t;
	}
	
	d->nbytes = a->nbytes;
	if(a->data[a->nbytes] == 0xFF) d->nbytes++;
	
	d->data = _bnMalloc((uint16_t)d->nbytes + 1);
	if(!d->data) return 0;
	
	i = 0;
	do{
		o = a->data[i];
		a->data[i] -= c;
		c = 0;
		if(a->data[i] > o) c = 1;
		o = a->data[i];
		a->data[i] -= b->data[i];
		if(a->data[i] > o) c = 1;
		
	}while(i++ != b->nbytes);

	do{
	
		o = a->data[i];
		a->data[i] -= c;
		c = 0;
		if(a->data[i] > o) c = 1;
	
	}while(i++ != a->nbytes);
	if(c) a->data[i]++;

	return 1;
}


static void bnLsr1(BN* bn){
	
	uint8_t i = bn->nbytes;
	uint8_t t, top = 0;
	
	do{
		
		if(bn->data[i] & 1) t = 0x80;
		else t = 0;
		bn->data[i] = (bn->data[i] >> 1) | top;
		top = t;
	}while(i--);
}

static char bnLslMul8(BN* dst, uint8_t byteShift){
	
	uint8_t* newData;
	uint8_t i;
		
	newData = _bnRealloc(dst->data, dst->nbytes + 1 + byteShift);
	if(!newData) return 0;
	dst->data = newData;
	
	i = 0;
	do{
		newData[dst->nbytes + byteShift - i] = newData[dst->nbytes - i];
	}while(i++ != dst->nbytes);
	dst->nbytes += byteShift;
	while(byteShift--) *newData++ = 0;
		
	return 1;
}

static char bnMod(BN* top, BN* bot){
	
	if(bot->nbytes <= top->nbytes){
		
		uint16_t lsh = top->nbytes - bot->nbytes + 1;
		
		if(!bnLslMul8(bot, lsh)) return 0;
		lsh <<= 3;
		
		while(lsh--){
			
			bnLsr1(bot);
			if(bnIsGE(top, bot)) bnSub(top, bot);
		}
		
		bnNormalize(top);	//normalize the result
		bnNormalize(bot);	//fix any damade we did to "bot->nbytes"
	}
}

char bnMul(BN* d, BN* a, BN* b){	//numbers may be normalized but will otherwse be unmodified
	
	uint8_t i, j, c, o;
	uint16_t v16;
	uint8_t *buf, *dst;
	
	i = d->nbytes = a->nbytes + b->nbytes + 1;
	d->data = _bnMalloc((uint16_t)i + 1);
	if(!d->data) return 0;
	
	buf = _bnMalloc((uint16_t)b->nbytes + 2);
	if(!buf){
		_bnFree(d->data);
		return 0;
	}
	
	do{
		d->data[i] = 0;
		
	}while(i--);
	
	dst = d->data;
	j = 0;
	do{
	
		i = 0;
		buf[0] = 0;
		do{
		
			v16 = ((uint16_t)a->data[j] * (uint16_t)b->data[i]) + (uint16_t)buf[i];
			buf[i + 0] = v16;
			buf[i + 1] = v16 >> 8;
			
		}while(i++ != b->nbytes);
		
		c = 0;
		i = 0;
		do{
		
			o = dst[i];
			dst[i] += c;
			c = 0;
			if(o > dst[i]) c = 1;
			o = dst[i];
			dst[i] += buf[i];
			if(o > dst[i]) c = 1;
		
		}while(i++ != b->nbytes + 1);
		dst++;
	
	}while(j++ != a->nbytes);
	
	bnNormalize(d);
		
	return 1;
}

char bnExpMod(BN* dst, BN* a, BN* b, BN* n){	//a^b mod n		a and b are destroyed, n stays
	
	BN ret, t;
	
	bnMod(b, n);
	bnMod(a, n);
	if(!bnSet(&ret, 1)) return 0;
	
	while(bnIsNonzero(b)){
		
		if(b->data[0] & 1){
			
			if(!bnMul(&t, &ret, a)) return 0;
			bnFree(&ret);
			bnMod(&t, n);
			ret = t;
		}
		if(!bnMul(&t, a, a)) return 0;
		bnFree(a);
		bnMod(&t, n);
		*a = t;
		bnLsr1(b);
	}
	*dst = ret;
	
	return 1;
}



