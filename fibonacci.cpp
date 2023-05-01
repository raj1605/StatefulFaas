//#include <unistd.h>
extern "C" void add_one() __attribute__(( __import_module__("env"),__import_name__("add_one")));
extern "C" void chain_call(int, const char*) __attribute__((__import_module__("env"), __import_name__("chain_call")));
extern "C" int get(int) __attribute__((__import_module__("env"), __import_name__("get")));

extern "C"{

	int fib(int x) __attribute__((used));
	int fib(int x){
		//sleep(5);
        if(x == 0 || x == 1)
            return 1;
		int val1 = get(x-1);
		int val2 = get(x-2);
		
		if(val1 == -1){
			const char *temp = "test";
			chain_call(x-1, temp);
			val1 = get(x-1);
		}
		if(val2 == -1){
			const char *temp = "test";
			chain_call(x-2, temp);
			val2 = get(x-2);
		}

		return (val1 + val2);
	}

}
