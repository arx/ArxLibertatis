
int main(int argc, const char * argv[]) {
	char buffer[512];
	__builtin_ia32_fxsave(buffer);
	return *argv[0] | buffer[0];
}
