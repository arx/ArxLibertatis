
#ifndef _LIBCPP_AVAILABILITY_CUSTOM_VERBOSE_ABORT_PROVIDED
#define _LIBCPP_AVAILABILITY_CUSTOM_VERBOSE_ABORT_PROVIDED
#endif

#include <version>
#include <cstdlib>

void std::__libcpp_verbose_abort(char const * format, ...) {
	(void)format;
	std::abort();
}

int main() {
	return 0;
}
