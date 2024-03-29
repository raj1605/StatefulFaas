#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <inttypes.h>
#include <zmq.hpp>
#include <string>
#include <chrono>
#include <thread>

#include "BS_thread_pool.hpp"
#include "wasm.h"
#include "host_operations.h"
#include "helpers.hpp"
#define own

using namespace zmq;
struct my_struct{
    int key;
    int value;
};
void print_wasmer_error()
{
    int error_len = wasmer_last_error_length();
    if (error_len > 0) {
        printf("Error len: `%d`\n", error_len);
        char *error_str = (char *)malloc(error_len);
        wasmer_last_error_message(error_str, error_len);
        printf("Error str: `%s`\n", error_str);
    }
}

void run_function(socket_t* chainResponse, std::string *key_temp, char func_id, int arg_val) {

    // Register with ZMQ
//    register_fun();

    // Initialize.
    message_t key(*key_temp);
//                  , (int)key_temp->length());
    message_t discard("", 0);
    std::cout << std::endl;
    printf("\nInitializing...%d\n", arg_val);
    wasm_engine_t* engine = wasm_engine_new();
    wasm_store_t* store = wasm_store_new(engine);

    // Load binary.
    printf("Loading binary...\n");
    FILE* file = fopen("fibonacci.wasm", "rb");
    if (!file) {
        printf("> Error loading module!\n");
//    return 1;
    }
    fseek(file, 0L, SEEK_END);
    size_t file_size = ftell(file);
    fseek(file, 0L, SEEK_SET);
    wasm_byte_vec_t binary;
    wasm_byte_vec_new_uninitialized(&binary, file_size);
    if (fread(binary.data, file_size, 1, file) != 1) {
        printf("> Error loading module!\n");
//    return 1;
    }
    fclose(file);

    // Compile.
    printf("Compiling module...\n");
    own wasm_module_t* module = wasm_module_new(store, &binary);
    if (!module) {
        printf("> Error compiling module!\n");
//    return 1;
    }

    wasm_byte_vec_delete(&binary);

    // Create external print functions.
    printf("Creating callback...\n");
    own wasm_functype_t* future_func_type = wasm_functype_new_1_0(wasm_valtype_new_i32());
    own wasm_functype_t* get_func_type = wasm_functype_new_1_1(wasm_valtype_new_i32(), wasm_valtype_new_i32());
    own wasm_functype_t* chain_func_type = wasm_functype_new_2_1(wasm_valtype_new_i32(), wasm_valtype_new_i32(),
                                                                 wasm_valtype_new_i32());
    own wasm_functype_t* put_func_type =
//wasm_functype_new_0_0(wasm_valtype_new_i32());
            wasm_functype_new_3_0(wasm_valtype_new_i32(), wasm_valtype_new_i32(), wasm_valtype_new_i32());

    own wasm_func_t* put_func =
            wasm_func_new_with_env(store, put_func_type, put, store, NULL);
    own wasm_func_t* chain_func =
            wasm_func_new_with_env(store, chain_func_type, chain_call, store, NULL);
    own wasm_func_t* get_func =
            wasm_func_new_with_env(store, get_func_type, get, store, NULL);
    own wasm_func_t* future_func =
            wasm_func_new_with_env(store, future_func_type, use_future, store, NULL);

//    printf("%p %p %p %p +++++++++++++++++++++++++++++++++++++++++++++++++++++++++\n", put_func, chain_func, get_func, future_func);
    wasi_config_t* config = wasi_config_new("example_program");
    wasi_config_capture_stdout(config);
    wasi_env_t* wasi_env = wasi_env_new(store, config);
    if (!wasi_env) {
        printf("> Error building WASI env!\n");
        print_wasmer_error();
    }
    // Instantiate.
    printf("Instantiating module...\n");
    wasm_extern_t* externs[] = {
//wasi_import_obj.data[0]
//	wasm_func_as_extern(get_func),
	wasm_func_as_extern(chain_func),
	wasm_func_as_extern(future_func),
	wasm_func_as_extern(get_func)
//wasi_import_obj.data[0]
    };
    print_wasmer_error();
    printf("before imports and imports size is \n");
    wasm_extern_vec_t imports = WASM_ARRAY_VEC(externs);
    own wasm_instance_t* instance =
            wasm_instance_new(store, module, &imports, NULL);

    if (!instance) {
        print_wasmer_error();
        printf("> Error instantiating module!\n");
//    return 1;
    }

    //wasm_func_delete(put_func);
    //wasm_func_delete(chain_func);
    //wasm_func_delete(future_func);

    // Extract export.
    wasm_exporttype_vec_t export_type;
    wasm_module_exports(module, &export_type);
    printf("Extracting exports...\n");
    own wasm_extern_vec_t exports;
    wasm_instance_exports(instance, &exports);
    for(int mno = 0;mno<exports.size;mno++){
        const wasm_name_t *name = wasm_exporttype_name(export_type.data[mno]);
        std::cout <<mno<<" "<< name->data << " " << name->size << " <- exports info" << std::endl;
    }

    printf("export size %lu\n", exports.size);

    printf("boolean %s", exports.size < 2 ? "true" : "false");
    if (exports.size < 2) {
        printf("> Error accessing exports!\n");
        //return 1;
    }

    wasm_module_delete(module);
    wasm_instance_delete(instance);
    //
//    wasm_memory_t* 
	memory = wasm_extern_as_memory(exports.data[0]);
    printf("\nprinting the exported memory %p\n", memory);
    //
//    return;
    // Call.
    for (int i = func_id; i < exports.size; ++i) {

        printf("func id is == %d\n", i);

        const wasm_func_t* fib_func = wasm_extern_as_func(exports.data[i]);

        if (fib_func == NULL) {
            printf("> Error accessing export!\n");
//      return 1;
        }

        printf("Calling export %d...\n", i);
//        const char* test_string = "Testing wasm_ref_val";
        wasm_val_vec_t args;
        wasm_val_t get_at_args_val[1] = { WASM_I32_VAL(arg_val) };
	//printf("The string here is ******** %d", args.size);
        args = WASM_ARRAY_VEC(get_at_args_val);

        wasm_val_vec_t results;
        wasm_val_t get_at_results_val[1] = { WASM_INIT_VAL };

        results = WASM_ARRAY_VEC(get_at_results_val);

	wasm_val_vec_t* chumma = &results;
	printf("*********** %d ****** %d \n", results.size, chumma->size);
        own wasm_trap_t *trap;
        try {
            trap = wasm_func_call(fib_func, &args, &results);
            printf("I'm here -----> %d\n", arg_val);
            printf("I'm here ----->\n");
        }catch(...) {
            print_wasmer_error();
        }
	arr[arg_val] = get_at_results_val[0].of.i64;
//        if(func_id == 5 || func_id == 6|| func_id == 8 || func_id == 9){
            printf("Value at %d for %d\n", get_at_results_val[0].of.i64, arg_val);
//        return get_at_results_val[0].of.i64
        break;
        if (!trap) {
            printf("> Error calling function, expected trap!\n");
            print_wasmer_error();
            //return 1;
        }

        printf("Printing message...\n");
        own wasm_name_t message;
        wasm_trap_message(trap, &message);
        printf("> %s\n", message.data);

        printf("Printing origin...\n");
        own wasm_frame_t* frame = wasm_trap_origin(trap);
        if (frame) {
//            print_frame(frame);
            wasm_frame_delete(frame);
        } else {
            printf("> Empty origin.\n");
        }

        printf("Printing trace...\n");
        own wasm_frame_vec_t trace;
        wasm_trap_trace(trap, &trace);
        if (trace.size > 0) {
            for (size_t i = 0; i < trace.size; ++i) {
//                print_frame(trace.data[i]);
            }
        } else {
            printf("> Empty trace.\n");
        }

        wasm_frame_vec_delete(&trace);
        wasm_trap_delete(trap);
        wasm_name_delete(&message);
    }

    wasm_extern_vec_delete(&exports);

    // Shut down.
    printf("Shutting down...%d\n", arg_val);
    wasm_store_delete(store);
    wasm_engine_delete(engine);

    //
    std::cout<<"Sending final message for ///////////////" << arg_val << std::endl;
    char buf[256];
    sprintf(buf, "%d", arr[arg_val]);
    zmq::message_t msg(buf, strlen(buf));
    std::cout << key.to_string() <<"-< key"<< discard.to_string() << "-< discard" << std::endl;
    chainResponse->send(key, zmq::send_flags::sndmore);
    chainResponse->send(discard, zmq::send_flags::sndmore);
    chainResponse->send(msg, zmq::send_flags::none);
    //

    // All done.
    printf("Done.-> %d\n", arg_val);
}

