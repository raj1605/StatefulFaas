#include <errno.h>
#include <stdio.h>
#include <string.h>
#include <inttypes.h>
#include <typeinfo>
#include<string>
#include <iostream>

#include "host_operations.h"
#include "wasmer.h"

#define BUF_SIZE 128
#define own

using namespace std;

// Use the last_error API to retrieve error messages
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

int main(int argc, const char* argv[]) {

    // Initialize.
    printf("Initializing...\n");
    wasm_engine_t* engine = wasm_engine_new();
    wasm_store_t* store = wasm_store_new(engine);

    printf("Setting up WASI...\n");
    wasi_config_t* config = wasi_config_new("example_program");
//    // TODO: error checking
//    const char* js_string = "function greet(name) { return JSON.stringify('Hello, ' + name); }; print(greet('World'));";
//    wasi_config_arg(config, "--eval");
//    wasi_config_arg(config, js_string);
    wasi_config_capture_stdout(config);

    // Load binary.
    printf("Loading binary...\n");
    FILE* file = fopen("HelloWorld.wasm", "rb");
    if (!file) {
        printf("> Error loading module!\n");
        return 1;
    }

    fseek(file, 0L, SEEK_END);
    size_t file_size = ftell(file);
    fseek(file, 0L, SEEK_SET);
    wasm_byte_vec_t binary;
    wasm_byte_vec_new_uninitialized(&binary, file_size);
    if (fread(binary.data, file_size, 1, file) != 1) {
        printf("> Error initializing module!\n");
        return 1;
    }
    fclose(file);

    // Compile.
    printf("Compiling module...\n");
    own wasm_module_t* module = wasm_module_new(store, &binary);
    if (!module) {
        printf("> Error compiling module!\n");
        return 1;
    }

    wasm_byte_vec_delete(&binary);

    wasi_env_t* wasi_env = wasi_env_new(store, config);

    if (!wasi_env) {
        printf("> Error building WASI env!\n");
        print_wasmer_error();
        return 1;
    }

    printf("Creating the imported function...\n");
    wasm_functype_t* host_func_type = wasm_functype_new_0_0();
    wasm_func_t* host_func = wasm_func_new_with_env(store, host_func_type, add_one, store, NULL);
//            wasm_func_new(store, host_func_type, add_one);
    wasm_functype_delete(host_func_type);

    // Instantiate.
    printf("Instantiating module...\n");
    wasm_extern_vec_t wasi_import_obj;

    bool get_imports_result = wasi_get_imports(store, wasi_env,module,&wasi_import_obj);

    if(!get_imports_result){
        print_wasmer_error();
        cout << "get import result failed \n";
    }

    wasm_extern_t* externs[] = {
            wasm_func_as_extern(host_func),
            wasi_import_obj.data[0]
    };

    if (!get_imports_result) {
        printf("> Error getting WASI imports!\n");
        print_wasmer_error();
        return 1;
    }

    wasm_extern_vec_t imports = WASM_ARRAY_VEC(externs);
//    wasm_extern_vec_new_uninitialized(&imports, 2);
//    imports.data[0] = wasm_func_as_extern(host_func);
//    imports.data[1] = wasi_import_obj.data[0];

    own wasm_instance_t* instance =
            wasm_instance_new(store, module, &imports, NULL);

    if (!instance) {
        printf("> Error instantiating module!\n");
        print_wasmer_error();
        return 1;
    }

    if (!wasi_env_initialize_instance(wasi_env, store, instance)) {
        printf("> Error initializing wasi env memory!\n");
        print_wasmer_error();
        return 1;
    }

    // Extract export.
    printf("Extracting export...\n");
    own wasm_extern_vec_t exports;
    wasm_instance_exports(instance, &exports);
    if (exports.size == 0) {
        printf("> Error accessing exports!\n");
        return 1;
    }
    fprintf(stderr, "found %zu exports\n", exports.size);


    wasm_func_t* run_func = wasm_extern_as_func(exports.data[0]);
//            wasi_get_start_function(instance);
    if (run_func == NULL) {
        printf("> Error accessing export!\n");
        print_wasmer_error();
        return 1;
    }

    // Call.
    printf("Calling export...\n");

//    wasm_val_vec_t args = WASM_EMPTY_VEC;
//    wasm_val_vec_t res = WASM_EMPTY_VEC;

    wasm_val_t results_val[1] = { WASM_INIT_VAL };
    wasm_val_vec_t args = WASM_EMPTY_VEC;
    wasm_val_vec_t res = WASM_EMPTY_VEC;


    if (wasm_func_call(run_func, &args, &res)) {
        printf("> Error calling function!\n");
        return 1;
    }

    cout << i << " value of i\n";
    printf("Call completed\n");

//    if(true) {
//
//        // NOTE: previously, this used open_memstream,
//        // which is not cross-platform
//        FILE *memory_stream = NULL;
//        memory_stream = tmpfile(); // stdio.h
//
//        if (NULL == memory_stream) {
//            printf("> Error creating a memory stream.\n");
//            return 1;
//        }
//
//        char buffer[BUF_SIZE] = { 0 };
//        size_t data_read_size = BUF_SIZE;
//
//        do {
//            data_read_size = wasi_env_read_stdout(wasi_env, buffer, BUF_SIZE);
//            if (data_read_size == -1) {
//                printf("failed to read stdout: %s\n", strerror(errno));
//                print_wasmer_error();
//                return -1;
//            }
//
//            if (data_read_size > 0) {
//                fwrite(buffer, sizeof(char), data_read_size, memory_stream);
//            }
//        } while (BUF_SIZE == data_read_size);
//
//        // print memory_stream
//        rewind(memory_stream);
//        fputs("WASI Stdout: ", stdout);
//        char buffer2[256];
//        while (!feof(memory_stream)) {
//            if (fgets(buffer2, 256, memory_stream) == NULL) break;
//            fputs(buffer2, stdout);
//        }
//        fputs("\n", stdout);
//        fclose(memory_stream);
//    }


    wasm_extern_vec_delete(&exports);
    wasm_extern_vec_delete(&imports);

    // Shut down.
    printf("Shutting down...\n");
    wasm_func_delete(run_func);
    wasm_module_delete(module);
    wasm_instance_delete(instance);
    wasm_store_delete(store);
    wasm_engine_delete(engine);

    // All done.
    printf("Done.\n");
    return 0;
}