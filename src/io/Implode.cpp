/*
 * Copyright 2011-2013 Arx Libertatis Team (see the AUTHORS file)
 *
 * This file is part of Arx Libertatis.
 *
 * Arx Libertatis is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * Arx Libertatis is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with Arx Libertatis.  If not, see <http://www.gnu.org/licenses/>.
 */
// Based on:
/*****************************************************************************/
/* pkimplode.c                                Copyright (c) ShadowFlare 2003 */
/*---------------------------------------------------------------------------*/
/* Implode function which creates compressed data compatible with PKWARE     */
/* Data Compression library                                                  */
/*                                                                           */
/* Author: ShadowFlare (blakflare@hotmail.com)                               */
/*                                                                           */
/* This code was created from a format specification that was posted on      */
/* a newsgroup.  No reverse-engineering of any kind was performed by         */
/* me to produce this code.                                                  */
/*                                                                           */
/* This code is free and you may perform any modifications to it that        */
/* you wish to perform, but please leave my name in the file as the          */
/* original author of the code.                                              */
/*                                                                           */
/*---------------------------------------------------------------------------*/
/*   Date    Ver   Comment                                                   */
/* --------  ----  -------                                                   */
/* 10/24/03  1.01  Added checks for when the end of a buffer is reached      */
/*                 Extended error codes added                                */
/* 07/01/03  1.00  First version                                             */
/*---------------------------------------------------------------------------*/
/*                                                                           */
/* Note:  This code is getting very close to being able to compress as       */
/* well as PKWARE Data Compression library, but it is currently somewhat     */
/* slow.                                                                     */
/*****************************************************************************/

#include "io/Implode.h"

#if BUILD_EDIT_LOADSAVE

#include <cstring>

#include "io/log/Logger.h"

// Truncate value to a specified number of bits
#define TRUNCATE_VALUE(value,bits) ((value) & ((1 << (bits)) - 1))

// Bit sequences used to represent literal bytes
static unsigned short ChCode[] = {
	0x0490, 0x0FE0, 0x07E0, 0x0BE0, 0x03E0, 0x0DE0, 0x05E0, 0x09E0, 
	0x01E0, 0x00B8, 0x0062, 0x0EE0, 0x06E0, 0x0022, 0x0AE0, 0x02E0, 
	0x0CE0, 0x04E0, 0x08E0, 0x00E0, 0x0F60, 0x0760, 0x0B60, 0x0360, 
	0x0D60, 0x0560, 0x1240, 0x0960, 0x0160, 0x0E60, 0x0660, 0x0A60, 
	0x000F, 0x0250, 0x0038, 0x0260, 0x0050, 0x0C60, 0x0390, 0x00D8, 
	0x0042, 0x0002, 0x0058, 0x01B0, 0x007C, 0x0029, 0x003C, 0x0098, 
	0x005C, 0x0009, 0x001C, 0x006C, 0x002C, 0x004C, 0x0018, 0x000C, 
	0x0074, 0x00E8, 0x0068, 0x0460, 0x0090, 0x0034, 0x00B0, 0x0710, 
	0x0860, 0x0031, 0x0054, 0x0011, 0x0021, 0x0017, 0x0014, 0x00A8, 
	0x0028, 0x0001, 0x0310, 0x0130, 0x003E, 0x0064, 0x001E, 0x002E, 
	0x0024, 0x0510, 0x000E, 0x0036, 0x0016, 0x0044, 0x0030, 0x00C8, 
	0x01D0, 0x00D0, 0x0110, 0x0048, 0x0610, 0x0150, 0x0060, 0x0088, 
	0x0FA0, 0x0007, 0x0026, 0x0006, 0x003A, 0x001B, 0x001A, 0x002A, 
	0x000A, 0x000B, 0x0210, 0x0004, 0x0013, 0x0032, 0x0003, 0x001D, 
	0x0012, 0x0190, 0x000D, 0x0015, 0x0005, 0x0019, 0x0008, 0x0078, 
	0x00F0, 0x0070, 0x0290, 0x0410, 0x0010, 0x07A0, 0x0BA0, 0x03A0, 
	0x0240, 0x1C40, 0x0C40, 0x1440, 0x0440, 0x1840, 0x0840, 0x1040, 
	0x0040, 0x1F80, 0x0F80, 0x1780, 0x0780, 0x1B80, 0x0B80, 0x1380, 
	0x0380, 0x1D80, 0x0D80, 0x1580, 0x0580, 0x1980, 0x0980, 0x1180, 
	0x0180, 0x1E80, 0x0E80, 0x1680, 0x0680, 0x1A80, 0x0A80, 0x1280, 
	0x0280, 0x1C80, 0x0C80, 0x1480, 0x0480, 0x1880, 0x0880, 0x1080, 
	0x0080, 0x1F00, 0x0F00, 0x1700, 0x0700, 0x1B00, 0x0B00, 0x1300, 
	0x0DA0, 0x05A0, 0x09A0, 0x01A0, 0x0EA0, 0x06A0, 0x0AA0, 0x02A0, 
	0x0CA0, 0x04A0, 0x08A0, 0x00A0, 0x0F20, 0x0720, 0x0B20, 0x0320, 
	0x0D20, 0x0520, 0x0920, 0x0120, 0x0E20, 0x0620, 0x0A20, 0x0220, 
	0x0C20, 0x0420, 0x0820, 0x0020, 0x0FC0, 0x07C0, 0x0BC0, 0x03C0, 
	0x0DC0, 0x05C0, 0x09C0, 0x01C0, 0x0EC0, 0x06C0, 0x0AC0, 0x02C0, 
	0x0CC0, 0x04C0, 0x08C0, 0x00C0, 0x0F40, 0x0740, 0x0B40, 0x0340, 
	0x0300, 0x0D40, 0x1D00, 0x0D00, 0x1500, 0x0540, 0x0500, 0x1900, 
	0x0900, 0x0940, 0x1100, 0x0100, 0x1E00, 0x0E00, 0x0140, 0x1600, 
	0x0600, 0x1A00, 0x0E40, 0x0640, 0x0A40, 0x0A00, 0x1200, 0x0200, 
	0x1C00, 0x0C00, 0x1400, 0x0400, 0x1800, 0x0800, 0x1000, 0x0000
};

