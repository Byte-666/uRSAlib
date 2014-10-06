/* 
 * pgpRSAGlue.h - RSA encryption and decryption
 *
 * This is a PRIVATE header file, for use only within the PGP Library.
 * You should not be using these functions in an application.
 *
 * $Id: pgpRSAGlue.h,v 1.5 2003/12/02 22:50:01 ajivsov Exp $
 */
#ifndef Included_pgpRSAGlue_h
#define Included_pgpRSAGlue_h


#include "pgpSDKBuildFlags.h"

#ifndef PGP_RSA
#error "PGP_RSA requires a value"
#endif

#if PGP_RSA


#include "bn.h"
#include "pgpUsuals.h"
#include "pgpOpaqueStructs.h"

PGP_BEGIN_C_DECLARATIONS

/* A structure to hold a public key */
typedef struct RSApub
{
	BigNum n;	/* The public modulus */
	BigNum e;	/* The public exponent */
} RSApub;

/* A structure to hold a secret key */
typedef struct RSAsec
{
	BigNum n;	/* Copy of public parameters */
	BigNum e;
	BigNum d;	/* Decryption exponent */
	BigNum p;	/* The smaller factor of n */
	BigNum q;	/* The larger factor of n */
	BigNum u;	/* 1/p (mod q) */
} RSAsec ;

#define MAX_RSA_MOD_BITS 8192 /* is stronger than 192-bit symmetrical key in 2003 */

/* Declarations */
int rsaKeyTooBig(RSApub const *pub, RSAsec const *sec);

/* Encrypt */
int rsaPublicEncrypt(BigNum *bn, PGPByte const *in, unsigned len,
                     RSApub const *pub,
		     PGPRandomContext const *rc, PGPByte padtype);
/* Sign */
int rsaPrivateEncrypt(BigNum *bn, PGPByte const *in, unsigned len,
                      RSAsec const *sec);
/* Verify */
int rsaPublicDecrypt(PGPByte *buf, unsigned len, BigNum *bn,
                     RSApub const *pub);
/* Decrypt */
int rsaPrivateDecrypt(PGPByte *buf, unsigned len, BigNum *bn,
                      RSAsec const *sec, PGPByte padtype);

/* Slightly higher level functions for CAPI */
#if PGP_USECAPIFORRSA
PGPBoolean
rsaVerifyHashSignature(BigNum *bn, RSApub const *pub, PGPHashVTBL const *h,
	PGPByte const *hash);

int
rsaSignHash(BigNum *bn, RSAsec const *sec, PGPHashVTBL const *h,
	PGPByte const *hash);

#endif /* PGP_USECAPIFORRSA */


PGP_END_C_DECLARATIONS

#endif /* PGP_RSA */

#endif /* !Included_pgpRSAGlue_h */
