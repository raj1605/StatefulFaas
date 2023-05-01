// Created by Monish on 24-04-2023.
//

//#ifndef STATEFULFAAS_HOST_OPERATIONS_H
//#define STATEFULFAAS_HOST_OPERATIONS_H

#include "wasmer.h"
#include <zmq.hpp>
#define own

using namespace zmq;

static int it = 0;
static int bt = 0;

int arr[50];

context_t context;
wasm_memory_t* memory;
//__attribute__((__export_module__("env"), __export_name__("add_one")))
own wasm_trap_t *add_one(void* env, const wasm_val_vec_t* args, wasm_val_vec_t* results){
    it+=459;
    return NULL;
}

own wasm_trap_t *get(void* env, const wasm_val_vec_t* args, wasm_val_vec_t* results){
    int sizes = arr[args->data[0].of.i32];
    wasm_val_vec_t args_real;
    wasm_val_t get_at_args_val[1] = { WASM_I32_VAL(sizes) };
    //printf("The string here is ******** %d", args.size);

    args_real = WASM_ARRAY_VEC(get_at_args_val);
    wasm_val_copy(&results->data[0], &args_real.data[0]);
    printf("The return value is %d \n", sizes);
    return NULL;
}

own wasm_trap_t *chain_call(void* env, const wasm_val_vec_t* args, wasm_val_vec_t* results){

    socket_t chainRequest(context, socket_type::req);
    chainRequest.connect("ipc://chainCall.ipc");
//    printf("Inside chain call %d\n", args->size);
//    printf("%d", args->data[0].of.i32);
//    printf("%s\n", (char *)args->data[0].of.i32);

    wasm_val_vec_t actual_args;

    printf("Inside import function \n %d \n", args->size);
    //if(true || args->size > 0 || true){
       	printf("%d chumma data\n", args->data[0].of.i32);
    wasm_val_t val = WASM_INIT_VAL;
    wasm_val_copy(&val, &args->data[0]);
    int sizes = val.of.i32;
    printf("Crossed this ^^^ \n");
    unsigned char buffer[sizes+1];
    for(int r = 0;r<sizes;r++){
//	buffer[r] = wasm_memory_data(memory)[args->data[1].of.i32 + r];
    }
    buffer[sizes] = '\0';
//    char arg_string = wasm_memory_data(memory)[args->data[1].of.i32];
    printf("%s BUFFER", buffer);
       //printf("%s\n", (char *)args->data[0].of.i32);
    //}
    char buf[256];
    sprintf(buf, "%d", sizes);
    const char *client_msg = "Sending using ZMQ";
    message_t msg(buf, strlen(buf));
    chainRequest.send(msg, zmq::send_flags::none);
    message_t command;
    chainRequest.recv(command, zmq::recv_flags::none);



    wasm_val_t get_at_args_val[1] = { WASM_I32_VAL(atoi(command.to_string().c_str())) };
    wasm_val_vec_t args_real = WASM_ARRAY_VEC(get_at_args_val);
    wasm_val_copy(&results->data[0], &args_real.data[0]);
    return NULL;
}


//#endif //STATEFULFAAS_HOST_OPERATIONS_H
