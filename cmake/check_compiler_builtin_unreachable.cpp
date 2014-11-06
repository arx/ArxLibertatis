
int main(int argc, const char * argv[]) {
	if(argc < 1) {
		__builtin_unreachable();
	}
	return *argv[0];
}