// Lengths of bit sequences used to represent literal bytes
static unsigned char ChBits[] = {
	0x0B, 0x0C, 0x0C, 0x0C, 0x0C, 0x0C, 0x0C, 0x0C, 0x0C, 0x08, 0x07, 0x0C, 0x0C, 0x07, 0x0C, 0x0C, 
	0x0C, 0x0C, 0x0C, 0x0C, 0x0C, 0x0C, 0x0C, 0x0C, 0x0C, 0x0C, 0x0D, 0x0C, 0x0C, 0x0C, 0x0C, 0x0C, 
	0x04, 0x0A, 0x08, 0x0C, 0x0A, 0x0C, 0x0A, 0x08, 0x07, 0x07, 0x08, 0x09, 0x07, 0x06, 0x07, 0x08, 
	0x07, 0x06, 0x07, 0x07, 0x07, 0x07, 0x08, 0x07, 0x07, 0x08, 0x08, 0x0C, 0x0B, 0x07, 0x09, 0x0B, 
	0x0C, 0x06, 0x07, 0x06, 0x06, 0x05, 0x07, 0x08, 0x08, 0x06, 0x0B, 0x09, 0x06, 0x07, 0x06, 0x06, 
	0x07, 0x0B, 0x06, 0x06, 0x06, 0x07, 0x09, 0x08, 0x09, 0x09, 0x0B, 0x08, 0x0B, 0x09, 0x0C, 0x08, 
	0x0C, 0x05, 0x06, 0x06, 0x06, 0x05, 0x06, 0x06, 0x06, 0x05, 0x0B, 0x07, 0x05, 0x06, 0x05, 0x05, 
	0x06, 0x0A, 0x05, 0x05, 0x05, 0x05, 0x08, 0x07, 0x08, 0x08, 0x0A, 0x0B, 0x0B, 0x0C, 0x0C, 0x0C, 
	0x0D, 0x0D, 0x0D, 0x0D, 0x0D, 0x0D, 0x0D, 0x0D, 0x0D, 0x0D, 0x0D, 0x0D, 0x0D, 0x0D, 0x0D, 0x0D, 
	0x0D, 0x0D, 0x0D, 0x0D, 0x0D, 0x0D, 0x0D, 0x0D, 0x0D, 0x0D, 0x0D, 0x0D, 0x0D, 0x0D, 0x0D, 0x0D, 
	0x0D, 0x0D, 0x0D, 0x0D, 0x0D, 0x0D, 0x0D, 0x0D, 0x0D, 0x0D, 0x0D, 0x0D, 0x0D, 0x0D, 0x0D, 0x0D, 
	0x0C, 0x0C, 0x0C, 0x0C, 0x0C, 0x0C, 0x0C, 0x0C, 0x0C, 0x0C, 0x0C, 0x0C, 0x0C, 0x0C, 0x0C, 0x0C, 
	0x0C, 0x0C, 0x0C, 0x0C, 0x0C, 0x0C, 0x0C, 0x0C, 0x0C, 0x0C, 0x0C, 0x0C, 0x0C, 0x0C, 0x0C, 0x0C, 
	0x0C, 0x0C, 0x0C, 0x0C, 0x0C, 0x0C, 0x0C, 0x0C, 0x0C, 0x0C, 0x0C, 0x0C, 0x0C, 0x0C, 0x0C, 0x0C, 
	0x0D, 0x0C, 0x0D, 0x0D, 0x0D, 0x0C, 0x0D, 0x0D, 0x0D, 0x0C, 0x0D, 0x0D, 0x0D, 0x0D, 0x0C, 0x0D, 
	0x0D, 0x0D, 0x0C, 0x0C, 0x0C, 0x0D, 0x0D, 0x0D, 0x0D, 0x0D, 0x0D, 0x0D, 0x0D, 0x0D, 0x0D, 0x0D
};

