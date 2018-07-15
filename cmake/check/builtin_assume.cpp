
int main(int argc, const char * argv[]) {
	__builtin_assume(argc >= 1);
	return *argv[0];
}
