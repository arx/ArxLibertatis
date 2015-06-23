
struct my_struct {
	double d;
	char c;
};

int main() {
	return alignof(my_struct);
}