// Bit sequences used to represent the base values of the copy length
static unsigned char LenCode[] = {
	0x05, 0x03, 0x01, 0x06, 0x0A, 0x02, 0x0C, 0x14, 0x04, 0x18, 0x08, 0x30, 0x10, 0x20, 0x40, 0x00
};

// Lengths of bit sequences used to represent the base values of the copy length
static unsigned char LenBits[] = {
	0x03, 0x02, 0x03, 0x03, 0x04, 0x04, 0x04, 0x05, 0x05, 0x05, 0x05, 0x06, 0x06, 0x06, 0x07, 0x07
};

// Base values used for the copy length
static unsigned short LenBase[] = {
	0x0002, 0x0003, 0x0004, 0x0005, 0x0006, 0x0007, 0x0008, 0x0009, 
	0x000A, 0x000C, 0x0010, 0x0018, 0x0028, 0x0048, 0x0088, 0x0108
};

// Lengths of extra bits used to represent the copy length
static unsigned char ExLenBits[] = {
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08
};

// Bit sequences used to represent the most significant 6 bits of the copy offset
static unsigned char OffsCode[] = {
	0x03, 0x0D, 0x05, 0x19, 0x09, 0x11, 0x01, 0x3E, 0x1E, 0x2E, 0x0E, 0x36, 0x16, 0x26, 0x06, 0x3A, 
	0x1A, 0x2A, 0x0A, 0x32, 0x12, 0x22, 0x42, 0x02, 0x7C, 0x3C, 0x5C, 0x1C, 0x6C, 0x2C, 0x4C, 0x0C, 
	0x74, 0x34, 0x54, 0x14, 0x64, 0x24, 0x44, 0x04, 0x78, 0x38, 0x58, 0x18, 0x68, 0x28, 0x48, 0x08, 
	0xF0, 0x70, 0xB0, 0x30, 0xD0, 0x50, 0x90, 0x10, 0xE0, 0x60, 0xA0, 0x20, 0xC0, 0x40, 0x80, 0x00
};

// Lengths of bit sequences used to represent the most significant 6 bits of the copy offset
static unsigned char OffsBits[] = {
	0x02, 0x04, 0x04, 0x05, 0x05, 0x05, 0x05, 0x06, 0x06, 0x06, 0x06, 0x06, 0x06, 0x06, 0x06, 0x06, 
	0x06, 0x06, 0x06, 0x06, 0x06, 0x06, 0x07, 0x07, 0x07, 0x07, 0x07, 0x07, 0x07, 0x07, 0x07, 0x07, 
	0x07, 0x07, 0x07, 0x07, 0x07, 0x07, 0x07, 0x07, 0x07, 0x07, 0x07, 0x07, 0x07, 0x07, 0x07, 0x07, 
	0x08, 0x08, 0x08, 0x08, 0x08, 0x08, 0x08, 0x08, 0x08, 0x08, 0x08, 0x08, 0x08, 0x08, 0x08, 0x08
};

