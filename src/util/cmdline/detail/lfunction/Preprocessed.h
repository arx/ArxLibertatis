//////////////////////////////////////////////////////////////////
// XXX.hpp header file
//
// Copyright 2010 - 2011. Alexey Tsoy.
// Distributed under the Boost Software License, Version 1.0. (See
// accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

#ifdef BOOST_PP_IS_ITERATING


#define N BOOST_PP_ITERATION()

#include "util/cmdline/detail/lfunction/LFNInvoker.h"
#include "util/cmdline/detail/lfunction/MakeLFunctionC.h"

#define FUNCTION_SPECIFICATION
#include "util/cmdline/detail/lfunction/MakeLFunctionOp.h"
#undef FUNCTION_SPECIFICATION

#define FUNCTION_SPECIFICATION const
#include "util/cmdline/detail/lfunction/MakeLFunctionOp.h"
#undef FUNCTION_SPECIFICATION


#define FUNCTION_SPECIFICATION volatile
#include "util/cmdline/detail/lfunction/MakeLFunctionOp.h"
#undef FUNCTION_SPECIFICATION


#define FUNCTION_SPECIFICATION const volatile
#include "util/cmdline/detail/lfunction/MakeLFunctionOp.h"
#undef FUNCTION_SPECIFICATION

#undef N

#endif //BOOST_PP_IS_ITERATING
