/*****************************************************************************/
/* pkcomp.h                                   Copyright (c) ShadowFlare 2003 */
/*---------------------------------------------------------------------------*/
/* Header file for implode and explode functions                             */
/*                                                                           */
/* Author: ShadowFlare (blakflare@hotmail.com)                               */
/*                                                                           */
/*---------------------------------------------------------------------------*/
/*   Date    Ver   Comment                                                   */
/* --------  ----  -------                                                   */
/* 10/24/03  1.01  Added checks for when the end of a buffer is reached      */
/*                 Extended error codes added                                */
/* 07/01/03  1.00  First version                                             */
/*---------------------------------------------------------------------------*/
/*                                                                           */
/* Note:  The code for the implode function is getting very close to         */
/* being able to compress as well as PKWARE Data Compression library,        */
/* but it is currently somewhat slow.                                        */
/*****************************************************************************/

#ifndef ARX_IO_IMPLODE_H
#define ARX_IO_IMPLODE_H

#include <stddef.h>

enum ImplodeLiteralSize {
	PK_LITERAL_SIZE_FIXED = 0, // Use fixed size literal bytes, used for binary data
	PK_LITERAL_SIZE_VARIABLE  = 1 // Use variable size literal bytes, used for text
};

enum ImplodeResult {
	PK_ERR_SUCCESS = 0, // No errors occurred
	PK_ERR_INVALID_DICTSIZE = 1, // An invalid dictionary size was selected
	PK_ERR_INVALID_MODE = 2, // An invalid mode for literal bytes was selected
	PK_ERR_BUFFER_TOO_SMALL = 4 // Output buffer is too small
};

struct pkstream {
	
	// The first six members of this struct need to be
	// set when calling pkimplode.
	const unsigned char * pInBuffer; // Pointer to input buffer
	size_t nInSize; // Size of input buffer
	unsigned char * pOutBuffer; // Pointer to output buffer
	size_t nOutSize; // Size of output buffer, will be resulting size when function returns
	ImplodeLiteralSize nLitSize; // Specifies whether to use fixed or variable size literal bytes
	unsigned char nDictSizeByte; // Dictionary size; valid values are 4, 5, and 6 which represent 1024, 2048, and 4096 respectively
	
	// The rest of the members of this struct are used
	// internally, so setting these values outside
	// pkimplode has no effect
	const unsigned char * pInPos; // Current position in input buffer
	unsigned char * pOutPos; // Current position in output buffer
	unsigned char nBits; // Number of bits in bit buffer
	unsigned long nBitBuffer; // Stores bits until there are enough to output a byte of data
	unsigned char * pDictPos; // Position in dictionary
	unsigned int nDictSize; // Maximum size of dictionary
	unsigned int nCurDictSize; // Current size of dictionary
	unsigned char Dict[0x1000]; // Sliding dictionary used for compression and decompression
	
};

/**
 * @return 0 on success and nonzero value on failure
 */
ImplodeResult implode(pkstream * pStr);

char * implodeAlloc(const char * buf, size_t inSize, size_t & outSize);

#endif // ARX_IO_IMPLODE_H