ImplodeResult implode(pkstream * pStr) {
	
	unsigned char ch; // Byte from input buffer
	int nMaxCopyLen; // Length of longest duplicate data in the dictionary
	unsigned char * pMaxCopyOffs; // Offset to longest duplicate data in the dictionary
	int nCopyOffs = 0; // Offset used in actual compressed data
	unsigned char * pCopyOffs; // Offset to duplicate data in the dictionary
	unsigned char * pOldCopyOffs; // Temporarily holds previous value of pCopyOffs
	const unsigned char * pNewInPos; // Secondary offset into input buffer
	unsigned char * pNewDictPos; // Secondary offset into dictionary
	int i; // Index into tables
	
	// Initialize buffer positions
	pStr->pInPos = pStr->pInBuffer;
	pStr->pOutPos = pStr->pOutBuffer;
	
	// Check for a valid compression type
	if(pStr->nLitSize != IMPLODE_LITERAL_FIXED && pStr->nLitSize != IMPLODE_LITERAL_VARIABLE) {
		return IMPLODE_INVALID_MODE;
	}
	
	// Only dictionary sizes of 1024, 2048, and 4096 are allowed.
	// The values 4, 5, and 6 correspond with those sizes
	if(4 > pStr->nDictSizeByte || pStr->nDictSizeByte > 6) {
		return IMPLODE_INVALID_DICTSIZE;
	}
	
	// Store actual dictionary size
	pStr->nDictSize = 64 << pStr->nDictSizeByte;
	
	// Initialize dictionary position
	pStr->pDictPos = pStr->Dict;
	
	// Initialize current dictionary size to zero
	pStr->nCurDictSize = 0;
	
	// If the output buffer size is less than 4, there
	// is not enough room for the compressed data
	if(pStr->nOutSize < 4 && !(pStr->nInSize == 0 && pStr->nOutSize == 4)) {
		return IMPLODE_BUFFER_TOO_SMALL;
	}
	
	// Store compression type and dictionary size
	*pStr->pOutPos++ = pStr->nLitSize;
	*pStr->pOutPos++ = pStr->nDictSizeByte;
	
	// Initialize bit buffer
	pStr->nBitBuffer = 0;
	pStr->nBits = 0;
	
	// Compress until input buffer is empty
	while(pStr->pInPos < pStr->pInBuffer + pStr->nInSize) {
		
		// Get a byte from the input buffer
		ch = *pStr->pInPos++;
		nMaxCopyLen = 0;
		
		// If the dictionary is not empty, search for duplicate data in the dictionary
		if(pStr->nCurDictSize > 0 && pStr->nInSize - (pStr->pInPos - pStr->pInBuffer) > 0) {
			
			// Initialize offsets and lengths used in search
			pCopyOffs = pStr->Dict;
			pMaxCopyOffs = pCopyOffs;
			nMaxCopyLen = 0;
			
			// Store position of last written dictionary byte
			pNewDictPos = pStr->pDictPos - 1;
			if(pNewDictPos < pStr->Dict) {
				pNewDictPos = pStr->Dict + pStr->nCurDictSize - 1;
			}
			
			// Search dictionary for duplicate data
			while(pCopyOffs < pStr->Dict + pStr->nCurDictSize) {
				
				// Check for a match with first byte
				if(ch == *pCopyOffs) {
					pOldCopyOffs = pCopyOffs;
					int nCopyLen = 0;
					pNewInPos = pStr->pInPos - 1;
					
					// If there was a match, check for additional duplicate bytes
					do {
						
						// Increment pointers and length
						nCopyLen++;
						pNewInPos++;
						pCopyOffs++;
						
						// Wrap around pointer to beginning of dictionary buffer if the end of the buffer was reached
						if(pCopyOffs >= pStr->Dict + pStr->nDictSize) {
							pCopyOffs = pStr->Dict;
						}
						
						// Wrap dictionary bytes if end of the dictionary was reached
						if(pCopyOffs == pStr->pDictPos) {
							pCopyOffs = pOldCopyOffs;
						}
						
						// Stop checking for additional bytes if there is no more input or maximum length was reached
						if(nCopyLen >= 518 || pStr->nInSize - (pNewInPos - pStr->pInBuffer) == 0) {
							break;
						}
					} while (*pNewInPos == *pCopyOffs);
					
					// Return the pointer to the beginning of the matching data
					pCopyOffs = pOldCopyOffs;
					
					// Copying less than two bytes from dictionary wastes space, so don't do it ;)
					if(nCopyLen >= 2) {
						
						// Only use the most efficient length and offset in dictionary
						if(nCopyLen > nMaxCopyLen) {
							
							// Store the offset that will be outputted into the compressed data
							nCopyOffs = (pNewDictPos - (pCopyOffs - pStr->nCurDictSize)) % pStr->nCurDictSize;
							
							if(nCopyLen > 2) {
								pMaxCopyOffs = pCopyOffs;
								nMaxCopyLen = nCopyLen;
							} else {
								
								// If the copy length is 2, check for a valid dictionary offset
								if(nCopyOffs <= 255) {
									pMaxCopyOffs = pCopyOffs;
									nMaxCopyLen = nCopyLen;
								}
							}
						}
						
						// If the length is equal, check for a more efficient offset
						else if(nCopyLen == nMaxCopyLen) {
							if((pNewDictPos - (pCopyOffs - pStr->nCurDictSize)) % pStr->nCurDictSize < (pNewDictPos - (pMaxCopyOffs - pStr->nCurDictSize)) % pStr->nCurDictSize) {
								nCopyOffs = (pNewDictPos - (pCopyOffs - pStr->nCurDictSize)) % pStr->nCurDictSize;
								pMaxCopyOffs = pCopyOffs;
								nMaxCopyLen = nCopyLen;
							}
						}
					}
				}
				
				// Increment pointer to the next dictionary offset to check
				pCopyOffs++;
			}
			
			// If there were at least 2 matching bytes in the dictionary that were found, output the length/offset pair
			if(nMaxCopyLen >= 2) {
				
				// Reset the input pointers to the bytes that will be added to the dictionary
				pStr->pInPos--;
				pNewInPos = pStr->pInPos + nMaxCopyLen;
				
				while(pStr->pInPos < pNewInPos) {
					
					// Add a byte to the dictionary
					*pStr->pDictPos++ = ch;
					
					// If the dictionary is not full yet, increment the current dictionary size
					if(pStr->nCurDictSize < pStr->nDictSize)
						pStr->nCurDictSize++;
					
					// If the current end of the dictionary is past the end of the buffer,
					// wrap around back to the start
					if(pStr->pDictPos >= pStr->Dict + pStr->nDictSize)
						pStr->pDictPos = pStr->Dict;
					
					// Get the next byte to be added
					if(++pStr->pInPos < pNewInPos)
						ch = *pStr->pInPos;
				}
				
				// Find bit code for the base value of the length from the table
				for(i = 0; i < 0x0F; i++) {
					if(LenBase[i] <= nMaxCopyLen && nMaxCopyLen < LenBase[i+1]) {
						break;
					}
				}
				
				// Store the base value of the length
				pStr->nBitBuffer += (1 + (LenCode[i] << 1)) << pStr->nBits;
				pStr->nBits += 1 + LenBits[i];
				
				// Store the extra bits for the length
				pStr->nBitBuffer += (nMaxCopyLen - LenBase[i]) << pStr->nBits;
				pStr->nBits += ExLenBits[i];
				
				// Output the data from the bit buffer
				while(pStr->nBits >= 8) {
					// If output buffer has become full, stop immediately!
					if(pStr->pOutPos >= pStr->pOutBuffer + pStr->nOutSize) {
						return IMPLODE_BUFFER_TOO_SMALL;
					}
					
					*pStr->pOutPos++ = (unsigned char)pStr->nBitBuffer;
					pStr->nBitBuffer >>= 8;
					pStr->nBits -= 8;
				}
				
				// The most significant 6 bits of the dictionary offset are encoded with a
				// bit sequence then the first 2 after that if the copy length is 2,
				// otherwise it is the first 4, 5, or 6 (based on the dictionary size)
				if(nMaxCopyLen == 2) {
					
					// Store most significant 6 bits of offset using bit sequence
					pStr->nBitBuffer += OffsCode[nCopyOffs >> 2] << pStr->nBits;
					pStr->nBits += OffsBits[nCopyOffs >> 2];
					
					// Store the first 2 bits
					pStr->nBitBuffer += (nCopyOffs & 0x03) << pStr->nBits;
					pStr->nBits += 2;
					
				} else {
					
					// Store most significant 6 bits of offset using bit sequence
					pStr->nBitBuffer += OffsCode[nCopyOffs >> pStr->nDictSizeByte] << pStr->nBits;
					pStr->nBits += OffsBits[nCopyOffs >> pStr->nDictSizeByte];
					
					// Store the first 4, 5, or 6 bits
					pStr->nBitBuffer += TRUNCATE_VALUE(nCopyOffs,pStr->nDictSizeByte) << pStr->nBits;
					pStr->nBits += pStr->nDictSizeByte;
				}
			}
		}
		
		// If the copy length was less than two, include the byte as a literal byte
		if(nMaxCopyLen < 2) {
			if(pStr->nLitSize == IMPLODE_LITERAL_FIXED) {
				
				// Store a fixed size literal byte
				pStr->nBitBuffer += ch << (pStr->nBits + 1);
				pStr->nBits += 9;
			} else {
				
				// Store a variable size literal byte
				pStr->nBitBuffer += ChCode[ch] << (pStr->nBits + 1);
				pStr->nBits += 1 + ChBits[ch];
			}
			
			// Add the byte into the dictionary
			*pStr->pDictPos++ = ch;
			
			// If the dictionary is not full yet, increment the current dictionary size
			if(pStr->nCurDictSize < pStr->nDictSize) {
				pStr->nCurDictSize++;
			}
			
			// If the current end of the dictionary is past the end of the buffer,
			// wrap around back to the start
			if(pStr->pDictPos >= pStr->Dict + pStr->nDictSize) {
				pStr->pDictPos = pStr->Dict;
			}
		}
		
		// Write any whole bytes from the bit buffer into the output buffer
		while(pStr->nBits >= 8) {
			// If output buffer has become full, stop immediately!
			if(pStr->pOutPos >= pStr->pOutBuffer + pStr->nOutSize) {
				return IMPLODE_BUFFER_TOO_SMALL;
			}
			
			*pStr->pOutPos++ = (unsigned char)pStr->nBitBuffer;
			pStr->nBitBuffer >>= 8;
			pStr->nBits -= 8;
		}
	}
	
	// Store the code for the end of the compressed data stream
	pStr->nBitBuffer += (1 + (LenCode[0x0F] << 1)) << pStr->nBits;
	pStr->nBits += 1 + LenBits[0x0F];
	
	pStr->nBitBuffer += 0xFF << pStr->nBits;
	pStr->nBits += 8;
	
	// Write any remaining bits from the bit buffer into the output buffer
	while(pStr->nBits > 0) {
		// If output buffer has become full, stop immediately!
		if(pStr->pOutPos >= pStr->pOutBuffer + pStr->nOutSize) {
			return IMPLODE_BUFFER_TOO_SMALL;
		}
			
		*pStr->pOutPos++ = (unsigned char)pStr->nBitBuffer;
		pStr->nBitBuffer >>= 8;
		if(pStr->nBits >= 8) {
			pStr->nBits -= 8;
		} else {
			pStr->nBits = 0;
		}
	}
	
	// Store the compressed size
	pStr->nOutSize = pStr->pOutPos - pStr->pOutBuffer;
	
	return IMPLODE_SUCCESS;
}

