
extern "C" void add_one() __attribute__(( __import_module__("env"),__import_name__("add_one")));
extern "C" void chain_call(int, const char*) __attribute__((__import_module__("env"), __import_name__("chain_call")));
extern "C" int get(int) __attribute__((__import_module__("env"), __import_name__("get")));

extern "C"{

	int* fib(int x) __attribute__((used));
	int* fib(int x){
        if(x == 0 || x == 1){
		int temp = 10;
		return &temp;
	}

	}

}
