#include <stdio.h>
#include "debug.h"
#include "value.h"

static int simpleInstruction(const char *name, int offset)
{
  printf("%s\n", name);
  return offset + 1;
}

static int constantInstruction(const char *name, Program *program, int offset)
{
  uint8_t constant = program->code[offset + 1];
  printf("%-16s %4d '", name, constant);
  printValue(program->consts.values[constant]);
  printf("'\n");
  return offset + 2;
}

void disassembleProgram(Program *program, const char *name)
{
  printf("== %s ==\n", name);

  for (int offset = 0; offset < program->actuallyInUse;)
  {
    offset = disassembleInstruction(program, offset);
  }
}

int disassembleInstruction(Program *program, int offset)
{
  printf("%04d ", offset);

  if (offset > 0 && program->lines[offset] == program->lines[offset - 1])
  {
    printf("   | ");
  }
  else
  {
    printf("%4d ", program->lines[offset]);
  }

  uint8_t instruction = program->code[offset];

  switch (instruction)
  {
  case OP_CONST:
    return constantInstruction("OP_CONST", program, offset);
  case OP_EQUAL:
    return simpleInstruction("OP_EQUAL", offset);
  case OP_GREATER:
    return simpleInstruction("OP_GREATER", offset);
  case OP_LESS:
    return simpleInstruction("OP_LESS", offset);
  case OP_ADD:
    return simpleInstruction("OP_ADD", offset);
  case OP_SUBTRACT:
    return simpleInstruction("OP_SUBTRACT", offset);
  case OP_MULTIPLY:
    return simpleInstruction("OP_MULTIPLY", offset);
  case OP_DIVIDE:
    return simpleInstruction("OP_DIVIDE", offset);
  case OP_NOT:
    return simpleInstruction("OP_NOT", offset);
  case OP_NEGATE:
    return simpleInstruction("OP_NEGATE", offset);
  case OP_RETURN:
    return simpleInstruction("OP_RETURN", offset);
  default:
    printf("Unknown opcode %d\n", instruction);
    return offset + 1;
  }
}
