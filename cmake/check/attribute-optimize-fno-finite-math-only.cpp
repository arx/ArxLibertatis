
#include <cmath>

__attribute__((optimize("-fno-finite-math-only")))
bool safe_isfinite(float f) {
	return (std::isfinite)(f);
}

int main(int argc, const char* argv[]) {
	(void)argv;
	return safe_isfinite(1.f / argc);
}
