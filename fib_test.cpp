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

// A function to be called from Wasm code.
own wasm_trap_t* fail_callback(
  void* env, const wasm_val_vec_t* args, wasm_val_vec_t* results
) {
  printf("Calling back...\n");
  own wasm_name_t message;
  wasm_name_new_from_string_nt(&message, "callback abort");
  own wasm_trap_t* trap = wasm_trap_new((wasm_store_t*)env, &message);
  wasm_name_delete(&message);
  return trap;
}


void print_frame(wasm_frame_t* frame) {
  printf("> %p @ 0x%zx = %" PRIu32 ".0x%zx\n",
    wasm_frame_instance(frame),
    wasm_frame_module_offset(frame),
    wasm_frame_func_index(frame),
    wasm_frame_func_offset(frame)
  );
}

void run_function(int func_id, int arg_val) {

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
    own wasm_functype_t* fail_type3 = wasm_functype_new_1_1(wasm_valtype_new_i32(), wasm_valtype_new_i32());
    own wasm_functype_t* fail_type2 = wasm_functype_new_2_1(wasm_valtype_new_i32(), wasm_valtype_new_i32(), wasm_valtype_new_i32());
    own wasm_functype_t* fail_type =
//wasm_functype_new_0_0(wasm_valtype_new_i32());
            wasm_functype_new_0_0();
    own wasm_func_t* fail_func =
            wasm_func_new_with_env(store, fail_type, add_one, store, NULL);

    own wasm_func_t* fail_func2 =
            wasm_func_new_with_env(store, fail_type2, chain_call, store, NULL);

    own wasm_func_t* fail_func3 =
            wasm_func_new_with_env(store, fail_type3, get, store, NULL);

    if (!fail_func) {
        printf("> Error compiling fail_func!\n");
//    return 1;
    }

    wasm_functype_delete(fail_type);

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
            wasm_func_as_extern(fail_func3),
            wasm_func_as_extern(fail_func2),
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

    wasm_func_delete(fail_func);

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

    // =====================

//    printf("Initializing wasm memory...\n");
//    wasm_limits_t limits1 = {
//            .min = 0,
//            .max = wasm_limits_max_default,
//    };
//    own wasm_memorytype_t* memtype1 = wasm_memorytype_new(&limits1);
//    own wasm_memory_t* memory = wasm_memory_new(store, memtype1);
////    print_wasmer_error();
////    std::cout << wasm_memory_data_size(memory) << std::endl;
////    std::cout<<wasm_memory_data(memory)[1024];
//    wasm_memory_pages_t new_pages1 = wasm_memory_size(memory);
//    printf("Old memory size (pages): %d\n", new_pages1);
//
//    printf("Growing memory...\n");
//    if (!wasm_memory_grow(memory, 4)) {
//        printf("> Error growing memory!\n");
//    }
//    std::cout << wasm_memory_data_size(memory) <<" line 1" << std::endl;
//    char *b = wasm_memory_data(memory);
//    std::cout << b <<" -< byte array" <<std::endl;
//    std::cout<<wasm_memory_data(memory)[0x1024] << " line 2" << std::endl;
//
//    wasm_memory_pages_t new_pages = wasm_memory_size(memory);
//    printf("New memory size (pages): %d\n", new_pages);
//    std::cout << "\n";
    // ===================== //

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

//        if(func_id == 5 || func_id == 6|| func_id == 8 || func_id == 9){
            printf("Value at %d\n", get_at_results_val[0].of.i64);

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
            print_frame(frame);
            wasm_frame_delete(frame);
        } else {
            printf("> Empty origin.\n");
        }

        printf("Printing trace...\n");
        own wasm_frame_vec_t trace;
        wasm_trap_trace(trap, &trace);
        if (trace.size > 0) {
            for (size_t i = 0; i < trace.size; ++i) {
                print_frame(trace.data[i]);
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

    std::future<void> main_future = pool.submit(run_function, 1, 5);
    std::chrono::time_point<std::chrono::system_clock> start =
            std::chrono::system_clock::now() + std::chrono::seconds(5);
    while(std::chrono::system_clock::now() < start){
        message_t key;
        chainResponse.recv(key, recv_flags::dontwait);
        if(key.size() > 0){
            message_t discard, command;
            chainResponse.recv(discard);
            chainResponse.recv(command);
            std::cout << "Message from the client is - " << command.to_string() << std::endl;
            std::vector<std::string> vec = split(command.to_string());
            for(unsigned int i = 0; i < vec.size(); i++)
            {
                std::cout << vec[i] << " " << std::endl;
            }

            std::cout << "Before submitting to thread pool the value of vec[0] is " << vec[0] << std::endl;
            std::future<void> secondary_future = pool.submit(run_function,1, stoi(vec[0]));
//            while(arr[stoi(vec[0])] == -1){
//                int mnop = 1;
//            }
            std::this_thread::sleep_for (std::chrono::seconds(2));
            char buf[256];
            sprintf(buf, "%d", arr[stoi(vec[0])]);

            zmq::message_t msg(buf, strlen(buf));
            chainResponse.send(key, zmq::send_flags::sndmore);
            chainResponse.send(discard, zmq::send_flags::sndmore);
            chainResponse.send(msg, zmq::send_flags::none);
            start = std::chrono::system_clock::now() + std::chrono::seconds(5);
        }

    }

//    for(int i = 0;i<50;i++){
//        std::cout << arr[i] << std::endl;
//    }
}
