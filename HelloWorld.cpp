//#include <iostream>
//#include "HostOperations.h"
//using namespace std;
struct my_struct{
	int key;
	int value;
};
extern "C" void add_one() __attribute__(( __import_module__("env"),__import_name__("add_one")));
extern "C" void chain_call() __attribute__((__import_module__("env"), __import_name__("chain_call")));

extern "C" {
	int mem __attribute__((used)) = 63;
	const char* strptr __attribute__((used)) = "mo";
	const char * strptr2 __attribute__((used)) = " fine by me";
	int p __attribute__((used)) = 14;
	my_struct myStruct __attribute__((used)) = {3, 4};
	//int mem = 0;
	void pred() __attribute__((used));
	void pred(){
		for(int x = 0; x< 10; x++)
			add_one();
		mem += 1;
		//chain_call();
	}
//}
	void chain_call_test() __attribute__((used));
	void chain_call_test(){
		const char* str = "monish raj raghu from inside the wasm module";
		chain_call(1);
	}

	//const char* 
	int* getGlobal() __attribute__((used));
	//const char* 
	int* getGlobal(){
		return &mem; 
	//strptr;
	}

	const char* getGlobal2() __attribute__((used));
	const char* getGlobal2(){
		return strptr;
	}

	int setGlobal(int *val) __attribute__((used));
	int setGlobal(int *val){
		//extern int mem;
		return *val;
	}
	
	struct my_struct* setStruct() __attribute__((used));
	struct my_struct* setStruct(){
		myStruct.key = 1;
		myStruct.value = 2;
		return &myStruct;
	}
//extern "C" {
	int main() __attribute__((used));
	int main(){
	//cout << "Hello world" << endl;
	//printf("hello world inside printf");
	
	}
}
