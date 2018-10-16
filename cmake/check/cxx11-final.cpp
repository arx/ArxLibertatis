
struct my_interface {
	
	virtual int member() const = 0;
	
};

struct my_class final : public my_interface {
	
	int member() const { return 0; }
	
};

int main() {
	return my_class().member();
}
