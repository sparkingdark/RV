#ifndef DEBUG_H
#define DEBUG_H

#include "program.h"

void disassembleProgram(Program *program, const char *name);
int disassembleInstruction(Program *program, int offset);

#endif
