
#include <utility>

template <typename T>
T my_forward(T && var) {
	return std::forward<T>(var);
}

int main() {
	return my_forward(0);
}
