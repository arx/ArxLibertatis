#include <atomic>

int main() {
	std::atomic<int> test;
	test.fetch_add(1);
	return 0;
}
