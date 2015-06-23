
struct my_struct {
	double d;
	char c;
};

int main() {
	return __alignof__(my_struct);
}