char * implodeAlloc(const char * buf, size_t inSize, size_t & outSize) {
	
	pkstream strm;
	
	strm.pInBuffer = (const unsigned char*)buf;
	strm.nInSize = inSize;
	strm.nLitSize = IMPLODE_LITERAL_FIXED;
	
	if(inSize <= 32768) {
		strm.nDictSizeByte = 4;
	} else if(inSize <= 131072) {
		strm.nDictSizeByte = 5;
	} else {
		strm.nDictSizeByte = 6;
	}
	
	// TODO what is the maximum size that the data can grow?
	char * outBuf = new char[inSize * 2];
	
	strm.pOutBuffer = (unsigned char *)outBuf;
	strm.nOutSize = inSize * 2;
	
	LogWarning << "Very slow implode " << inSize << " " << strm.nOutSize;
	
	ImplodeResult res = implode(&strm);
	if(res) {
		LogError << "Error compressing " << inSize << " bytes: " << res;
		outSize = 0;
		delete[] outBuf;
		return NULL;
	}
	
	memcpy(outBuf, buf, inSize);
	strm.nOutSize = inSize;
	
	LogDebug(" Compressed to " << strm.nOutSize << " " << (((float)strm.nOutSize)/inSize));
	
	outSize = strm.nOutSize;
	return outBuf;
}

#endif // BUILD_EDIT_LOADSAVE
