
#ifndef ARX_D3DWRAPPER_H
#define ARX_D3DWRAPPER_H

#define D3D_OVERLOADS

#include <cmath>
#include "core/Common.h"

#if (ARX_COMPILER == ARX_COMPILER_GCC) || (ARX_COMPILER == ARX_COMPILER_GCC64)
    #pragma GCC diagnostic ignored "-fpermissive"
#endif

#include <d3d.h>

#if (ARX_COMPILER == ARX_COMPILER_GCC) || (ARX_COMPILER == ARX_COMPILER_GCC64)
    #pragma GCC diagnostic error "-fpermissive"
#endif
    
#endif // ARX_D3DWRAPPER_H

