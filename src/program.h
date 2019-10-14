#ifndef PROGRAM_H
#define PROGRAM_H

#include "common.h"
#include "value.h"

typedef enum
{
  OP_CONST,
  OP_NONE,
  OP_TRUE,
  OP_FALSE,
  OP_EQUAL,
  OP_GREATER,
  OP_LESS,
  OP_ADD,
  OP_SUBTRACT,
  OP_MULTIPLY,
  OP_DIVIDE,
  OP_NOT,
  OP_NEGATE,
  OP_RETURN,
} OperationCode;

typedef struct
{
  int actuallyInUse;
  int numOfAllocated;
  uint8_t *code; // machine independent unsigned char
  int *lines;
  ValueArray consts;
} Program;

void initProgram(Program *program);
void freeProgram(Program *program);
void writeProgram(Program *program, uint8_t byte, int line);
int addConst(Program *program, Value value);

#endif
