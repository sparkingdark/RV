#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <string.h>
#include "common.h"
#include "program.h"
#include "debug.h"
#include "cvm.h"

static void repl()
{
  char line[1024];

  time_t t = time(NULL);
  struct tm tm = *localtime(&t);

  printf("RV (%d-%d-%d)\nType 'exit' or press CTRL + C to exit\n", tm.tm_year + 1900, tm.tm_mon + 1, tm.tm_mday);

  while (true)
  {
    printf(">>> ");

    if (!fgets(line, sizeof(line), stdin))
    {
      printf("\n");
      break;
    }

    interpret(line);
  }
}

static char *readFile(const char *path)
{
  FILE *file = fopen(path, "rb");

  if (file == NULL)
  {
    fprintf(stderr, "Cannot open file \"%s\".\n", path);
    exit(74);
  }

  fseek(file, 0L, SEEK_END);
  size_t fileSize = ftell(file); // Return the current position of STREAM.
  rewind(file);

  char *buffer = (char *)malloc(fileSize + 1);

  if (buffer == NULL)
  {
    fprintf(stderr, "Wow! There isn't enough memory to read \"%s\".\n", path);
    exit(74);
  }

  size_t bytesRead = fread(buffer, sizeof(char), fileSize, file);

  if (bytesRead < fileSize)
  {
    fprintf(stderr, "Couldn't read file \"%s\".\n", path);
    exit(74);
  }

  buffer[bytesRead] = '\0';

  fclose(file);
  return buffer;
}

static void runFile(const char *path)
{
  char *src = readFile(path);

  InterpretResult result = interpret(src);

  free(src);

  if (result == INTERPRET_COMPILE_ERROR)
    exit(65);
  if (result == INTERPRET_RUNTIME_ERROR)
    exit(70);
}

int main(int argc, const char *argv[])
{
  initCVM();

  Program program;

  initProgram(&program);

  if (argc == 1)
    repl();
  else if (argc == 2)
    runFile(argv[1]);
  else
  {
    fprintf(stderr, "Usage: rv <file>\n");
    exit(64);
  }

  freeProgram(&program);

  freeCVM();

  return 0;
}
