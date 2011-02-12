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

#ifndef __PKCOMP_H__
#define __PKCOMP_H__

#ifdef __cplusplus
extern "C" {
#endif

#define PK_LITERAL_SIZE_FIXED    0 // Use fixed size literal bytes, used for binary data
#define PK_LITERAL_SIZE_VARIABLE 1 // Use variable size literal bytes, used for text

// Error codes
#define PK_ERR_SUCCESS          0 // No errors occurred
#define PK_ERR_INVALID_DICTSIZE 1 // An invalid dictionary size was selected
#define PK_ERR_INVALID_MODE     2 // An invalid mode for literal bytes was selected
#define PK_ERR_BAD_DATA         3 // Input buffer contains invalid compressed data
#define PK_ERR_BUFFER_TOO_SMALL 4 // Output buffer is too small
#define PK_ERR_INCOMPLETE_INPUT 5 // Input buffer does not contain entire compressed data

struct pkstream {
	// The first six members of this struct need to be
	// set when calling pkimplode, but only the first
	// four members are used when calling pkexplode
	unsigned char *pInBuffer; // Pointer to input buffer
	unsigned int nInSize; // Size of input buffer
	unsigned char *pOutBuffer; // Pointer to output buffer
	unsigned int nOutSize; // Size of output buffer, will be resulting size when function returns
	unsigned char nLitSize; // Specifies whether to use fixed or variable size literal bytes
	unsigned char nDictSizeByte; // Dictionary size; valid values are 4, 5, and 6 which represent 1024, 2048, and 4096 respectively

	// The rest of the members of this struct are used
	// internally, so setting these values outside
	// pkimplode or pkexplode has no effect
	unsigned char *pInPos; // Current position in input buffer
	unsigned char *pOutPos; // Current position in output buffer
	unsigned char nBits; // Number of bits in bit buffer
	unsigned long nBitBuffer; // Stores bits until there are enough to output a byte of data
	unsigned char *pDictPos; // Position in dictionary
	unsigned int nDictSize; // Maximum size of dictionary
	unsigned int nCurDictSize; // Current size of dictionary
	unsigned char Dict[0x1000]; // Sliding dictionary used for compression and decompression
};

// Both functions return 0 on success and nonzero value on failure
int pkimplode(struct pkstream *pStr);
int pkexplode(struct pkstream *pStr);

#ifdef __cplusplus
};  // extern "C" 
#endif

#endif // __PKCOMP_H__

