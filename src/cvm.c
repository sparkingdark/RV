#include <stdarg.h>
#include <stdio.h>
#include "common.h"
#include "compiler.h"
#include "debug.h"
#include "cvm.h"

CVM vm;

static void resetStack()
{
    vm.stackTop = vm.stack;
}

static void runtimeError(const char *format, ...)
{
    va_list args;
    va_start(args, format);
    vfprintf(stderr, format, args);
    va_end(args);
    fputs("\n", stderr);

    size_t instruction = vm.ip - vm.program->code;
    int line = vm.program->lines[instruction];
    fprintf(stderr, "on line %d\n", line);

    resetStack();
}

void initCVM()
{
    resetStack();
}

void freeCVM()
{
    vm.stackTop = vm.stack;
}

void push(Value value)
{
    *vm.stackTop = value;
    ++vm.stackTop;
}

Value pop()
{
    --vm.stackTop;
    return *vm.stackTop;
}

static Value peek(int distance)
{
    return vm.stackTop[-1 - distance];
}

static bool isFalsy(Value value)
{
    return IS_NONE(value) || (IS_BOOL(value) && !AS_BOOL(value));
}

static InterpretResult run()
{
#define READ_BYTE() (*vm.ip++)
#define READ_CONST() (vm.program->consts.values[READ_BYTE()])

#define BINARY_OPERATOR(valueType, operator)                           \
    do                                                                 \
    {                                                                  \
        if (!IS_NUMBER(peek(0)) || !IS_NUMBER(peek(1)))                \
        {                                                              \
            runtimeError("Unmatching type, operands must be numbers"); \
            return INTERPRET_RUNTIME_ERROR;                            \
        }                                                              \
        double b = AS_NUMBER(pop());                                   \
        double a = AS_NUMBER(pop());                                   \
        push(valueType(a operator b));                                 \
    } while (false)

    while (true)
    {
#ifdef DEBUG_TRACE_EXECUTION
        printf("          ");
        for (Value *slot = vm.stack; slot < vm.stackTop; ++slot)
        {
            printf("[ ");
            printValue(*slot);
            printf(" ]");
        }
        printf("\n");

        disassembleInstruction(vm.program, (int)(vm.ip - vm.program->code));
#endif

        uint8_t instruction;

        switch (instruction = READ_BYTE())
        {
        case OP_CONST:
        {
            Value constant = READ_CONST();
            push(constant);
            break;
        }
        case OP_NONE:
            push(NONE_VAL);
            break;
        case OP_TRUE:
            push(BOOL_VAL(true));
            break;
        case OP_FALSE:
            push(BOOL_VAL(false));
            break;
        case OP_EQUAL:
        {
            Value b = pop();
            Value a = pop();
            push(BOOL_VAL(areValuesEqual(a, b)));
            break;
        }
        case OP_GREATER:
            BINARY_OPERATOR(BOOL_VAL, >);
            break;
        case OP_LESS:
            BINARY_OPERATOR(BOOL_VAL, <);
            break;
        case OP_ADD:
            BINARY_OPERATOR(NUMBER_VAL, +);
            break;
        case OP_SUBTRACT:
            BINARY_OPERATOR(NUMBER_VAL, -);
            break;
        case OP_MULTIPLY:
            BINARY_OPERATOR(NUMBER_VAL, *);
            break;
        case OP_DIVIDE:
            BINARY_OPERATOR(NUMBER_VAL, /);
            break;
        case OP_NOT:
            push(BOOL_VAL(isFalsy(pop())));
            break;
        case OP_NEGATE:
            if (!IS_NUMBER(peek(0)))
            {
                runtimeError("Unmatching type, operand must be a number");
                return INTERPRET_RUNTIME_ERROR;
            }
            push(NUMBER_VAL(-AS_NUMBER(pop())));
            break;
        case OP_RETURN:
            printValue(pop());
            printf("\n");
            return INTERPRET_OK;
        }
    }

#undef READ_BYTE
#undef READ_CONST
#undef BINARY_OPERATOR
}

InterpretResult interpret(const char *src)
{
    Program program;
    initProgram(&program);

    if (!compile(src, &program))
    {
        freeProgram(&program);
        return INTERPRET_COMPILE_ERROR;
    }

    vm.program = &program;
    vm.ip = vm.program->code;

    InterpretResult result = run();

    freeProgram(&program);
    return result;
}
