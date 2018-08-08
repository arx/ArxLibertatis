
int my_function() {
	return 0;
}

template <typename Arg0, typename... Args>
int my_function(Arg0 arg0, Args... args) {
	(void)arg0;
	return my_function(args...);
}


template <typename... Args>
struct my_struct {
	
	my_struct(Args... args) : value(my_function(args...)) { }
	
	int value;
	
};

int main() {
	return my_struct<int, char, const char *>(0, '1', "2").value;
}