void run_main_function(int func_id, int arg_val) {

    // Register with ZMQ
//    register_fun();

    // Initialize.
    std::cout << std::endl;
    printf("\nInitializing...\n");
    wasm_engine_t* engine = wasm_engine_new();
    wasm_store_t* store = wasm_store_new(engine);

    // Load binary.
    printf("Loading binary...\n");
    FILE* file = fopen("fibonacci.wasm", "rb");
    if (!file) {
        printf("> Error loading module!\n");
//    return 1;
    }
    fseek(file, 0L, SEEK_END);
    size_t file_size = ftell(file);
    fseek(file, 0L, SEEK_SET);
    wasm_byte_vec_t binary;
    wasm_byte_vec_new_uninitialized(&binary, file_size);
    if (fread(binary.data, file_size, 1, file) != 1) {
        printf("> Error loading module!\n");
//    return 1;
    }
    fclose(file);

    // Compile.
    printf("Compiling module...\n");
    own wasm_module_t* module = wasm_module_new(store, &binary);
    if (!module) {
        printf("> Error compiling module!\n");
//    return 1;
    }

    wasm_byte_vec_delete(&binary);

    // Create external print functions.
    printf("Creating callback...\n");
    own wasm_functype_t* get_func_type = wasm_functype_new_1_1(wasm_valtype_new_i32(), wasm_valtype_new_i32());
    own wasm_functype_t* chain_call_func_type = wasm_functype_new_2_1(wasm_valtype_new_i32(), wasm_valtype_new_i32(),
                                                                      wasm_valtype_new_i32());
    own wasm_functype_t* put_func_type =
//wasm_functype_new_0_0(wasm_valtype_new_i32());
            wasm_functype_new_3_0(wasm_valtype_new_i32(), wasm_valtype_new_i32(), wasm_valtype_new_i32());
     own wasm_functype_t* use_future_func_type = wasm_functype_new_1_0(wasm_valtype_new_i32());

    own wasm_func_t* put_func =
            wasm_func_new_with_env(store, put_func_type, put, store, NULL);

    own wasm_func_t* chain_call_func =
            wasm_func_new_with_env(store, chain_call_func_type, chain_call, store, NULL);

    own wasm_func_t* get_func =
            wasm_func_new_with_env(store, get_func_type, get, store, NULL);
    
    own wasm_func_t* use_future_func =
            wasm_func_new_with_env(store, use_future_func_type, use_future, store, NULL);

    //wasm_functype_delete(fail_type);

    wasi_config_t* config = wasi_config_new("example_program");
    wasi_config_capture_stdout(config);
    wasi_env_t* wasi_env = wasi_env_new(store, config);
    if (!wasi_env) {
        printf("> Error building WASI env!\n");
        print_wasmer_error();
//        return 1;
    }
    //wasm_extern_vec_t wasi_import_obj;

    //bool get_imports_result = wasi_get_imports(store, wasi_env,module,&wasi_import_obj);

    //if(!get_imports_result){
    //  print_wasmer_error();
    //printf("inside wasi_get_imports\n");
    //cout << "get import result failed \n";
    //}


    // Instantiate.
    printf("Instantiating module...\n");
    wasm_extern_t* externs[] = {
//wasi_import_obj.data[0]
//,

//            wasm_func_as_extern(fail_func2),
            wasm_func_as_extern(chain_call_func),
            wasm_func_as_extern(use_future_func),
            wasm_func_as_extern(get_func),
//wasi_import_obj.data[0]
    };
    print_wasmer_error();
    printf("before imports\n");
    wasm_extern_vec_t imports = WASM_ARRAY_VEC(externs);
    own wasm_instance_t* instance =
            wasm_instance_new(store, module, &imports, NULL);
    if (!instance) {
        print_wasmer_error();
        printf("> Error instantiating module!\n");
//    return 1;
    }

    wasm_func_delete(chain_call_func);
    wasm_func_delete(get_func);
    wasm_func_delete(use_future_func);

    // Extract export.
    wasm_exporttype_vec_t export_type;
    wasm_module_exports(module, &export_type);
    printf("Extracting exports...\n");
    own wasm_extern_vec_t exports;
    wasm_instance_exports(instance, &exports);
    for(int mno = 0;mno<exports.size;mno++){
        const wasm_name_t *name = wasm_exporttype_name(export_type.data[mno]);
        std::cout <<mno<<" "<< name->data << " " << name->size << " <- exports info" << std::endl;
    }

    printf("export size %lu\n", exports.size);

    printf("boolean %s", exports.size < 2 ? "true" : "false");
    if (exports.size < 2) {
        printf("> Error accessing exports!\n");
        //return 1;
    }

    wasm_module_delete(module);
    wasm_instance_delete(instance);

    memory = wasm_extern_as_memory(exports.data[0]);
    printf("\nprinting the exported memory %p\n", memory);
    //
//    return;
    // Call.
    for (int i = func_id; i < exports.size; ++i) {

        printf("func id is == %d\n", i);

        const wasm_func_t* fib_func = wasm_extern_as_func(exports.data[i]);

        if (fib_func == NULL) {
            printf("> Error accessing export!\n");
//      return 1;
        }

        printf("Calling export %d...\n", i);
//        const char* test_string = "Testing wasm_ref_val";
        wasm_val_vec_t args;
        wasm_val_t get_at_args_val[1] = { WASM_I32_VAL(arg_val) };
        //printf("The string here is ******** %d", args.size);
        args = WASM_ARRAY_VEC(get_at_args_val);

        wasm_val_vec_t results;
        wasm_val_t get_at_results_val[1] = { WASM_INIT_VAL };

        results = WASM_ARRAY_VEC(get_at_results_val);

        wasm_val_vec_t* chumma = &results;
        printf("*********** %d ****** %d \n", results.size, chumma->size);
        own wasm_trap_t *trap;
        try {
            trap = wasm_func_call(fib_func, &args, &results);
            printf("I'm here -----> %d\n", arg_val);
            printf("I'm here ----->\n");
        }catch(...) {
            print_wasmer_error();
        }

//        if(func_id == 5 || func_id == 6|| func_id == 8 || func_id == 9){
        printf("Value at %d for %d\n", get_at_results_val[0].of.i64, arg_val);
//        return get_at_results_val[0].of.i64
	arr[arg_val] = get_at_results_val[0].of.i64;
        break;
        if (!trap) {
            printf("> Error calling function, expected trap!\n");
            print_wasmer_error();
            //return 1;
        }

        printf("Printing message...\n");
        own wasm_name_t message;
        wasm_trap_message(trap, &message);
        printf("> %s\n", message.data);

        printf("Printing origin...\n");
        own wasm_frame_t* frame = wasm_trap_origin(trap);
        if (frame) {
            //print_frame(frame);
            wasm_frame_delete(frame);
        } else {
            printf("> Empty origin.\n");
        }

        printf("Printing trace...\n");
        own wasm_frame_vec_t trace;
        wasm_trap_trace(trap, &trace);
        if (trace.size > 0) {
            for (size_t i = 0; i < trace.size; ++i) {
                //print_frame(trace.data[i]);
            }
        } else {
            printf("> Empty trace.\n");
        }

        wasm_frame_vec_delete(&trace);
        wasm_trap_delete(trap);
        wasm_name_delete(&message);
    }

    wasm_extern_vec_delete(&exports);

    // Shut down.
    printf("Shutting down...\n");
    wasm_store_delete(store);
    wasm_engine_delete(engine);

    // All done.
    printf("Done.\n");
//  return 0;
}

