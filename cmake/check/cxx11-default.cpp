
struct my_struct {
	
	my_struct(int value_) : value(value_) { }
	
	my_struct() = default;
	~my_struct() = default;
	
	my_struct(const my_struct & o) = default;
	
	my_struct & operator=(const my_struct & o) = default;
	
	int value;
	
};

int main() {
	
	my_struct a(0);
	my_struct b(a);
	a = b;
	
	return a.value;
}
