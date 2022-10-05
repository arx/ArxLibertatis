
struct A {
	A() noexcept(false) { }
};

struct B : A {
	B() noexcept = default;
};

int main() {
	static_assert(noexcept(B()), "missing support for P1286R2 aka DR1912");
}
