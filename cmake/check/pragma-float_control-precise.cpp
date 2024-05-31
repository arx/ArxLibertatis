
#include <cmath>

#pragma float_control(precise, on, push)

bool safe_isfinite(float f) {
	return (std::isfinite)(f);
}

#pragma float_control(pop)

int main(int argc, const char* argv[]) {
	(void)argv;
	return safe_isfinite(1.f / argc);
}
