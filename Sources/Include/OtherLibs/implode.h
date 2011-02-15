/***************************************************************
 PKWARE Data Compression Library (R) for Win32
 Copyright 1991,1992,1994,1995 PKWARE Inc.  All Rights Reserved.
 PKWARE Data Compression Library Reg. U.S. Pat. and Tm. Off.
***************************************************************/


#ifndef IMPLODE_H
#define IMPLODE_H

#ifdef __cplusplus
extern "C" {
#endif


// TODO find replacement library
inline	unsigned int implode(
	    unsigned int (*read_buf)(char * buf, unsigned int * size, void * param),
	    void (*write_buf)(char * buf, unsigned int * size, void * param),
	    char     *    work_buf,
	    void     *    param,
	    unsigned int * type,

	    unsigned int * dsize) {
	
	return 0;
}


inline	unsigned int explode(
	    unsigned int (*read_buf)(char * buf, unsigned  int * size, void * param),
	    void (*write_buf)(char * buf, unsigned  int * size, void * param),
	    char     *    work_buf,
	    void     *    param) {
	return 0;
}

	unsigned long crc32(char * buffer, unsigned int * size, unsigned long * old_crc);

#ifdef __cplusplus
}                         // End of 'extern "C"' declaration
#endif


#define CMP_BUFFER_SIZE    36312
#define EXP_BUFFER_SIZE    12596

#define CMP_BINARY             0
#define CMP_ASCII              1

#define CMP_NO_ERROR           0
#define CMP_INVALID_DICTSIZE   1
#define CMP_INVALID_MODE       2
#define CMP_BAD_DATA           3
#define CMP_ABORT              4

#endif // IMPLODE_H
