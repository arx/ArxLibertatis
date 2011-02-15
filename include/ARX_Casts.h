#pragma once

#include <limits.h>

#define arx_assert(_Expression)						{}
#define ARX_LOG_INIT()								{}
#define ARX_LOG_CLEAN()								{}
#define ARX_LOG_TAG(_iId,_sTag)						{}

//ARX_BEGIN: xrichter (2010-06-23) - treat warnings C4244 for convertion problem
#define ARX_CLEAN_WARN_CAST_FLOAT(_x) (static_cast<float>( _x ))
#define ARX_CLEAN_WARN_CAST_UCHAR(_x) (static_cast<unsigned char>( _x ))
#define ARX_CLEAN_WARN_CAST_CHAR(_x) (static_cast<char>( _x ))
#define ARX_CLEAN_WARN_CAST_BYTE(_x) (static_cast<BYTE>( _x ))
#define ARX_CLEAN_WARN_CAST_SHORT(_x) (static_cast<short>( _x ))
#define ARX_CLEAN_WARN_CAST_USHORT(_x) (static_cast<unsigned short>( _x ))
#define ARX_CLEAN_WARN_CAST_WORD(_x)	(static_cast<WORD>( _x ))
#define ARX_CLEAN_WARN_CAST_INT(_x)	  (static_cast<int>( _x ))
#define ARX_CLEAN_WARN_CAST_UINT(_x)  (static_cast<unsigned int>( _x ))
#define ARX_CLEAN_WARN_CAST_SIZET(_x) (static_cast<size_t>( _x ))
#define ARX_CLEAN_WARN_CAST_LONG(_x)  (static_cast<long>( _x ))
#define ARX_CLEAN_WARN_CAST_ULONG(_x) (static_cast<unsigned long>( _x ))
#define ARX_CLEAN_WARN_CAST_DWORD(_x) (static_cast<DWORD>( _x ))
#define ARX_CLEAN_WARN_CAST_D3DVALUE(_x) (static_cast<D3DVALUE>( _x ))

//ARX_BEGIN: xrichter (2010-06-24) - Use to verify no overflow value by data type
#define ARX_CHECK_UCHAR(_x)	{arx_assert( static_cast<short>(_x) <= UCHAR_MAX		&& _x >= 0 ) ;}
#define ARX_CHECK_BYTE(_x)	ARX_CHECK_UCHAR(_x)
#define ARX_CHECK_CHAR(_x)	{arx_assert( static_cast<short>(_x) <= CHAR_MAX			&& static_cast<short>(_x) >= CHAR_MIN ) ;}
#define ARX_CHECK_SHORT(_x)	{arx_assert( static_cast<int>(_x) <= SHRT_MAX			&& static_cast<int>(_x) >= SHRT_MIN ) ;}
#define ARX_CHECK_USHORT(_x){arx_assert( static_cast<int>(_x) <= USHRT_MAX			&& _x >= 0 ) ;}
#define ARX_CHECK_WORD(_x)	ARX_CHECK_USHORT(_x)		//typedef unsigned short      WORD;
#define ARX_CHECK_INT(_x)	{arx_assert( static_cast<long long>(_x) <= INT_MAX		&& static_cast<long long>(_x) >= INT_MIN ) ;}
#define ARX_CHECK_UINT(_x)  {arx_assert( static_cast<long long>(_x) <= UINT_MAX		&& _x >= 0 ) ;}
#define ARX_CHECK_SIZET(_x) ARX_CHECK_UINT(_x)
#define ARX_CHECK_LONG(_x)	{arx_assert( static_cast<long long>(_x) <= LONG_MAX		&& static_cast<long long>(_x) >= LONG_MIN ) ;}
#define ARX_CHECK_ULONG(_x) {arx_assert( static_cast<long long>(_x) <= ULONG_MAX	&& _x >= 0	);}
#define ARX_CHECK_DWORD(_x) ARX_CHECK_ULONG(_x)
//ARX_END: xrichter (2010-06-24)

#define ARX_CHECK_NO_ENTRY( ) ( arx_assert( false ) )	//ARX: xrichter (2010-07-26) - Modified to use arx_assert
#define ARX_CHECK( _expr ) ( arx_assert( _expr ) )		//ARX: xrichter (2010-07-26) - Modified to use arx_assert
#define ARX_WARN( _str ) ( printf("WARN: %s=%s", #_str, ( _str ) ) )
#define ARX_WARN_F( _f ) ( printf("WARN: %s=%f", #_f, ( _f ) ) )
#define ARX_WARN_I( _i ) ( printf("WARN: %s=%d", #_i, ( _i ) ) )

# define ARX_OPAQUE_WHITE 0xFFFFFFFF

#define ARX_CHECK_NOT_NEG( _x )  ( arx_assert( ( _x ) >= 0 ) ) //ARX_BEGIN: jycorbel (2010-06-28) - Add description in assert

#define ARX_CAST_LONG( _x )	( static_cast<long>( _x ) )
#define ARX_CAST_ULONG( _x )( static_cast<unsigned long>( _x ) )
#define ARX_CAST_UINT( _x ) ( static_cast<unsigned int>( _x ) )
#define ARX_CAST_USHORT( _x ) ( static_cast<unsigned short>( _x ) )

#define ARX_DEAD_CODE() {arx_assert( false );} //ARX: xrichter (2010-06-23) - treat warnings C4700  Disable warning "local variable % used without having been initialized"
