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

context_t context;
wasm_memory_t* memory;
//__attribute__((__export_module__("env"), __export_name__("add_one")))
own wasm_trap_t *add_one(void* env, const wasm_val_vec_t* args, wasm_val_vec_t* results){
    it+=459;
    return NULL;
}

//void register_fun(context_t context){
//    //context_t context;
//    //socket_t chainResponse(context, socket_type::rep);
////    chainRequest.connect("ipc://chainCall.ipc");
//    chainRequest = socket_t(context, socket_type::rep);
//    chainRequest.connect("tcp://127.0.0.1:5555");
//
//}

own wasm_trap_t *chain_call(void* env, const wasm_val_vec_t* args, wasm_val_vec_t* results){

    socket_t chainRequest(context, socket_type::req);
    chainRequest.connect("ipc://chainCall.ipc");
//    printf("Inside chain call %d\n", args->size);
//    printf("%d", args->data[0].of.i32);
//    printf("%s\n", (char *)args->data[0].of.i32);
    printf("Inside import function \n %d \n", args->size);
    printf("one more here \n");
    //if(true || args->size > 0 || true){
        printf("Inside if condition\n");
       	printf("%d chumma data\n", args->data[0].of.i32);
    int sizes = args->data[0].of.i32;
    unsigned char buffer[sizes+1];
    for(int r = 0;r<sizes;r++){
	buffer[r] = wasm_memory_data(memory)[args->data[1].of.i32 + r];
    }
    buffer[sizes] = '\0';
    char arg_string = wasm_memory_data(memory)[args->data[1].of.i32];
    printf("%s BUFFER", buffer);
       //printf("%s\n", (char *)args->data[0].of.i32);
    //}
    const char *client_msg = "Sending using ZMQ";
    message_t msg(client_msg, strlen(client_msg));
    chainRequest.send(msg, zmq::send_flags::none);
//    bt = 0;
    bt += 26;
    return NULL;
}


//#endif //STATEFULFAAS_HOST_OPERATIONS_H
