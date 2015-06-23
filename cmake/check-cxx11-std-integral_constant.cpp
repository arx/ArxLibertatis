
#include <type_traits>

int main() {
	return std::true_type::value + std::false_type::value
	       + std::integral_constant<int, 0>::value;
}
