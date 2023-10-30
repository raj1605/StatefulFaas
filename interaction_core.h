extern "C" int chain_call(int, const char*) __attribute__((__import_module__("env"), __import_name__("chain_call")));
extern "C" int get(int) __attribute__((__import_module__("env"), __import_name__("get")));
extern "C" void put(int,int,int) __attribute__((__import_module__("env"), __import_name__("put")));
extern "C" void use_future(int) __attribute__((__import_module__("env"), __import_name__("use_future")));
