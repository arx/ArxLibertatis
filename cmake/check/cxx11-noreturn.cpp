
#include <cstdlib>

[[noreturn]] static void func() { std::abort(); }

int main() {
	func();
}