int main(){

    for(int i = 0;i<50;i++){
        arr[i] = -1;
        arr[0] = 1;
        arr[1] = 1;
    }

    //Setting up socket to act as a server which serves client requests
    socket_t chainResponse(context, socket_type::router);
    chainResponse.bind("ipc://chainCall.ipc");
    chainResponse.bind("tcp://*:5555");

    //Thread Pool from the BS_thread_pool.hpp
    BS::thread_pool pool;

    std::future<void> main_future = pool.submit(run_main_function, 1, 6);
    std::chrono::time_point<std::chrono::system_clock> start =
            std::chrono::system_clock::now() + std::chrono::seconds(5);
    while(pool.get_tasks_total() != 0){
//    || std::chrono::system_clock::now() < start){
        message_t key;
        chainResponse.recv(key, recv_flags::dontwait);
        if(key.size() > 0){
            message_t discard, command;
            chainResponse.recv(discard);
            chainResponse.recv(command);
            std::cout << "Message from the client is - " << command.to_string() <<" with key "<<key.to_string() << std::endl;
            std::vector<std::string> vec = split(command.to_string());
            std::string *s = new std::string(key.to_string());
            for(unsigned int i = 0; i < vec.size(); i++)
            {
                std::cout << vec[i] << " " << std::endl;
            }

            std::cout << "Before submitting to thread pool the value of vec[0] is " << vec[0] << std::endl;
            std::string *sly = new std::string(key.to_string());
            std::future<void> secondary_future = pool.submit(run_function, &chainResponse, sly, 1, stoi(vec[0]));
//            while(arr[stoi(vec[0])] == -1){
//                int mnop = 1;
//            }
//            std::this_thread::sleep_for (std::chrono::seconds(2));

            start = std::chrono::system_clock::now() + std::chrono::seconds(5);
        }

    }

    std::cout << "Total - " << pool.get_tasks_total() << std::endl;

    if(pool.get_tasks_total() == 0){
        for(int i = 0;i<50;i++){
            std::cout << arr[i] << " ";
        }
        std::cout << std::endl;
        return arr[6];
    }


}
