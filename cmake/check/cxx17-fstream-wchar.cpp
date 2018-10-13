
#include <fstream>

int main() {
	const wchar_t * filename = L"test";
	
	std::fstream fs(filename);
	
	fs.open(filename);
	
	return 0;
}
