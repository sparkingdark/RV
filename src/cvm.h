#ifndef CVM_H
#define CVM_H

#include "program.h"
#include "value.h"

#define STACK_MAX 256

typedef struct
{
    Program *program;
    uint8_t *ip;
    Value stack[STACK_MAX];
    Value *stackTop;

} CVM;

typedef enum
{
    INTERPRET_OK,
    INTERPRET_COMPILE_ERROR,
    INTERPRET_RUNTIME_ERROR
} InterpretResult;

void initCVM();
void freeCVM();
InterpretResult interpret(const char *src);
void push(Value value);
Value pop();

#endif
