
int main(int argc, const char * argv[]) {
	(void)argc;
	return *static_cast<const char *>(__builtin_assume_aligned(argv[0], 8));
}
