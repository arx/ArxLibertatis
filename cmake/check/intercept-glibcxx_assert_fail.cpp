
#include <version>
#include <cstdlib>

_GLIBCXX_NORETURN void std::__glibcxx_assert_fail(const char * file, int line, const char * function,
                                                  const char * condition) _GLIBCXX_NOEXCEPT {
	(void)file;
	(void)line;
	(void)function;
	(void)condition;
	std::abort();
}

int main() {
	return 0;
}
