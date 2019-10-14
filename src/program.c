#include <stdlib.h>

#include "program.h"
#include "memory.h"

void initProgram(Program *program)
{
  program->actuallyInUse = 0;
  program->numOfAllocated = 0;
  program->code = NULL;
  program->lines = NULL;
  initValueArray(&program->consts);
}

void freeProgram(Program *program)
{
  FREE_ARRAY(uint8_t, program->code, program->numOfAllocated);
  FREE_ARRAY(int, program->lines, program->numOfAllocated);
  freeValueArray(&program->consts);
  initProgram(program);
}

void writeProgram(Program *program, uint8_t byte, int line)
{
  if (program->numOfAllocated < program->actuallyInUse + 1)
  {
    int oldNumOfAllocated = program->numOfAllocated;

    program->numOfAllocated = GROW_NUM_OF_ALLOCATED(oldNumOfAllocated);

    program->code = GROW_ARRAY(program->code, uint8_t, oldNumOfAllocated, program->numOfAllocated);
    program->lines = GROW_ARRAY(program->lines, int, oldNumOfAllocated, program->numOfAllocated);
  }

  program->code[program->actuallyInUse] = byte;
  program->lines[program->actuallyInUse] = line;
  ++program->actuallyInUse;
}

int addConst(Program *program, Value value)
{
  writeValueArray(&program->consts, value);
  return program->consts.actuallyInUse - 1;
}
