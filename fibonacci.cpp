#include "interaction_core.h"
extern "C"{

int fib(int x) __attribute__((used));
int fib(int x){
    //sleep(5);
        if(x == 0 || x == 1){
            return 1;
        }

//		int val1 = get(x-1);
//		int val2 = get(x-2);
		int val1, val2;
		
			const char *temp = "test";
            int id = chain_call(x-1, temp);
			//val1 = temp_2;
//get(x-1);
		
	
//		put(x-1, val1, x);
        //return val1;
		
			const char *temp1 = "test";
			//int temp_2 = 
			//chain_call(x-2, temp1);
			//int total = 5;
			//use_future(2*(total-x));
			//use_future((2*(total-x))+1);

			use_future(id);
			
		
			val1 = get(x-1);
			val2 = get(x-2);
//get(x-2);
		
		
//		put(x-2, val2, x);
//		put(x, val2 + val1, x);

		return (val1 + val2);
}

}
